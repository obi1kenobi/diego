#include "LegoTransactionMgr.h"

#include "Debug.h"
#include "LegoOps.h"
#include "LegoTransaction.h"
#include "LegoUniverse.h"
#include "LegoNotices.h"

#include <QtCore/QEventLoop>
#include <QtCore/QMutexLocker>
#include <QtCore/QObject>
#include <QtCore/QTextCodec>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <cassert>
#include <iostream>
#include <sstream>

#include <time.h>

LegoTransactionMgr::LegoTransactionMgr(LegoUniverse *universe) : 
    _network(true),
    _universe(universe),
    _xaIds(0),
    _xa(NULL),
    _reqID(0),
    _executing(false)
{
    srand48(time(NULL));
    _clientID = lrand48();
}

void
LegoTransactionMgr::SetNetworkEnabled(bool enabled)
{
    _network = enabled;
}

void
LegoTransactionMgr::OpenTransaction()
{
    assert(_xa == NULL);
    _xa = new LegoTransaction();
}

void
LegoTransactionMgr::CloseTransaction()
{
    assert(_xa != NULL);
    _ExecuteXa(*_xa);
    delete _xa;
    _xa = NULL;
}

bool
LegoTransactionMgr::ExecuteOp(const LegoOp &op)
{
    if (!_universe->IsValid(op)) {
        return false;
    }

    if (_executing) {
        return false;
    }

    QMutexLocker autoLocker(&_lock);
    _executing = true;

    bool success = false;
    if (!_network) {
        // Execute locally and keep track of ops we need to send to server
        _offlineXa.AddOp(op);
        bool doNotify = true;
        _ExecuteOp(op, doNotify);
        success = true;
    } else if (_xa) {
        // Accumulating ops into a transaction for lazy execution
        _xa->AddOp(op);
        success = true;
    } else if (_network) {
        // Network is up and active; send transaction as usual
        LegoTransaction xa;
        xa.AddOp(op);
        success = _ExecuteXa(xa);
    }

    _executing = false;
    return success;
}

bool
LegoTransactionMgr::_ExecuteXa(const LegoTransaction &xa)
{
    if (xa.GetOps().empty()) {
        return false;
    }

    // Send transaction to server
    std::string response = _SendToServer(xa);

    if (response.empty()) {
        return false;
    }

    // Was our transaction successful?
    std::istringstream is(response);
    bool success = false;
    is >> success;

    // Parse response (server log) and execute transactions from it
    std::vector<LegoTransaction> serverLog;
    _ParseResponse(is, &serverLog);
    _ExecuteXas(serverLog);

    LegoBricksChangedNotice().Send();
    if (!success) {
        LegoConflictNotice().Send();
    }

    return success;
}

void
LegoTransactionMgr::Sync()
{
    _ExecuteXa(_offlineXa);
    _offlineXa.Clear();
}

std::string
LegoTransactionMgr::_SendToServer(const LegoTransaction &xa)
{
    // Serialize transaction
    std::ostringstream os;
    _EmitXaPrologue(os);
    xa.Serialize(os);
    _EmitXaEpilogue(os);

    // Send over wire
    std::string msg = os.str();
    SfDPrintf(1, "Sending transaction:\n%s", msg.c_str());
    std::string response = _SendMessage(msg);
    SfDPrintf(1, "Got response:\n%s", response.c_str());

    return response;
}


void
LegoTransactionMgr::_EmitXaPrologue(std::ostream &os)
{
    // Assign transaction id
    uint64_t xaID = _xaIds;

    int64_t clientID = _clientID;
    int64_t reqID = _reqID++;

    os << "Submit\n";
    os << _universe->GetID() << " " << xaID 
       << " " << clientID << " " << reqID << "\n";
}

void
LegoTransactionMgr::_EmitXaEpilogue(std::ostream &os)
{
    os << "*\n";
}

void
LegoTransactionMgr::_ParseResponse(std::istream &is,
                                   std::vector<LegoTransaction> *serverLog)
{
    // Parse transaction log
    //
    // Format of response is
    //
    // <namespace_id> <xa_id>
    // op
    // op
    // ...
    // *
    // #
    //
    // '*' is the sentinel that represents end of transaction
    // '#' is the sentinel that represents end of response
    SfDPrintf(1, "Parsing response:\n");
    int numXas = 0;
    while (is.good()) {
        // Skip white space or/and newline
        _SkipWhiteSpace(is);

        if (!is.good() || is.peek() == '#') {
            break;
        }

        // Parse namespace id
        int recNamespace;
        is >> recNamespace;
        SfDPrintf(1, "Namespace: %d\n", recNamespace);

        // Parse transaction id
        int64_t recXaID;
        is >> recXaID;
        SfDPrintf(1, "XaID: %d\n", recXaID);
        if (recXaID >= _xaIds) {
            _xaIds = recXaID + 1;
        }

        // Parse at-most-once tokens
        int64_t clientID, reqID;
        is >> clientID;
        is >> reqID;
        SfDPrintf(1, "Client ID %ld, Req ID %ld\n", clientID, reqID);

        // Skip white space or/and newline
        _SkipWhiteSpace(is);

        LegoTransaction recXa;
        while (is.good()) {
            // Check for "end of ops" sentinel
            if (is.peek() == '*') {
                is.get();
                break;
            }

            // Read op line
            char buffer[4096];
            is.getline(buffer, sizeof(buffer));
            SfDPrintf(1, "Read line: %s\n", buffer);

            // Deserialize op
            std::istringstream ops(buffer);
            LegoOp op(ops);
            assert(op.IsValid());

            // Add to transaction's op list
            recXa.AddOp(op);
        }

        // Add transaction to log
        if (recXa.GetOps().empty()) {
            continue;
        }
        serverLog->push_back(recXa);
        ++numXas;
    }
    SfDPrintf(1, "Parsed %d\n", numXas);
}

