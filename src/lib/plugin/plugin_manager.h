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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <shared_enums_p.h>

#include <QSharedDataPointer>
#include <QMap>
#include <QPair>
#include <QObject>
#include <QStringList>

class QDesignerFormEditorInterface;
class QDesignerCustomWidgetInterface;
class QDesignerPluginManagerPrivate;
class QDesignerCustomWidgetSharedData;

/* Information contained in the Dom XML of a custom widget. */
class QDesignerCustomWidgetData
{
 public:
   // StringPropertyType: validation mode and translatable flag.
   typedef QPair<qdesigner_internal::TextPropertyValidationMode, bool> StringPropertyType;

   explicit QDesignerCustomWidgetData(const QString &pluginPath = QString());

   enum ParseResult { ParseOk, ParseWarning, ParseError };
   ParseResult parseXml(const QString &xml, const QString &name, QString *errorMessage);

   QDesignerCustomWidgetData(const QDesignerCustomWidgetData &);
   QDesignerCustomWidgetData &operator=(const QDesignerCustomWidgetData &);
   ~QDesignerCustomWidgetData();

   bool isNull() const;

   QString pluginPath() const;

   // Data as parsed from the widget's domXML().
   QString xmlClassName() const;
   // Optional. The language the plugin is supposed to be used with.
   QString xmlLanguage() const;
   // Optional. method used to add pages to a container with a container extension
   QString xmlAddPageMethod() const;
   // Optional. Base class
   QString xmlExtends() const;
   // Optional. The name to be used in the widget box.
   QString xmlDisplayName() const;
   // Type of a string property
   bool xmlStringPropertyType(const QString &name, StringPropertyType *type) const;
   // Custom tool tip of property
   QString propertyToolTip(const QString &name) const;

 private:
   QSharedDataPointer<QDesignerCustomWidgetSharedData> m_d;
};

class QDesignerPluginManager: public QObject
{
   CS_OBJECT(QDesignerPluginManager)

 public:
   typedef QList<QDesignerCustomWidgetInterface *> CustomWidgetList;

   explicit QDesignerPluginManager(QDesignerFormEditorInterface *core);
   virtual ~QDesignerPluginManager();

   QDesignerFormEditorInterface *core() const;

   QObject *instance(const QString &plugin) const;

   QStringList registeredPlugins() const;

   QStringList findPlugins(const QString &path);

   QStringList pluginPaths() const;
   void setPluginPaths(const QStringList &plugin_paths);

   QStringList disabledPlugins() const;
   void setDisabledPlugins(const QStringList &disabled_plugins);

   QStringList failedPlugins() const;
   QString failureReason(const QString &pluginName) const;

   QObjectList instances() const;

   CustomWidgetList registeredCustomWidgets() const;
   QDesignerCustomWidgetData customWidgetData(QDesignerCustomWidgetInterface *w) const;
   QDesignerCustomWidgetData customWidgetData(const QString &className) const;

   bool registerNewPlugins();

   CS_SLOT_1(Public, bool syncSettings())
   CS_SLOT_2(syncSettings)
   CS_SLOT_1(Public, void ensureInitialized())
   CS_SLOT_2(ensureInitialized)

 private:
   void updateRegisteredPlugins();
   void registerPath(const QString &path);
   void registerPlugin(const QString &plugin);

   static QStringList defaultPluginPaths();

   QDesignerPluginManagerPrivate *m_d;
};

#endif

