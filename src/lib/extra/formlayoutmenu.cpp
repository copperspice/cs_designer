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
#include <abstract_language.h>
#include <abstract_widgetdatabase.h>
#include <abstract_widgetfactory.h>
#include <designer_command.h>
#include <designer_property_command.h>
#include <designer_propertysheet.h>
#include <designer_utils.h>
#include <extension.h>
#include <layout_info.h>
#include <ui_formlayout_addrow.h>

#include <formlayoutmenu_p.h>

#include <QAction>
#include <QCoreApplication>
#include <QDialog>
#include <QFormLayout>
#include <QMultiHash>
#include <QPair>
#include <QPushButton>
#include <QRegularExpression>
#include <QUndoStack>
#include <QValidator>
#include <QWidget>

static const QString buddyPropertyC = "buddy";

static const QStringList fieldWidgetList = {
   "QLineEdit",
   "QComboBox",
   "QSpinBox",
   "QDoubleSpinBox",
   "QCheckBox",
   "QDateEdit",
   "QTimeEdit",
   "QDateTimeEdit",
   "QDial",
   "QWidget"
};

namespace qdesigner_internal {

// Struct that describes a row of controls (descriptive label and control) to
// be added to a form layout.

struct FormLayoutRow {
   FormLayoutRow()
      : buddy(false) {}

   QString labelName;
   QString labelText;
   QString fieldClassName;
   QString fieldName;
   bool buddy;
};

// A Dialog to edit a FormLayoutRow. Lets the user input a label text, label
// name, field widget type, field object name and buddy setting. As the
// user types the label text; the object names to be used for label and field
// are updated. It also checks the buddy setting depending on whether  the
// label text contains a buddy marker.
class FormLayoutRowDialog : public QDialog
{
   Q_DISABLE_COPY(FormLayoutRowDialog)
   CS_OBJECT(FormLayoutRowDialog)
 public:
   explicit FormLayoutRowDialog(QDesignerFormEditorInterface *core,
      QWidget *parent);

   FormLayoutRow formLayoutRow() const;

   bool buddy() const;
   void setBuddy(bool);

   // Accessors for form layout row numbers using 0..[n-1] convention
   int row() const;
   void setRow(int);
   void setRowRange(int, int);

   QString fieldClass() const;
   QString labelText() const;

   static QStringList fieldWidgetClasses(QDesignerFormEditorInterface *core);

 private:
   CS_SLOT_1(Private, void labelTextEdited(const QString &text))
   CS_SLOT_2(labelTextEdited)
   CS_SLOT_1(Private, void labelNameEdited(const QString &text))
   CS_SLOT_2(labelNameEdited)
   CS_SLOT_1(Private, void fieldNameEdited(const QString &text))
   CS_SLOT_2(fieldNameEdited)
   CS_SLOT_1(Private, void buddyClicked())
   CS_SLOT_2(buddyClicked)
   CS_SLOT_1(Private, void fieldClassChanged(int un_named_arg1))
   CS_SLOT_2(fieldClassChanged)

   bool isValid() const;
   void updateObjectNames(bool updateLabel, bool updateField);
   void updateOkButton();

   // Check for buddy marker in string
   QRegularExpression m_buddyMarkerRegex;

