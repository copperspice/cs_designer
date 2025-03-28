/***********************************************************************
*
* Copyright (c) 2021-2025 Barbara Geller
* Copyright (c) 2021-2025 Ansel Sermersheim
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

#include <abstract_formeditor.h>
#include <abstract_widgetfactory.h>
#include <action_editor.h>
#include <action_provider.h>
#include <action_repository.h>
#include <designer_command.h>
#include <designer_menu.h>
#include <designer_menubar.h>
#include <designer_object_inspector.h>
#include <designer_property_command.h>
#include <designer_toolbar.h>
#include <designer_utils.h>
#include <extension.h>

#include <metadatabase_p.h>

#include <QAction>
#include <QApplication>
#include <QDrag>
#include <QEvent>
#include <QLineEdit>
#include <QPainter>
#include <QRubberBand>
#include <QTimer>
#include <QToolBar>
#include <QToolTip>

CS_DECLARE_METATYPE(QAction *)

using namespace qdesigner_internal;

// give the user a little more space to click on the sub menu rectangle
static inline void extendClickableArea(QRect *subMenuRect, Qt::LayoutDirection dir)
{
   switch (dir) {
      case Qt::LayoutDirectionAuto: // Should never happen
      case Qt::LeftToRight:
         subMenuRect->setLeft(subMenuRect->left() - 20);
         break;
      case Qt::RightToLeft:
         subMenuRect->setRight(subMenuRect->right() + 20);
         break;
   }
}

QDesignerMenu::QDesignerMenu(QWidget *parent) :
   QMenu(parent),
   m_subMenuPixmap(QPixmap(QString(":/resources/form_editor/images/submenu.png"))),
   m_currentIndex(0),
   m_addItem(new SpecialMenuAction(this)),
   m_addSeparator(new SpecialMenuAction(this)),
   m_showSubMenuTimer(new QTimer(this)),
   m_deactivateWindowTimer(new QTimer(this)),
   m_adjustSizeTimer(new QTimer(this)),
   m_editor(new QLineEdit(this)),
   m_dragging(false),
   m_lastSubMenuIndex(-1)
{
   setContextMenuPolicy(Qt::DefaultContextMenu);
   setAcceptDrops(true); // ### fake
   setSeparatorsCollapsible(false);

   connect(m_adjustSizeTimer, &QTimer::timeout, this, &QDesignerMenu::slotAdjustSizeNow);
   m_addItem->setText(tr("Type Here"));
   addAction(m_addItem);

   m_addSeparator->setText(tr("Add Separator"));
   addAction(m_addSeparator);

   connect(m_showSubMenuTimer, &QTimer::timeout, this, &QDesignerMenu::slotShowSubMenuNow);

   connect(m_deactivateWindowTimer, &QTimer::timeout, this, &QDesignerMenu::slotDeactivateNow);

   m_editor->setObjectName(QString("__qt__passive_editor"));
   m_editor->hide();

   m_editor->installEventFilter(this);
   installEventFilter(this);
}

QDesignerMenu::~QDesignerMenu()
{
}

void QDesignerMenu::slotAdjustSizeNow()
{
   // Not using a single-shot, since we want to compress the timers if many items are being
   // adjusted
   m_adjustSizeTimer->stop();
   adjustSize();
}

bool QDesignerMenu::handleEvent(QWidget *widget, QEvent *event)
{
   if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) {
      update();

      if (widget == m_editor) {
         return false;
      }
   }

   switch (event->type()) {
      default:
         break;

      case QEvent::MouseButtonPress:
         return handleMousePressEvent(widget, static_cast<QMouseEvent *>(event));
      case QEvent::MouseButtonRelease:
         return handleMouseReleaseEvent(widget, static_cast<QMouseEvent *>(event));
      case QEvent::MouseButtonDblClick:
         return handleMouseDoubleClickEvent(widget, static_cast<QMouseEvent *>(event));
      case QEvent::MouseMove:
         return handleMouseMoveEvent(widget, static_cast<QMouseEvent *>(event));
      case QEvent::ContextMenu:
         return handleContextMenuEvent(widget, static_cast<QContextMenuEvent *>(event));
      case QEvent::KeyPress:
         return handleKeyPressEvent(widget, static_cast<QKeyEvent *>(event));
   }

   return true;
}

void QDesignerMenu::startDrag(const QPoint &pos, Qt::KeyboardModifiers modifiers)
{
   const int index = findAction(pos);
   if (index >= realActionCount()) {
      return;
   }

   QAction *action = safeActionAt(index);

   QDesignerFormWindowInterface *fw = formWindow();
   const Qt::DropAction dropAction = (modifiers & Qt::ControlModifier) ? Qt::CopyAction : Qt::MoveAction;
   if (dropAction == Qt::MoveAction) {
      RemoveActionFromCommand *cmd = new RemoveActionFromCommand(fw);
      cmd->init(this, action, actions().at(index + 1));
      fw->commandHistory()->push(cmd);
   }

   QDrag *drag = new QDrag(this);
   drag->setPixmap(ActionRepositoryMimeData::actionDragPixmap(action));
   drag->setMimeData(new ActionRepositoryMimeData(action, dropAction));

   const int old_index = m_currentIndex;
   m_currentIndex = -1;

   if (drag->start(dropAction) == Qt::IgnoreAction) {
      if (dropAction == Qt::MoveAction) {
         QAction *previous = safeActionAt(index);
         InsertActionIntoCommand *cmd = new InsertActionIntoCommand(fw);
         cmd->init(this, action, previous);
         fw->commandHistory()->push(cmd);
      }

      m_currentIndex = old_index;
   }
}

bool QDesignerMenu::handleKeyPressEvent(QWidget * /*widget*/, QKeyEvent *e)
{
   m_showSubMenuTimer->stop();

   if (m_editor->isHidden() && hasFocus()) { // In navigation mode
      switch (e->key()) {

         case Qt::Key_Delete:
            if (m_currentIndex == -1 || m_currentIndex >= realActionCount()) {
               break;
            }
            hideSubMenu();
            deleteAction();
            break;

         case Qt::Key_Left:
            e->accept();
            moveLeft();
            return true;

         case Qt::Key_Right:
            e->accept();
            moveRight();
            return true; // no update

         case Qt::Key_Up:
            e->accept();
            moveUp(e->modifiers() & Qt::ControlModifier);
            return true;

         case Qt::Key_Down:
            e->accept();
            moveDown(e->modifiers() & Qt::ControlModifier);
            return true;

         case Qt::Key_PageUp:
            m_currentIndex = 0;
            break;

         case Qt::Key_PageDown:
            m_currentIndex = actions().count() - 1;
            break;

         case Qt::Key_Enter:
         case Qt::Key_Return:
         case Qt::Key_F2:
            e->accept();
            enterEditMode();
            return true; // no update

         case Qt::Key_Escape:
            e->ignore();
            setFocus();
            hide();
            closeMenuChain();
            return true;

         case Qt::Key_Alt:
         case Qt::Key_Shift:
         case Qt::Key_Control:
            e->ignore();
            setFocus(); // FIXME: this is because some other widget get the focus when CTRL is pressed
            return true; // no update

         default: {
            QAction *action = currentAction();
            if (!action || action->isSeparator() || action == m_addSeparator) {
               e->ignore();
               return true;
            } else if (!e->text().isEmpty() && e->text().at(0).toLatin1() >= 32) {
               showLineEdit();
               QApplication::sendEvent(m_editor, e);
               e->accept();
            } else {
               e->ignore();
            }
         }
         return true;
      }
   } else if (m_editor->hasFocus()) { // In edit mode
      switch (e->key()) {
         default:
            e->ignore();
            return false;

         case Qt::Key_Enter:
         case Qt::Key_Return:
            if (!m_editor->text().isEmpty()) {
               leaveEditMode(ForceAccept);
               m_editor->hide();
               setFocus();
               moveDown(false);
               break;
            }
         // fall through

         case Qt::Key_Escape:
            m_editor->hide();
            setFocus();
            break;
      }
   }

   e->accept();
   update();

   return true;
}

