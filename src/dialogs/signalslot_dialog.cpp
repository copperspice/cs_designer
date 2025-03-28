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

#include <abstract_dialoggui.h>
#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <abstract_widgetfactory.h>
#include <designer_formwindow_command.h>
#include <extension.h>
#include <membersheet.h>
#include <signalslot_dialog.h>
#include <ui_signal_slot_dialog.h>

#include <iconloader_p.h>
#include <metadatabase_p.h>
#include <widgetdatabase_p.h>

#include <QApplication>
#include <QItemDelegate>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QValidator>

#include <qalgorithms.h>

// match a function signature, arguments potentially with namespace colons.
static const QString signatureRegex  = "^[\\w+_]+\\(([\\w+:]\\*?,?)*\\)$";
static const QString methodNameRegex = "^[\\w+_]+$";

static QStandardItem *createEditableItem(const QString &text)
{
   QStandardItem *rc = new QStandardItem(text);
   rc->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
   return rc;
}

static  QStandardItem *createDisabledItem(const QString &text)
{
   QStandardItem *rc = new QStandardItem(text);
   Qt::ItemFlags flags = rc->flags();
   rc->setFlags(flags & ~(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable));
   return rc;
}

static void fakeMethodsFromMetaDataBase(QDesignerFormEditorInterface *core, QObject *o, QStringList &slotList, QStringList &signalList)
{
   slotList.clear();
   signalList.clear();

   if (qdesigner_internal::MetaDataBase *metaDataBase = dynamic_cast<qdesigner_internal::MetaDataBase *>(core->metaDataBase()))
      if (const qdesigner_internal::MetaDataBaseItem *item = metaDataBase->metaDataBaseItem(o)) {
         slotList = item->fakeSlots();
         signalList = item->fakeSignals();
      }
}

static void fakeMethodsToMetaDataBase(QDesignerFormEditorInterface *core, QObject *o, const QStringList &slotList,
   const QStringList &signalList)
{
   if (qdesigner_internal::MetaDataBase *metaDataBase = dynamic_cast<qdesigner_internal::MetaDataBase *>(core->metaDataBase())) {
      qdesigner_internal::MetaDataBaseItem *item = metaDataBase->metaDataBaseItem(o);
      Q_ASSERT(item);
      item->setFakeSlots(slotList);
      item->setFakeSignals(signalList);
   }
}

static void existingMethodsFromMemberSheet(QDesignerFormEditorInterface *core,
   QObject *o,
   QStringList &slotList, QStringList &signalList)
{
   slotList.clear();
   signalList.clear();

   QDesignerMemberSheetExtension *msheet = qt_extension<QDesignerMemberSheetExtension *>(core->extensionManager(), o);
   if (!msheet) {
      return;
   }

   for (int i = 0, count = msheet->count(); i < count; ++i)
      if (msheet->isVisible(i)) {
         if (msheet->isSlot(i)) {
            slotList += msheet->signature(i);
         } else if (msheet->isSignal(i)) {
            signalList += msheet->signature(i);
         }
      }
}

namespace {
// Internal helper class: A Delegate that validates using RegExps and additionally checks
// on closing (adds missing parentheses).
class SignatureDelegate : public QItemDelegate
{
 public:
   SignatureDelegate(QObject *parent = nullptr);
   QWidget *createEditor (QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
   void setModelData (QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

 private:
   QRegularExpression m_signatureRegex;
   QRegularExpression m_methodNameRegex;
};

SignatureDelegate::SignatureDelegate(QObject *parent)
   : QItemDelegate(parent), m_signatureRegex(signatureRegex, QPatternOption::ExactMatchOption),
     m_methodNameRegex(methodNameRegex, QPatternOption::ExactMatchOption)
{
   Q_ASSERT(m_signatureRegex.isValid());
   Q_ASSERT(m_methodNameRegex.isValid());
}

QWidget *SignatureDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
   QWidget *rc = QItemDelegate::createEditor(parent, option, index);
   QLineEdit *le = dynamic_cast<QLineEdit *>(rc);
   Q_ASSERT(le);

   le->setValidator(new QRegularExpressionValidator(m_signatureRegex, le));
   return rc;
}

void SignatureDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
   QLineEdit *le = dynamic_cast<QLineEdit *>(editor);
   Q_ASSERT(le);

   // Did the user just type a name? .. Add parentheses
   QString signature = le->text();

   auto signatureMatch = m_signatureRegex.match(signature);