   Ui::FormLayoutRowDialog m_ui;
   bool m_labelNameEdited;
   bool m_fieldNameEdited;
   bool m_buddyClicked;
};

FormLayoutRowDialog::FormLayoutRowDialog(QDesignerFormEditorInterface *core, QWidget *parent)
   : QDialog(parent), m_buddyMarkerRegex("\\&[^&]"),
     m_labelNameEdited(false), m_fieldNameEdited(false), m_buddyClicked(false)
{
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
   setModal(true);
   m_ui.setupUi(this);

   connect(m_ui.labelTextLineEdit, &QLineEdit::textEdited, this, &FormLayoutRowDialog::labelTextEdited);

   static const QRegularExpression regex("^[a-zA-Z0-9_]+$");

   QRegularExpressionValidator *nameValidator = new QRegularExpressionValidator(regex, this);

   m_ui.labelNameLineEdit->setValidator(nameValidator);
   connect(m_ui.labelNameLineEdit, &QLineEdit::textEdited, this, &FormLayoutRowDialog::labelNameEdited);

   m_ui.fieldNameLineEdit->setValidator(nameValidator);
   connect(m_ui.fieldNameLineEdit, &QLineEdit::textEdited, this, &FormLayoutRowDialog::fieldNameEdited);

   connect(m_ui.buddyCheckBox, &QAbstractButton::clicked, this, &FormLayoutRowDialog::buddyClicked);

   m_ui.fieldClassComboBox->addItems(fieldWidgetClasses(core));
   m_ui.fieldClassComboBox->setCurrentIndex(0);

   connect(m_ui.fieldClassComboBox, cs_mp_cast<int>(&QComboBox::currentIndexChanged),
      this, &FormLayoutRowDialog::fieldClassChanged);

   updateOkButton();
}

FormLayoutRow FormLayoutRowDialog::formLayoutRow() const
{
   FormLayoutRow rc;
   rc.labelText = labelText();
   rc.labelName = m_ui.labelNameLineEdit->text();
   rc.fieldClassName = fieldClass();
   rc.fieldName = m_ui.fieldNameLineEdit->text();
   rc.buddy = buddy();

   return rc;
}

bool FormLayoutRowDialog::buddy() const
{
   return m_ui.buddyCheckBox->checkState() == Qt::Checked;
}

void FormLayoutRowDialog::setBuddy(bool b)
{
   m_ui.buddyCheckBox->setCheckState(b ? Qt::Checked : Qt::Unchecked);
}

// Convert rows to 1..n convention for users
int FormLayoutRowDialog::row() const
{
   return m_ui.rowSpinBox->value()  - 1;
}

void FormLayoutRowDialog::setRow(int row)
{
   m_ui.rowSpinBox->setValue(row + 1);
}

void FormLayoutRowDialog::setRowRange(int from, int to)
{
   m_ui.rowSpinBox->setMinimum(from + 1);
   m_ui.rowSpinBox->setMaximum(to + 1);
   m_ui.rowSpinBox->setEnabled(to - from > 0);
}

QString FormLayoutRowDialog::fieldClass() const
{
   return m_ui.fieldClassComboBox->itemText(m_ui.fieldClassComboBox->currentIndex());
}

QString FormLayoutRowDialog::labelText() const
{
   return m_ui.labelTextLineEdit->text();
}

bool FormLayoutRowDialog::isValid() const
{
   // Check for non-empty names and presence of buddy marker if checked
   const QString name = labelText();

   if (name.isEmpty() || m_ui.labelNameLineEdit->text().isEmpty() || m_ui.fieldNameLineEdit->text().isEmpty()) {
      return false;
   }

   if (buddy() && ! name.contains(m_buddyMarkerRegex)) {
      return false;
   }

   return true;
}

void FormLayoutRowDialog::updateOkButton()
{
   m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid());
}

void FormLayoutRowDialog::labelTextEdited(const QString &text)
{
   updateObjectNames(true, true);
   // Set buddy if '&' is present unless the user changed it

   if (!m_buddyClicked) {
      setBuddy(text.contains(m_buddyMarkerRegex));
   }

   updateOkButton();
}

// Get a suitable object name postfix from a class name:
// "namespace::QLineEdit"->"LineEdit"
static inline QString postFixFromClassName(QString className)
{
   const int index = className.lastIndexOf("::");

   if (index != -1) {
      className.remove(0, index + 2);
   }

   if (className.size() > 2)

      if (className.at(0) == 'Q' || className.at(0) == 'K')
         if (className.at(1).isUpper()) {
            className.remove(0, 1);
         }

   return className;
}

// Helper routines to filter out characters for converting texts into
// class name prefixes. Only accepts ASCII characters/digits and underscores.

enum PrefixCharacterKind {
   PC_Digit,
   PC_UpperCaseLetter,
   PC_LowerCaseLetter,
   PC_Other,
   PC_Invalid
};

static inline PrefixCharacterKind prefixCharacterKind(const QChar &c)
{
   switch (c.category()) {
      case QChar::Number_DecimalDigit:
         return PC_Digit;
      case QChar::Letter_Lowercase: {
         const char a = c.toLatin1();
         if (a >= 'a' && a <= 'z') {
            return PC_LowerCaseLetter;
         }
      }
      break;
      case QChar::Letter_Uppercase: {
         const char a = c.toLatin1();
         if (a >= 'A' && a <= 'Z') {
            return PC_UpperCaseLetter;
         }
      }
      break;
      case QChar::Punctuation_Connector:
         if (c.toLatin1() == '_') {
            return PC_Other;
         }
         break;
      default:
         break;
   }
   return PC_Invalid;
}

// Convert the text the user types into a usable class name prefix by filtering
// characters, lower-casing the first character and camel-casing subsequent
// words. ("zip code:") --> ("zipCode").