static void sendMouseEventTo(QWidget *target, const QPoint &targetPoint, const QMouseEvent *event)
{
   QMouseEvent e(event->type(), targetPoint, event->globalPos(), event->button(), event->buttons(), event->modifiers());
   QApplication::sendEvent(target, &e);
}

bool QDesignerMenu::handleMouseDoubleClickEvent(QWidget *, QMouseEvent *event)
{
   event->accept();
   m_startPosition = QPoint();

   if ((event->buttons() & Qt::LeftButton) != Qt::LeftButton) {
      return true;
   }

   if (!rect().contains(event->pos())) {
      // special case for menubar
      QWidget *target = QApplication::widgetAt(event->globalPos());
      QMenuBar *mb    = dynamic_cast<QMenuBar *>(target);

      QDesignerMenu *menu = dynamic_cast<QDesignerMenu *>(target);

      if (mb != nullptr || menu != nullptr) {
         const QPoint pt = target->mapFromGlobal(event->globalPos());
         QAction *action = (mb == nullptr) ? menu->actionAt(pt) : mb->actionAt(pt);

         if (action) {
            sendMouseEventTo(target, pt, event);
         }
      }
      return true;
   }

   m_currentIndex = findAction(event->pos());
   QAction *action = safeActionAt(m_currentIndex);

   QRect pm_rect;
   if (action->menu() || hasSubMenuPixmap(action)) {
      pm_rect = subMenuPixmapRect(action);
      extendClickableArea(&pm_rect, layoutDirection());
   }

   if (! pm_rect.contains(event->pos()) && m_currentIndex != -1) {
      enterEditMode();
   }

   return true;
}

