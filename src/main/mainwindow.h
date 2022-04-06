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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class QDesignerActions;
class QDesignerWorkbench;
class QDesignerToolWindow;
class QDesignerFormWindow;
class QDesignerSettings;
class QtToolBarManager;

#include <QList>
#include <QMainWindow>
#include <QMdiArea>

class QToolBar;
class QMdiArea;
class QMenu;
class QByteArray;
class QMimeData;

class MainWindowBase : public QMainWindow
{
   CS_OBJECT(MainWindowBase)

 protected:
   explicit MainWindowBase(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::Window);

 public:
   enum CloseEventPolicy {
      AcceptCloseEvents,         //  Always accept close events
      EmitCloseEventSignal       //  Emit a signal with the event, have it handled elsewhere
   };

   CloseEventPolicy closeEventPolicy() const {
      return m_policy;
   }
   void setCloseEventPolicy(CloseEventPolicy pol) {
      m_policy = pol;
   }

   static QList<QToolBar *> createToolBars(const QDesignerActions *actions, bool singleToolBar);
   static QString mainWindowTitle();

   // Use the minor version as settings versions to avoid conflicts
   static int settingsVersion();


 public:
   CS_SIGNAL_1(Public, void closeEventReceived(QCloseEvent *e))
   CS_SIGNAL_2(closeEventReceived, e)

 protected:
   void closeEvent(QCloseEvent *e) override;

 private:
   Q_DISABLE_COPY(MainWindowBase)
   CloseEventPolicy m_policy;
};


class DockedMdiArea : public QMdiArea
{
   CS_OBJECT(DockedMdiArea)

 public:
   explicit DockedMdiArea(const QString &extension, QWidget *parent = nullptr);

   CS_SIGNAL_1(Public, void fileDropped(const QString &un_named_arg1))
   CS_SIGNAL_2(fileDropped, un_named_arg1)

 protected:
   bool event (QEvent *event);

 private:
   Q_DISABLE_COPY(DockedMdiArea)

   QStringList uiFiles(const QMimeData *d) const;
   const QString m_extension;
};

// manages a QtToolBarManager and an action to trigger  on a mainwindow.
class ToolBarManager : public QObject
{
   CS_OBJECT(ToolBarManager)

 public:
   explicit ToolBarManager(QMainWindow *configureableMainWindow,
      QWidget *parent,
      QMenu *toolBarMenu,
      const QDesignerActions *actions,
      const QList<QToolBar *> &toolbars,
      const QList<QDesignerToolWindow *> &toolWindows);

   QByteArray saveState(int version = 0) const;
   bool restoreState(const QByteArray &state, int version = 0);

 private:
   CS_SLOT_1(Private, void configureToolBars())
   CS_SLOT_2(configureToolBars)
   CS_SLOT_1(Private, void updateToolBarMenu())
   CS_SLOT_2(updateToolBarMenu)

   Q_DISABLE_COPY(ToolBarManager)

   QMainWindow *m_configureableMainWindow;
   QWidget *m_parent;
   QMenu *m_toolBarMenu;
   QtToolBarManager *m_manager;
   QAction *m_configureAction;
   QList<QToolBar *> m_toolbars;
};

/* Main window to be used for docked mode */
class DockedMainWindow : public MainWindowBase
{
   CS_OBJECT(DockedMainWindow)

 public:
   typedef QList<QDesignerToolWindow *> DesignerToolWindowList;
   typedef QList<QDockWidget *> DockWidgetList;

   explicit DockedMainWindow(QDesignerWorkbench *wb,
      QMenu *toolBarMenu, const DesignerToolWindowList &toolWindows);

   // Create a MDI subwindow for the form.
   QMdiSubWindow *createMdiSubWindow(QWidget *fw, Qt::WindowFlags f, const QKeySequence &designerCloseActionShortCut);

   QMdiArea *mdiArea() const;

   DockWidgetList addToolWindows(const DesignerToolWindowList &toolWindows);

   void restoreSettings(const QDesignerSettings &s, const DockWidgetList &dws, const QRect &desktopArea);
   void saveSettings(QDesignerSettings &) const;

 public:
   CS_SIGNAL_1(Public, void fileDropped(const QString &un_named_arg1))
   CS_SIGNAL_2(fileDropped, un_named_arg1)
   CS_SIGNAL_1(Public, void formWindowActivated(QDesignerFormWindow *un_named_arg1))
   CS_SIGNAL_2(formWindowActivated, un_named_arg1)

 private:
   CS_SLOT_1(Private, void slotSubWindowActivated(QMdiSubWindow *un_named_arg1))
   CS_SLOT_2(slotSubWindowActivated)

   Q_DISABLE_COPY(DockedMainWindow)
   ToolBarManager *m_toolBarManager;
};

#endif
