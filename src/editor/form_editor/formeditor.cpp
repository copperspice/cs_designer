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

#include <abstract_integration.h>
#include <default_actionprovider.h>
#include <default_container.h>
#include <default_layoutdecoration.h>
#include <designer_introspection.h>
#include <designer_membersheet.h>
#include <designer_promotion.h>
#include <designer_qsettings.h>
#include <designer_stackedbox.h>
#include <designer_tabwidget.h>
#include <designer_taskmenu.h>
#include <designer_toolbox.h>
#include <embedded_options_page.h>
#include <extension.h>
#include <formeditor.h>
#include <formeditor_optionspage.h>
#include <formwindowmanager.h>
#include <itemview_propertysheet.h>
#include <layout_propertysheet.h>
#include <layoutwidget_propertysheet.h>
#include <line_propertysheet.h>
#include <mainwindow_container.h>
#include <mdi_area_container.h>
#include <plugin_manager.h>
#include <resource_model.h>
#include <spacer_propertysheet.h>
#include <templateoptionspage.h>
#include <widgetfactory.h>
#include <wizard_container.h>

#include <dialoggui_p.h>
#include <metadatabase_p.h>
#include <widgetdatabase_p.h>

namespace qdesigner_internal {

FormEditor::FormEditor(QObject *parent)
   : QDesignerFormEditorInterface(parent)
{
   setIntrospection(new QDesignerIntrospection);
   setDialogGui(new DialogGui);
   QDesignerPluginManager *pluginManager = new QDesignerPluginManager(this);
   setPluginManager(pluginManager);

   WidgetDataBase *widgetDatabase = new WidgetDataBase(this, this);
   setWidgetDataBase(widgetDatabase);

   MetaDataBase *metaDataBase = new MetaDataBase(this, this);
   setMetaDataBase(metaDataBase);

   WidgetFactory *widgetFactory = new WidgetFactory(this, this);
   setWidgetFactory(widgetFactory);

   FormWindowManager *formWindowManager = new FormWindowManager(this, this);
   setFormManager(formWindowManager);

   connect(formWindowManager, &QDesignerFormWindowManagerInterface::formWindowAdded,
         widgetFactory, &WidgetFactory::formWindowAdded);

   connect(formWindowManager, &QDesignerFormWindowManagerInterface::activeFormWindowChanged,
         widgetFactory, &WidgetFactory::activeFormWindowChanged);

   QExtensionManager *mgr = new QExtensionManager(this);

   // 1
   const QString containerExtensionId = CS_TYPEID(QDesignerContainerExtension);

   QDesignerStackedWidgetContainerFactory::registerExtension(mgr, containerExtensionId);
   QDesignerTabWidgetContainerFactory::registerExtension(mgr, containerExtensionId);
   QDesignerToolBoxContainerFactory::registerExtension(mgr, containerExtensionId);
   QMainWindowContainerFactory::registerExtension(mgr, containerExtensionId);
   QDockWidgetContainerFactory::registerExtension(mgr, containerExtensionId);
   QScrollAreaContainerFactory::registerExtension(mgr, containerExtensionId);
   QMdiAreaContainerFactory::registerExtension(mgr, containerExtensionId);
   QWizardContainerFactory::registerExtension(mgr, containerExtensionId);

   // 2
   mgr->registerExtensions(new QDesignerLayoutDecorationFactory(mgr),
         CS_TYPEID(QDesignerLayoutDecorationExtension));

   // 3
   const QString actionProviderExtensionId = CS_TYPEID(QDesignerActionProviderExtension);

   QToolBarActionProviderFactory::registerExtension(mgr, actionProviderExtensionId);
   QMenuBarActionProviderFactory::registerExtension(mgr, actionProviderExtensionId);
   QMenuActionProviderFactory::registerExtension(mgr,    actionProviderExtensionId);

   // 4
   QDesignerDefaultPropertySheetFactory::registerExtension(mgr);
   QLayoutWidgetPropertySheetFactory::registerExtension(mgr);
   SpacerPropertySheetFactory::registerExtension(mgr);
   LinePropertySheetFactory::registerExtension(mgr);
   LayoutPropertySheetFactory::registerExtension(mgr);
   QStackedWidgetPropertySheetFactory::registerExtension(mgr);
   QToolBoxWidgetPropertySheetFactory::registerExtension(mgr);
   QTabWidgetPropertySheetFactory::registerExtension(mgr);
   QMdiAreaPropertySheetFactory::registerExtension(mgr);
   QWizardPagePropertySheetFactory::registerExtension(mgr);
   QWizardPropertySheetFactory::registerExtension(mgr);
   QTreeViewPropertySheetFactory::registerExtension(mgr);
   QTableViewPropertySheetFactory::registerExtension(mgr);

   // 5
   const QString internalTaskMenuId = QString("QDesignerInternalTaskMenuExtension");
   QDesignerTaskMenuFactory::registerExtension(mgr, internalTaskMenuId);

   // 6
   mgr->registerExtensions(new QDesignerMemberSheetFactory(mgr),
         CS_TYPEID(QDesignerMemberSheetExtension));

   setExtensionManager(mgr);

   setPromotion(new QDesignerPromotion(this));

   QtResourceModel *resourceModel = new QtResourceModel(this);
   setResourceModel(resourceModel);

   connect(resourceModel, &QtResourceModel::qrcFileModifiedExternally,
         this, &FormEditor::slotQrcFileChangedExternally);

   QList<QDesignerOptionsPageInterface *> optionsPages;
   optionsPages << new TemplateOptionsPage(this) << new FormEditorOptionsPage(this) << new EmbeddedOptionsPage(this);
   setOptionsPages(optionsPages);

   setSettingsManager(new QDesignerQSettings());
}

FormEditor::~FormEditor()
{
}

void FormEditor::slotQrcFileChangedExternally(const QString &path)
{
   if (!integration()) {
      return;
   }

   QDesignerIntegration::ResourceFileWatcherBehaviour behaviour = integration()->resourceFileWatcherBehaviour();
   if (behaviour == QDesignerIntegration::NoResourceFileWatcher) {
      return;

   } else if (behaviour == QDesignerIntegration::PromptToReloadResourceFile) {
      QMessageBox::StandardButton button = dialogGui()->message(topLevel(), QDesignerDialogGuiInterface::FileChangedMessage,
            QMessageBox::Warning, tr("Resource File Changed"),
            tr("The file \"%1\" has changed outside Designer. Do you want to reload it?").formatArg(path),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

      if (button != QMessageBox::Yes) {
         return;
      }
   }

   resourceModel()->reload(path);
}

}