bool QDesignerMenu::handleMousePressEvent(QWidget * /*widget*/, QMouseEvent *event)
{
   if (!rect().contains(event->pos())) {
      QWidget *clickedWidget = QApplication::widgetAt(event->globalPos());

      if (QMenuBar *mb = dynamic_cast<QMenuBar *>(clickedWidget)) {
         const QPoint pt = mb->mapFromGlobal(event->globalPos());

         if (QAction *action = mb->actionAt(pt)) {
            QMenu *menu = action->menu();

            if (menu == findRootMenu()) {
               // propagate the mouse press event (but don't close the popup)
               sendMouseEventTo(mb, pt, event);
               return true;
            }
         }
      }

      if (QDesignerMenu *m = dynamic_cast<QDesignerMenu *>(clickedWidget)) {
         m->hideSubMenu();
         sendMouseEventTo(m, m->mapFromGlobal(event->globalPos()), event);

      } else {
         QDesignerMenu *root = findRootMenu();
         root->hide();
         root->hideSubMenu();
      }

      if (clickedWidget) {
         if (QWidget *focusProxy = clickedWidget->focusProxy()) {
            clickedWidget = focusProxy;
         }
         if (clickedWidget->focusPolicy() != Qt::NoFocus) {
            clickedWidget->setFocus(Qt::OtherFocusReason);
         }
      }
      return true;
   }

   m_showSubMenuTimer->stop();
   m_startPosition = QPoint();
   event->accept();

   if (event->button() != Qt::LeftButton) {
      return true;
   }

   m_startPosition = mapFromGlobal(event->globalPos());

   const int index = findAction(m_startPosition);

   QAction *action = safeActionAt(index);
   QRect pm_rect = subMenuPixmapRect(action);
   extendClickableArea(&pm_rect, layoutDirection());

   const int old_index = m_currentIndex;
   m_currentIndex = index;

   if ((hasSubMenuPixmap(action) || action->menu() != nullptr) && pm_rect.contains(m_startPosition)) {
      if (m_currentIndex == m_lastSubMenuIndex) {
         hideSubMenu();
      } else {
         slotShowSubMenuNow();
      }

   } else {
      if (index == old_index) {
         if (m_currentIndex == m_lastSubMenuIndex) {
            hideSubMenu();
         }
      } else {
         hideSubMenu();
      }
   }

   update();
   if (index != old_index) {
      selectCurrentAction();
   }

   return true;
}

bool QDesignerMenu::handleMouseReleaseEvent(QWidget *, QMouseEvent *event)
{
   event->accept();
   m_startPosition = QPoint();

   return true;
}

bool QDesignerMenu::handleMouseMoveEvent(QWidget *, QMouseEvent *event)
{
   if ((event->buttons() & Qt::LeftButton) != Qt::LeftButton) {
      return true;
   }

   if (!rect().contains(event->pos())) {

      if (QMenuBar *mb = dynamic_cast<QMenuBar *>(QApplication::widgetAt(event->globalPos()))) {
         const QPoint pt = mb->mapFromGlobal(event->globalPos());
         QAction *action = mb->actionAt(pt);

         if (action && action->menu() == findRootMenu()) {
            // propagate the mouse press event (but don't close the popup)
            sendMouseEventTo(mb, pt, event);
            return true;
         }

         // hide the popup and replay the event
         slotDeactivateNow();
      }
      return true;
   }

   if (m_startPosition.isNull()) {
      return true;
   }

   event->accept();

   const QPoint pos = mapFromGlobal(event->globalPos());

   if ((pos - m_startPosition).manhattanLength() < qApp->startDragDistance()) {
      return true;
   }

   startDrag(m_startPosition, event->modifiers());
   m_startPosition = QPoint();

   return true;
}

bool QDesignerMenu::handleContextMenuEvent(QWidget *, QContextMenuEvent *event)
{
   event->accept();

   const int index = findAction(mapFromGlobal(event->globalPos()));
   QAction *action = safeActionAt(index);
   if (dynamic_cast<SpecialMenuAction *>(action)) {
      return true;
   }

   QMenu menu;
   QVariant itemData;
   itemData.setValue(action);

   QAction *addSeparatorAction = menu.addAction(tr("Insert separator"));
   addSeparatorAction->setData(itemData);

   QAction *removeAction = nullptr;
   if (action->isSeparator()) {
      removeAction = menu.addAction(tr("Remove separator"));
   } else {
      removeAction = menu.addAction(tr("Remove action '%1'").formatArg(action->objectName()));
   }
   removeAction->setData(itemData);

   connect(addSeparatorAction, &QAction::triggered, this, &QDesignerMenu::slotAddSeparator);
   connect(removeAction, &QAction::triggered, this, &QDesignerMenu::slotRemoveSelectedAction);
   menu.exec(event->globalPos());

   return true;
}

void QDesignerMenu::slotAddSeparator()
{
   QAction *action = dynamic_cast<QAction *>(sender());
   if (!action) {
      return;
   }

   QAction *a = action->data().value<QAction *>();
   Q_ASSERT(a != nullptr);

   const int pos = actions().indexOf(a);
   QAction *action_before = nullptr;

   if (pos != -1) {
      action_before = safeActionAt(pos);
   }

   QDesignerFormWindowInterface *fw = formWindow();
   fw->beginCommand(tr("Add separator"));
   QAction *sep = createAction(QString(), true);

   InsertActionIntoCommand *insertCmd = new InsertActionIntoCommand(fw);
   insertCmd->init(this, sep, action_before);
   fw->commandHistory()->push(insertCmd);

   if (parentMenu()) {
      QAction *parent_action = parentMenu()->currentAction();

      if (parent_action->menu() == nullptr) {
         CreateSubmenuCommand *createCmd = new CreateSubmenuCommand(fw);
         createCmd->init(parentMenu(), parentMenu()->currentAction());
         fw->commandHistory()->push(createCmd);
      }
   }

   fw->endCommand();
}