static QString prefixFromLabel(const QString &prefix)
{
   QString rc;
   const int length = prefix.size();
   bool lastWasAcceptable = false;
   for (int i = 0 ; i < length; i++) {
      const QChar c = prefix.at(i);
      const PrefixCharacterKind kind = prefixCharacterKind(c);
      const bool acceptable = kind != PC_Invalid;
      if (acceptable) {
         if (rc.isEmpty()) {
            // Lower-case first character
            rc += kind == PC_UpperCaseLetter ? c.toLower() : c;
         } else {
            // Camel-case words
            rc += !lastWasAcceptable && kind == PC_LowerCaseLetter ? c.toUpper() : c;
         }
      }
      lastWasAcceptable = acceptable;
   }
   return rc;
}

void FormLayoutRowDialog::updateObjectNames(bool updateLabel, bool updateField)
{
   // Generate label + field object names from the label text, that is,
   // "&Zip code:" -> "zipcodeLabel", "zipcodeLineEdit" unless the user
   // edited it.
   const bool doUpdateLabel = !m_labelNameEdited && updateLabel;
   const bool doUpdateField = !m_fieldNameEdited && updateField;
   if (!doUpdateLabel && !doUpdateField) {
      return;
   }

   const QString prefix = prefixFromLabel(labelText());
   // Set names
   if (doUpdateLabel) {
      m_ui.labelNameLineEdit->setText(prefix + QString("Label"));
   }
   if (doUpdateField) {
      m_ui.fieldNameLineEdit->setText(prefix + postFixFromClassName(fieldClass()));
   }
}

void FormLayoutRowDialog::fieldClassChanged(int)
{
   updateObjectNames(false, true);
}

void FormLayoutRowDialog::labelNameEdited(const QString & /*text*/)
{
   m_labelNameEdited = true; // stop auto-updating after user change
   updateOkButton();
}

void FormLayoutRowDialog::fieldNameEdited(const QString & /*text*/)
{
   m_fieldNameEdited = true; // stop auto-updating after user change
   updateOkButton();
}

void FormLayoutRowDialog::buddyClicked()
{
   m_buddyClicked = true; // stop auto-updating after user change
   updateOkButton();
}

/* Create a list of classes suitable for field widgets. Take the fixed base
 * classes provided and look in the widget database for custom widgets derived
 * from them ("QLineEdit", "CustomLineEdit", "QComboBox"...). */
QStringList FormLayoutRowDialog::fieldWidgetClasses(QDesignerFormEditorInterface *core)
{
   static QStringList rc;

   if (rc.empty()) {
      // Scan for custom widgets that inherit them and store them in a
      // multimap of base class->custom widgets unless we have a language
      // extension installed which might do funny things with custom widgets.
      QMultiHash<QString, QString> customClassMap;

      if (qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core) == nullptr) {
         const QDesignerWidgetDataBaseInterface *wdb = core->widgetDataBase();
         const int wdbCount = wdb->count();

         for (int w = 0; w < wdbCount; ++w) {
            // Check for non-container custom types that extend the respective base class
            const QDesignerWidgetDataBaseItemInterface *dbItem = wdb->item(w);

            if (!dbItem->isPromoted() && !dbItem->isContainer() && dbItem->isCustom()) {
               const int index = fieldWidgetList.indexOf(dbItem->extends());

               if (index != -1) {
                  customClassMap.insert(fieldWidgetList.at(index), dbItem->name());
               }
            }
         }
      }

      // Compile final list, taking each base class and append custom widgets based on it
      for (const QString &item : fieldWidgetList) {
         rc.push_back(item);
         rc += customClassMap.values(item);
      }
   }

   return rc;
}

static QFormLayout *managedFormLayout(const QDesignerFormEditorInterface *core, const QWidget *w)
{
   QLayout *tmp = nullptr;

   if (LayoutInfo::managedLayoutType(core, w, &tmp) == LayoutInfo::Form) {
      return dynamic_cast<QFormLayout *>(tmp);
   }

   return nullptr;
}

