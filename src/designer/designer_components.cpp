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

#include <abstract_integration.h>
#include <abstract_language.h>
#include <abstract_resourcebrowser.h>
#include <action_editor.h>
#include <buddyeditor_plugin.h>
#include <designer_components.h>
#include <extension.h>
#include <formeditor.h>
#include <object_inspector.h>
#include <property_editor.h>
#include <resource_view.h>
#include <signalsloteditor_plugin.h>
#include <signalsloteditorwindow.h>
#include <tabordereditor_plugin.h>
#include <taskmenu_component.h>
#include <widgetbox.h>
#include <widgetfactory.h>

#include <widgetdatabase_p.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

/* emerald - unused, plugins
#include <QPlugin>

void QDesignerComponents::initializePlugins(QDesignerFormEditorInterface *core)
{
   QDesignerIntegration::initializePlugins(core);
}

*/

// create all the panes from here down

QDesignerFormEditorInterface *QDesignerComponents::createFormEditor(QObject *parent)
{
   return new qdesigner_internal::FormEditor(parent);
}

QObject *QDesignerComponents::createTaskMenu(QDesignerFormEditorInterface *core, QObject *parent)
{
   return new qdesigner_internal::TaskMenuComponent(core, parent);
}

// Build the version dependent name of the user widget box file, 'HOME/.designer/widgetbox.xml'
static inline QString widgetBoxFileName(const QDesignerLanguageExtension *lang = nullptr)
{
   QString retval = QDir::homePath() + QDir::separator() + ".designer" + QDir::separator() + "widgetbox";

   if (lang != nullptr) {
      retval += '.' + lang->uiExtension();
   }

   retval += ".xml";

   return retval;
}

QDesignerWidgetBoxInterface *QDesignerComponents::createWidgetBox(QDesignerFormEditorInterface *core, QWidget *parent)
{
   qdesigner_internal::WidgetBox *widgetBox = new qdesigner_internal::WidgetBox(core, parent);

   const QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core);

   do {
      if (lang) {
         const QString languageWidgetBox = lang->widgetBoxContents();

         if (! languageWidgetBox.isEmpty()) {
            widgetBox->loadContents(lang->widgetBoxContents());
            break;
         }
      }

      widgetBox->setFileName(":/resources/widgetbox.xml");
      widgetBox->load();

   } while (false);

   // locate if the user is overriding the built in widgetbox.xml file
   const QString userWidgetBoxFile = widgetBoxFileName(lang);

   widgetBox->setFileName(userWidgetBoxFile);
   widgetBox->load();

   return widgetBox;
}

QDesignerPropertyEditorInterface *QDesignerComponents::createPropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
   return new qdesigner_internal::PropertyEditor(core, parent);
}

QDesignerObjectInspectorInterface *QDesignerComponents::createObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent)
{
   return new qdesigner_internal::ObjectInspector(core, parent);
}

QDesignerActionEditorInterface *QDesignerComponents::createActionEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
   return new qdesigner_internal::ActionEditor(core, parent);
}

QWidget *QDesignerComponents::createResourceEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
   if (QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core)) {
      QWidget *w = lang->createResourceBrowser(parent);

      if (w) {
         return w;
      }
   }

   QtResourceView *resourceView = new QtResourceView(core, parent);
   resourceView->setResourceModel(core->resourceModel());
   resourceView->setSettingsKey("ResourceEditor");

   const QDesignerIntegrationInterface *integration = core->integration();

   if (integration && !integration->hasFeature(QDesignerIntegrationInterface::ResourceEditorFeature)) {
      resourceView->setResourceEditingEnabled(false);
   }

   return resourceView;
}

QWidget *QDesignerComponents::createSignalSlotEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
   return new qdesigner_internal::SignalSlotEditorWindow(core, parent);
}