void QDesignerMenu::slotRemoveSelectedAction()
{
   if (QAction *action = dynamic_cast<QAction *>(sender()))
      if (QAction *a = action->data().value<QAction *>()) {
         deleteAction(a);
      }
}

void QDesignerMenu::deleteAction(QAction *a)
{
   const int pos = actions().indexOf(a);
   QAction *action_before = nullptr;
   if (pos != -1) {
      action_before = safeActionAt(pos + 1);
   }

   QDesignerFormWindowInterface *fw = formWindow();
   RemoveActionFromCommand *cmd = new RemoveActionFromCommand(fw);
   cmd->init(this, a, action_before);
   fw->commandHistory()->push(cmd);
}

QRect QDesignerMenu::subMenuPixmapRect(QAction *action) const
{
   const QRect g = actionGeometry(action);
   const int x = layoutDirection() == Qt::LeftToRight ? (g.right() - m_subMenuPixmap.width() - 2) : 2;
   const int y = g.top() + (g.height() - m_subMenuPixmap.height()) / 2 + 1;
   return QRect(x, y, m_subMenuPixmap.width(), m_subMenuPixmap.height());
}

bool QDesignerMenu::hasSubMenuPixmap(QAction *action) const
{
   return action != nullptr
      && dynamic_cast<SpecialMenuAction *>(action) == nullptr
      && !action->isSeparator() && !action->menu() && canCreateSubMenu(action);
}

void QDesignerMenu::showEvent ( QShowEvent *event )
{
   selectCurrentAction();
   QMenu::showEvent (event);
}

void QDesignerMenu::paintEvent(QPaintEvent *event)
{
   QMenu::paintEvent(event);

   QPainter p(this);

   QAction *current = currentAction();

   for (QAction *a : actions()) {
      const QRect g = actionGeometry(a);

      if (dynamic_cast<SpecialMenuAction *>(a)) {
         QLinearGradient lg(g.left(), g.top(), g.left(), g.bottom());
         lg.setColorAt(0.0, Qt::transparent);
         lg.setColorAt(0.7, QColor(0, 0, 0, 32));
         lg.setColorAt(1.0, Qt::transparent);

         p.fillRect(g, lg);
      } else if (hasSubMenuPixmap(a)) {
         p.drawPixmap(subMenuPixmapRect(a).topLeft(), m_subMenuPixmap);
      }
   }

   if (!hasFocus() || !current || m_dragging) {
      return;
   }

   if (QDesignerMenu *menu = parentMenu()) {
      if (menu->dragging()) {
         return;
      }
   }

   if (QDesignerMenuBar *menubar = dynamic_cast<QDesignerMenuBar *>(parentWidget())) {
      if (menubar->dragging()) {
         return;
      }
   }

   const QRect g = actionGeometry(current);
   drawSelection(&p, g.adjusted(1, 1, -3, -3));
}

bool QDesignerMenu::dragging() const
{
   return m_dragging;
}

QDesignerMenu *QDesignerMenu::findRootMenu() const
{
   if (parentMenu()) {
      return parentMenu()->findRootMenu();
   }

   return const_cast<QDesignerMenu *>(this);
}

QDesignerMenu *QDesignerMenu::findActivatedMenu() const
{
   QList<QDesignerMenu *> candidates;
   candidates.append(const_cast<QDesignerMenu *>(this));
   candidates += findChildren<QDesignerMenu *>();

   for (QDesignerMenu *m : candidates) {
      if (m == qApp->activeWindow()) {
         return m;
      }
   }

   return nullptr;
}

bool QDesignerMenu::eventFilter(QObject *object, QEvent *event)
{
   if (object != this && object != m_editor) {
      return false;
   }

   if (!m_editor->isHidden() && object == m_editor && event->type() == QEvent::FocusOut) {
      leaveEditMode(Default);
      m_editor->hide();
      update();
      return false;
   }

   bool dispatch = true;

   switch (event->type()) {
      default:
         break;

      case QEvent::WindowDeactivate:
         deactivateMenu();
         break;

      case QEvent::ContextMenu:
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:

         while (QApplication::activePopupWidget() && !dynamic_cast<QDesignerMenu *>(QApplication::activePopupWidget())) {
            QApplication::activePopupWidget()->close();
         }

         [[fallthrough]];

      case QEvent::KeyPress:
      case QEvent::KeyRelease:
      case QEvent::MouseMove:
         dispatch = (object != m_editor);
         [[fallthrough]];

      case QEvent::Enter:
      case QEvent::Leave:
      case QEvent::FocusIn:
      case QEvent::FocusOut:
         if (dispatch) {
            if (QWidget *widget = dynamic_cast<QWidget *>(object))
               if (widget == this || isAncestorOf(widget)) {
                  return handleEvent(widget, event);
               }
         }
         break;
   }

   return false;
}

