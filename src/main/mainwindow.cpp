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

#include <abstract_formwindow.h>
#include <mainwindow.h>
#include <designer.h>
#include <designer_actions.h>
#include <designer_workbench.h>
#include <designer_formwindow.h>
#include <designer_toolwindow.h>
#include <designer_settings.h>
#include <toolbar_dialog.h>

#include <QAction>
#include <QCloseEvent>
#include <QToolBar>
#include <QMdiSubWindow>
#include <QStatusBar>
#include <QMenu>
#include <QLayout>
#include <QDockWidget>
#include <QUrl>
#include <QDebug>
#include <QMimeData>

#include <algorithm>

static const QString uriListMimeFormatC = "text/uri-list";

typedef QList<QAction *> ActionList;

static void addActionsToToolBar(const ActionList &actions, QToolBar *t)
{
   for (auto item : actions) {
      if (item->property(QDesignerActions::defaultToolbarPropertyName).toBool()) {
         t->addAction(item);
      }
   }
}

static QToolBar *createToolBar(const QString &title, const QString &objectName, const ActionList &actions)
{
   QToolBar *rc =  new QToolBar;
   rc->setObjectName(objectName);
   rc->setWindowTitle(title);
   addActionsToToolBar(actions, rc);

   return rc;
}

MainWindowBase::MainWindowBase(QWidget *parent, Qt::WindowFlags flags)
   : QMainWindow(parent, flags), m_policy(AcceptCloseEvents)
{

#ifndef Q_OS_DARWIN
   setWindowIcon(qDesignerApp->windowIcon());
#endif
}

void MainWindowBase::closeEvent(QCloseEvent *e)
{
   switch (m_policy) {
      case AcceptCloseEvents:
         QMainWindow::closeEvent(e);
         break;

      case EmitCloseEventSignal:
         emit closeEventReceived(e);
         break;
   }
}

QList<QToolBar *>  MainWindowBase::createToolBars(const QDesignerActions *actions, bool singleToolBar)
{
   // whenever you want to add a new tool bar here, you also have to update the default
   // action groups added to the toolbar manager in the mainwindow constructor

   QList<QToolBar *> rc;

   if (singleToolBar) {
      // Not currently used (main tool bar)
      QToolBar *main = createToolBar(tr("Main"), QString("mainToolBar"), actions->fileActions()->actions());
      addActionsToToolBar(actions->editActions()->actions(), main);
      addActionsToToolBar(actions->toolActions()->actions(), main);
      addActionsToToolBar(actions->formActions()->actions(), main);
      rc.push_back(main);

   } else {
      rc.push_back(createToolBar(tr("File"),  QString("fileToolBar"),  actions->fileActions()->actions()));
      rc.push_back(createToolBar(tr("Edit"),  QString("editToolBar"),  actions->editActions()->actions()));
      rc.push_back(createToolBar(tr("Tools"), QString("toolsToolBar"), actions->toolActions()->actions()));
      rc.push_back(createToolBar(tr("Form"),  QString("formToolBar"),  actions->formActions()->actions()));
   }

   return rc;
}

QString MainWindowBase::mainWindowTitle()
{
   return tr("CS Designer");
}

int MainWindowBase::settingsVersion()
{
   const int version = CS_VERSION;
   return (version & 0x00FF00) >> 8;
}

