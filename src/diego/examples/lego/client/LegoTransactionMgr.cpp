#include "LegoTransactionMgr.h"
#include "LegoOps.h"
#include "LegoTransaction.h"
#include "LegoUniverse.h"

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
    std::vector<LegoTransaction> serverLog;
    bool success = _SendToServer(xa, &serverLog);
    _Execute(serverLog);
    return success;
}

bool
LegoTransactionMgr::_SendToServer(const LegoTransaction &xa,
                                  std::vector<LegoTransaction> *serverLog)
{
    // Assign transaction id
    uint64_t xaID = _xaIds;

    // Serialize xa
    std::ostringstream os;
    os << _universe->GetID() << " " << xaID << "\n";
    xa.Serialize(os);
    os << "*\n";

    // XXX: send over wire
    std::string msg = os.str();
    std::cout << "Sending transaction:\n" << os.str() << std::endl;
    std::string response = _SendText(msg);
    std::cout << "Got response:\n" << response << std::endl;

    // XXX: receive response
    // Assume for now we got back the same transaction
    serverLog->push_back(xa);

    return true;
}

std::string
LegoTransactionMgr::_SendText(const std::string &text)
{
#if 0
    QByteArray data;
    QUrl params;

    params.addQueryItem("userid","user");
    params.addQueryItem("apiKey","key");
    data.append(params.toString());
    data.remove(0,1);
#else
    QByteArray postData;
    postData.append(text.c_str());
#endif

    QNetworkRequest request(QUrl("http://localhost:8080/lego"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QNetworkAccessManager nwam;
    std::cerr << "Launch post request: " << text << std::endl;
    QNetworkReply *reply = nwam.post(request, postData);

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QByteArray rawData;
    if (reply->error() == QNetworkReply::NoError) {
        rawData = reply->readAll();
    } else {
        qDebug() << reply->errorString();
    }

    std::string response =
        QTextCodec::codecForHtml(rawData)->toUnicode(rawData).toStdString();
    qDebug() << response.c_str();

    return response;
#if 0
    std::cerr << "Done.\n";
    std::cerr << "Waiting for a reply...\n";
    while (!reply->isFinished()) {}
    std::cerr << "Done.\n";
    char *responseData = reply->readAll().data();
    std::cerr << "Got response:\n";
    std::cerr << responseData << std::endl;
    return std::string(responseData);
#endif
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
}