   if (! signatureMatch.hasMatch()) {
      auto methodNameMatch = m_methodNameRegex.match(signature);

      if (methodNameMatch.hasMatch()) {
         signature += "()";
         le->setText(signature);

      } else {
         return;
      }
   }

   QItemDelegate::setModelData(editor, model, index);
}

// ------ FakeMethodMetaDBCommand: Undo Command to change fake methods in the meta DB.
class FakeMethodMetaDBCommand : public qdesigner_internal::QDesignerFormWindowCommand
{
 public:
   explicit FakeMethodMetaDBCommand(QDesignerFormWindowInterface *formWindow);

   void init(QObject *o,
      const QStringList &oldFakeSlots, const QStringList &oldFakeSignals,
      const QStringList &newFakeSlots, const QStringList &newFakeSignals);

   void undo() override {
      fakeMethodsToMetaDataBase(core(), m_object, m_oldFakeSlots, m_oldFakeSignals);
   }

   void redo() override {
      fakeMethodsToMetaDataBase(core(), m_object, m_newFakeSlots, m_newFakeSignals);
   }

 private:
   QObject *m_object;
   QStringList m_oldFakeSlots;
   QStringList m_oldFakeSignals;
   QStringList m_newFakeSlots;
   QStringList m_newFakeSignals;
};

FakeMethodMetaDBCommand::FakeMethodMetaDBCommand(QDesignerFormWindowInterface *formWindow)
   : qdesigner_internal::QDesignerFormWindowCommand(QApplication::translate("Command", "Change signals/slots"),
     formWindow), m_object(nullptr)
{
}

void FakeMethodMetaDBCommand::init(QObject *o,
   const QStringList &oldFakeSlots, const QStringList &oldFakeSignals,
   const QStringList &newFakeSlots, const QStringList &newFakeSignals)
{
   m_object = o;
   m_oldFakeSlots   = oldFakeSlots;
   m_oldFakeSignals = oldFakeSignals;
   m_newFakeSlots   = newFakeSlots;
   m_newFakeSignals = newFakeSignals;
}
}

namespace qdesigner_internal {

void SignalSlotDialogData::clear()
{
   m_existingMethods.clear();
   m_fakeMethods.clear();
}

SignatureModel::SignatureModel(QObject *parent)
   : QStandardItemModel(parent)
{
}

bool SignatureModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   if (role != Qt::EditRole) {
      return QStandardItemModel::setData(index, value, role);
   }
   // check via signal (unless it is the same), in which case we can't be bothered.
   const QStandardItem *item = itemFromIndex(index);
   Q_ASSERT(item);

   const QString signature = value.toString();
   if (item->text() == signature) {
      return true;
   }

   bool ok = true;
   emit checkSignature(signature, &ok);
   if (!ok) {
      return false;
   }

   return QStandardItemModel::setData(index, value, role);
}

// ------ SignaturePanel
SignaturePanel::SignaturePanel(QObject *parent, QListView *listView, QToolButton *addButton, QToolButton *removeButton,
   const QString &newPrefix) :
   QObject(parent),
   m_newPrefix(newPrefix),
   m_model(new SignatureModel(this)),
   m_listView(listView),
   m_removeButton(removeButton)
{
   m_removeButton->setEnabled(false);

   connect(addButton, &QAbstractButton::clicked, this, &SignaturePanel::slotAdd);
   connect(m_removeButton, &QAbstractButton::clicked, this, &SignaturePanel::slotRemove);

   m_listView->setModel(m_model);
   SignatureDelegate *delegate = new SignatureDelegate(this);
   m_listView->setItemDelegate(delegate);
   connect(m_model, &SignatureModel::checkSignature,
      this, &SignaturePanel::checkSignature);
   connect(m_listView->selectionModel(), &QItemSelectionModel::selectionChanged,
      this, &SignaturePanel::slotSelectionChanged);
}

void SignaturePanel::slotAdd()
{
   m_listView->selectionModel()->clearSelection();
   // find unique name
   for (int i = 1; ; i++) {
      QString newSlot = m_newPrefix;
      newSlot += QString::number(i); // Always add number, Avoid setting 'slot' for first entry
      newSlot += QLatin1Char('(');
      // check for function name independent of parameters
      if (m_model->findItems(newSlot, Qt::MatchStartsWith, 0).empty()) {
         newSlot += QLatin1Char(')');
         QStandardItem *item = createEditableItem(newSlot);
         m_model->appendRow(item);
         const  QModelIndex index = m_model->indexFromItem (item);
         m_listView->setCurrentIndex (index);
         m_listView->edit(index);
         return;
      }
   }
}

