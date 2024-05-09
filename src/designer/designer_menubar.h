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

#ifndef QDESIGNER_MENUBAR_H
#define QDESIGNER_MENUBAR_H

#include <QAction>
#include <QMenuBar>
#include <QMimeData>
#include <QPointer>

class QDesignerActionProviderExtension;
class QDesignerFormWindowInterface;

class QLineEdit;
class QMimeData;

namespace qdesigner_internal {
class PromotionTaskMenu;

class SpecialMenuAction: public QAction
{
   CS_OBJECT(SpecialMenuAction)
 public:
   SpecialMenuAction(QObject *parent = nullptr);
   virtual ~SpecialMenuAction();
};

} // namespace qdesigner_internal

class QDesignerMenuBar: public QMenuBar
{
   CS_OBJECT(QDesignerMenuBar)
 public:
   QDesignerMenuBar(QWidget *parent = nullptr);
   virtual ~QDesignerMenuBar();

   bool eventFilter(QObject *object, QEvent *event) override;

   QDesignerFormWindowInterface *formWindow() const;
   QDesignerActionProviderExtension *actionProvider();

   void adjustSpecialActions();
   bool interactive(bool i);
   bool dragging() const;

   void moveLeft(bool ctrl = false);
   void moveRight(bool ctrl = false);
   void moveUp();
   void moveDown();

   // Helpers for MenuTaskMenu/MenuBarTaskMenu extensions
   QList<QAction *> contextMenuActions();
   void deleteMenuAction(QAction *action);

 private:
   CS_SLOT_1(Private, void deleteMenu())
   CS_SLOT_2(deleteMenu)
   CS_SLOT_1(Private, void slotRemoveMenuBar())
   CS_SLOT_2(slotRemoveMenuBar)

 protected:
   void actionEvent(QActionEvent *event) override;
   void dragEnterEvent(QDragEnterEvent *event) override;
   void dragMoveEvent(QDragMoveEvent *event) override;
   void dragLeaveEvent(QDragLeaveEvent *event) override;
   void dropEvent(QDropEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void keyReleaseEvent(QKeyEvent *event) override;

   bool handleEvent(QWidget *widget, QEvent *event);
   bool handleMouseDoubleClickEvent(QWidget *widget, QMouseEvent *event);
   bool handleMousePressEvent(QWidget *widget, QMouseEvent *event);
   bool handleMouseReleaseEvent(QWidget *widget, QMouseEvent *event);
   bool handleMouseMoveEvent(QWidget *widget, QMouseEvent *event);
   bool handleContextMenuEvent(QWidget *widget, QContextMenuEvent *event);
   bool handleKeyPressEvent(QWidget *widget, QKeyEvent *event);

   void startDrag(const QPoint &pos);

   enum ActionDragCheck { NoActionDrag, ActionDragOnSubMenu, AcceptActionDrag };
   ActionDragCheck checkAction(QAction *action) const;

   void adjustIndicator(const QPoint &pos);
   int findAction(const QPoint &pos) const;

   QAction *currentAction() const;
   int realActionCount() const;

   enum LeaveEditMode {
      Default = 0,
      ForceAccept
   };

   void enterEditMode();
   void leaveEditMode(LeaveEditMode mode);
   void showLineEdit();

   void showMenu(int index = -1);
   void hideMenu(int index = -1);

   QAction *safeActionAt(int index) const;

   bool swapActions(int a, int b);

 private:
   void updateCurrentAction(bool selectAction);
   void movePrevious(bool ctrl);
   void moveNext(bool ctrl);

   QAction *m_addMenu;
   QPointer<QMenu> m_activeMenu;
   QPoint m_startPosition;
   int m_currentIndex;
   bool m_interactive;
   QLineEdit *m_editor;
   bool m_dragging;
   int m_lastMenuActionIndex;
   QPointer<QWidget> m_lastFocusWidget;
   qdesigner_internal::PromotionTaskMenu *m_promotionTaskMenu;
};



#endif // QDESIGNER_MENUBAR_H
