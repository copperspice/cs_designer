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

#ifndef _SIGNALSLOTDIALOG_H
#define _SIGNALSLOTDIALOG_H

#include <QStringList>
#include <QDialog>
#include <QStandardItemModel>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QDesignerDialogGuiInterface;
class QDesignerMemberSheet;
class QListView;
class QToolButton;
class QItemSelection;

namespace Ui {
class SignalSlotDialogClass;
}

namespace qdesigner_internal {

// Dialog data
struct SignalSlotDialogData {
   void clear();
   QStringList m_existingMethods;
   QStringList m_fakeMethods;
};

// Internal helper class: A model for signatures that allows for verifying duplicates
// (checking signals versus slots and vice versa).
class SignatureModel : public QStandardItemModel
{
   CS_OBJECT(SignatureModel)

 public:
   SignatureModel(QObject *parent = nullptr);
   bool setData (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

 public:
   CS_SIGNAL_1(Public, void checkSignature(const QString &signature, bool *ok))
   CS_SIGNAL_2(checkSignature, signature, ok)
};

// Internal helper class: Panel for editing method signatures. List view with validator,
// add and remove button
class SignaturePanel  : public QObject
{
   CS_OBJECT(SignaturePanel)

 public:
   SignaturePanel(QObject *parent, QListView *listView, QToolButton *addButton, QToolButton *removeButton, const QString &newPrefix);

   QStringList fakeMethods() const;
   void setData(const SignalSlotDialogData &d);
   int count(const QString &signature) const;

 public:
   CS_SIGNAL_1(Public, void checkSignature(const QString &signature, bool *ok))
   CS_SIGNAL_2(checkSignature, signature, ok)

 private:
   CS_SLOT_1(Private, void slotAdd())
   CS_SLOT_2(slotAdd)
   CS_SLOT_1(Private, void slotRemove())
   CS_SLOT_2(slotRemove)
   CS_SLOT_1(Private, void slotSelectionChanged(const QItemSelection &un_named_arg1, const QItemSelection &un_named_arg2))
   CS_SLOT_2(slotSelectionChanged)

 private:
   void closeEditor();

   const QString m_newPrefix;
   SignatureModel *m_model;
   QListView *m_listView;
   QToolButton *m_removeButton;
};

// Dialog for  editing signals and slots.
// Provides static convenience function
// to modify fake signals and slots. They are
// handled in 2 ways:
// 1) For the MainContainer: Fake signals and slots are stored
//    in the meta database (per-instance)
// 2) For promoted widgets: Fake signals and slots are stored
//    in the widget database (per-class)
// Arguably, we could require the MainContainer to be promoted for that, too,
// but that would require entering a header.

class SignalSlotDialog : public QDialog
{
   CS_OBJECT(SignalSlotDialog)

 public:
   enum FocusMode { FocusSlots, FocusSignals };

   explicit SignalSlotDialog(QDesignerDialogGuiInterface *dialogGui, QWidget *parent = nullptr, FocusMode m = FocusSlots);
   virtual ~SignalSlotDialog();

   DialogCode showDialog(SignalSlotDialogData &slotData, SignalSlotDialogData &signalData);

   // Edit fake methods stored in MetaDataBase (per instance, used for main containers)
   static bool editMetaDataBase(QDesignerFormWindowInterface *fw, QObject *object, QWidget *parent = nullptr, FocusMode m = FocusSlots);

   // Edit fake methods of a promoted class stored in WidgetDataBase (synthesizes a widget to obtain existing members).
   static bool editPromotedClass(QDesignerFormEditorInterface *core, const QString &promotedClassName, QWidget *parent = nullptr,
      FocusMode m = FocusSlots);
   // Edit fake methods of a promoted class stored in WidgetDataBase on a base class instance.
   static bool editPromotedClass(QDesignerFormEditorInterface *core, QObject *baseObject, QWidget *parent = nullptr,
      FocusMode m = FocusSlots);

 private:
   CS_SLOT_1(Private, void slotCheckSignature(const QString &signature, bool *ok))
   CS_SLOT_2(slotCheckSignature)

 private:
   // Edit fake methods of a promoted class stored in WidgetDataBase using an instance of the base class.
   static bool editPromotedClass(QDesignerFormEditorInterface *core, const QString &promotedClassName, QObject *baseObject,
      QWidget *parent, FocusMode m);

   const FocusMode m_focusMode;
   Ui::SignalSlotDialogClass *m_ui;
   QDesignerDialogGuiInterface *m_dialogGui;
   SignaturePanel *m_slotPanel;
   SignaturePanel *m_signalPanel;
};
}



#endif
