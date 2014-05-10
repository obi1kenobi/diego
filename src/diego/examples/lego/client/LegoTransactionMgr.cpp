#include "LegoTransactionMgr.h"

#include "Debug.h"
#include "LegoOps.h"
#include "LegoTransaction.h"
#include "LegoUniverse.h"
#include "LegoNotices.h"

#include <QtCore/QEventLoop>
#include <QtCore/QObject>
#include <QtCore/QTextCodec>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <cassert>
#include <iostream>
#include <sstream>

LegoTransactionMgr::LegoTransactionMgr(LegoUniverse *universe) : 
    _universe(universe),
    _xaIds(0)
{
}

bool
LegoTransactionMgr::Execute(const LegoTransaction &xa)
{
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
    _Execute(serverLog);

    return success;
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

    os << "Submit\n";
    os << _universe->GetID() << " " << xaID << "\n";
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
    while (is.peek() != '#' && is.good()) {
        // Skip white space or/and newline
        _SkipWhiteSpace(is);

        // Parse namespace id
        int recNamespace;
        is >> recNamespace;
        SfDPrintf(1, "Namespace: %d\n", recNamespace);

        // Parse transaction id
        int recXaID;
        is >> recXaID;
        SfDPrintf(1, "XaID: %d\n", recXaID);

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
        assert(!recXa.GetOps().empty());
        serverLog->push_back(recXa);
        ++numXas;

        _SkipWhiteSpace(is);
    }
    SfDPrintf(1, "Parsed %d\n", numXas);
}

std::string
LegoTransactionMgr::_SendMessage(const std::string &message)
{
    SfDPrintf(1, "Sending message to server:\n%s", message.c_str());

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

    SfDPrintf(1, "Got response:\n%s", response.c_str());

    return response;
}

void
LegoTransactionMgr::_Execute(const std::vector<LegoTransaction> &xas)
{
    if (xas.empty()) {
        return;
    }

    int numXas = 0;
    for (const auto &xa : xas) {
        ++numXas;
        const auto &ops = xa.GetOps();
        for (const auto &op : ops) {
            switch (op.GetType()) {
            case LegoOp::CREATE_BRICK: {
                _universe->_CreateBrick(op.GetPosition(),
                                        op.GetSize(),
                                        op.GetOrientation(),
                                        op.GetColor());
            } break;
            case LegoOp::MODIFY_BRICK_POSITION: {
                LegoBrick *brick = _universe->GetBrick(op.GetBrickID());
                brick->_SetPosition(op.GetPosition());
            } break;
            case LegoOp::MODIFY_BRICK_SIZE: {
                LegoBrick *brick = _universe->GetBrick(op.GetBrickID());
                brick->_SetSize(op.GetSize());
            } break;
            case LegoOp::MODIFY_BRICK_ORIENTATION: {
                LegoBrick *brick = _universe->GetBrick(op.GetBrickID());
                brick->_SetOrientation(op.GetOrientation());
            } break;
            case LegoOp::MODIFY_BRICK_COLOR: {
                LegoBrick *brick = _universe->GetBrick(op.GetBrickID());
                brick->_SetColor(op.GetColor());
            } break;
            case LegoOp::DELETE_BRICK: {
                LegoBrick *brick = _universe->GetBrick(op.GetBrickID());
                brick->_Destroy();
            } break;
            }
        }
        _xas.push_back(xa);
        ++_xaIds;
    }
    SfDPrintf(1, "Executed %d transactions\n", numXas);
    LegoBricksChangedNotice().Send();
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
    _Execute(serverLog);
}
