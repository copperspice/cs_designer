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

#include <abstract_formwindow.h>
#include <abstract_formeditor.h>
#include <buddyeditor.h>
#include <designer_command.h>
#include <designer_property_command.h>
#include <designer_utils.h>
#include <extension_manager.h>
#include <layout_widget.h>
#include <propertysheet.h>

#include <metadatabase_p.h>

#include <QDebug>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QApplication>

static const QString buddyPropertyC = "buddy";

static bool canBeBuddy(QWidget *w, QDesignerFormWindowInterface *form)
{
   if (dynamic_cast<const QLayoutWidget *>(w) || dynamic_cast<const QLabel *>(w)) {
      return false;
   }

   if (w == form->mainContainer() || w->isHidden() ) {
      return false;
   }

   QExtensionManager *ext = form->core()->extensionManager();

   if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(ext, w)) {
      const int index = sheet->indexOf(QString("focusPolicy"));

      if (index != -1) {
         bool ok = false;
         const Qt::FocusPolicy q = static_cast<Qt::FocusPolicy>(qdesigner_internal::Utils::valueOf(sheet->property(index), &ok));
         // Refuse No-focus unless the widget is promoted.
         return (ok && q != Qt::NoFocus) || qdesigner_internal::isPromoted(form->core(), w);
      }
   }

   return false;
}

static QString buddy(QLabel *label, QDesignerFormEditorInterface *core)
{
   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), label);
   if (sheet == nullptr) {
      return QString();
   }

   const int prop_idx = sheet->indexOf(buddyPropertyC);

   if (prop_idx == -1) {
      return QString();
   }

   return sheet->property(prop_idx).toString();
}

typedef QList<QLabel *> LabelList;

