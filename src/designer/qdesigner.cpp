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

#include <qdesigner.h>
#include <qdesigner_components.h>
#include <qdesigner_actions.h>
#include <qdesigner_server.h>
#include <qdesigner_settings.h>
#include <qdesigner_workbench.h>
#include <mainwindow.h>

#include <qdesigner_propertysheet_p.h>

#include <QDebug>
#include <QCloseEvent>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QErrorMessage>
#include <QFile>
#include <QFileInfo>
#include <QFileOpenEvent>
#include <QFontDatabase>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QMainWindow>
#include <QMessageBox>
#include <QMetaObject>
#include <QPainter>
#include <QTextOption>
#include <QTimer>
#include <QTranslator>

struct Options;

static QDesigner::ParseArgumentsResult parseDesignerCommandLineArguments(
   QCommandLineParser &parser, Options *options, QString *errorMessage);

static void showHelp(QCommandLineParser &parser, const QString &errorMessage = QString());

struct Options {
   Options()
      : resourceDir(QLibraryInfo::location(QLibraryInfo::TranslationsPath)),
        server(false), enableInternalDynamicProperties(false), clientPort(0) {
   }

   QStringList files;
   QString resourceDir;
   bool server;
   bool enableInternalDynamicProperties;
   quint16 clientPort;
};

QDesigner::QDesigner(int &argc, char **argv)
   : QApplication(argc, argv), m_server(0), m_client(0), m_workbench(0), m_suppressNewFormShow(false)
{
   setOrganizationName("CopperSpice");
   setApplicationName("CS Designer");

#ifndef Q_OS_DARWIN
   setWindowIcon(QIcon(":/resources/designer.png"));
#endif
}

QDesigner::~QDesigner()
{
   if (m_workbench) {
      delete m_workbench;
   }

   if (m_server) {
      delete m_server;
   }

   if (m_client) {
      delete m_client;
   }
}

QDesigner::ParseArgumentsResult QDesigner::initSystem()
{
   QString errorMessage;
   Options options;

   QCommandLineParser parser;

   const ParseArgumentsResult result = parseDesignerCommandLineArguments(parser, &options, &errorMessage);

   if (result != ParseArgumentsSuccess) {
      showHelp(parser, errorMessage);
      return result;
   }

   // initialize sub components
   if (options.clientPort) {
      m_client = new QDesignerClient(options.clientPort, this);
   }

   if (options.server) {
      m_server = new QDesignerServer();

      printf("%d\n", m_server->serverPort());
      fflush(stdout);
   }

   if (options.enableInternalDynamicProperties) {
      QDesignerPropertySheet::setInternalDynamicPropertiesEnabled(true);
   }

   const QString localSysName = QLocale::system().name();
   QScopedPointer<QTranslator> designerTranslator(new QTranslator(this));

   if (designerTranslator->load("designer_" + localSysName, options.resourceDir)) {
      installTranslator(designerTranslator.take());
   }

   // ensure enum is registered
   QFontDatabase::staticMetaObject();
   QPalette::staticMetaObject();
   QPainter::staticMetaObject();
   QSizePolicy::staticMetaObject();
   QTabWidget::staticMetaObject();
   QTextOption::staticMetaObject();

   // work bench will set up the menu bar
   m_workbench = new QDesignerWorkbench();

   emit initialized();

   m_suppressNewFormShow = m_workbench->readInBackup();

   if (! options.files.empty()) {
      const QStringList::const_iterator cend = options.files.constEnd();

      for (QStringList::const_iterator it = options.files.constBegin(); it != cend; ++it) {
         // Ensure absolute paths for recent file list is unique
         QString fileName = *it;

         const QFileInfo fi(fileName);

         if (fi.exists() && fi.isRelative()) {
            fileName = fi.absoluteFilePath();
         }

         m_workbench->readInForm(fileName);
      }
   }

   if ( m_workbench->formWindowCount()) {
      m_suppressNewFormShow = true;
   }

   // Show error box if something went wrong
   if (m_initializationErrors.isEmpty()) {

      if (! m_suppressNewFormShow && QDesignerSettings(m_workbench->core()).showNewFormOnStartup()) {
         // will not show anything if suppressed
         QTimer::singleShot(100, this, &QDesigner::callCreateForm);
      }

   } else {
      showErrorMessageBox(m_initializationErrors);
      m_initializationErrors.clear();
   }

   return result;
}

void QDesigner::showErrorMessage(const QString &message)
{
   if (m_mainWindow) {
      showErrorMessageBox(message);

   } else {
      qWarning(csPrintable(message));

      m_initializationErrors += message + '\n';
   }
}