int SignaturePanel::count(const QString &signature) const
{
   return m_model->findItems(signature).size();
}

void SignaturePanel::slotRemove()
{
   const QModelIndexList selectedIndexes = m_listView->selectionModel()->selectedIndexes ();
   if (selectedIndexes.empty()) {
      return;
   }

   closeEditor();
   // scroll to previous
   if (const int row = selectedIndexes.front().row()) {
      m_listView->setCurrentIndex (selectedIndexes.front().sibling(row - 1, 0));
   }

   for (int  i = selectedIndexes.size() - 1; i >= 0; i--) {
      qDeleteAll(m_model->takeRow(selectedIndexes[i].row()));
   }
}

void SignaturePanel::slotSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
   m_removeButton->setEnabled(!selected.indexes().empty());
}

void SignaturePanel::setData(const SignalSlotDialogData &d)
{
   m_model->clear();

   QStandardItem *lastExisting = nullptr;

   for (const QString &s : d.m_existingMethods) {
      lastExisting = createDisabledItem(s);
      m_model->appendRow(lastExisting);
   }

   for (const QString &s : d.m_fakeMethods) {
      m_model->appendRow(createEditableItem(s));
   }

   if (lastExisting) {
      m_listView->scrollTo(m_model->indexFromItem(lastExisting));
   }
}

QStringList SignaturePanel::fakeMethods() const
{
   QStringList rc;

   if (const int rowCount = m_model->rowCount()) {
       for (int  i = 0; i < rowCount; i++) {
         const QStandardItem *item =  m_model->item(i);

         if (item->flags() & Qt::ItemIsEditable) {
            rc += item->text();
         }
      }
   }

   return rc;
}

void SignaturePanel::closeEditor()
{
   const QModelIndex idx = m_listView->currentIndex();
   if (idx.isValid()) {
      m_listView->closePersistentEditor(idx);
   }
}

// ------ SignalSlotDialog

SignalSlotDialog::SignalSlotDialog(QDesignerDialogGuiInterface *dialogGui, QWidget *parent, FocusMode mode)
   : QDialog(parent), m_focusMode(mode), m_ui(new Ui::SignalSlotDialogClass), m_dialogGui(dialogGui)
{
   setModal(true);
   m_ui->setupUi(this);

   const QIcon plusIcon  = qdesigner_internal::createIconSet("plus.png");
   const QIcon minusIcon = qdesigner_internal::createIconSet("minus.png");

   m_ui->addSlotButton->setIcon(plusIcon);
   m_ui->removeSlotButton->setIcon(minusIcon);
   m_ui->addSignalButton->setIcon(plusIcon);
   m_ui->removeSignalButton->setIcon(minusIcon);

   m_slotPanel   = new SignaturePanel(this, m_ui->slotListView, m_ui->addSlotButton, m_ui->removeSlotButton, QString("slot"));
   m_signalPanel = new SignaturePanel(this, m_ui->signalListView, m_ui->addSignalButton, m_ui->removeSignalButton, QString("signal"));

   connect(m_slotPanel, &SignaturePanel::checkSignature,
         this, &SignalSlotDialog::slotCheckSignature);

   connect(m_signalPanel, &SignaturePanel::checkSignature,
         this, &SignalSlotDialog::slotCheckSignature);

   connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
   connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

   switch (m_focusMode) {
      case FocusSlots:
         m_ui->slotListView->setFocus(Qt::OtherFocusReason);
         break;

      case  FocusSignals:
         m_ui->signalListView->setFocus(Qt::OtherFocusReason);
         break;
   }
}

SignalSlotDialog::~SignalSlotDialog()
{
   delete m_ui;
}

void SignalSlotDialog::slotCheckSignature(const QString &signature, bool *ok)
{
   QString errorMessage;
   do {
      if (m_slotPanel->count(signature)) {
         errorMessage = tr("There is already a slot with the signature '%1'.").formatArg(signature);
         *ok = false;
         break;
      }
      if (m_signalPanel->count(signature)) {
         errorMessage = tr("There is already a signal with the signature '%1'.").formatArg(signature);
         *ok = false;
         break;
      }
   } while (false);
   if (!*ok)
      m_dialogGui->message(this, QDesignerDialogGuiInterface::SignalSlotDialogMessage,
         QMessageBox::Warning, tr("%1 - Duplicate Signature").formatArg(windowTitle()), errorMessage, QMessageBox::Close);
}