namespace qdesigner_internal {

BuddyEditor::BuddyEditor(QDesignerFormWindowInterface *form, QWidget *parent)
   : ConnectionEdit(parent, form), m_formWindow(form), m_updating(false)
{
}

QWidget *BuddyEditor::widgetAt(const QPoint &pos) const
{
   QWidget *w = ConnectionEdit::widgetAt(pos);

   while (w != nullptr && !m_formWindow->isManaged(w)) {
      w = w->parentWidget();
   }
   if (!w) {
      return w;
   }

   if (state() == Editing) {
      QLabel *label = dynamic_cast<QLabel *>(w);

      if (label == nullptr) {
         return nullptr;
      }

      const int cnt = connectionCount();
      for (int i = 0; i < cnt; ++i) {
         Connection *con = connection(i);
         if (con->widget(EndPoint::Source) == w) {
            return nullptr;
         }
      }

   } else {
      if (! canBeBuddy(w, m_formWindow)) {
         return nullptr;
      }
   }

   return w;
}

Connection *BuddyEditor::createConnection(QWidget *source, QWidget *destination)
{
   return new Connection(this, source, destination);
}

QDesignerFormWindowInterface *BuddyEditor::formWindow() const
{
   return m_formWindow;
}

void BuddyEditor::updateBackground()
{
   if (m_updating || background() == nullptr) {
      return;
   }
   ConnectionEdit::updateBackground();

   m_updating = true;
   QList<Connection *> newList;
   const LabelList label_list = background()->findChildren<QLabel *>();

   for (QLabel *label : label_list) {
      const QString buddy_name = buddy(label, m_formWindow->core());
      if (buddy_name.isEmpty()) {
         continue;
      }

      const QList<QWidget *> targets = background()->findChildren<QWidget *>(buddy_name);
      if (targets.isEmpty()) {
         continue;
      }

      QWidget *target = nullptr;

      QListIterator<QWidget *> it(targets);

      while (it.hasNext()) {
         QWidget *widget = it.next();

         if (widget && !widget->isHidden()) {
            target = widget;
            break;
         }
      }

      if (target == nullptr) {
         continue;
      }

      Connection *con = new Connection(this);
      con->setEndPoint(EndPoint::Source, label, widgetRect(label).center());
      con->setEndPoint(EndPoint::Target, target, widgetRect(target).center());
      newList.append(con);
   }

   QList<Connection *> toRemove;

   const int c = connectionCount();

   for (int i = 0; i < c; i++) {
      Connection *con = connection(i);
      QObject *source = con->object(EndPoint::Source);
      QObject *target = con->object(EndPoint::Target);

      bool found = false;
      QListIterator<Connection *> it(newList);

      while (it.hasNext()) {
         Connection *newConn = it.next();
         if (newConn->object(EndPoint::Source) == source && newConn->object(EndPoint::Target) == target) {
            found = true;
            break;
         }
      }
      if (found == false) {
         toRemove.append(con);
      }
   }
   if (!toRemove.isEmpty()) {
      DeleteConnectionsCommand command(this, toRemove);
      command.redo();
      for (Connection *con : toRemove) {
         delete takeConnection(con);
      }
   }

   QListIterator<Connection *> it(newList);

   while (it.hasNext()) {
      Connection *newConn = it.next();

      bool found = false;
      const int c = connectionCount();

      for (int i = 0; i < c; i++) {
         Connection *con = connection(i);
         if (con->object(EndPoint::Source) == newConn->object(EndPoint::Source) &&
               con->object(EndPoint::Target) == newConn->object(EndPoint::Target)) {
            found = true;
            break;
         }
      }

      if (found == false) {
         AddConnectionCommand command(this, newConn);
         command.redo();
      } else {
         delete newConn;
      }
   }

   m_updating = false;
}

void BuddyEditor::setBackground(QWidget *background)
{
   clear();
   ConnectionEdit::setBackground(background);

   const LabelList label_list = background->findChildren<QLabel *>();
   for (QLabel *label : label_list) {
      const QString buddy_name = buddy(label, m_formWindow->core());
      if (buddy_name.isEmpty()) {
         continue;
      }

      QWidget *target = background->findChild<QWidget *>(buddy_name);
      if (target == nullptr) {
         continue;
      }

      Connection *con = new Connection(this);
      con->setEndPoint(EndPoint::Source, label, widgetRect(label).center());
      con->setEndPoint(EndPoint::Target, target, widgetRect(target).center());
      addConnectionX(con);
   }
}

static QUndoCommand *createBuddyCommand(QDesignerFormWindowInterface *fw, QLabel *label, QWidget *buddy)
{
   SetPropertyCommand *command = new SetPropertyCommand(fw);
   command->init(label, buddyPropertyC, buddy->objectName());
   command->setText(BuddyEditor::tr("Add buddy"));

   return command;
}

void BuddyEditor::endConnection(QWidget *target, const QPoint &pos)
{
   Connection *tmp_con = newlyAddedConnection();
   Q_ASSERT(tmp_con != nullptr);

   tmp_con->setEndPoint(EndPoint::Target, target, pos);

   QWidget *source = tmp_con->widget(EndPoint::Source);
   Q_ASSERT(source != nullptr);
   Q_ASSERT(target != nullptr);

   setEnabled(false);
   Connection *new_con = createConnection(source, target);
   setEnabled(true);

   if (new_con != nullptr) {
      new_con->setEndPoint(EndPoint::Source, source, tmp_con->endPointPos(EndPoint::Source));
      new_con->setEndPoint(EndPoint::Target, target, tmp_con->endPointPos(EndPoint::Target));

      selectNone();
      addConnectionX(new_con);

      QLabel *source  = dynamic_cast<QLabel *>(new_con->widget(EndPoint::Source));
      QWidget *target = new_con->widget(EndPoint::Target);

      if (source) {
         undoStack()->push(createBuddyCommand(m_formWindow, source, target));
      } else {
         qDebug("BuddyEditor::endConnection(): not a label");
      }

      setSelected(new_con, true);
   }

   clearNewlyAddedConnection();
   findObjectsUnderMouse(mapFromGlobal(QCursor::pos()));
}

void BuddyEditor::widgetRemoved(QWidget *widget)
{
   QList<QWidget *> child_list = widget->findChildren<QWidget *>();
   child_list.prepend(widget);

   ConnectionSet remove_set;

   for (QWidget *w : child_list) {
      const ConnectionList &cl = connectionList();

      for (Connection *con : cl) {
         if (con->widget(EndPoint::Source) == w || con->widget(EndPoint::Target) == w) {
            remove_set.insert(con, con);
         }
      }
   }

   if (!remove_set.isEmpty()) {
      undoStack()->beginMacro(tr("Remove buddies"));

      for (Connection *con : remove_set) {
         setSelected(con, false);
         con->update();
         QWidget *source = con->widget(EndPoint::Source);

         if (dynamic_cast<QLabel *>(source) == nullptr) {
            qDebug("BuddyConnection::widgetRemoved(): not a label");
         } else {
            ResetPropertyCommand *command = new ResetPropertyCommand(formWindow());
            command->init(source, buddyPropertyC);
            undoStack()->push(command);
         }
         delete takeConnection(con);
      }

      undoStack()->endMacro();
   }
}

void BuddyEditor::deleteSelected()
{
   const ConnectionSet selectedConnections = selection(); // want copy for unselect
   if (selectedConnections.isEmpty()) {
      return;
   }

   undoStack()->beginMacro(tr("Remove %n buddies", nullptr, selectedConnections.size()));
   for (Connection *con : selectedConnections) {
      setSelected(con, false);
      con->update();

      QWidget *source = con->widget(EndPoint::Source);

      if (dynamic_cast<QLabel *>(source) == nullptr) {
         qDebug("BuddyConnection::deleteSelected(): not a label");
      } else {
         ResetPropertyCommand *command = new ResetPropertyCommand(formWindow());
         command->init(source, buddyPropertyC);
         undoStack()->push(command);
      }
      delete takeConnection(con);
   }

   undoStack()->endMacro();
}

void BuddyEditor::autoBuddy()
{
   // Any labels?
   LabelList labelList = background()->findChildren<QLabel *>();
   if (labelList.empty()) {
      return;
   }
   // Find already used buddies
   QWidgetList usedBuddies;
   const ConnectionList &beforeConnections = connectionList();

   for (const Connection *c : beforeConnections) {
      usedBuddies.push_back(c->widget(EndPoint::Target));
   }

   // Find potential new buddies, keep lists in sync
   QWidgetList buddies;

   for (LabelList::iterator it = labelList.begin(); it != labelList.end(); ) {
      QLabel *label     = *it;
      QWidget *newBuddy = nullptr;

      if (m_formWindow->isManaged(label)) {
         const QString buddy_name = buddy(label, m_formWindow->core());
         if (buddy_name.isEmpty()) {
            newBuddy = findBuddy(label, usedBuddies);
         }
      }

      if (newBuddy) {
         buddies.push_back(newBuddy);
         usedBuddies.push_back(newBuddy);
         ++it;
      } else {
         it = labelList.erase(it);
      }
   }

   // Add the list in one go.
   if (labelList.empty()) {
      return;
   }

   const int count = labelList.size();
   Q_ASSERT(count == buddies.size());

   undoStack()->beginMacro(tr("Add %n buddies", nullptr, count));
   for (int i = 0; i < count; i++) {
      undoStack()->push(createBuddyCommand(m_formWindow, labelList.at(i), buddies.at(i)));
   }

   undoStack()->endMacro();

   // Now select all new ones
   const ConnectionList &connections = connectionList();

   for (Connection *con : connections) {
      setSelected(con, buddies.contains(con->widget(EndPoint::Target)));
   }
}

// Geometrically find  a potential buddy for label by checking neighbouring children of parent
QWidget *BuddyEditor::findBuddy(QLabel *l, const QWidgetList &existingBuddies) const
{
   constexpr const int DELTA = 5;

   const QWidget *parent = l->parentWidget();

   // Try to find next managed neighbour on horizontal line
   const QRect geom = l->geometry();
   const int y = geom.center().y();

   QWidget *neighbour = nullptr;

   switch (l->layoutDirection()) {
      case Qt::LayoutDirectionAuto:
      case Qt::LeftToRight: {
         // Walk right to find next managed neighbour
         const int xEnd = parent->size().width();

         for (int x = geom.right() + 1; x < xEnd; x += DELTA) {
            if (QWidget *c = parent->childAt (x, y)) {
               if (m_formWindow->isManaged(c)) {
                  neighbour = c;
                  break;
               }
            }
         }
      }
      break;

      case Qt::RightToLeft:
         // Walk left to find next managed neighbor

         for (int x = geom.x() - 1; x >= 0; x -= DELTA) {
            if (QWidget *c = parent->childAt (x, y)) {
               if (m_formWindow->isManaged(c)) {
                  neighbour = c;
                  break;
               }
            }
         }
         break;
   }

   if (neighbour && !existingBuddies.contains(neighbour) && canBeBuddy(neighbour, m_formWindow)) {
      return neighbour;
   }

   return nullptr;
}

void BuddyEditor::createContextMenu(QMenu &menu)
{
   QAction *autoAction = menu.addAction(tr("Set automatically"));
   connect(autoAction, &QAction::triggered, this, &BuddyEditor::autoBuddy);
   menu.addSeparator();
   ConnectionEdit::createContextMenu(menu);
}

}
