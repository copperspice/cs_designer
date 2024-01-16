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

#ifndef PROMOTIONEDITORDIALOG_H
#define PROMOTIONEDITORDIALOG_H

#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <abstract_promotioninterface.h>
#include <abstract_widgetdatabase.h>

#include <QDialog>
#include <QGroupBox>
#include <QItemSelection>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QPushButton;
class QTreeView;

namespace qdesigner_internal {
struct PromotionParameters {
   QString m_baseClass;
   QString m_className;
   QString m_includeFile;
};

class PromotionModel;

// Panel for adding a new promoted class. Separate class for code cleanliness.
class NewPromotedClassPanel : public QGroupBox
{
   CS_OBJECT(NewPromotedClassPanel)

 public:
   explicit NewPromotedClassPanel(const QStringList &baseClasses, int selectedBaseClass = -1, QWidget *parent = nullptr);

   QString promotedHeaderSuffix() const           {
      return m_promotedHeaderSuffix;
   }
   void setPromotedHeaderSuffix(const QString &s) {
      m_promotedHeaderSuffix = s;
   }

   bool isPromotedHeaderLowerCase() const    {
      return m_promotedHeaderLowerCase;
   }
   void setPromotedHeaderLowerCase(bool l) {
      m_promotedHeaderLowerCase = l;
   }

   CS_SIGNAL_1(Public, void newPromotedClass(const PromotionParameters &un_named_arg1, bool *ok))
   CS_SIGNAL_2(newPromotedClass, un_named_arg1, ok)

   CS_SLOT_1(Public, void grabFocus())
   CS_SLOT_2(grabFocus)

   CS_SLOT_1(Public, void chooseBaseClass(const QString &un_named_arg1))
   CS_SLOT_2(chooseBaseClass)

 private:
   CS_SLOT_1(Private, void slotNameChanged(const QString &un_named_arg1))
   CS_SLOT_2(slotNameChanged)
   CS_SLOT_1(Private, void slotIncludeFileChanged(const QString &un_named_arg1))
   CS_SLOT_2(slotIncludeFileChanged)
   CS_SLOT_1(Private, void slotAdd())
   CS_SLOT_2(slotAdd)
   CS_SLOT_1(Private, void slotReset())
   CS_SLOT_2(slotReset)

   PromotionParameters promotionParameters() const;
   void enableButtons();

   QString m_promotedHeaderSuffix;
   bool m_promotedHeaderLowerCase;

   QComboBox *m_baseClassCombo;
   QLineEdit *m_classNameEdit;
   QLineEdit *m_includeFileEdit;
   QCheckBox *m_globalIncludeCheckBox;
   QPushButton *m_addButton;
};

// Dialog for editing promoted classes.
class QDesignerPromotionDialog : public QDialog
{
   CS_OBJECT(QDesignerPromotionDialog)

 public:
   enum Mode { ModeEdit, ModeEditChooseClass };

   explicit QDesignerPromotionDialog(QDesignerFormEditorInterface *core,
      QWidget *parent = nullptr, const QString &promotableWidgetClassName = QString(), QString *promoteTo = nullptr);

   // Return an alphabetically ordered list of base class names for adding new classes.
   static const QStringList &baseClassNames(const QDesignerPromotionInterface *promotion);

   CS_SIGNAL_1(Public, void selectedBaseClassChanged(const QString &un_named_arg1))
   CS_SIGNAL_2(selectedBaseClassChanged, un_named_arg1)

 private:
   CS_SLOT_1(Private, void slotRemove())
   CS_SLOT_2(slotRemove)

   CS_SLOT_1(Private, void slotAcceptPromoteTo())
   CS_SLOT_2(slotAcceptPromoteTo)

   CS_SLOT_1(Private, void slotSelectionChanged(const QItemSelection &un_named_arg1, const QItemSelection &un_named_arg2))
   CS_SLOT_2(slotSelectionChanged)

   CS_SLOT_1(Private, void slotNewPromotedClass(const PromotionParameters &un_named_arg1, bool *ok))
   CS_SLOT_2(slotNewPromotedClass)

   CS_SLOT_1(Private, void slotIncludeFileChanged(QDesignerWidgetDataBaseItemInterface *un_named_arg1, const QString &includeFile))
   CS_SLOT_2(slotIncludeFileChanged)

   CS_SLOT_1(Private, void slotClassNameChanged(QDesignerWidgetDataBaseItemInterface *un_named_arg1, const QString &newName))
   CS_SLOT_2(slotClassNameChanged)

   CS_SLOT_1(Private, void slotUpdateFromWidgetDatabase())
   CS_SLOT_2(slotUpdateFromWidgetDatabase)

   CS_SLOT_1(Private, void slotTreeViewContextMenu(const QPoint &un_named_arg1))
   CS_SLOT_2(slotTreeViewContextMenu)

   CS_SLOT_1(Private, void slotEditSignalsSlots())
   CS_SLOT_2(slotEditSignalsSlots)

   QDialogButtonBox *createButtonBox();
   void delayedUpdateFromWidgetDatabase();

   QDesignerWidgetDataBaseItemInterface *databaseItemAt(const QItemSelection &, unsigned &flags) const;
   void displayError(const QString &message);

   const Mode m_mode;
   const QString m_promotableWidgetClassName;
   QDesignerFormEditorInterface *m_core;
   QString *m_promoteTo;
   QDesignerPromotionInterface *m_promotion;
   PromotionModel *m_model;
   QTreeView *m_treeView;
   QDialogButtonBox *m_buttonBox;
   QPushButton *m_removeButton;
   QString m_lastSelectedBaseClass;
};

}   // end namespace qdesigner_internal

#endif
