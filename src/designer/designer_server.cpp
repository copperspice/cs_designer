/***********************************************************************
*
* Copyright (c) 2021-2024 Barbara Geller
* Copyright (c) 2021-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
*
* Designer is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* Designer is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <designer.h>
#include <designer_server.h>

#include <QFileInfo>
#include <QHostAddress>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>

#include <qevent.h>

QDesignerServer::QDesignerServer(QObject *parent)
   : QObject(parent)
{
   m_socket = nullptr;
   m_server = new QTcpServer(this);
   m_server->listen(QHostAddress::LocalHost, 0);

   if (m_server->isListening()) {
      connect(m_server, &QTcpServer::newConnection,
         this, &QDesignerServer::handleNewConnection);
   }
}

QDesignerServer::~QDesignerServer()
{
}

quint16 QDesignerServer::serverPort() const
{
   return m_server ? m_server->serverPort() : 0;
}

void QDesignerServer::sendOpenRequest(int port, const QStringList &files)
{
   QTcpSocket *sSocket = new QTcpSocket();
   sSocket->connectToHost(QHostAddress::LocalHost, port);

   if (sSocket->waitForConnected(3000)) {
      for (const QString &file : files) {
         QFileInfo fi(file);
         sSocket->write(fi.absoluteFilePath().toUtf8() + '\n');
      }
      sSocket->waitForBytesWritten(3000);
      sSocket->close();
   }
   delete sSocket;
}

void QDesignerServer::readFromClient()
{
   while (m_socket->canReadLine()) {
      QString file = QString::fromUtf8(m_socket->readLine());

      if (! file.isEmpty()) {
         file.remove('\n');
         file.remove('\r');
         qDesignerApp->postEvent(qDesignerApp, new QFileOpenEvent(file));
      }
   }
}

void QDesignerServer::socketClosed()
{
   m_socket = nullptr;
}

void QDesignerServer::handleNewConnection()
{
   // no need for more than one connection
   if (m_socket == nullptr) {
      m_socket = m_server->nextPendingConnection();

      connect(m_socket, &QTcpSocket::readyRead,    this, &QDesignerServer::readFromClient);
      connect(m_socket, &QTcpSocket::disconnected, this, &QDesignerServer::socketClosed);
   }
}

QDesignerClient::QDesignerClient(quint16 port, QObject *parent)
   : QObject(parent)
{
   m_socket = new QTcpSocket(this);
   m_socket->connectToHost(QHostAddress::LocalHost, port);

   connect(m_socket, &QTcpSocket::readyRead, this, &QDesignerClient::readFromSocket);
}

QDesignerClient::~QDesignerClient()
{
   m_socket->close();
   m_socket->flush();
}

void QDesignerClient::readFromSocket()
{
   while (m_socket->canReadLine()) {
      QString file = QString::fromUtf8(m_socket->readLine());

      if (! file.isEmpty()) {
         file.remove('\n');
         file.remove('\r');

         if (QFile::exists(file)) {
            qDesignerApp->postEvent(qDesignerApp, new QFileOpenEvent(file));
         }
      }
   }
}

