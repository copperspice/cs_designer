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
#include <abstract_formwindow.h>
#include <designer_formwindow_command.h>
#include <signalsloteditor.h>
#include <ui4.h>

#include <connectdialog_p.h>
#include <metadatabase_p.h>
#include <metadatabase_p.h>
#include <signalslot_utils_p.h>
#include <signalsloteditor_p.h>

#include <QApplication>
#include <QCoreApplication>
#include <QMenu>
#include <QUndoCommand>

namespace qdesigner_internal {

SignalSlotConnection::SignalSlotConnection(ConnectionEdit *edit, QWidget *source, QWidget *target)
   : Connection(edit, source, target)
{
}

DomConnection *SignalSlotConnection::toUi() const
{
   DomConnection *result = new DomConnection;

   result->setElementSender(sender());
   result->setElementSignal(signal());
   result->setElementReceiver(receiver());
   result->setElementSlot(slot());

   DomConnectionHints *hints = new DomConnectionHints;
   QList<DomConnectionHint *> list;

   QPoint sp = endPointPos(EndPoint::Source);
   QPoint tp = endPointPos(EndPoint::Target);

   DomConnectionHint *hint = new DomConnectionHint;
   hint->setAttributeType("sourcelabel");
   hint->setElementX(sp.x());
   hint->setElementY(sp.y());
   list.append(hint);

   hint = new DomConnectionHint;
   hint->setAttributeType("destinationlabel");
   hint->setElementX(tp.x());
   hint->setElementY(tp.y());
   list.append(hint);

   hints->setElementHint(list);
   result->setElementHints(hints);

   return result;
}

void SignalSlotConnection::setSignal(const QString &signal)
{
   m_signal = signal;
   setLabel(EndPoint::Source, m_signal);
}

void SignalSlotConnection::setSlot(const QString &slot)
{
   m_slot = slot;
   setLabel(EndPoint::Target, m_slot);
}

QString SignalSlotConnection::sender() const
{
   QObject *source = object(EndPoint::Source);
   if (!source) {
      return QString();
   }

   SignalSlotEditor *edit = dynamic_cast<SignalSlotEditor *>(this->edit());
   Q_ASSERT(edit != nullptr);

   return realObjectName(edit->formWindow()->core(), source);
}

QString SignalSlotConnection::receiver() const
{
   QObject *sink = object(EndPoint::Target);

   if (!sink) {
      return QString();
   }

   SignalSlotEditor *edit = dynamic_cast<SignalSlotEditor *>(this->edit());
   Q_ASSERT(edit != nullptr);

   return realObjectName(edit->formWindow()->core(), sink);
}

void SignalSlotConnection::updateVisibility()
{
   Connection::updateVisibility();
   if (isVisible() && (signal().isEmpty() || slot().isEmpty())) {
      setVisible(false);
   }
}

QString SignalSlotConnection::toString() const
{
   return QCoreApplication::translate("SignalSlotConnection", "SENDER(%1), SIGNAL(%2), RECEIVER(%3), SLOT(%4)")
      .formatArg(sender()).formatArg(signal()).formatArg(receiver()).formatArg(slot());
}

SignalSlotConnection::State SignalSlotConnection::isValid(const QWidget *background) const
{
   const QObject *source = object(EndPoint::Source);
   if (!source) {
      return ObjectDeleted;
   }

   const QObject *target = object(EndPoint::Target);
   if (!target) {
      return ObjectDeleted;
   }

   if (m_slot.isEmpty() || m_signal.isEmpty()) {
      return InvalidMethod;
   }

   if (const QWidget *sourceWidget = dynamic_cast<const QWidget *>(source))
      if (!background->isAncestorOf(sourceWidget)) {
         return NotAncestor;
      }

   if (const QWidget *targetWidget = dynamic_cast<const QWidget *>(target))
      if (!background->isAncestorOf(targetWidget)) {
         return NotAncestor;
      }

   return Valid;
}

/*******************************************************************************
** Commands
*/

class SetMemberCommand : public QUndoCommand, public CETypes
{
 public:
   SetMemberCommand(SignalSlotConnection *con, EndPoint::Type type,
      const QString &member, SignalSlotEditor *editor);

