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

#ifndef QDESIGNER_MENU_H
#define QDESIGNER_MENU_H

#include <QAction>
#include <QHash>
#include <QMenu>
#include <QPixmap>

class QDesignerActionProviderExtension;
class QDesignerFormWindowInterface;
class QDesignerMenu;
class QDesignerMenuBar;

class QLineEdit;
class QMimeData;
class QPainter;
class QTimer;

namespace qdesigner_internal {
class CreateSubmenuCommand;
class ActionInsertionCommand;
}

class QDesignerMenu: public QMenu
{
   CS_OBJECT(QDesignerMenu)
 public:
   QDesignerMenu(QWidget *parent = nullptr);
   virtual ~QDesignerMenu();

   bool eventFilter(QObject *object, QEvent *event) override;

   QDesignerFormWindowInterface *formWindow() const;
   QDesignerActionProviderExtension *actionProvider();

   QDesignerMenu *parentMenu() const;
   QDesignerMenuBar *parentMenuBar() const;

   void setVisible(bool visible) override;

   void adjustSpecialActions();

   bool interactive(bool i);
   void createRealMenuAction(QAction *action);
   void removeRealMenu(QAction *action);

   static void drawSelection(QPainter *p, const QRect &r);

   bool dragging() const;

   void closeMenuChain();

   void moveLeft();
   void moveRight();
   void moveUp(bool ctrl);
   void moveDown(bool ctrl);

   // Helper for MenuTaskMenu extension
   void deleteAction(QAction *a);

 private:
   CS_SLOT_1(Private, void slotAddSeparator())
   CS_SLOT_2(slotAddSeparator)
   CS_SLOT_1(Private, void slotRemoveSelectedAction())
   CS_SLOT_2(slotRemoveSelectedAction)
   CS_SLOT_1(Private, void slotShowSubMenuNow())
   CS_SLOT_2(slotShowSubMenuNow)
   CS_SLOT_1(Private, void slotDeactivateNow())
   CS_SLOT_2(slotDeactivateNow)
   CS_SLOT_1(Private, void slotAdjustSizeNow())
   CS_SLOT_2(slotAdjustSizeNow)

 protected:
   void actionEvent(QActionEvent *event) override;
   void dragEnterEvent(QDragEnterEvent *event) override;
   void dragMoveEvent(QDragMoveEvent *event) override;
   void dragLeaveEvent(QDragLeaveEvent *event) override;
   void dropEvent(QDropEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void keyReleaseEvent(QKeyEvent *event) override;
   void showEvent(QShowEvent *event) override;

   bool handleEvent(QWidget *widget, QEvent *event);
   bool handleMouseDoubleClickEvent(QWidget *widget, QMouseEvent *event);
   bool handleMousePressEvent(QWidget *widget, QMouseEvent *event);
   bool handleMouseReleaseEvent(QWidget *widget, QMouseEvent *event);
   bool handleMouseMoveEvent(QWidget *widget, QMouseEvent *event);
   bool handleContextMenuEvent(QWidget *widget, QContextMenuEvent *event);
   bool handleKeyPressEvent(QWidget *widget, QKeyEvent *event);

   void startDrag(const QPoint &pos, Qt::KeyboardModifiers modifiers);

   void adjustIndicator(const QPoint &pos);
   int findAction(const QPoint &pos) const;

   QAction *currentAction() const;
   int realActionCount() const;
   enum ActionDragCheck { NoActionDrag, ActionDragOnSubMenu, AcceptActionDrag };
   ActionDragCheck checkAction(QAction *action) const;

   void showSubMenu(QAction *action);

   enum LeaveEditMode {
      Default = 0,
      ForceAccept
   };

   void enterEditMode();
   void leaveEditMode(LeaveEditMode mode);
   void showLineEdit();

   QAction *createAction(const QString &text, bool separator = false);
   QDesignerMenu *findOrCreateSubMenu(QAction *action);

   QAction *safeActionAt(int index) const;
   QAction *safeMenuAction(QDesignerMenu *menu) const;
   bool swap(int a, int b);

   void hideSubMenu();
   void deleteAction();
   void deactivateMenu();

   bool canCreateSubMenu(QAction *action) const;
   QDesignerMenu *findRootMenu() const;
   QDesignerMenu *findActivatedMenu() const;

   QRect subMenuPixmapRect(QAction *action) const;
   bool hasSubMenuPixmap(QAction *action) const;

   void selectCurrentAction();

 private:
   bool hideSubMenuOnCursorKey();
   bool showSubMenuOnCursorKey();
   const QPixmap m_subMenuPixmap;

   QPoint m_startPosition;
   int m_currentIndex;
   QAction *m_addItem;
   QAction *m_addSeparator;
   QHash<QAction *, QDesignerMenu *> m_subMenus;
   QTimer *m_showSubMenuTimer;
   QTimer *m_deactivateWindowTimer;
   QTimer *m_adjustSizeTimer;
   bool m_interactive;
   QLineEdit *m_editor;
   bool m_dragging;
   int m_lastSubMenuIndex;

   friend class qdesigner_internal::CreateSubmenuCommand;
   friend class qdesigner_internal::ActionInsertionCommand;
};



#endif // QDESIGNER_MENU_H
