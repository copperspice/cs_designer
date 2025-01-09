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
#include <designer_property_editor.h>
#include <designer_propertysheet.h>
#include <dynamicpropertysheet.h>
#include <extension.h>
#include <plugin_manager.h>
#include <widgetfactory.h>

#include <QAbstractButton>
#include <QAction>
#include <QLineEdit>

namespace qdesigner_internal {

using TextValidPair = QDesignerPropertyEditor::TextValidPair;

// Compile a map of hard coded string property types
static const QHash<QString, TextValidPair> &stringPropertyTypes()
{
   static QHash<QString, TextValidPair> propertyHash;

   if (propertyHash.empty()) {
      const TextValidPair richtext(ValidationRichText, true);

      // Accessibility. Both are texts the narrator reads
      propertyHash.insert("accessibleDescription", richtext);
      propertyHash.insert("accessibleName", richtext);

      // object names
      const TextValidPair objectName(ValidationObjectName, false);
      propertyHash.insert(QString("buddy"), objectName);
      propertyHash.insert(QString("currentItemName"), objectName);
      propertyHash.insert(QString("currentPageName"), objectName);
      propertyHash.insert(QString("currentTabName"), objectName);
      propertyHash.insert(QString("layoutName"), objectName);
      propertyHash.insert(QString("spacerName"), objectName);

      // Style sheet
      propertyHash.insert(QString("styleSheet"), TextValidPair(ValidationStyleSheet, false));

      // Buttons/  QCommandLinkButton
      const TextValidPair multiline(ValidationMultiLine, true);
      propertyHash.insert(QString("description"), multiline);
      propertyHash.insert(QString("iconText"), multiline);

      // Tooltips, etc.
      propertyHash.insert(QString("toolTip"), richtext);
      propertyHash.insert(QString("whatsThis"), richtext);
      propertyHash.insert(QString("windowIconText"), richtext);
      propertyHash.insert(QString("html"), richtext);

      //  QWizard page id
      propertyHash.insert(QString("pageId"), TextValidPair(ValidationSingleLine, false));

      // QPlainTextEdit
      propertyHash.insert(QString("plainText"), TextValidPair(ValidationMultiLine, true));
   }

   return propertyHash;
}

QDesignerPropertyEditor::QDesignerPropertyEditor(QWidget *parent, Qt::WindowFlags flags)
   : QDesignerPropertyEditorInterface(parent, flags), m_propertyChangedForwardingBlocked(false)
{
   // Make old signal work for compatibility
   connect(this, &QDesignerPropertyEditorInterface::propertyChanged,
         this, &QDesignerPropertyEditor::slotPropertyChanged);
}

static inline bool isDynamicProperty(QDesignerFormEditorInterface *core, QObject *object,
   const QString &propertyName)
{
   if (const QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension *>
         (core->extensionManager(), object)) {
      if (dynamicSheet->dynamicPropertiesAllowed()) {

         if (QDesignerPropertySheetExtension *propertySheet =
               qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), object)) {

            const int index = propertySheet->indexOf(propertyName);
            return index >= 0 && dynamicSheet->isDynamicProperty(index);
         }
      }
   }

   return false;
}

QDesignerPropertyEditor::TextValidPair QDesignerPropertyEditor::textPropertyValidationMode(
      QDesignerFormEditorInterface *core, const QObject *object,
      const QString &propertyName, bool isMainContainer)
{
   // object name - no comment
   if (propertyName == "objectName") {
      const TextPropertyValidationMode vm =  isMainContainer ? ValidationObjectNameScope : ValidationObjectName;
      return TextValidPair(vm, false);
   }

   // Check custom widgets by class.
   const QString className = WidgetFactory::classNameOf(core, object);
   const QDesignerCustomWidgetData customData = core->pluginManager()->customWidgetData(className);

   if (!customData.isNull()) {
      TextValidPair customType;
      if (customData.xmlStringPropertyType(propertyName, &customType)) {
         return customType;
      }
   }

   if (isDynamicProperty(core, const_cast<QObject *>(object), propertyName)) {
      return TextValidPair(ValidationMultiLine, true);
   }

   // Check hardcoded property names
   auto &tmpMap = stringPropertyTypes();
   auto iter    = tmpMap.constFind(propertyName);

   if (iter != tmpMap.constEnd()) {
      return iter.value();
   }

   // text: Check according to widget type
   if (propertyName == "text") {
      if (dynamic_cast<const QAction *>(object) || dynamic_cast<const QLineEdit *>(object)) {
         return TextValidPair(ValidationSingleLine, true);
      }

      if (dynamic_cast<const QAbstractButton *>(object)) {
         return TextValidPair(ValidationMultiLine, true);
      }
      return TextValidPair(ValidationRichText, true);
   }

   // Fuzzy matching
   if (propertyName.endsWith(QString("Name"))) {
      return TextValidPair(ValidationSingleLine, true);
   }

   if (propertyName.endsWith(QString("ToolTip"))) {
      return TextValidPair(ValidationRichText, true);
   }

#ifdef Q_OS_WIN
   // No translation for the active X "control" property
   if (propertyName == "control" && className == "QAxWidget") {
      return TextValidPair(ValidationSingleLine, false);
   }
#endif

   // default to single
   return TextValidPair(ValidationSingleLine, true);
}

void QDesignerPropertyEditor::emitPropertyValueChanged(const QString &name, const QVariant &value, bool enableSubPropertyHandling)
{
   // Avoid duplicate signal emission - see below
   m_propertyChangedForwardingBlocked = true;

   emit propertyValueChanged(name, value, enableSubPropertyHandling);
   emit propertyChanged(name, value);

   m_propertyChangedForwardingBlocked = false;
}

void QDesignerPropertyEditor::slotPropertyChanged(const QString &name, const QVariant &value)
{
   // Forward signal from Integration using the old interfaces
   if (! m_propertyChangedForwardingBlocked) {
      emit propertyValueChanged(name, value, true);
   }
}

}   // end namespce - qdesigner_internal