   void redo() override;
   void undo() override;

 private:
   const QString m_old_member;
   const QString m_new_member;
   const EndPoint::Type m_type;
   SignalSlotConnection *m_con;
   SignalSlotEditor *m_editor;
};

SetMemberCommand::SetMemberCommand(SignalSlotConnection *con, EndPoint::Type type,
   const QString &member, SignalSlotEditor *editor) :
   m_old_member(type == EndPoint::Source ? con->signal() : con->slot()),
   m_new_member(member),
   m_type(type),
   m_con(con),
   m_editor(editor)
{
   if (type == EndPoint::Source) {
      setText(QApplication::translate("Command", "Change signal"));
   } else {
      setText(QApplication::translate("Command", "Change slot"));
   }
}

void SetMemberCommand::redo()
{
   m_con->update();
   if (m_type == EndPoint::Source) {
      m_con->setSignal(m_new_member);
   } else {
      m_con->setSlot(m_new_member);
   }
   m_con->update();
   emit m_editor->connectionChanged(m_con);
}

void SetMemberCommand::undo()
{
   m_con->update();
   if (m_type == EndPoint::Source) {
      m_con->setSignal(m_old_member);
   } else {
      m_con->setSlot(m_old_member);
   }
   m_con->update();
   emit m_editor->connectionChanged(m_con);
}

// Command to modify a connection
class ModifyConnectionCommand : public QDesignerFormWindowCommand
{
 public:
   explicit ModifyConnectionCommand(QDesignerFormWindowInterface *form,
      SignalSlotConnection *conn, const QString &newSignal, const QString &newSlot);

   void redo() override;
   void undo() override;

