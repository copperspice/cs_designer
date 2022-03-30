/***********************************************************************
*
* Copyright (c) 2021-2022 Barbara Geller
* Copyright (c) 2021-2022 Ansel Sermersheim
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
#include <abstract_optionspage.h>
#include <abstract_settings.h>
#include <abstract_promotioninterface.h>
#include <abstract_propertyeditor.h>
#include <abstract_formwindowmanager.h>
#include <abstract_metadatabase.h>
#include <abstract_objectinspector.h>
#include <abstract_integration.h>
#include <abstract_actioneditor.h>
#include <abstract_dialoggui.h>
#include <abstract_introspection.h>
#include <extension_manager.h>
#include <resource_model.h>
#include <plugin_manager.h>

#include <widgetdatabase_p.h>
#include <widgetfactory_p.h>
#include <qdesigner_widgetbox_p.h>
#include <qtgradientmanager.h>
#include <widgetfactory_p.h>
#include <shared_settings_p.h>
#include <formwindowbase_p.h>
#include <grid_p.h>
#include <iconloader_p.h>

#include <QIcon>

#include <qalgorithms.h>

static void initResources()
{
   /* emerald - temporary hold on cell phones

       Q_INIT_RESOURCE(shared);
       Q_INIT_RESOURCE(ClamshellPhone);
       Q_INIT_RESOURCE(PortableMedia);
       Q_INIT_RESOURCE(S60_nHD_Touchscreen);
       Q_INIT_RESOURCE(S60_QVGA_Candybar);
       Q_INIT_RESOURCE(SmartPhone2);
       Q_INIT_RESOURCE(SmartPhone);
       Q_INIT_RESOURCE(SmartPhoneWithButtons);
       Q_INIT_RESOURCE(TouchscreenPhone);
   */
}

QString getResourceLocation()
{
#ifdef Q_OS_DARWIN
   return QString(":/resources/form_editor/images/mac");
#else
   return QString(":/resources/form_editor/images/win");
#endif
}

class QDesignerFormEditorInterfacePrivate
{
 public:
   QDesignerFormEditorInterfacePrivate();
   ~QDesignerFormEditorInterfacePrivate();

   QPointer<QWidget> m_topLevel;
   QPointer<QDesignerWidgetBoxInterface> m_widgetBox;
   QPointer<QDesignerPropertyEditorInterface> m_propertyEditor;
   QPointer<QDesignerFormWindowManagerInterface> m_formWindowManager;
   QPointer<QExtensionManager> m_extensionManager;
   QPointer<QDesignerMetaDataBaseInterface> m_metaDataBase;
   QPointer<QDesignerWidgetDataBaseInterface> m_widgetDataBase;
   QPointer<QDesignerWidgetFactoryInterface> m_widgetFactory;
   QPointer<QDesignerObjectInspectorInterface> m_objectInspector;
   QPointer<QDesignerIntegrationInterface> m_integration;
   QPointer<QDesignerActionEditorInterface> m_actionEditor;

   QDesignerSettingsInterface *m_settingsManager;
   QDesignerPluginManager *m_pluginManager;
   QDesignerPromotionInterface *m_promotion;
   QDesignerIntrospectionInterface *m_introspection;
   QDesignerDialogGuiInterface *m_dialogGui;

   QPointer<QtResourceModel> m_resourceModel;
   QPointer<QtGradientManager> m_gradientManager; // instantiated and deleted by designer_integration

   QList<QDesignerOptionsPageInterface *> m_optionsPages;
};

QDesignerFormEditorInterfacePrivate::QDesignerFormEditorInterfacePrivate()
   : m_settingsManager(0), m_pluginManager(0), m_promotion(0),
     m_introspection(0), m_dialogGui(0), m_resourceModel(0), m_gradientManager(0)
{
}

QDesignerFormEditorInterfacePrivate::~QDesignerFormEditorInterfacePrivate()
{
   delete m_settingsManager;
   delete m_formWindowManager;
   delete m_promotion;
   delete m_introspection;
   delete m_dialogGui;
   delete m_resourceModel;

   qDeleteAll(m_optionsPages);
}

QDesignerFormEditorInterface::QDesignerFormEditorInterface(QObject *parent)
   : QObject(parent), d(new QDesignerFormEditorInterfacePrivate())
{
   initResources();
}

QDesignerFormEditorInterface::~QDesignerFormEditorInterface()
{
}

QDesignerWidgetBoxInterface *QDesignerFormEditorInterface::widgetBox() const
{
   return d->m_widgetBox;
}

void QDesignerFormEditorInterface::setWidgetBox(QDesignerWidgetBoxInterface *widgetBox)
{
   d->m_widgetBox = widgetBox;
}

QDesignerPropertyEditorInterface *QDesignerFormEditorInterface::propertyEditor() const
{
   return d->m_propertyEditor;
}

void QDesignerFormEditorInterface::setPropertyEditor(QDesignerPropertyEditorInterface *propertyEditor)
{
   d->m_propertyEditor = propertyEditor;
}

QDesignerActionEditorInterface *QDesignerFormEditorInterface::actionEditor() const
{
   return d->m_actionEditor;
}

void QDesignerFormEditorInterface::setActionEditor(QDesignerActionEditorInterface *actionEditor)
{
   d->m_actionEditor = actionEditor;
}

QWidget *QDesignerFormEditorInterface::topLevel() const
{
   return d->m_topLevel;
}

