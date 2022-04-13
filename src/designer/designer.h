/***********************************************************************
*
* Copyright (c) 2021-2022 Barbara Geller
* Copyright (c) 2021-2022 Ansel Sermersheim
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

#ifndef QDESIGNER_H
#define QDESIGNER_H

class QDesignerWorkbench;
class QDesignerToolWindow;
class MainWindowBase;
class QDesignerServer;
class QDesignerClient;

#include <QApplication>
#include <QPointer>

class QCommandLineParser;
class QErrorMessage;

#define qDesignerApp  (static_cast<QDesigner*>(QCoreApplication::instance()))

class QDesigner: public QApplication
{
   CS_OBJECT(QDesigner)

 public:
   enum ParseArgumentsResult {
      ParseArgumentsSuccess,
      ParseArgumentsError,
      ParseArgumentsHelpRequested
   };

   QDesigner(int &argc, char **argv);
   virtual ~QDesigner();

   ParseArgumentsResult initSystem();

   QDesignerWorkbench *workbench() const;
   QDesignerServer *server() const;
   MainWindowBase *mainWindow() const;
   void setMainWindow(MainWindowBase *tw);

   CS_SIGNAL_1(Public, void initialized())
   CS_SIGNAL_2(initialized)

   CS_SLOT_1(Public, void showErrorMessage(const QString &message))
   CS_SLOT_2(showErrorMessage)

 protected:
   bool event(QEvent *ev) override;

 private:
   CS_SLOT_1(Private, void callCreateForm())
   CS_SLOT_2(callCreateForm)

   void showErrorMessageBox(const QString &);

   QDesignerServer *m_server;
   QDesignerClient *m_client;
   QDesignerWorkbench *m_workbench;

   QPointer<MainWindowBase> m_mainWindow;
   QPointer<QErrorMessage> m_errorMessageDialog;

   QString m_initializationErrors;
   QString m_lastErrorMessage;

   bool m_suppressNewFormShow;
};

#endif
