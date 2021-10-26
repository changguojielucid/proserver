
/*
*             dP                   oo       dP          oo
*             88                            88
*    .d8888b. 88 dP    dP .d8888b. dP .d888b88 dP   .dP dP dP   .dP .d8888b.
*    88ooood8 88 88    88 88'  `"" 88 88'  `88 88   d8' 88 88   d8' 88'  `88
*    88.  ... 88 88.  .88 88.  ... 88 88.  .88 88 .88'  88 88 .88'  88.  .88
*    `88888P' dP `88888P' `88888P' dP `88888P8 8888P'   dP 8888P'   `88888P'
*    oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
*

*
*    Created:
*
*    Modified:
*
*
*
*
*
*                              Copyright (c) Elucid Bioimaging
*/


#include "EVFirst.h"
#include "EVWebSocketServer.h"
#include "QtWebSockets/QWebSocketServer"
#include "QtWebSockets/QWebSocket"
#include <QMetaEnum>
#include <QtCore/QFile>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include "EVWebSocketClient.h"
#include "EVMacros.h"
#include "EVWorkClient.h"
#include "EVWorkClientManager.h"
#include "EVProServer.h"

EVProServer::EVProServer(quint16 port, bool debug, QObject *parent) : EVWebSocketServer(parent)
{
  m_port = port;
  m_bDebug = debug;

  m_pWorkClientManager = new EVWorkClientManager();
}

EVProServer::~EVProServer()
{
  SAFE_REMOVAL(m_pWorkClientManager)

  qDeleteAll(m_webSocketClients.begin(), m_webSocketClients.end());
  m_webSocketClients.clear();
}



int EVProServer::GetPortNumber()
{
  return m_port;
}

QString EVProServer::GetCertificateFullPath()
{
  return QStringLiteral("D:/ev/proserver/localhost.cert");
}

QString EVProServer::GetKeyFileFullPath()
{
  return QStringLiteral("D:/ev/proserver/localhost.key");
}

EVWorkClient* EVProServer::FindClientForSocket(QWebSocket* pSocket)
{
  if (!pSocket)
    return nullptr;

  for (int i = 0; i < m_webSocketClients.size(); ++i)
  {
    if (m_webSocketClients.at(i)->m_pWebSocket == pSocket)
      return m_webSocketClients.at(i);
  }

  return nullptr;
}

void EVProServer::SocketDisconnected()
{
  QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
  ebAssert(pClient);

  EVWorkClient *wsClient = EVProServer::FindClientForSocket(pClient);

  if (wsClient)
  {
    m_webSocketClients.removeAll(wsClient);
    wsClient->deleteLater();
  }
  qDebug() << (QString::number(m_webSocketClients.length()) + QString(" clients connected"));
}


void EVProServer::OnNewConnection()
{
  QWebSocketServer* pWebSocketServer = GetWebSocketServer();
  ebAssert(pWebSocketServer);

  QWebSocket *pSocket = pWebSocketServer->nextPendingConnection();
  ebAssert(pSocket);

  qDebug() << (QString("WebSocket Client connected:") + pSocket->peerName() + " " + pSocket->origin());

  connect(pSocket, &QWebSocket::textMessageReceived, this, &EVProServer::ProcessTextMessage);
  connect(pSocket, &QWebSocket::binaryMessageReceived, this, &EVProServer::ProcessBinaryMessage);
  connect(pSocket, &QWebSocket::disconnected, this, &EVProServer::SocketDisconnected);

  EVWorkClientManager* pWorkClientManager = GetWorkClientManager();
  ebAssert(pWorkClientManager);

  EVWorkClient *wsClient = pWorkClientManager->CreateWorkClient();
  ebAssert(wsClient);

 
  wsClient->m_pWebSocket = pSocket;


  m_webSocketClients << wsClient;

  qDebug() << (QString::number(m_webSocketClients.length()) + QString(" clients connected"));

}

void EVProServer::ProcessTextMessage(const QString& message)
{
  QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

  EVWorkClient *wsClient = FindClientForSocket(pClient);
  ebAssert(wsClient);

  if (wsClient)
    wsClient->Receive(message);
}

void EVProServer::ProcessBinaryMessage(const QByteArray& message)
{
  QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

  EVWorkClient *wsClient = FindClientForSocket(pClient);
  ebAssert(wsClient);

  if (wsClient)
    wsClient->Receive(message);
}
