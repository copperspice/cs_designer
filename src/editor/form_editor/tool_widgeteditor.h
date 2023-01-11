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

#ifndef TOOL_WIDGETEDITOR_H
#define TOOL_WIDGETEDITOR_H

#include <abstract_formwindowtool.h>

#include <qevent.h>
#include <QPointer>

class QAction;
class QMainWindow;

namespace qdesigner_internal {

class FormWindow;
class QDesignerMimeData;

class WidgetEditorTool: public QDesignerFormWindowToolInterface
{
   CS_OBJECT(WidgetEditorTool)
 public:
   explicit WidgetEditorTool(FormWindow *formWindow);
   virtual ~WidgetEditorTool();

   QDesignerFormEditorInterface *core() const override;
   QDesignerFormWindowInterface *formWindow() const override;
   QWidget *editor() const override;
   QAction *action() const override;

   void activated() override;
   void deactivated() override;

   bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) override;

   bool handleContextMenu(QWidget *widget, QWidget *managedWidget, QContextMenuEvent *e);
   bool handleMouseButtonDblClickEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
   bool handleMousePressEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
   bool handleMouseMoveEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
   bool handleMouseReleaseEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
   bool handleKeyPressEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e);
   bool handleKeyReleaseEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e);
   bool handlePaintEvent(QWidget *widget, QWidget *managedWidget, QPaintEvent *e);

   bool handleDragEnterMoveEvent(QWidget *widget, QWidget *managedWidget, QDragMoveEvent *e, bool isEnter);
   bool handleDragLeaveEvent(QWidget *widget, QWidget *managedWidget, QDragLeaveEvent *e);
   bool handleDropEvent(QWidget *widget, QWidget *managedWidget, QDropEvent *e);

 private:
   bool restoreDropHighlighting();
   void detectDockDrag(const QDesignerMimeData *mimeData);

   FormWindow *m_formWindow;
   QAction *m_action;

   bool mainWindowSeparatorEvent(QWidget *widget, QEvent *event);
   QPointer<QMainWindow> m_separator_drag_mw;
   QPointer<QWidget> m_lastDropTarget;
   bool m_specialDockDrag;
};

}   // end namespace qdesigner_internal

#endif