int QDesignerMenu::findAction(const QPoint &pos) const
{
   const int index = actionIndexAt(this, pos, Qt::Vertical);
   if (index == -1) {
      return realActionCount();
   }

   return index;
}

void QDesignerMenu::adjustIndicator(const QPoint &pos)
{
   if (QDesignerActionProviderExtension *a = actionProvider()) {
      a->adjustIndicator(pos);
   }
}

QDesignerMenu::ActionDragCheck QDesignerMenu::checkAction(QAction *action) const
{
   if (!action || (action->menu() && action->menu()->parentWidget() != const_cast<QDesignerMenu *>(this))) {
      return NoActionDrag;   // menu action!! nothing to do
   }

   if (!Utils::isObjectAncestorOf(formWindow()->mainContainer(), action)) {
      return NoActionDrag;   // the action belongs to another form window
   }

   if (actions().contains(action)) {
      return ActionDragOnSubMenu;   // we already have the action in the menu
   }

   return AcceptActionDrag;
}

void QDesignerMenu::dragEnterEvent(QDragEnterEvent *event)
{
   const ActionRepositoryMimeData *d = dynamic_cast<const ActionRepositoryMimeData *>(event->mimeData());
   if (!d || d->actionList().empty()) {
      event->ignore();
      return;
   }

   QAction *action = d->actionList().first();

   switch (checkAction(action)) {
      case NoActionDrag:
         event->ignore();
         break;
      case ActionDragOnSubMenu:
         d->accept(event);
         m_dragging = true;
         break;
      case AcceptActionDrag:
         d->accept(event);
         m_dragging = true;
         adjustIndicator(event->pos());
         break;
   }
}

void QDesignerMenu::dragMoveEvent(QDragMoveEvent *event)
{
   if (actionGeometry(m_addSeparator).contains(event->pos())) {
      event->ignore();
      adjustIndicator(QPoint(-1, -1));
      return;
   }

   const ActionRepositoryMimeData *d = dynamic_cast<const ActionRepositoryMimeData *>(event->mimeData());
   if (!d || d->actionList().empty()) {
      event->ignore();
      return;
   }

   QAction *action = d->actionList().first();
   const ActionDragCheck dc = checkAction(action);
   switch (dc) {
      case NoActionDrag:
         event->ignore();
         break;
      case ActionDragOnSubMenu:
      case AcceptActionDrag: { // Do not pop up submenu of action being dragged
         const int newIndex = findAction(event->pos());
         if (safeActionAt(newIndex) != action) {
            m_currentIndex = newIndex;
            if (m_lastSubMenuIndex != m_currentIndex) {
               m_showSubMenuTimer->start(300);
            }
         }
         if (dc == AcceptActionDrag) {
            adjustIndicator(event->pos());
            d->accept(event);
         } else {
            event->ignore();
         }
      }
      break;
   }
}

void QDesignerMenu::dragLeaveEvent(QDragLeaveEvent *)
{
   m_dragging = false;
   adjustIndicator(QPoint(-1, -1));
   m_showSubMenuTimer->stop();
}

void QDesignerMenu::dropEvent(QDropEvent *event)
{
   m_showSubMenuTimer->stop();
   hideSubMenu();
   m_dragging = false;

   QDesignerFormWindowInterface *fw = formWindow();
   const ActionRepositoryMimeData *d = dynamic_cast<const ActionRepositoryMimeData *>(event->mimeData());
   if (!d || d->actionList().empty()) {
      event->ignore();
      return;
   }

   QAction *action = d->actionList().first();
   if (action && checkAction(action) == AcceptActionDrag) {
      event->acceptProposedAction();
      int index = findAction(event->pos());
      index = qMin(index, actions().count() - 1);

      fw->beginCommand(tr("Insert action"));
      InsertActionIntoCommand *insertCmd = new InsertActionIntoCommand(fw);
      insertCmd->init(this, action, safeActionAt(index));
      fw->commandHistory()->push(insertCmd);

      m_currentIndex = index;

      if (parentMenu()) {
         QAction *parent_action = parentMenu()->currentAction();

         if (parent_action->menu() == nullptr) {
            CreateSubmenuCommand *createCmd = new CreateSubmenuCommand(fw);
            createCmd->init(parentMenu(), parentMenu()->currentAction(), action);
            fw->commandHistory()->push(createCmd);
         }
      }

      update();
      fw->endCommand();

   } else {
      event->ignore();
   }

   adjustIndicator(QPoint(-1, -1));
}

void QDesignerMenu::actionEvent(QActionEvent *event)
{
   QMenu::actionEvent(event);
   m_adjustSizeTimer->start(0);
}

QDesignerFormWindowInterface *QDesignerMenu::formWindow() const
{
   if (parentMenu()) {
      return parentMenu()->formWindow();
   }

   return QDesignerFormWindowInterface::findFormWindow(parentWidget());
}

QDesignerActionProviderExtension *QDesignerMenu::actionProvider()
{
   if (QDesignerFormWindowInterface *fw = formWindow()) {
      QDesignerFormEditorInterface *core = fw->core();
      return qt_extension<QDesignerActionProviderExtension *>(core->extensionManager(), this);
   }

   return nullptr;
}

