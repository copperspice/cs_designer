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

#include <abstract_formeditor.h>
#include <abstract_language.h>
#include <extension.h>
#include <extension_manager.h>
#include <extrainfo.h>
#include <customwidget.h>
#include <properties.h>
#include <ui4.h>
#include <simple_resource.h>
#include <widgetfactory.h>

#include <widgetdatabase_p.h>

#include <QIcon>
#include <QWidget>
#include <QAction>
#include <QDebug>
#include <QCoreApplication>

namespace {

typedef QList<DomWidgetData *> DomWidgetDataList;
typedef QList<QDesignerCustomWidgetInterface *> CustomWidgetInterfaces;

}

namespace qdesigner_internal {

bool QSimpleResource::m_warningsEnabled = true;

QSimpleResource::QSimpleResource(QDesignerFormEditorInterface *core)
   : QAbstractFormBuilder(), m_core(core)
{
   QString workingDirectory = QDir::homePath();
   workingDirectory +=  QDir::separator();
   workingDirectory +=  ".designer";

   setWorkingDirectory(QDir(workingDirectory));
}

QSimpleResource::~QSimpleResource()
{
}

QBrush QSimpleResource::setupBrush(DomBrush *brush)
{
   return QAbstractFormBuilder::setupBrush(brush);
}

DomBrush *QSimpleResource::saveBrush(const QBrush &brush)
{
   return QAbstractFormBuilder::saveBrush(brush);
}

DomScript *QSimpleResource::createScript(const QString &script, ScriptSource source)
{
   if (script.isEmpty()) {
      return nullptr;
   }

   DomScript *domScript = new DomScript();

   switch (source) {
      case ScriptExtension:
         domScript->setAttributeSource("extension");
         break;

      case ScriptDesigner:
         domScript->setAttributeSource("designer");
         break;

      case ScriptCustomWidgetPlugin:
         domScript->setAttributeSource("customwidgetplugin");
         break;
   }

   domScript->setAttributeLanguage("Qt Script");
   domScript->setText(script);

   return domScript;
}

// Add a script to a list of DomScripts unless empty
void QSimpleResource::addScript(const QString &script, ScriptSource source, DomScripts &domScripts)
{
   if (DomScript *domScript = createScript(script, source)) {
      domScripts += domScript;
   }
}

void QSimpleResource::addExtensionDataToDOM(QAbstractFormBuilder * /* afb */,
   QDesignerFormEditorInterface *core,
   DomWidget *ui_widget, QWidget *widget)
{
   QExtensionManager *emgr = core->extensionManager();
   if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension *>(emgr, widget)) {
      extra->saveWidgetExtraInfo(ui_widget);
   }
}

void QSimpleResource::applyExtensionDataFromDOM(QAbstractFormBuilder * /* afb */,
   QDesignerFormEditorInterface *core,
   DomWidget *ui_widget, QWidget *widget)
{
   QExtensionManager *emgr = core->extensionManager();
   if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension *>(emgr, widget)) {
      extra->loadWidgetExtraInfo(ui_widget);
   }
}

QString QSimpleResource::customWidgetScript(QDesignerFormEditorInterface *core, QObject *object)
{
   return customWidgetScript(core, qdesigner_internal::WidgetFactory::classNameOf(core, object));
}

bool QSimpleResource::hasCustomWidgetScript(QDesignerFormEditorInterface *, QObject *)
{
   return false;
}

QString QSimpleResource::customWidgetScript(QDesignerFormEditorInterface *, const QString &)
{
   return QString();
}

// Custom widgets handling helpers

// Add unique fake slots and signals to lists
bool QSimpleResource::addFakeMethods(const DomSlots *domSlots, QStringList &fakeSlots, QStringList &fakeSignals)
{
   if (!domSlots) {
      return false;
   }

   bool rc = false;
   for (const QString &fakeSlot : domSlots->elementSlot())
      if (fakeSlots.indexOf(fakeSlot) == -1) {
         fakeSlots += fakeSlot;
         rc = true;
      }

   for (const QString &fakeSignal : domSlots->elementSignal())
      if (fakeSignals.indexOf(fakeSignal) == -1) {
         fakeSignals += fakeSignal;
         rc = true;
      }
   return rc;
}

void QSimpleResource::addFakeMethodsToWidgetDataBase(const DomCustomWidget *domCustomWidget, WidgetDataBaseItem *item)
{
   const DomSlots *domSlots = domCustomWidget->elementSlots();
   if (!domSlots) {
      return;
   }

   // Merge in new slots, signals
   QStringList fakeSlots = item->fakeSlots();
   QStringList fakeSignals = item->fakeSignals();

   if (addFakeMethods(domSlots, fakeSlots, fakeSignals)) {
      item->setFakeSlots(fakeSlots);
      item->setFakeSignals(fakeSignals);
   }
}

