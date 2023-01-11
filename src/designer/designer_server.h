/***********************************************************************
*
* Copyright (c) 2021-2023 Barbara Geller
* Copyright (c) 2021-2023 Ansel Sermersheim
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

#ifndef QDESIGNER_SERVER_H
#define QDESIGNER_SERVER_H

#include <QObject>

class QTcpServer;
class QTcpSocket;

class QDesignerServer: public QObject
{
   CS_OBJECT(QDesignerServer)
 public:
   explicit QDesignerServer(QObject *parent = nullptr);
   virtual ~QDesignerServer();

   quint16 serverPort() const;

   static void sendOpenRequest(int port, const QStringList &files);

 private:
   CS_SLOT_1(Private, void handleNewConnection())
   CS_SLOT_2(handleNewConnection)
   CS_SLOT_1(Private, void readFromClient())
   CS_SLOT_2(readFromClient)
   CS_SLOT_1(Private, void socketClosed())
   CS_SLOT_2(socketClosed)

 private:
   QTcpServer *m_server;
   QTcpSocket *m_socket;
};

class QDesignerClient: public QObject
{
   CS_OBJECT(QDesignerClient)
 public:
   explicit QDesignerClient(quint16 port, QObject *parent = nullptr);
   virtual ~QDesignerClient();

 private:
   CS_SLOT_1(Private, void readFromSocket())
   CS_SLOT_2(readFromSocket)

 private:
   QTcpSocket *m_socket;
};

#endif
