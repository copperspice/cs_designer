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

#ifndef QDESIGNER_TOOLWINDOW_H
#define QDESIGNER_TOOLWINDOW_H

#include <mainwindow.h>

#include <QFontDatabase>
#include <QMainWindow>
#include <QPointer>

struct ToolWindowFontSettings {
   ToolWindowFontSettings();
   bool equals(const ToolWindowFontSettings &) const;

   QFont m_font;
   QFontDatabase::WritingSystem m_writingSystem;
   bool m_useFont;
};

inline bool operator==(const ToolWindowFontSettings &tw1, const ToolWindowFontSettings &tw2)
{
   return tw1.equals(tw2);
}

inline bool operator!=(const ToolWindowFontSettings &tw1, const ToolWindowFontSettings &tw2)
{
   return !tw1.equals(tw2);
}

class QDesignerWorkbench;

/* A tool window with an action that activates it. Note that in toplevel mode,
 * the Widget box is a tool window as well as the applications' main window,
 * So, we need to inherit from MainWindowBase. */

class QDesignerToolWindow : public MainWindowBase
{
   CS_OBJECT(QDesignerToolWindow)

 public:
   // Note: The order influences the dock widget position.
   enum StandardToolWindow { WidgetBox,  ObjectInspector, PropertyEditor,
      ResourceEditor, ActionEditor, SignalSlotEditor,
      StandardToolWindowCount
   };

   static QDesignerToolWindow *createStandardToolWindow(StandardToolWindow which, QDesignerWorkbench *workbench);

   QDesignerWorkbench *workbench() const;
   QAction *action() const;

   Qt::DockWidgetArea dockWidgetAreaHint() const {
      return m_dockAreaHint;
   }
   virtual QRect geometryHint() const;

 protected:
   explicit QDesignerToolWindow(QDesignerWorkbench *workbench, QWidget *w,
      const QString &objectName, const QString &title, const QString &actionObjectName,
      Qt::DockWidgetArea dockAreaHint, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::Window);

   void showEvent(QShowEvent *e) override;
   void hideEvent(QHideEvent *e) override;
   void changeEvent(QEvent *e) override;

   QRect availableToolWindowGeometry() const;

 private:
   CS_SLOT_1(Private, void showMe(bool un_named_arg1))
   CS_SLOT_2(showMe)

   const Qt::DockWidgetArea m_dockAreaHint;
   QDesignerWorkbench *m_workbench;
   QAction *m_action;
};

#endif // QDESIGNER_TOOLWINDOW_H