void QDesignerMenu::closeMenuChain()
{
   m_showSubMenuTimer->stop();

   QWidget *w = this;
   while (w && dynamic_cast<QMenu *>(w)) {
      w = w->parentWidget();
   }

   if (w) {
      for (QMenu *subMenu : w->findChildren<QMenu *>()) {
         subMenu->hide();
      }
   }

   m_lastSubMenuIndex = -1;
}

// Close submenu using the left/right keys according to layoutDirection().
// Return false to indicate the event must be propagated to the menu bar.
bool QDesignerMenu::hideSubMenuOnCursorKey()
{
   if (parentMenu()) {
      hide();
      return true;
   }
   closeMenuChain();
   update();
   if (parentMenuBar()) {
      return false;
   }
   return true;
}

// Open a submenu using the left/right keys according to layoutDirection().
// Return false to indicate the event must be propagated to the menu bar.
bool QDesignerMenu::showSubMenuOnCursorKey()
{
   const QAction *action = currentAction();

   if (dynamic_cast<const SpecialMenuAction *>(action) || action->isSeparator()) {
      closeMenuChain();
      if (parentMenuBar()) {
         return false;
      }
      return true;
   }
   m_lastSubMenuIndex = -1; // force a refresh
   slotShowSubMenuNow();
   return true;
}

void QDesignerMenu::moveLeft()
{
   const bool handled = layoutDirection() == Qt::LeftToRight ?
      hideSubMenuOnCursorKey() : showSubMenuOnCursorKey();
   if (!handled) {
      parentMenuBar()->moveLeft();
   }
}

void QDesignerMenu::moveRight()
{
   const bool handled = layoutDirection() == Qt::LeftToRight ?
      showSubMenuOnCursorKey() : hideSubMenuOnCursorKey();
   if (!handled) {
      parentMenuBar()->moveRight();
   }
}

void QDesignerMenu::moveUp(bool ctrl)
{
   if (m_currentIndex == 0) {
      hide();
      return;
   }

   if (ctrl) {
      (void) swap(m_currentIndex, m_currentIndex - 1);
   }
   --m_currentIndex;
   m_currentIndex = qMax(0, m_currentIndex);
   // Always re-select, swapping destroys order
   update();
   selectCurrentAction();
}

void QDesignerMenu::moveDown(bool ctrl)
{
   if (m_currentIndex == actions().count() - 1) {
      return;
   }

   if (ctrl) {
      (void) swap(m_currentIndex + 1, m_currentIndex);
   }

   ++m_currentIndex;
   m_currentIndex = qMin(actions().count() - 1, m_currentIndex);
   update();
   if (!ctrl) {
      selectCurrentAction();
   }
}

QAction *QDesignerMenu::currentAction() const
{
   if (m_currentIndex < 0 || m_currentIndex >= actions().count()) {
      return nullptr;
   }

   return safeActionAt(m_currentIndex);
}

int QDesignerMenu::realActionCount() const
{
   return actions().count() - 2; // 2 fake actions
}

void QDesignerMenu::selectCurrentAction()
{
   QAction *action = currentAction();
   if (!action || action == m_addSeparator || action == m_addItem) {
      return;
   }

   QDesignerObjectInspector *oi = nullptr;
   if (QDesignerFormWindowInterface *fw = formWindow()) {
      oi = dynamic_cast<QDesignerObjectInspector *>(fw->core()->objectInspector());
   }

   if (!oi) {
      return;
   }

   oi->clearSelection();
   if (QMenu *menu = action->menu()) {
      oi->selectObject(menu);
   } else {
      oi->selectObject(action);
   }
}

void QDesignerMenu::createRealMenuAction(QAction *action)
{
   if (action->menu()) {
      return;   // nothing to do
   }

   QDesignerFormWindowInterface *fw = formWindow();
   QDesignerFormEditorInterface *core = formWindow()->core();

   QDesignerMenu *menu = findOrCreateSubMenu(action);
   m_subMenus.remove(action);

   action->setMenu(menu);
   menu->setTitle(action->text());

   Q_ASSERT(fw);

   core->widgetFactory()->initialize(menu);

   const QString niceObjectName = ActionEditor::actionTextToName(menu->title(), QString("menu"));
   menu->setObjectName(niceObjectName);

   core->metaDataBase()->add(menu);
   fw->ensureUniqueObjectName(menu);

   QAction *menuAction = menu->menuAction();
   core->metaDataBase()->add(menuAction);
}

void QDesignerMenu::removeRealMenu(QAction *action)
{
   QDesignerMenu *menu = dynamic_cast<QDesignerMenu *>(action->menu());

   if (menu == nullptr) {
      return;
   }

   action->setMenu(nullptr);
   m_subMenus.insert(action, menu);
   QDesignerFormEditorInterface *core = formWindow()->core();
   core->metaDataBase()->remove(menu);
}

QDesignerMenu *QDesignerMenu::findOrCreateSubMenu(QAction *action)
{
   if (action->menu()) {
      return dynamic_cast<QDesignerMenu *>(action->menu());
   }

   QDesignerMenu *menu = m_subMenus.value(action);
   if (! menu) {
      menu = new QDesignerMenu(this);
      m_subMenus.insert(action, menu);
   }

   return menu;
}

