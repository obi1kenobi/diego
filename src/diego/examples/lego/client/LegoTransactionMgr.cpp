#include "LegoTransactionMgr.h"
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
    CatchupWithServer();
}

bool
LegoTransactionMgr::Execute(const LegoTransaction &xa)
{
    std::string response = _SendToServer(xa);
    std::vector<LegoTransaction> serverLog;
    bool success = _ParseResponse(response, &serverLog);
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
    std::cout << "Sending transaction:\n" << os.str() << std::endl;
    std::string response = _SendText(msg);
    std::cout << "Got response:\n" << response << std::endl;

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

bool
LegoTransactionMgr::_ParseResponse(const std::string &response,
                                   std::vector<LegoTransaction> *serverLog)
{
    // Parse response
    std::istringstream is(response);

    // Was our transaction successful?
    bool success;
    is >> success;

    // Parse transaction log
    while (is.good()) {
        uint64_t receivedNamespace;
        is >> receivedNamespace;

        uint64_t receivedXaID;
        is >> receivedXaID;

        LegoTransaction receivedXa;
        while (is.peek() != '*' && is.good()) {
            _SkipWhiteSpace(is);
            char buffer[4096];
            is.getline(buffer, sizeof(buffer));
            std::cerr << "Read line: " << buffer << std::endl;
            std::istringstream ops(buffer);
            LegoOp op(ops);
            if (!op.IsValid()) {
                std::cerr << "ERROR: Could not parse op: " << buffer << std::endl;
                return false;
            }
            receivedXa.AddOp(op);
        }
        if (!receivedXa.GetOps().empty()) {
            serverLog->push_back(receivedXa);
        }

        _SkipWhiteSpace(is);
    }

    return success;
}

std::string
LegoTransactionMgr::_SendText(const std::string &text)
{
    QByteArray postData;
    postData.append(text.c_str());

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

    return response;
}

void
LegoTransactionMgr::_Execute(const std::vector<LegoTransaction> &xas)
{
    for (const auto &xa : xas) {
        const auto &ops = xa.GetOps();
        for (const auto &op : ops) {
            switch (op.GetType()) {
            case LegoOp::CREATE_BRICK: {
                _universe->_CreateBrick(op.GetPosition(),
                                        op.GetSize(),
                                        op.GetOrientation(),
                                        op.GetColor());
            } break;
            case LegoOp::MODIFY_POSITION: break; {
                LegoBrick *brick = _universe->GetBrick(op.GetBrickID());
                brick->_SetPosition(op.GetPosition());
            } break;
            case LegoOp::MODIFY_SIZE: break;
            case LegoOp::MODIFY_ORIENTATION: break;
            case LegoOp::MODIFY_COLOR: break;
            case LegoOp::DELETE_BRICK: break;
            }
        }
        ++_xaIds;
    }
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
    std::ostringstream os;
    os << "TransactionsSince\n";
    os << _xaIds << "\n";

    std::string response = _SendText(os.str());
    std::vector<LegoTransaction> serverLog;
    bool success = _ParseResponse(response, &serverLog);
    _Execute(serverLog);
}