DockedMdiArea::DockedMdiArea(const QString &extension, QWidget *parent)
   : QMdiArea(parent), m_extension(extension)
{
   setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
   setLineWidth(1);
   setAcceptDrops(true);
   setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
   setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

QStringList DockedMdiArea::uiFiles(const QMimeData *d) const
{
   // extract dropped UI files from Mime data
   QStringList retval;

   if (! d->hasFormat(uriListMimeFormatC)) {
      return retval;
   }

   const QList<QUrl> urls = d->urls();
   if (urls.empty()) {
      return retval;
   }

   for (auto item : urls) {
      const QString fileName = item.toLocalFile();

      if (! fileName.isEmpty() && fileName.endsWith(m_extension)) {
         retval.push_back(fileName);
      }
   }

   return retval;
}

bool DockedMdiArea::event(QEvent *event)
{
   // Listen for desktop file manager drop and emit a signal once a file is dropped

   switch (event->type()) {
      case QEvent::DragEnter: {
         QDragEnterEvent *e = static_cast<QDragEnterEvent *>(event);

         if (! uiFiles(e->mimeData()).empty()) {
            e->acceptProposedAction();
            return true;
         }
      }
      break;

      case QEvent::Drop: {
         QDropEvent *e = static_cast<QDropEvent *>(event);

         const QStringList files = uiFiles(e->mimeData());

         for (auto item : files) {
            emit fileDropped(item);
         }

         e->acceptProposedAction();
         return true;
      }
      break;

      default:
         break;
   }

   return QMdiArea::event(event);
}

static void addActionsToToolBarManager(const ActionList &al, const QString &title, QtToolBarManager *tbm)
{
   for (auto item : al) {
      tbm->addAction(item, title);
   }
}

ToolBarManager::ToolBarManager(QMainWindow *configureableMainWindow, QWidget *parent,
         QMenu *toolBarMenu, const QDesignerActions *actions, const QList<QToolBar *> &toolbars,
         const QList<QDesignerToolWindow *> &toolWindows)
   : QObject(parent), m_configureableMainWindow(configureableMainWindow),
     m_parent(parent), m_toolBarMenu(toolBarMenu), m_manager(new QtToolBarManager(this)),
     m_configureAction(new QAction(tr("Configure Toolbars..."), this)), m_toolbars(toolbars)
{
   m_configureAction->setMenuRole(QAction::NoRole);
   m_configureAction->setObjectName(QString("__qt_configure_tool_bars_action"));
   connect(m_configureAction, &QAction::triggered, this, &ToolBarManager::configureToolBars);

   m_manager->setMainWindow(configureableMainWindow);

   for (QToolBar *tb : m_toolbars) {
      const QString title = tb->windowTitle();

      m_manager->addToolBar(tb, title);
      addActionsToToolBarManager(tb->actions(), title, m_manager);
   }

   addActionsToToolBarManager(actions->windowActions()->actions(), tr("Window"), m_manager);
   addActionsToToolBarManager(actions->helpActions()->actions(),   tr("Help"),   m_manager);

   // Filter out the device profile preview actions which have int data().
   ActionList previewActions = actions->styleActions()->actions();
   ActionList::iterator iter = previewActions.begin();

   while ( (*iter)->isSeparator() || (*iter)->data().type() == QVariant::Int)  {
      ++iter;
   }

   previewActions.erase(previewActions.begin(), iter);
   addActionsToToolBarManager(previewActions, tr("Style"), m_manager);

   const QString dockTitle = tr("Dock views");

   for (QDesignerToolWindow *tw : toolWindows) {
      if (QAction *action = tw->action()) {
         m_manager->addAction(action, dockTitle);
      }
   }

   QString category(tr("File"));

   for (QAction *action : actions->fileActions()->actions()) {
      m_manager->addAction(action, category);
   }

   category = tr("Edit");

   for (QAction *action : actions->editActions()->actions()) {
      m_manager->addAction(action, category);
   }

   category = tr("Tools");

   for (QAction *action : actions->toolActions()->actions()) {
      m_manager->addAction(action, category);
   }

   category = tr("Form");

   for (QAction *action : actions->formActions()->actions()) {
      m_manager->addAction(action, category);
   }

   m_manager->addAction(m_configureAction, tr("Toolbars"));
   updateToolBarMenu();
}

bool toolBarTitleLessThan(const QToolBar *t1, const QToolBar *t2)
{
   return t1->windowTitle() < t2->windowTitle();
}

void ToolBarManager::updateToolBarMenu()
{
   // sort tool bars alphabetically by title
   std::stable_sort(m_toolbars.begin(), m_toolbars.end(), toolBarTitleLessThan);

   // add to menu
   m_toolBarMenu->clear();

   for (QToolBar *tb :  m_toolbars) {
      m_toolBarMenu->addAction(tb->toggleViewAction());
   }

   m_toolBarMenu->addAction(m_configureAction);
}

void ToolBarManager::configureToolBars()
{
   QtToolBarDialog dlg(m_parent);
   dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
   dlg.setToolBarManager(m_manager);
   dlg.exec();

   updateToolBarMenu();
}

QByteArray ToolBarManager::saveState(int version) const
{
   return m_manager->saveState(version);
}

bool ToolBarManager::restoreState(const QByteArray &state, int version)
{
   return m_manager->restoreState(state, version);
}

DockedMainWindow::DockedMainWindow(QDesignerWorkbench *wb, QMenu *toolBarMenu,
      const QList<QDesignerToolWindow *> &toolWindows)
   : m_toolBarManager(nullptr)
{
   setObjectName("MDIWindow");
   setWindowTitle(mainWindowTitle());

   const QList<QToolBar *> toolbars = createToolBars(wb->actionManager(), false);
   for (QToolBar *tb : toolbars) {
      addToolBar(tb);
   }

   DockedMdiArea *dma = new DockedMdiArea(wb->actionManager()->uiExtension());

   connect(dma, &DockedMdiArea::fileDropped,   this, &DockedMainWindow::fileDropped);
   connect(dma, &QMdiArea::subWindowActivated, this, &DockedMainWindow::slotSubWindowActivated);

   setCentralWidget(dma);

   QStatusBar *tmp = statusBar();
   (void) tmp;

   m_toolBarManager = new ToolBarManager(this, this, toolBarMenu, wb->actionManager(), toolbars, toolWindows);
}

QMdiArea *DockedMainWindow::mdiArea() const
{
   return static_cast<QMdiArea *>(centralWidget());
}

void DockedMainWindow::slotSubWindowActivated(QMdiSubWindow *subWindow)
{
   if (subWindow != nullptr) {
      QWidget *widget = subWindow->widget();

      if (QDesignerFormWindow *fw = dynamic_cast<QDesignerFormWindow *>(widget)) {
         emit formWindowActivated(fw);
         mdiArea()->setActiveSubWindow(subWindow);
      }
   }
}

QMdiSubWindow *DockedMainWindow::createMdiSubWindow(QWidget *fw, Qt::WindowFlags f, const QKeySequence &designerCloseActionShortCut)
{
   QMdiSubWindow *retval = mdiArea()->addSubWindow(fw, f);

   // Make action shortcuts respond only if focused to avoid conflicts with
   // designer menu actions

   if (designerCloseActionShortCut == QKeySequence(QKeySequence::Close)) {
      const ActionList systemMenuActions = retval->systemMenu()->actions();

      if (! systemMenuActions.empty()) {
         for (auto item : systemMenuActions) {
            if (item->shortcut() == designerCloseActionShortCut) {
               item->setShortcutContext(Qt::WidgetShortcut);
               break;
            }
         }
      }
   }

   return retval;
}

DockedMainWindow::DockWidgetList DockedMainWindow::addToolWindows(const DesignerToolWindowList &tls)
{
   DockWidgetList rc;

   for (QDesignerToolWindow *tw : tls) {
      QDockWidget *dockWidget = new QDockWidget;
      dockWidget->setObjectName(tw->objectName() + "_dock");
      dockWidget->setWindowTitle(tw->windowTitle());

      addDockWidget(tw->dockWidgetAreaHint(), dockWidget);
      dockWidget->setWidget(tw);
      rc.push_back(dockWidget);
   }

   return rc;
}

// Settings consist of MainWindow state and tool bar manager state
void DockedMainWindow::restoreSettings(const QDesignerSettings &settings, const DockWidgetList &dws, const QRect &desktopArea)
{
   const int version = settingsVersion();

   // check if there are prior geometry settings and uses them, otherwise shows the main window full screen
   settings.restoreGeometry(this, QRect(desktopArea.topLeft(), QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)));

   // restore the user tool bar
   m_toolBarManager->restoreState(settings.toolBarsState(DockedMode), version);

   // docked windows
   const QByteArray mainWindowState = settings.mainWindowState(DockedMode);
   const bool restored = ! mainWindowState.isEmpty() && restoreState(mainWindowState, version);

   if (! restored) {
      // put tab windows in the bottom right
      tabifyDockWidget(dws.at(QDesignerToolWindow::SignalSlotEditor), dws.at(QDesignerToolWindow::ActionEditor));
      tabifyDockWidget(dws.at(QDesignerToolWindow::ActionEditor),     dws.at(QDesignerToolWindow::ResourceEditor));
   }
}

void DockedMainWindow::saveSettings(QDesignerSettings &s) const
{
   const int version = settingsVersion();

   s.setToolBarsState(DockedMode, m_toolBarManager->saveState(version));
   s.saveGeometryFor(this);
   s.setMainWindowState(DockedMode, saveState(version));
}