// Create the widgets of a control row and apply text properties contained
// in the struct, called by addFormLayoutRow()
static QPair<QWidget *, QWidget *> createWidgets(const FormLayoutRow &row, QWidget *parent,
   QDesignerFormWindowInterface *formWindow)
{
   QDesignerFormEditorInterface *core = formWindow->core();
   QDesignerWidgetFactoryInterface *wf = core->widgetFactory();

   QPair<QWidget *, QWidget *> rc = QPair<QWidget *, QWidget *>(wf->createWidget(QString("QLabel"), parent),
         wf->createWidget(row.fieldClassName, parent));
   // Set up properties of the label
   const QString objectNameProperty = QString("objectName");
   QDesignerPropertySheetExtension *labelSheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), rc.first);
   int nameIndex = labelSheet->indexOf(objectNameProperty);
   labelSheet->setProperty(nameIndex, QVariant::fromValue(PropertySheetStringValue(row.labelName)));
   labelSheet->setChanged(nameIndex, true);
   formWindow->ensureUniqueObjectName(rc.first);
   const int textIndex = labelSheet->indexOf(QString("text"));
   labelSheet->setProperty(textIndex, QVariant::fromValue(PropertySheetStringValue(row.labelText)));
   labelSheet->setChanged(textIndex, true);
   // Set up properties of the control
   QDesignerPropertySheetExtension *controlSheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), rc.second);
   nameIndex = controlSheet->indexOf(objectNameProperty);
   controlSheet->setProperty(nameIndex, QVariant::fromValue(PropertySheetStringValue(row.fieldName)));
   controlSheet->setChanged(nameIndex, true);
   formWindow->ensureUniqueObjectName(rc.second);
   return rc;
}

// Create a command sequence on the undo stack of the form window that creates
// the widgets of the row and inserts them into the form layout.
static void addFormLayoutRow(const FormLayoutRow &formLayoutRow, int row, QWidget *w,
   QDesignerFormWindowInterface *formWindow)
{
   QFormLayout *formLayout = managedFormLayout(formWindow->core(), w);
   Q_ASSERT(formLayout);
   QUndoStack *undoStack = formWindow->commandHistory();
   const QString macroName = QCoreApplication::translate("Command", "Add '%1' to '%2'").formatArgs(formLayoutRow.labelText,
         formLayout->objectName());
   undoStack->beginMacro(macroName);

   // Create a list of widget insertion commands and pass them a cell position
   const QPair<QWidget *, QWidget *> widgetPair = createWidgets(formLayoutRow, w, formWindow);

   InsertWidgetCommand *labelCmd = new InsertWidgetCommand(formWindow);
   labelCmd->init(widgetPair.first, false, row, 0);
   undoStack->push(labelCmd);
   InsertWidgetCommand *controlCmd = new InsertWidgetCommand(formWindow);
   controlCmd->init(widgetPair.second, false, row, 1);
   undoStack->push(controlCmd);

   if (formLayoutRow.buddy) {
      SetPropertyCommand *buddyCommand = new SetPropertyCommand(formWindow);
      buddyCommand->init(widgetPair.first, buddyPropertyC, widgetPair.second->objectName());
      undoStack->push(buddyCommand);
   }

   undoStack->endMacro();
}

FormLayoutMenu::FormLayoutMenu(QObject *parent)
   : QObject(parent),
     m_separator1(new QAction(this)),
     m_populateFormAction(new QAction(tr("Add form layout row..."), this)),
     m_separator2(new QAction(this))
{
   m_separator1->setSeparator(true);
   connect(m_populateFormAction, &QAction::triggered, this, &FormLayoutMenu::slotAddRow);
   m_separator2->setSeparator(true);
}

void FormLayoutMenu::populate(QWidget *w, QDesignerFormWindowInterface *fw, ActionList &actions)
{
   switch (LayoutInfo::managedLayoutType(fw->core(), w)) {
      case LayoutInfo::Form:
         if (!actions.empty() && !actions.back()->isSeparator()) {
            actions.push_back(m_separator1);
         }

         actions.push_back(m_populateFormAction);
         actions.push_back(m_separator2);
         m_widget = w;
         break;

      default:
         m_widget = nullptr;
         break;
   }
}

void FormLayoutMenu::slotAddRow()
{
   QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(m_widget);
   Q_ASSERT(m_widget && fw);
   const int rowCount = managedFormLayout(fw->core(), m_widget)->rowCount();

   FormLayoutRowDialog dialog(fw->core(), fw);
   dialog.setRowRange(0, rowCount);
   dialog.setRow(rowCount);

   if (dialog.exec() != QDialog::Accepted) {
      return;
   }
   addFormLayoutRow(dialog.formLayoutRow(), dialog.row(), m_widget, fw);
}

QAction *FormLayoutMenu::preferredEditAction(QWidget *w, QDesignerFormWindowInterface *fw)
{
   if (LayoutInfo::managedLayoutType(fw->core(), w) == LayoutInfo::Form) {
      m_widget = w;
      return m_populateFormAction;
   }

   return nullptr;
}

}   // end namesapce