bool QDesignerMenu::canCreateSubMenu(QAction *action) const // ### improve it's a bit too slow
{
   for (const QWidget *aw : action->associatedWidgets())
      if (aw != this) {
         if (const QMenu *m = dynamic_cast<const QMenu *>(aw)) {
            if (m->actions().contains(action)) {
               return false;   // sorry
            }
         } else {
            if (const QToolBar *tb = dynamic_cast<const QToolBar *>(aw))
               if (tb->actions().contains(action)) {
                  return false;   // sorry
               }
         }
      }
   return true;
}

void QDesignerMenu::slotShowSubMenuNow()
{
   m_showSubMenuTimer->stop();

   if (m_lastSubMenuIndex == m_currentIndex) {
      return;
   }

   if (m_lastSubMenuIndex != -1) {
      hideSubMenu();
   }

   if (m_currentIndex >= realActionCount()) {
      return;
   }

   QAction *action = currentAction();

   if (action->isSeparator() || !canCreateSubMenu(action)) {
      return;
   }

   if (QMenu *menu = findOrCreateSubMenu(action)) {
      if (!menu->isVisible()) {
         if ((menu->windowFlags() & Qt::Popup) != Qt::Popup) {
            menu->setWindowFlags(Qt::Popup);
         }
         const QRect g = actionGeometry(action);
         if (layoutDirection() == Qt::LeftToRight) {
            menu->move(mapToGlobal(g.topRight()));
         } else {
            // The position is not initially correct due to the unknown width,
            // causing it to overlap a bit the first time it is invoked.
            QPoint point = g.topLeft() - QPoint(menu->width() + 10, 0);
            menu->move(mapToGlobal(point));
         }
         menu->show();
         menu->setFocus();
      } else {
         menu->raise();
      }
      menu->setFocus();

      m_lastSubMenuIndex = m_currentIndex;
   }
}

void QDesignerMenu::showSubMenu(QAction *action)
{
   m_showSubMenuTimer->stop();

   if (m_editor->isVisible() || !action || dynamic_cast<SpecialMenuAction *>(action)
      || action->isSeparator() || !isVisible()) {
      return;
   }

   m_showSubMenuTimer->start(300);
}

QDesignerMenu *QDesignerMenu::parentMenu() const
{
   return dynamic_cast<QDesignerMenu *>(parentWidget());
}

QDesignerMenuBar *QDesignerMenu::parentMenuBar() const
{
   if (QDesignerMenuBar *mb = dynamic_cast<QDesignerMenuBar *>(parentWidget())) {
      return mb;
   } else if (QDesignerMenu *m = parentMenu()) {
      return m->parentMenuBar();
   }

   return nullptr;
}

void QDesignerMenu::setVisible(bool visible)
{
   if (visible) {
      m_currentIndex = 0;
   } else {
      m_lastSubMenuIndex = -1;
   }

   QMenu::setVisible(visible);

}

void QDesignerMenu::adjustSpecialActions()
{
   removeAction(m_addItem);
   removeAction(m_addSeparator);
   addAction(m_addItem);
   addAction(m_addSeparator);
}

bool QDesignerMenu::interactive(bool i)
{
   const bool old = m_interactive;
   m_interactive = i;
   return old;
}

void QDesignerMenu::enterEditMode()
{
   if (m_currentIndex >= 0 && m_currentIndex <= realActionCount()) {
      showLineEdit();

   } else {
      hideSubMenu();
      QDesignerFormWindowInterface *fw = formWindow();
      fw->beginCommand(tr("Add separator"));
      QAction *sep = createAction(QString(), true);

      InsertActionIntoCommand *insertCmd = new InsertActionIntoCommand(fw);
      insertCmd->init(this, sep, safeActionAt(realActionCount()));
      fw->commandHistory()->push(insertCmd);

      if (parentMenu()) {
         QAction *parent_action = parentMenu()->currentAction();

         if (parent_action->menu() == nullptr) {
            CreateSubmenuCommand *createCmd = new CreateSubmenuCommand(fw);
            createCmd->init(parentMenu(), parentMenu()->currentAction());
            fw->commandHistory()->push(createCmd);
         }
      }

      fw->endCommand();

      m_currentIndex = actions().indexOf(m_addItem);
      update();
   }
}

void QDesignerMenu::leaveEditMode(LeaveEditMode mode)
{
   if (mode == Default) {
      return;
   }

   QAction *action = nullptr;

   QDesignerFormWindowInterface *fw = formWindow();

   if (m_currentIndex < realActionCount()) {
      action = safeActionAt(m_currentIndex);
      fw->beginCommand(QApplication::translate("Command", "Set action text"));

   } else {
      Q_ASSERT(fw != nullptr);
      fw->beginCommand(QApplication::translate("Command", "Insert action"));
      action = createAction(ActionEditor::actionTextToName(m_editor->text()));

      InsertActionIntoCommand *insertCmd = new InsertActionIntoCommand(fw);
      insertCmd->init(this, action, currentAction());
      fw->commandHistory()->push(insertCmd);
   }

   SetPropertyCommand *cmd = new SetPropertyCommand(fw);
   cmd->init(action, QString("text"), m_editor->text());
   fw->commandHistory()->push(cmd);

   if (parentMenu()) {
      QAction *parent_action = parentMenu()->currentAction();

      if (parent_action->menu() == nullptr) {
         CreateSubmenuCommand *createCmd = new CreateSubmenuCommand(fw);
         createCmd->init(parentMenu(), parentMenu()->currentAction(), action);
         fw->commandHistory()->push(createCmd);
      }
   }

   update();
   fw->endCommand();
}

