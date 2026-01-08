/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#ifndef ABSTRACTFORMEDITOR_H
#define ABSTRACTFORMEDITOR_H

#include <abstract_formwindowmanager.h>
#include <abstract_settings.h>
#include <extension_manager.h>
#include <gradient_manager.h>
#include <resource_model.h>

#include <QObject>
#include <QPointer>

class QDesignerActionEditorInterface;
class QDesignerDialogGuiInterface;
class QDesignerFormEditorInterfacePrivate;
class QDesignerIntegrationInterface;
class QDesignerIntrospectionInterface;
class QDesignerMetaDataBaseInterface;
class QDesignerObjectInspectorInterface;
class QDesignerOptionsPageInterface;
class QDesignerPluginManager;
class QDesignerPromotionInterface;
class QDesignerPropertyEditorInterface;
class QDesignerWidgetBoxInterface;
class QDesignerWidgetDataBaseInterface;
class QDesignerWidgetFactoryInterface;

class QIcon;

QString getResourceLocation();

class QDesignerFormEditorInterface : public QObject
{
   CS_OBJECT(QDesignerFormEditorInterface)

 public:
   explicit QDesignerFormEditorInterface(QObject *parent = nullptr);
   virtual ~QDesignerFormEditorInterface();

   QExtensionManager *extensionManager() const;

   QWidget *topLevel() const;
   QDesignerWidgetBoxInterface *widgetBox() const;
   QDesignerPropertyEditorInterface *propertyEditor() const;
   QDesignerObjectInspectorInterface *objectInspector() const;
   QDesignerFormWindowManagerInterface *formWindowManager() const;
   QDesignerWidgetDataBaseInterface *widgetDataBase() const;
   QDesignerMetaDataBaseInterface *metaDataBase() const;
   QDesignerPromotionInterface *promotion() const;
   QDesignerWidgetFactoryInterface *widgetFactory() const;
   QDesignerActionEditorInterface *actionEditor() const;
   QDesignerIntegrationInterface *integration() const;
   QDesignerPluginManager *pluginManager() const;
   QDesignerIntrospectionInterface *introspection() const;
   QDesignerDialogGuiInterface *dialogGui() const;
   QDesignerSettingsInterface *settingsManager() const;

   QtResourceModel *resourceModel() const;
   QtGradientManager *gradientManager() const;
   QList<QDesignerOptionsPageInterface *> optionsPages() const;

   void setTopLevel(QWidget *topLevel);
   void setWidgetBox(QDesignerWidgetBoxInterface *widgetBox);
   void setPropertyEditor(QDesignerPropertyEditorInterface *propertyEditor);
   void setObjectInspector(QDesignerObjectInspectorInterface *objectInspector);
   void setPluginManager(QDesignerPluginManager *pluginManager);
   void setActionEditor(QDesignerActionEditorInterface *actionEditor);
   void setIntegration(QDesignerIntegrationInterface *integration);
   void setIntrospection(QDesignerIntrospectionInterface *introspection);
   void setDialogGui(QDesignerDialogGuiInterface *dialogGui);
   void setSettingsManager(QDesignerSettingsInterface *settingsManager);
   void setResourceModel(QtResourceModel *model);
   void setGradientManager(QtGradientManager *manager);
   void setOptionsPages(const QList<QDesignerOptionsPageInterface *> &optionsPages);

   QObjectList pluginInstances() const;

   static QIcon createIcon(const QString &name);

 protected:
   void setFormManager(QDesignerFormWindowManagerInterface *formWindowManager);
   void setMetaDataBase(QDesignerMetaDataBaseInterface *metaDataBase);
   void setWidgetDataBase(QDesignerWidgetDataBaseInterface *widgetDataBase);
   void setPromotion(QDesignerPromotionInterface *promotion);
   void setWidgetFactory(QDesignerWidgetFactoryInterface *widgetFactory);
   void setExtensionManager(QExtensionManager *extensionManager);

 private:
   QScopedPointer<QDesignerFormEditorInterfacePrivate> d;

   QDesignerFormEditorInterface(const QDesignerFormEditorInterface &other);
   void operator = (const QDesignerFormEditorInterface &other);
};

#endif