void QDesignerFormEditorInterface::setTopLevel(QWidget *topLevel)
{
   d->m_topLevel = topLevel;
}

QDesignerFormWindowManagerInterface *QDesignerFormEditorInterface::formWindowManager() const
{
   return d->m_formWindowManager;
}

void QDesignerFormEditorInterface::setFormManager(QDesignerFormWindowManagerInterface *formWindowManager)
{
   d->m_formWindowManager = formWindowManager;
}

QExtensionManager *QDesignerFormEditorInterface::extensionManager() const
{
   return d->m_extensionManager;
}

void QDesignerFormEditorInterface::setExtensionManager(QExtensionManager *extensionManager)
{
   d->m_extensionManager = extensionManager;
}

QDesignerMetaDataBaseInterface *QDesignerFormEditorInterface::metaDataBase() const
{
   return d->m_metaDataBase;
}

void QDesignerFormEditorInterface::setMetaDataBase(QDesignerMetaDataBaseInterface *metaDataBase)
{
   d->m_metaDataBase = metaDataBase;
}

QDesignerWidgetDataBaseInterface *QDesignerFormEditorInterface::widgetDataBase() const
{
   return d->m_widgetDataBase;
}

void QDesignerFormEditorInterface::setWidgetDataBase(QDesignerWidgetDataBaseInterface *widgetDataBase)
{
   d->m_widgetDataBase = widgetDataBase;
}

QDesignerPromotionInterface *QDesignerFormEditorInterface::promotion() const
{
   return d->m_promotion;
}

void QDesignerFormEditorInterface::setPromotion(QDesignerPromotionInterface *promotion)
{
   if (d->m_promotion) {
      delete d->m_promotion;
   }
   d->m_promotion = promotion;
}

QDesignerWidgetFactoryInterface *QDesignerFormEditorInterface::widgetFactory() const
{
   return d->m_widgetFactory;
}

void QDesignerFormEditorInterface::setWidgetFactory(QDesignerWidgetFactoryInterface *widgetFactory)
{
   d->m_widgetFactory = widgetFactory;
}

QDesignerObjectInspectorInterface *QDesignerFormEditorInterface::objectInspector() const
{
   return d->m_objectInspector;
}

void QDesignerFormEditorInterface::setObjectInspector(QDesignerObjectInspectorInterface *objectInspector)
{
   d->m_objectInspector = objectInspector;
}

QDesignerIntegrationInterface *QDesignerFormEditorInterface::integration() const
{
   return d->m_integration;
}

void QDesignerFormEditorInterface::setIntegration(QDesignerIntegrationInterface *integration)
{
   d->m_integration = integration;
}

QList<QDesignerOptionsPageInterface *> QDesignerFormEditorInterface::optionsPages() const
{
   return d->m_optionsPages;
}

void QDesignerFormEditorInterface::setOptionsPages(const QList<QDesignerOptionsPageInterface *> &optionsPages)
{
   d->m_optionsPages = optionsPages;
}

QDesignerPluginManager *QDesignerFormEditorInterface::pluginManager() const
{
   return d->m_pluginManager;
}

void QDesignerFormEditorInterface::setPluginManager(QDesignerPluginManager *pluginManager)
{
   d->m_pluginManager = pluginManager;
}

QtResourceModel *QDesignerFormEditorInterface::resourceModel() const
{
   return d->m_resourceModel;
}

void QDesignerFormEditorInterface::setResourceModel(QtResourceModel *resourceModel)
{
   d->m_resourceModel = resourceModel;
}


QtGradientManager *QDesignerFormEditorInterface::gradientManager() const
{
   return d->m_gradientManager;
}

void QDesignerFormEditorInterface::setGradientManager(QtGradientManager *gradientManager)
{
   d->m_gradientManager = gradientManager;
}

QDesignerSettingsInterface *QDesignerFormEditorInterface::settingsManager() const
{
   return d->m_settingsManager;
}

void QDesignerFormEditorInterface::setSettingsManager(QDesignerSettingsInterface *settingsManager)
{
   if (d->m_settingsManager) {
      delete d->m_settingsManager;
   }
   d->m_settingsManager = settingsManager;

   // This is a (hopefully) safe place to perform settings-dependent
   // initializations.
   const qdesigner_internal::QDesignerSharedSettings settings(this);
   qdesigner_internal::FormWindowBase::setDefaultDesignerGrid(settings.defaultGrid());
}

QDesignerIntrospectionInterface *QDesignerFormEditorInterface::introspection() const
{
   return d->m_introspection;
}

void QDesignerFormEditorInterface::setIntrospection(QDesignerIntrospectionInterface *introspection)
{
   if (d->m_introspection) {
      delete d->m_introspection;
   }
   d->m_introspection = introspection;
}

QDesignerDialogGuiInterface *QDesignerFormEditorInterface::dialogGui() const
{
   return d->m_dialogGui;
}

void QDesignerFormEditorInterface::setDialogGui(QDesignerDialogGuiInterface *dialogGui)
{
   delete  d->m_dialogGui;
   d->m_dialogGui = dialogGui;
}

QObjectList QDesignerFormEditorInterface::pluginInstances() const
{
   return d->m_pluginManager->instances();
}

QIcon QDesignerFormEditorInterface::createIcon(const QString &name)
{
   return qdesigner_internal::createIconSet(name);
}