QAction *QDesignerMenu::safeMenuAction(QDesignerMenu *menu) const
{
   QAction *action = menu->menuAction();

   if (!action) {
      action = m_subMenus.key(menu);
   }

   return action;
}

void QDesignerMenu::showLineEdit()
{
   m_showSubMenuTimer->stop();

   QAction *action = nullptr;

   if (m_currentIndex < realActionCount()) {
      action = safeActionAt(m_currentIndex);
   } else {
      action = m_addItem;
   }

   if (action->isSeparator()) {
      return;
   }

   hideSubMenu();

   // open edit field for item name
   setFocus();

   const QString text = action != m_addItem ? action->text() : QString();
   m_editor->setText(text);
   m_editor->selectAll();
   m_editor->setGeometry(actionGeometry(action).adjusted(1, 1, -2, -2));
   m_editor->show();
   m_editor->setFocus();
}

QAction *QDesignerMenu::createAction(const QString &objectName, bool separator)
{
   QDesignerFormWindowInterface *fw = formWindow();
   Q_ASSERT(fw);

   return ToolBarEventFilter::createAction(fw, objectName, separator);
}

// ### share with QDesignerMenu::swap
bool QDesignerMenu::swap(int a, int b)
{
   const int left = qMin(a, b);
   int right = qMax(a, b);

   QAction *action_a = safeActionAt(left);
   QAction *action_b = safeActionAt(right);

   if (action_a == action_b
      || !action_a
      || !action_b
      || dynamic_cast<SpecialMenuAction *>(action_a)
      || dynamic_cast<SpecialMenuAction *>(action_b)) {
      return false;   // nothing to do
   }

   right = qMin(right, realActionCount());
   if (right < 0) {
      return false;   // nothing to do
   }

   QDesignerFormWindowInterface *fw = formWindow();
   fw->beginCommand(QApplication::translate("Command", "Move action"));

   QAction *action_b_before = safeActionAt(right + 1);

   RemoveActionFromCommand *cmd1 = new RemoveActionFromCommand(fw);
   cmd1->init(this, action_b, action_b_before, false);
   fw->commandHistory()->push(cmd1);

   QAction *action_a_before = safeActionAt(left + 1);

   InsertActionIntoCommand *cmd2 = new InsertActionIntoCommand(fw);
   cmd2->init(this, action_b, action_a_before, false);
   fw->commandHistory()->push(cmd2);

   RemoveActionFromCommand *cmd3 = new RemoveActionFromCommand(fw);
   cmd3->init(this, action_a, action_b, false);
   fw->commandHistory()->push(cmd3);

   InsertActionIntoCommand *cmd4 = new InsertActionIntoCommand(fw);
   cmd4->init(this, action_a, action_b_before, true);
   fw->commandHistory()->push(cmd4);

   fw->endCommand();

   return true;
}

QAction *QDesignerMenu::safeActionAt(int index) const
{
   if (index < 0 || index >= actions().count()) {
      return nullptr;
   }

   return actions().at(index);
}

void QDesignerMenu::hideSubMenu()
{
   m_lastSubMenuIndex = -1;
   for (QMenu *subMenu : findChildren<QMenu *>()) {
      subMenu->hide();
   }
}

void QDesignerMenu::deleteAction()
{
   QAction *action = currentAction();
   const int pos   = actions().indexOf(action);
   QAction *action_before = nullptr;

   if (pos != -1) {
      action_before = safeActionAt(pos + 1);
   }

   QDesignerFormWindowInterface *fw = formWindow();
   RemoveActionFromCommand *cmd = new RemoveActionFromCommand(fw);

   cmd->init(this, action, action_before);
   fw->commandHistory()->push(cmd);

   update();
}

void QDesignerMenu::deactivateMenu()
{
   m_deactivateWindowTimer->start(10);
}

void QDesignerMenu::slotDeactivateNow()
{
   m_deactivateWindowTimer->stop();

   if (m_dragging) {
      return;
   }

   QDesignerMenu *root = findRootMenu();

   if (! root->findActivatedMenu()) {
      root->hide();
      root->hideSubMenu();
   }
}

void QDesignerMenu::drawSelection(QPainter *p, const QRect &r)
{
   p->save();

   QColor c = Qt::blue;
   p->setPen(QPen(c, 1));
   c.setAlpha(32);
   p->setBrush(c);
   p->drawRect(r);

   p->restore();
}

void QDesignerMenu::keyPressEvent(QKeyEvent *event)
{
   event->ignore();
}

void QDesignerMenu::keyReleaseEvent(QKeyEvent *event)
{
   event->ignore();
}


