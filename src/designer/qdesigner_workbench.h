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

#ifndef QDESIGNER_WORKBENCH_H
#define QDESIGNER_WORKBENCH_H

#include <designer_enums.h>

class QDesignerActions;
class QDesignerToolWindow;
class QDesignerFormWindow;
class DockedMainWindow;
class QDesignerSettings;
class QtToolBarManager;
class ToolBarManager;
class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QDesignerFormWindowManagerInterface;
class QDesignerIntegration;

#include <QObject>
#include <QHash>
#include <QSet>
#include <QList>
#include <QRect>

class QAction;
class QActionGroup;
class QDockWidget;
class QMenu;
class QMenuBar;
class QMainWindow;
class QToolBar;
class QMdiArea;
class QMdiSubWindow;
class QCloseEvent;
class QFont;

class QDesignerWorkbench: public QObject
{
   CS_OBJECT(QDesignerWorkbench)

 public:
   QDesignerWorkbench();
   virtual ~QDesignerWorkbench();

   UIMode mode() const;

   QDesignerFormEditorInterface *core() const;
   QDesignerFormWindow *findFormWindow(QWidget *widget) const;

   QDesignerFormWindow *openForm(const QString &fileName, QString *errorMessage);
   QDesignerFormWindow *openTemplate(const QString &templateFileName,
      const QString &editorFileName, QString *errorMessage);

   int toolWindowCount() const;
   QDesignerToolWindow *toolWindow(int index) const;

   int formWindowCount() const;
   QDesignerFormWindow *formWindow(int index) const;

   QDesignerActions *actionManager() const;

   QActionGroup *modeActionGroup() const;

   QRect availableGeometry() const;
   QRect desktopGeometry() const;

   int marginHint() const;

   bool readInForm(const QString &fileName) const;
   bool writeOutForm(QDesignerFormWindowInterface *formWindow, const QString &fileName) const;
   bool saveForm(QDesignerFormWindowInterface *fw);
   bool handleClose();
   bool readInBackup();
   void updateBackup(QDesignerFormWindowInterface *fwi);
   void applyUiSettings();

   CS_SIGNAL_1(Public, void modeChanged(UIMode mode))
   CS_SIGNAL_2(modeChanged, mode)
   CS_SIGNAL_1(Public, void initialized())
   CS_SIGNAL_2(initialized)

   CS_SLOT_1(Public, void addFormWindow(QDesignerFormWindow *formWindow))
   CS_SLOT_2(addFormWindow)
   CS_SLOT_1(Public, void removeFormWindow(QDesignerFormWindow *formWindow))
   CS_SLOT_2(removeFormWindow)
   CS_SLOT_1(Public, void bringAllToFront())
   CS_SLOT_2(bringAllToFront)
   CS_SLOT_1(Public, void toggleFormMinimizationState())
   CS_SLOT_2(toggleFormMinimizationState)

 private:
   CS_SLOT_1(Private, void switchToNeutralMode())
   CS_SLOT_2(switchToNeutralMode)
   CS_SLOT_1(Private, void switchToDockedMode())
   CS_SLOT_2(switchToDockedMode)
   CS_SLOT_1(Private, void switchToTopLevelMode())
   CS_SLOT_2(switchToTopLevelMode)
   CS_SLOT_1(Private, void initializeCorePlugins())
   CS_SLOT_2(initializeCorePlugins)
   CS_SLOT_1(Private, void handleCloseEvent(QCloseEvent *un_named_arg1))
   CS_SLOT_2(handleCloseEvent)
   CS_SLOT_1(Private, void slotFormWindowActivated(QDesignerFormWindow *fw))
   CS_SLOT_2(slotFormWindowActivated)
   CS_SLOT_1(Private, void updateWindowMenu(QDesignerFormWindowInterface *fw))
   CS_SLOT_2(updateWindowMenu)
   CS_SLOT_1(Private, void formWindowActionTriggered(QAction *a))
   CS_SLOT_2(formWindowActionTriggered)
   CS_SLOT_1(Private, void adjustMDIFormPositions())
   CS_SLOT_2(adjustMDIFormPositions)
   CS_SLOT_1(Private, void minimizationStateChanged(QDesignerFormWindowInterface *formWindow, bool minimized))
   CS_SLOT_2(minimizationStateChanged)

   CS_SLOT_1(Private, void restoreUISettings())
   CS_SLOT_2(restoreUISettings)
   CS_SLOT_1(Private, void notifyUISettingsChanged())
   CS_SLOT_2(notifyUISettingsChanged)
   CS_SLOT_1(Private, void slotFileDropped(const QString &f))
   CS_SLOT_2(slotFileDropped)

   QWidget *magicalParent(const QWidget *w) const;
   Qt::WindowFlags magicalWindowFlags(const QWidget *widgetForFlags) const;
   QDesignerFormWindowManagerInterface *formWindowManager() const;
   void closeAllToolWindows();
   QDesignerToolWindow *widgetBoxToolWindow() const;
   QDesignerFormWindow *loadForm(const QString &fileName, bool detectLineTermiantorMode, QString *errorMessage);
   void resizeForm(QDesignerFormWindow *fw,  const QWidget *mainContainer) const;
   void saveGeometriesForModeChange();
   void saveGeometries(QDesignerSettings &settings) const;

   bool isFormWindowMinimized(const QDesignerFormWindow *fw);
   void setFormWindowMinimized(QDesignerFormWindow *fw, bool minimized);
   void saveSettings() const;

   QDesignerFormEditorInterface *m_core;
   QDesignerIntegration *m_integration;

   QDesignerActions *m_actionManager;
   QActionGroup *m_windowActions;

   QMenu *m_windowMenu;

   QMenuBar *m_globalMenuBar;

   struct TopLevelData {
      ToolBarManager *toolbarManager;
      QList<QToolBar *> toolbars;
   };
   TopLevelData m_topLevelData;

   UIMode m_mode;
   DockedMainWindow *m_dockedMainWindow;

   QList<QDesignerToolWindow *> m_toolWindows;
   QList<QDesignerFormWindow *> m_formWindows;

   QMenu *m_toolbarMenu;

   // Helper class to remember the position of a window while switching user
   // interface modes.
   class Position
   {
    public:
      Position(const QDockWidget *dockWidget);
      Position(const QMdiSubWindow *mdiSubWindow, const QPoint &mdiAreaOffset);
      Position(const QWidget *topLevelWindow, const QPoint &desktopTopLeft);

      void applyTo(QMdiSubWindow *mdiSubWindow, const QPoint &mdiAreaOffset) const;
      void applyTo(QWidget *topLevelWindow, const QPoint &desktopTopLeft) const;
      void applyTo(QDockWidget *dockWidget) const;

      QPoint position() const {
         return m_position;
      }

    private:
      bool m_minimized;
      // Position referring to top-left corner (desktop in top-level mode or
      // main window in MDI mode)
      QPoint m_position;
   };

   typedef  QHash<QWidget *, Position> PositionMap;
   PositionMap m_Positions;

   enum State { StateInitializing, StateUp, StateClosing };
   State m_state;
   bool m_uiSettingsChanged; // UI mode changed in preference dialog, trigger delayed slot.
};

#endif // QDESIGNER_WORKBENCH_H