 private:
   SignalSlotConnection *m_conn;
   const QString m_oldSignal;
   const QString m_oldSlot;
   const QString m_newSignal;
   const QString m_newSlot;
};

ModifyConnectionCommand::ModifyConnectionCommand(QDesignerFormWindowInterface *form,
      SignalSlotConnection *conn, const QString &newSignal, const QString &newSlot)
   : QDesignerFormWindowCommand(QCoreApplication::translate("Command", "Change signal-slot connection"), form),
     m_conn(conn), m_oldSignal(conn->signal()), m_oldSlot(conn->slot()), m_newSignal(newSignal), m_newSlot(newSlot)
{
}

void ModifyConnectionCommand::redo()
{
   m_conn->setSignal(m_newSignal);
   m_conn->setSlot(m_newSlot);
}

void ModifyConnectionCommand::undo()
{
   m_conn->setSignal(m_oldSignal);
   m_conn->setSlot(m_oldSlot);
}

SignalSlotEditor::SignalSlotEditor(QDesignerFormWindowInterface *form_window, QWidget *parent) :
   ConnectionEdit(parent, form_window),
   m_form_window(form_window),
   m_showAllSignalsSlots(false)
{
}

void SignalSlotEditor::modifyConnection(Connection *con)
{
   SignalSlotConnection *sigslot_con = static_cast<SignalSlotConnection *>(con);
   ConnectDialog dialog(m_form_window, sigslot_con->widget(EndPoint::Source),
         sigslot_con->widget(EndPoint::Target), m_form_window->core()->topLevel());

   dialog.setSignalSlot(sigslot_con->signal(), sigslot_con->slot());
   dialog.setShowAllSignalsSlots(m_showAllSignalsSlots);

   if (dialog.exec() == QDialog::Accepted) {
      const QString newSignal = dialog.signal();
      const QString newSlot = dialog.slot();

      if (sigslot_con->signal() != newSignal || sigslot_con->slot() != newSlot) {
         ModifyConnectionCommand *cmd = new ModifyConnectionCommand(m_form_window, sigslot_con, newSignal, newSlot);
         m_form_window->commandHistory()->push(cmd);
      }
   }

   m_showAllSignalsSlots = dialog.showAllSignalsSlots();
}

Connection *SignalSlotEditor::createConnection(QWidget *source, QWidget *destination)
{
   SignalSlotConnection *con = nullptr;

   Q_ASSERT(source != nullptr);
   Q_ASSERT(destination != nullptr);

   ConnectDialog dialog(m_form_window, source, destination, m_form_window->core()->topLevel());
   dialog.setShowAllSignalsSlots(m_showAllSignalsSlots);

   if (dialog.exec() == QDialog::Accepted) {
      con = new SignalSlotConnection(this, source, destination);
      con->setSignal(dialog.signal());
      con->setSlot(dialog.slot());
   }

   m_showAllSignalsSlots = dialog.showAllSignalsSlots();

   return con;
}

DomConnections *SignalSlotEditor::toUi() const
{
   DomConnections *result = new DomConnections;
   QList<DomConnection *> list;

   const int count = connectionCount();
   for (int i = 0; i < count; ++i) {
      const SignalSlotConnection *con = static_cast<const SignalSlotConnection *>(connection(i));
      Q_ASSERT(con != nullptr);

      // If a widget's parent has been removed or moved to a different form,
      // and the parent was not a managed widget
      // (a page in a tab widget), we never get a widgetRemoved(). So we filter out
      // these child widgets here (check QPointer and verify ancestor).
      // Also, the user might demote a promoted widget or remove a fake
      // slot in the editor, which causes the connection to become invalid
      // once he doubleclicks on the method combo.
      switch (con->isValid(background())) {
         case SignalSlotConnection::Valid:
            list.append(con->toUi());
            break;
         case SignalSlotConnection::ObjectDeleted:
         case SignalSlotConnection::InvalidMethod:
         case SignalSlotConnection::NotAncestor:
            break;
      }
   }

   result->setElementConnection(list);

   return result;
}

QObject *SignalSlotEditor::objectByName(QWidget *topLevel, const QString &name) const
{
   if (name.isEmpty()) {
      return nullptr;
   }

   Q_ASSERT(topLevel);

   QObject *object = nullptr;

   if (topLevel->objectName() == name) {
      object = topLevel;
   } else {
      object = topLevel->findChild<QObject *>(name);
   }

   const QDesignerMetaDataBaseInterface *mdb = formWindow()->core()->metaDataBase();
   if (mdb->item(object)) {
      return object;
   }

   return nullptr;
}

void SignalSlotEditor::fromUi(const DomConnections *connections, QWidget *parent)
{
   if (connections == nullptr) {
      return;
   }

   setBackground(parent);
   clear();

   for (const DomConnection *dom_con : connections->elementConnection()) {
      QObject *source = objectByName(parent, dom_con->elementSender());

      if (source == nullptr) {
         qWarning("SignalSlotEditor::fromUi() No source widget called %s",
               csPrintable(dom_con->elementSender()));

         continue;
      }

      QObject *destination = objectByName(parent, dom_con->elementReceiver());

      if (destination == nullptr) {
         qWarning("SignalSlotEditor::fromUi() No destination widget called %s",
               csPrintable(dom_con->elementReceiver()));
         continue;
      }

      QPoint sp = QPoint(20, 20), tp = QPoint(20, 20);
      const DomConnectionHints *dom_hints = dom_con->elementHints();

      if (dom_hints != nullptr) {

         for (DomConnectionHint *hint : dom_hints->elementHint()) {
            QString attr_type = hint->attributeType();
            QPoint p = QPoint(hint->elementX(), hint->elementY());

            if (attr_type == "sourcelabel") {
               sp = p;

            } else if (attr_type == "destinationlabel") {
               tp = p;
            }
         }
      }

      SignalSlotConnection *con = new SignalSlotConnection(this);

      con->setEndPoint(EndPoint::Source, source, sp);
      con->setEndPoint(EndPoint::Target, destination, tp);
      con->setSignal(dom_con->elementSignal());
      con->setSlot(dom_con->elementSlot());

      addConnectionX(con);
   }
}

static bool skipWidget(const QWidget *w)
{
   const QString name = w->metaObject()->className();

   if (name == "QDesignerWidget") {
      return true;
   }

   if (name == "QLayoutWidget") {
      return true;
   }

   if (name == "qdesigner_internal::FormWindow") {
      return true;
   }

   if (name == "Spacer") {
      return true;
   }

   return false;
}

QWidget *SignalSlotEditor::widgetAt(const QPoint &pos) const
{
   QWidget *widget = ConnectionEdit::widgetAt(pos);

   if (widget == m_form_window->mainContainer()) {
      return widget;
   }

   for (; widget != nullptr; widget = widget->parentWidget()) {
      QDesignerMetaDataBaseItemInterface *item = m_form_window->core()->metaDataBase()->item(widget);

      if (item == nullptr) {
         continue;
      }

      if (skipWidget(widget)) {
         continue;
      }

      break;
   }

   return widget;
}

void SignalSlotEditor::setSignal(SignalSlotConnection *con, const QString &member)
{
   if (member == con->signal()) {
      return;
   }

   m_form_window->beginCommand(QApplication::translate("Command", "Change signal"));
   undoStack()->push(new SetMemberCommand(con, EndPoint::Source, member, this));

   if (!signalMatchesSlot(m_form_window->core(), member, con->slot())) {
      undoStack()->push(new SetMemberCommand(con, EndPoint::Target, QString(), this));
   }

   m_form_window->endCommand();
}

void SignalSlotEditor::setSlot(SignalSlotConnection *con, const QString &member)
{
   if (member == con->slot()) {
      return;
   }

   m_form_window->beginCommand(QApplication::translate("Command", "Change slot"));
   undoStack()->push(new SetMemberCommand(con, EndPoint::Target, member, this));
   if (!signalMatchesSlot(m_form_window->core(), con->signal(), member)) {
      undoStack()->push(new SetMemberCommand(con, EndPoint::Source, QString(), this));
   }
   m_form_window->endCommand();
}

void SignalSlotEditor::setSource(Connection *_con, const QString &obj_name)
{
   SignalSlotConnection *con = static_cast<SignalSlotConnection *>(_con);

   if (con->sender() == obj_name) {
      return;
   }

   m_form_window->beginCommand(QApplication::translate("Command", "Change sender"));
   ConnectionEdit::setSource(con, obj_name);

   QObject *sourceObject = con->object(EndPoint::Source);

   if (!memberFunctionListContains(m_form_window->core(), sourceObject, SignalMember, con->signal())) {
      undoStack()->push(new SetMemberCommand(con, EndPoint::Source, QString(), this));
   }

   m_form_window->endCommand();
}

void SignalSlotEditor::setTarget(Connection *_con, const QString &obj_name)
{
   SignalSlotConnection *con = static_cast<SignalSlotConnection *>(_con);

   if (con->receiver() == obj_name) {
      return;
   }

   m_form_window->beginCommand(QApplication::translate("Command", "Change receiver"));
   ConnectionEdit::setTarget(con, obj_name);

   QObject *targetObject = con->object(EndPoint::Target);
   if (!memberFunctionListContains(m_form_window->core(),  targetObject, SlotMember, con->slot())) {
      undoStack()->push(new SetMemberCommand(con, EndPoint::Target, QString(), this));
   }

   m_form_window->endCommand();
}

void SignalSlotEditor::addEmptyConnection()
{
   SignalSlotConnection *con = new SignalSlotConnection(this);
   undoStack()->push(new AddConnectionCommand(this, con));
}

} // namespace qdesigner_internal