QDialog::DialogCode SignalSlotDialog::showDialog(SignalSlotDialogData &slotData, SignalSlotDialogData &signalData)
{
   m_slotPanel->setData(slotData);
   m_signalPanel->setData(signalData);

   const DialogCode rc = static_cast<DialogCode>(exec());
   if (rc == Rejected) {
      return rc;
   }

   slotData.m_fakeMethods   = m_slotPanel->fakeMethods();
   signalData.m_fakeMethods = m_signalPanel->fakeMethods();
   return rc;
}

bool SignalSlotDialog::editMetaDataBase(QDesignerFormWindowInterface *fw, QObject *object, QWidget *parent, FocusMode mode)
{
   QDesignerFormEditorInterface *core = fw->core();
   SignalSlotDialog dlg(core->dialogGui(), parent, mode);
   dlg.setWindowTitle(tr("Signals/Slots of %1").formatArg(object->objectName()));

   SignalSlotDialogData slotData;
   SignalSlotDialogData signalData;

   existingMethodsFromMemberSheet(core, object, slotData.m_existingMethods, signalData.m_existingMethods);
   fakeMethodsFromMetaDataBase(core, object, slotData.m_fakeMethods, signalData.m_fakeMethods);

   const QStringList oldSlots =  slotData.m_fakeMethods;
   const QStringList oldSignals = signalData.m_fakeMethods;

   if (dlg.showDialog(slotData, signalData) == QDialog::Rejected) {
      return false;
   }

   if (oldSlots == slotData.m_fakeMethods && oldSignals == signalData.m_fakeMethods) {
      return false;
   }

   FakeMethodMetaDBCommand *cmd = new FakeMethodMetaDBCommand(fw);
   cmd->init(object, oldSlots, oldSignals, slotData.m_fakeMethods, signalData.m_fakeMethods);
   fw->commandHistory()->push(cmd);
   return true;
}

bool SignalSlotDialog::editPromotedClass(QDesignerFormEditorInterface *core, const QString &promotedClassName, QWidget *parent,
   FocusMode mode)
{
   const int index = core->widgetDataBase()->indexOfClassName(promotedClassName);
   if (index == -1) {
      return false;
   }

   const QString baseClassName = core->widgetDataBase()->item(index)->extends();
   if (baseClassName.isEmpty()) {
      return false;
   }

   QWidget *widget = core->widgetFactory()->createWidget(baseClassName, nullptr);
   if (!widget) {
      return false;
   }
   const bool rc = editPromotedClass(core, promotedClassName, widget, parent, mode);
   widget->deleteLater();
   return rc;
}

bool SignalSlotDialog::editPromotedClass(QDesignerFormEditorInterface *core, QObject *baseObject, QWidget *parent, FocusMode mode)
{
   if (!baseObject->isWidgetType()) {
      return false;
   }

   const QString promotedClassName = promotedCustomClassName(core, dynamic_cast<QWidget *>(baseObject));
   if (promotedClassName.isEmpty()) {
      return false;
   }
   return  editPromotedClass(core, promotedClassName, baseObject, parent, mode);
}


bool SignalSlotDialog::editPromotedClass(QDesignerFormEditorInterface *core, const QString &promotedClassName, QObject *object,
   QWidget *parent, FocusMode mode)
{
   WidgetDataBase *db = dynamic_cast<WidgetDataBase *>(core->widgetDataBase());
   if (!db) {
      return false;
   }

   const int index = core->widgetDataBase()->indexOfClassName(promotedClassName);
   if (index == -1) {
      return false;
   }

   WidgetDataBaseItem *item = static_cast<WidgetDataBaseItem *>(db->item(index));

   SignalSlotDialogData slotData;
   SignalSlotDialogData signalData;

   existingMethodsFromMemberSheet(core, object, slotData.m_existingMethods, signalData.m_existingMethods);
   slotData.m_fakeMethods = item->fakeSlots();
   signalData.m_fakeMethods = item->fakeSignals();

   const QStringList oldSlots =  slotData.m_fakeMethods;
   const QStringList oldSignals = signalData.m_fakeMethods;

   SignalSlotDialog dlg(core->dialogGui(), parent, mode);
   dlg.setWindowTitle(tr("Signals/Slots of %1").formatArg(promotedClassName));

   if (dlg.showDialog(slotData, signalData) == QDialog::Rejected) {
      return false;
   }

   if (oldSlots == slotData.m_fakeMethods && oldSignals == signalData.m_fakeMethods) {
      return false;
   }

   item->setFakeSlots(slotData.m_fakeMethods);
   item->setFakeSignals(signalData.m_fakeMethods);

   return true;
}

}