std::string
LegoTransactionMgr::_SendMessage(const std::string &message)
{
    SfDPrintf(0, "Sending message to server:\n%s", message.c_str());

    QByteArray postData;
    postData.append(message.c_str());

    QNetworkRequest request(QUrl("http://localhost:8080/lego"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QNetworkAccessManager nwam;
    QNetworkReply *reply = nwam.post(request, postData);

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QByteArray rawData;
    if (reply->error() == QNetworkReply::NoError) {
        rawData = reply->readAll();
    } else {
        std::cerr << "Network request failed: " << reply->errorString().toStdString();
    }

    std::string response =
        QTextCodec::codecForHtml(rawData)->toUnicode(rawData).toStdString();

    SfDPrintf(0, "Got response:\n%s", response.c_str());

    return response;
}

int
LegoTransactionMgr::_ExecuteXas(const std::vector<LegoTransaction> &xas)
{
    if (xas.empty()) {
        return 0;
    }

    int numXas = 0;
    for (const auto &xa : xas) {
        _ExecuteXaOps(xa);
        ++numXas;
        _xas.push_back(xa);
        std::ostringstream os;
        xa.Serialize(os);
        LegoTransactionProcessed(os.str()).Send();
    }
    SfDPrintf(1, "Executed %d transactions\n", numXas);

    return numXas;
}

void
LegoTransactionMgr::_ExecuteXaOps(const LegoTransaction &xa)
{
    const auto &ops = xa.GetOps();
    for (const auto &op : ops) {
        _ExecuteOp(op);
    }
}

void
LegoTransactionMgr::_ExecuteOp(const LegoOp &op, bool doNotify)
{
    if (op.GetType() == LegoOp::CREATE_BRICK) {
        _universe->_CreateBrick(op.GetPosition(),
                                op.GetSize(),
                                op.GetOrientation(),
                                op.GetColor());
    } else {
        LegoBrick *brick = _universe->GetBrick(op.GetBrickID());
        if (!brick) {
            std::cerr << "ERROR: Unknown brick id " << op.GetBrickID() << "\n";
            return;
        }
        switch (op.GetType()) {
        case LegoOp::MODIFY_BRICK_POSITION: {
            brick->_SetPosition(op.GetPosition());
        } break;
        case LegoOp::MODIFY_BRICK_SIZE: {
            brick->_SetSize(op.GetSize());
        } break;
        case LegoOp::MODIFY_BRICK_ORIENTATION: {
            brick->_SetOrientation(op.GetOrientation());
        } break;
        case LegoOp::MODIFY_BRICK_COLOR: {
            brick->_SetColor(op.GetColor());
        } break;
        case LegoOp::DELETE_BRICK: {
            _universe->_DestroyBrick(brick);
        } break;
        default: 
            std::cerr << "FATAL: Unknown op\n";
            throw std::exception();
        }
    }

    if (doNotify) {
        LegoBricksChangedNotice().Send();
    }
}

void
LegoTransactionMgr::_SkipWhiteSpace(std::istream &input)
{
    while (input.good() && std::isspace(input.peek())) {
        input.get();
    }
}

void
LegoTransactionMgr::CatchupWithServer()
{
    if (_executing) {
        return;
    }

    if (!_network) {
        return;
    }

    QMutexLocker autoLocker(&_lock);
    _executing = true;

    // Request all transactions since the last one we've executed.
    //
    // Format of request is:
    //
    // TransactionsSince
    // <namespace_id> <starting_xa_id>
    //
    // The transactions sent are all transacitons with given id or higher.
    std::ostringstream os;
    os << "TransactionsSince\n";
    os << _universe->GetID() << " " << _xaIds << "\n";

    // Send message to server 
    std::string response = _SendMessage(os.str());

    // Parse response and execute received transactions if any
    std::istringstream is(response);
    std::vector<LegoTransaction> serverLog;
    _ParseResponse(is, &serverLog);
    SfDPrintf(1, "Received %d transactions from server\n", serverLog.size());
    int numXas = _ExecuteXas(serverLog);
    if (numXas != 0) {
        LegoBricksChangedNotice().Send();
    }

    _executing = false;
}
