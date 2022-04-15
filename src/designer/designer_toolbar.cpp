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

#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <abstract_propertyeditor.h>
#include <abstract_widgetfactory.h>
#include <action_editor.h>
#include <action_provider.h>
#include <action_repository.h>
#include <designer_command.h>
#include <designer_object_inspector.h>
#include <designer_toolbar.h>
#include <designer_utils.h>
#include <extension.h>
#include <promotion_taskmenu.h>

#include <QAction>
#include <QApplication>
#include <QApplication>
#include <QDebug>
#include <QDrag>
#include <QEvent>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>

CS_DECLARE_METATYPE(QAction *)

typedef QList<QAction *> ActionList;

namespace qdesigner_internal {

void ToolBarEventFilter::install(QToolBar *tb)
{
   ToolBarEventFilter *tf = new ToolBarEventFilter(tb);
   tb->installEventFilter(tf);
   tb->setAcceptDrops(true); // ### fake
}

ToolBarEventFilter::ToolBarEventFilter(QToolBar *tb)
   : QObject(tb), m_toolBar(tb), m_promotionTaskMenu(nullptr)
{
}

ToolBarEventFilter *ToolBarEventFilter::eventFilterOf(const QToolBar *tb)
{
   // Look for 1st order children only..otherwise, we might get filters of nested widgets
   const QObjectList children = tb->children();
   const QObjectList::const_iterator cend = children.constEnd();

   for (QObjectList::const_iterator it = children.constBegin(); it != cend; ++it) {
      QObject *o = *it;

      if (! o->isWidgetType())
         if (ToolBarEventFilter *ef = dynamic_cast<ToolBarEventFilter *>(o)) {
            return ef;
         }
   }

   return nullptr;
}

bool ToolBarEventFilter::eventFilter (QObject *watched, QEvent *event)
{
   if (watched != m_toolBar) {
      return QObject::eventFilter (watched, event);
   }

   switch (event->type()) {
      case QEvent::ChildAdded: {
         // Children should not interact with the mouse
         const QChildEvent *ce = static_cast<const QChildEvent *>(event);
         if (QWidget *w = dynamic_cast<QWidget *>(ce->child())) {
            w->setAttribute(Qt::WA_TransparentForMouseEvents, true);
            w->setFocusPolicy(Qt::NoFocus);
         }
      }
      break;

      case QEvent::ContextMenu:
         return handleContextMenuEvent(static_cast<QContextMenuEvent *>(event));

      case QEvent::DragEnter:
      case QEvent::DragMove:
         return handleDragEnterMoveEvent(static_cast<QDragMoveEvent *>(event));

      case QEvent::DragLeave:
         return handleDragLeaveEvent(static_cast<QDragLeaveEvent *>(event));

      case QEvent::Drop:
         return handleDropEvent(static_cast<QDropEvent *>(event));

      case QEvent::MouseButtonPress:
         return handleMousePressEvent(static_cast<QMouseEvent *>(event));

      case QEvent::MouseButtonRelease:
         return handleMouseReleaseEvent(static_cast<QMouseEvent *>(event));

      case QEvent::MouseMove:
         return handleMouseMoveEvent(static_cast<QMouseEvent *>(event));

      default:
         break;
   }

   return QObject::eventFilter (watched, event);
}

ActionList ToolBarEventFilter::contextMenuActions(const QPoint &globalPos)
{
   ActionList rc;
   const int index = actionIndexAt(m_toolBar, m_toolBar->mapFromGlobal(globalPos), m_toolBar->orientation());
   const ActionList actions = m_toolBar->actions();
   QAction *action = index != -1 ? actions.at(index) : nullptr;
   QVariant itemData;

   // Insert before
   if (action && index != 0 && !action->isSeparator()) {
      QAction *newSeperatorAct = new QAction(tr("Insert Separator before '%1'").formatArg(action->objectName()), nullptr);
      itemData.setValue(action);
      newSeperatorAct->setData(itemData);
      connect(newSeperatorAct, &QAction::triggered, this, &ToolBarEventFilter::slotInsertSeparator);
      rc.push_back(newSeperatorAct);
   }

   // Append separator
   if (actions.empty() || !actions.back()->isSeparator()) {
      QAction *newSeperatorAct = new QAction(tr("Append Separator"), nullptr);
      itemData.setValue(static_cast<QAction *>(nullptr));
      newSeperatorAct->setData(itemData);

      connect(newSeperatorAct, &QAction::triggered, this, &ToolBarEventFilter::slotInsertSeparator);

      rc.push_back(newSeperatorAct);
   }

   // Promotion
   if (!m_promotionTaskMenu) {
      m_promotionTaskMenu = new PromotionTaskMenu(m_toolBar, PromotionTaskMenu::ModeSingleWidget, this);
   }

   m_promotionTaskMenu->addActions(formWindow(), PromotionTaskMenu::LeadingSeparator | PromotionTaskMenu::TrailingSeparator, rc);

   // Remove
   if (action) {
      QAction *a = new QAction(tr("Remove action '%1'").formatArg(action->objectName()), nullptr);
      itemData.setValue(action);
      a->setData(itemData);
      connect(a, &QAction::triggered, this, &ToolBarEventFilter::slotRemoveSelectedAction);
      rc.push_back(a);
   }

   QAction *remove_toolbar = new QAction(tr("Remove Toolbar '%1'").formatArg(m_toolBar->objectName()), nullptr);
   connect(remove_toolbar, &QAction::triggered, this, &ToolBarEventFilter::slotRemoveToolBar);
   rc.push_back(remove_toolbar);
   return rc;
}

bool ToolBarEventFilter::handleContextMenuEvent(QContextMenuEvent *event )
{
   event->accept();

   const QPoint globalPos = event->globalPos();
   const ActionList al = contextMenuActions(event->globalPos());

   QMenu menu(nullptr);
   const ActionList::const_iterator acend = al.constEnd();
   for (ActionList::const_iterator it = al.constBegin(); it != acend; ++it) {
      menu.addAction(*it);
   }

   menu.exec(globalPos);

   return true;
}

void ToolBarEventFilter::slotRemoveSelectedAction()
{
   QAction *action = dynamic_cast<QAction *>(sender());
   if (!action) {
      return;
   }

   QAction *a = action->data().value<QAction *>();
   Q_ASSERT(a != nullptr);

   QDesignerFormWindowInterface *fw = formWindow();
   Q_ASSERT(fw);

   const ActionList actions = m_toolBar->actions();
   const int pos = actions.indexOf(a);
   QAction *action_before = nullptr;

   if (pos != -1 && actions.count() > pos + 1) {
      action_before = actions.at(pos + 1);
   }

   RemoveActionFromCommand *cmd = new RemoveActionFromCommand(fw);
   cmd->init(m_toolBar, a, action_before);
   fw->commandHistory()->push(cmd);
}

void ToolBarEventFilter::slotRemoveToolBar()
{
   QDesignerFormWindowInterface *fw = formWindow();
   Q_ASSERT(fw);
   DeleteToolBarCommand *cmd = new DeleteToolBarCommand(fw);
   cmd->init(m_toolBar);
   fw->commandHistory()->push(cmd);
}

void ToolBarEventFilter::slotInsertSeparator()
{
   QDesignerFormWindowInterface *fw = formWindow();
   QAction *theSender = dynamic_cast<QAction *>(sender());

   QAction *previous = theSender->data().value<QAction *>();
   fw->beginCommand(tr("Insert Separator"));

   QAction *action = createAction(fw, QString("separator"), true);
   InsertActionIntoCommand *cmd = new InsertActionIntoCommand(fw);
   cmd->init(m_toolBar, action, previous);
   fw->commandHistory()->push(cmd);
   fw->endCommand();
}

QDesignerFormWindowInterface *ToolBarEventFilter::formWindow() const
{
   return QDesignerFormWindowInterface::findFormWindow(m_toolBar);
}

QAction *ToolBarEventFilter::createAction(QDesignerFormWindowInterface *fw, const QString &objectName, bool separator)
{
   QAction *action = new QAction(fw);
   fw->core()->widgetFactory()->initialize(action);
   if (separator) {
      action->setSeparator(true);
   }

   action->setObjectName(objectName);
   fw->ensureUniqueObjectName(action);

   qdesigner_internal::AddActionCommand *cmd = new  qdesigner_internal::AddActionCommand(fw);
   cmd->init(action);
   fw->commandHistory()->push(cmd);

   return action;
}

void ToolBarEventFilter::adjustDragIndicator(const QPoint &pos)
{
   if (QDesignerFormWindowInterface *fw = formWindow()) {
      QDesignerFormEditorInterface *core = fw->core();
      if (QDesignerActionProviderExtension *a = qt_extension<QDesignerActionProviderExtension *>(core->extensionManager(), m_toolBar)) {
         a->adjustIndicator(pos);
      }
   }
}

void ToolBarEventFilter::hideDragIndicator()
{
   adjustDragIndicator(QPoint(-1, -1));
}

bool ToolBarEventFilter::handleMousePressEvent(QMouseEvent *event)
{
   if (event->button() != Qt::LeftButton || withinHandleArea(m_toolBar, event->pos())) {
      return false;
   }

   if (QDesignerFormWindowInterface *fw = formWindow()) {
      QDesignerFormEditorInterface *core = fw->core();
      // Keep selection in sync
      fw->clearSelection(false);
      if (QDesignerObjectInspector *oi = dynamic_cast<QDesignerObjectInspector *>(core->objectInspector())) {
         oi->clearSelection();
         oi->selectObject(m_toolBar);
      }
      core->propertyEditor()->setObject(m_toolBar);
   }
   m_startPosition = m_toolBar->mapFromGlobal(event->globalPos());
   event->accept();
   return true;
}

bool ToolBarEventFilter::handleMouseReleaseEvent(QMouseEvent *event)
{
   if (event->button() != Qt::LeftButton || m_startPosition.isNull() || withinHandleArea(m_toolBar, event->pos())) {
      return false;
   }

   // Accept the event, otherwise, form window selection will trigger
   m_startPosition = QPoint();
   event->accept();
   return true;
}

bool ToolBarEventFilter::handleMouseMoveEvent(QMouseEvent *event)
{
   if (m_startPosition.isNull() || withinHandleArea(m_toolBar, event->pos())) {
      return false;
   }

   const QPoint pos = m_toolBar->mapFromGlobal(event->globalPos());
   if ((pos - m_startPosition).manhattanLength() > qApp->startDragDistance()) {
      startDrag(m_startPosition, event->modifiers());
      m_startPosition = QPoint();
      event->accept();
      return true;
   }
   return false;
}

bool ToolBarEventFilter::handleDragEnterMoveEvent(QDragMoveEvent *event)
{
   const ActionRepositoryMimeData *d = dynamic_cast<const ActionRepositoryMimeData *>(event->mimeData());
   if (!d) {
      return false;
   }

   if (d->actionList().isEmpty()) {
      event->ignore();
      hideDragIndicator();
      return true;
   }

   QAction *action = d->actionList().first();
   if (!action || action->menu() || m_toolBar->actions().contains(action) ||
      !Utils::isObjectAncestorOf(formWindow()->mainContainer(), action)) {
      event->ignore();
      hideDragIndicator();
      return true;
   }

   d->accept(event);
   adjustDragIndicator(event->pos());
   return true;
}

bool ToolBarEventFilter::handleDragLeaveEvent(QDragLeaveEvent *)
{
   hideDragIndicator();
   return false;
}

bool ToolBarEventFilter::handleDropEvent(QDropEvent *event)
{
   const ActionRepositoryMimeData *d = dynamic_cast<const ActionRepositoryMimeData *>(event->mimeData());
   if (!d) {
      return false;
   }

   if (d->actionList().isEmpty()) {
      event->ignore();
      hideDragIndicator();
      return true;
   }

   QAction *action = d->actionList().first();

   const ActionList actions = m_toolBar->actions();
   if (!action || actions.contains(action)) {
      event->ignore();
      hideDragIndicator();
      return true;
   }

   // Try to find action to 'insert before'. Click on action or in free area, else ignore.
   QAction *beforeAction = nullptr;
   const QPoint pos = event->pos();
   const int index = actionIndexAt(m_toolBar, pos, m_toolBar->orientation());
   if (index != -1) {
      beforeAction = actions.at(index);
   } else {
      if (!freeArea(m_toolBar).contains(pos)) {
         event->ignore();
         hideDragIndicator();
         return true;
      }
   }

   event->acceptProposedAction();
   QDesignerFormWindowInterface *fw = formWindow();
   InsertActionIntoCommand *cmd = new InsertActionIntoCommand(fw);
   cmd->init(m_toolBar, action, beforeAction);
   fw->commandHistory()->push(cmd);
   hideDragIndicator();
   return true;
}

void ToolBarEventFilter::startDrag(const QPoint &pos, Qt::KeyboardModifiers modifiers)
{
   const int index = actionIndexAt(m_toolBar, pos, m_toolBar->orientation());

   if (index == - 1) {
      return;
   }

   const ActionList actions = m_toolBar->actions();
   QAction *action = actions.at(index);
   QDesignerFormWindowInterface *fw = formWindow();

   const Qt::DropAction dropAction = (modifiers & Qt::ControlModifier) ? Qt::CopyAction : Qt::MoveAction;
   if (dropAction == Qt::MoveAction) {
      RemoveActionFromCommand *cmd = new RemoveActionFromCommand(fw);
      const int nextIndex = index + 1;
      QAction *nextAction = nextIndex < actions.size() ? actions.at(nextIndex) : nullptr;
      cmd->init(m_toolBar, action, nextAction);
      fw->commandHistory()->push(cmd);
   }

   QDrag *drag = new QDrag(m_toolBar);
   drag->setPixmap(ActionRepositoryMimeData::actionDragPixmap( action));
   drag->setMimeData(new ActionRepositoryMimeData(action, dropAction));

   if (drag->start(dropAction) == Qt::IgnoreAction) {
      hideDragIndicator();
      if (dropAction == Qt::MoveAction) {
         const ActionList currentActions = m_toolBar->actions();
         QAction *previous = nullptr;
         if (index >= 0 && index < currentActions.size()) {
            previous = currentActions.at(index);
         }
         InsertActionIntoCommand *cmd = new InsertActionIntoCommand(fw);
         cmd->init(m_toolBar, action, previous);
         fw->commandHistory()->push(cmd);
      }
   }
}

QAction *ToolBarEventFilter::actionAt(const QToolBar *tb, const QPoint &pos)
{
   const int index = actionIndexAt(tb, pos, tb->orientation());
   if (index == -1) {
      return nullptr;
   }
   return tb->actions().at(index);
}

//that's a trick to get access to the initStyleOption which is a protected member
class FriendlyToolBar : public QToolBar
{
 public:
   friend class ToolBarEventFilter;
};

QRect ToolBarEventFilter::handleArea(const QToolBar *tb)
{
   QStyleOptionToolBar opt;
   static_cast<const FriendlyToolBar *>(tb)->initStyleOption(&opt);

   return tb->style()->subElementRect(QStyle::SE_ToolBarHandle, &opt, tb);
}

bool ToolBarEventFilter::withinHandleArea(const QToolBar *tb, const QPoint &pos)
{
   return handleArea(tb).contains(pos);
}

// Determine the free area behind the last action.
QRect ToolBarEventFilter::freeArea(const QToolBar *tb)
{
   QRect rc = QRect(QPoint(0, 0), tb->size());
   const ActionList actionList = tb->actions();
   QRect exclusionRectangle = actionList.empty() ? handleArea(tb) : tb->actionGeometry(actionList.back());
   switch (tb->orientation()) {
      case Qt::Horizontal:
         switch (tb->layoutDirection()) {
            case Qt::LayoutDirectionAuto: // Should never happen
            case Qt::LeftToRight:
               rc.setX(exclusionRectangle.right() + 1);
               break;
            case Qt::RightToLeft:
               rc.setRight(exclusionRectangle.x());
               break;
         }
         break;
      case Qt::Vertical:
         rc.setY(exclusionRectangle.bottom() + 1);
         break;
   }
   return rc;
}

}