// Perform one iteration of adding the custom widgets to the database,
// looking up the base class and inheriting its data.
// Remove the succeeded custom widgets from the list.
// Classes whose base class could not be found are left in the list.

void QSimpleResource::addCustomWidgetsToWidgetDatabase(const QDesignerFormEditorInterface *core,
   QList<DomCustomWidget *> &custom_widget_list)
{
   QDesignerWidgetDataBaseInterface *db = core->widgetDataBase();
   for (int i = 0; i < custom_widget_list.size(); ) {
      bool classInserted = false;
      DomCustomWidget *custom_widget = custom_widget_list[i];

      const QString customClassName = custom_widget->elementClass();
      const QString base_class = custom_widget->elementExtends();

      QString includeFile;

      IncludeType includeType = IncludeLocal;
      if (const DomHeader *header = custom_widget->elementHeader()) {
         includeFile = header->text();
         if (header->hasAttributeLocation() && header->attributeLocation() == "global") {
            includeType = IncludeGlobal;
         }
      }
      const bool domIsContainer = custom_widget->elementContainer();
      // Append a new item
      if (base_class.isEmpty()) {
         WidgetDataBaseItem *item = new WidgetDataBaseItem(customClassName);
         item->setPromoted(false);
         item->setGroup(QCoreApplication::translate("Designer", "Custom Widgets"));
         item->setIncludeFile(buildIncludeFile(includeFile, includeType));
         item->setContainer(domIsContainer);
         item->setCustom(true);
         addFakeMethodsToWidgetDataBase(custom_widget, item);
         db->append(item);
         custom_widget_list.removeAt(i);
         classInserted = true;

      } else {
         // Create a new entry cloned from base class. Note that this will ignore existing
         // classes, eg, plugin custom widgets.
         QDesignerWidgetDataBaseItemInterface *item =
               appendDerived(db, customClassName, QCoreApplication::translate("Designer", "Promoted Widgets"),
               base_class, buildIncludeFile(includeFile, includeType), true, true);

         // base class found
         if (item) {
            // Done to accommodate for old UI-files in which "container" is not set properly:
            // Apply "container" from DOM only if true (else, eg classes from QFrame might not accept
            // dropping child widgets on them as container=false). This also allows for
            // QWidget-derived stacked pages.

            if (domIsContainer) {
               item->setContainer(domIsContainer);
            }

            addFakeMethodsToWidgetDataBase(custom_widget, static_cast<WidgetDataBaseItem *>(item));
            custom_widget_list.removeAt(i);
            classInserted = true;
         }
      }

      // Skip failed item.
      if (!classInserted) {
         ++i;
      }
   }

}

void QSimpleResource::handleDomCustomWidgets(const QDesignerFormEditorInterface *core,
   const DomCustomWidgets *dom_custom_widgets)
{
   if (dom_custom_widgets == nullptr) {
      return;
   }
   QList<DomCustomWidget *> custom_widget_list = dom_custom_widgets->elementCustomWidget();
   // Attempt to insert each item derived from its base class.
   // This should at most require two iterations in the event that the classes are out of order
   // (derived first, max depth: promoted custom plugin = 2)
   for (int iteration = 0;  iteration < 2;  iteration++) {
      addCustomWidgetsToWidgetDatabase(core, custom_widget_list);
      if (custom_widget_list.empty()) {
         return;
      }
   }
   // Oops, there are classes left whose base class could not be found.
   // Default them to QWidget with warnings.
   const QString fallBackBaseClass = "QWidget";

   for (int i = 0; i < custom_widget_list.size(); i++ ) {
      DomCustomWidget *custom_widget = custom_widget_list[i];

      const QString customClassName = custom_widget->elementClass();
      const QString base_class = custom_widget->elementExtends();

      qDebug() << "** WARNING The base class " << base_class << " of the custom widget class " << customClassName
         << " could not be found. Defaulting to " << fallBackBaseClass << '.';

      custom_widget->setElementExtends(fallBackBaseClass);
   }

   // One more pass.
   addCustomWidgetsToWidgetDatabase(core, custom_widget_list);
}

// ------------ FormBuilderClipboard

FormBuilderClipboard::FormBuilderClipboard(QWidget *w)
{
   m_widgets += w;
}

bool FormBuilderClipboard::empty() const
{
   return m_widgets.empty() && m_actions.empty();
}
}