void QDesigner::showErrorMessageBox(const QString &msg)
{
   // manually suppress identical repeated messages
   if (m_errorMessageDialog && m_lastErrorMessage == msg) {
      return;
   }

   if (! m_errorMessageDialog) {
      m_lastErrorMessage.clear();
      m_errorMessageDialog = new QErrorMessage(m_mainWindow);

      m_errorMessageDialog->setWindowTitle(tr("CS Designer"));
      m_errorMessageDialog->setMinimumSize(QSize(600, 250));
      m_errorMessageDialog->setWindowFlags(m_errorMessageDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
   }

   m_errorMessageDialog->showMessage(msg);
   m_lastErrorMessage = msg;
}

QDesignerWorkbench *QDesigner::workbench() const
{
   return m_workbench;
}

QDesignerServer *QDesigner::server() const
{
   return m_server;
}

void showHelp(QCommandLineParser &parser, const QString &errorMessage)
{
   QString text;
   QTextStream str(&text);

   str << "<html><head/><body>";

   if (! errorMessage.isEmpty()) {
      str << "<p>" << errorMessage << "</p>";
   }

   str << "<pre>" << parser.helpText().toHtmlEscaped() << "</pre></body></html>";

   QMessageBox box(errorMessage.isEmpty() ? QMessageBox::Information : QMessageBox::Warning,
      QGuiApplication::applicationDisplayName(), text, QMessageBox::Ok);

   box.setTextInteractionFlags(Qt::TextBrowserInteraction);

   box.exec();
}

QDesigner::ParseArgumentsResult parseDesignerCommandLineArguments(
   QCommandLineParser &parser, Options *options, QString *errorMessage)
{
   parser.setApplicationDescription(QString("CS Designer ") + CS_VERSION_STR + "\n\nUI designer for CopperSpice applications.");

   const QCommandLineOption helpOption = parser.addHelpOption();

   parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
   const QCommandLineOption serverOption("server", "Server mode");
   parser.addOption(serverOption);

   const QCommandLineOption clientOption("client", "Client mode", "port");
   parser.addOption(clientOption);

   const QCommandLineOption resourceDirOption("resourcedir", "Resource directory", "directory");
   parser.addOption(resourceDirOption);

   const QCommandLineOption internalDynamicPropertyOption("enableinternaldynamicproperties",
      "Enable internal dynamic properties");

   parser.addOption(internalDynamicPropertyOption);

   parser.addPositionalArgument(QString("files"), QString("The UI files to open."));

   if (!parser.parse(QCoreApplication::arguments())) {
      *errorMessage = parser.errorText();
      return QDesigner::ParseArgumentsError;
   }

   if (parser.isSet(helpOption)) {
      return QDesigner::ParseArgumentsHelpRequested;
   }

   options->server = parser.isSet(serverOption);

   if (parser.isSet(clientOption)) {
      bool ok;
      options->clientPort = parser.value(clientOption).toInteger<quint16>(&ok);

      if (!ok) {
         *errorMessage = QString("Non-numeric argument specified for -client");
         return QDesigner::ParseArgumentsError;
      }
   }

   if (parser.isSet(resourceDirOption)) {
      options->resourceDir = parser.value(resourceDirOption);
   }

   options->enableInternalDynamicProperties = parser.isSet(internalDynamicPropertyOption);
   options->files = parser.positionalArguments();

   return QDesigner::ParseArgumentsSuccess;
}

bool QDesigner::event(QEvent *ev)
{
   bool eaten;

   switch (ev->type()) {

      case QEvent::FileOpen:
         m_suppressNewFormShow = true;

         if (!m_workbench->readInForm(static_cast<QFileOpenEvent *>(ev)->file())) {
            m_suppressNewFormShow = false;
         }

         eaten = true;
         break;

      case QEvent::Close: {
         QCloseEvent *closeEvent = static_cast<QCloseEvent *>(ev);
         closeEvent->setAccepted(m_workbench->handleClose());

         if (closeEvent->isAccepted()) {
            // shutting down, make sure that we do not get our settings saved twice
            if (m_mainWindow) {
               m_mainWindow->setCloseEventPolicy(MainWindowBase::AcceptCloseEvents);
            }

            eaten = QApplication::event(ev);
         }
         eaten = true;
         break;
      }

      default:
         eaten = QApplication::event(ev);
         break;
   }

   return eaten;
}

void QDesigner::setMainWindow(MainWindowBase *obj)
{
   m_mainWindow = obj;
}

MainWindowBase *QDesigner::mainWindow() const
{
   return m_mainWindow;
}

void QDesigner::callCreateForm()
{
   if (! m_suppressNewFormShow) {
      m_workbench->actionManager()->createForm();
   }
}
