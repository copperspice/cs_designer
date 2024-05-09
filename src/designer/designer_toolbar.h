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

#ifndef QDESIGNER_TOOLBAR_H
#define QDESIGNER_TOOLBAR_H

#include <QAction>
#include <QList>
#include <QPoint>
#include <QToolButton>

class QDesignerFormWindowInterface;

class QAction;
class QRect;
class QToolBar;

namespace qdesigner_internal {

class PromotionTaskMenu;

// Special event filter for  tool bars in designer.
// Handles drag and drop to and from. Ensures that each
// child widget is  WA_TransparentForMouseEvents to enable  drag and drop.

class ToolBarEventFilter : public QObject
{
   CS_OBJECT(ToolBarEventFilter)

 public:
   static void install(QToolBar *tb);

   // Find action by position. Note that QToolBar::actionAt() will
   // not work as designer sets WA_TransparentForMouseEvents on its tool bar buttons
   // to be able to drag them. This function will return the dummy
   // sentinel action when applied to tool bars created by designer if the position matches.
   static QAction *actionAt(const QToolBar *tb, const QPoint &pos);

   static bool withinHandleArea(const QToolBar *tb, const QPoint &pos);
   static QRect handleArea(const QToolBar *tb);
   static QRect freeArea(const QToolBar *tb);

   // Utility to create an action
   static QAction *createAction(QDesignerFormWindowInterface *fw, const QString &objectName, bool separator);

   bool eventFilter (QObject *watched, QEvent *event) override;

   // Helper for task menu extension
   QList<QAction *> contextMenuActions(const QPoint &globalPos = QPoint(-1, -1));

   static ToolBarEventFilter *eventFilterOf(const QToolBar *tb);

 private:
   CS_SLOT_1(Private, void slotRemoveSelectedAction())
   CS_SLOT_2(slotRemoveSelectedAction)
   CS_SLOT_1(Private, void slotRemoveToolBar())
   CS_SLOT_2(slotRemoveToolBar)
   CS_SLOT_1(Private, void slotInsertSeparator())
   CS_SLOT_2(slotInsertSeparator)

 private:
   explicit ToolBarEventFilter(QToolBar *tb);

   bool handleContextMenuEvent(QContextMenuEvent *event);
   bool handleDragEnterMoveEvent(QDragMoveEvent *event);
   bool handleDragLeaveEvent(QDragLeaveEvent *);
   bool handleDropEvent(QDropEvent *event);
   bool handleMousePressEvent(QMouseEvent *event);
   bool handleMouseReleaseEvent(QMouseEvent *event);
   bool handleMouseMoveEvent(QMouseEvent *event);

   QDesignerFormWindowInterface *formWindow() const;
   void adjustDragIndicator(const QPoint &pos);
   void hideDragIndicator();
   void startDrag(const QPoint &pos, Qt::KeyboardModifiers modifiers);
   bool withinHandleArea(const QPoint &pos) const;

   QToolBar *m_toolBar;
   PromotionTaskMenu *m_promotionTaskMenu;
   QPoint m_startPosition;
};
} // namespace qdesigner_internal



#endif // QDESIGNER_TOOLBAR_H
