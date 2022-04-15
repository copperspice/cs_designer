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

#ifndef ABSTRACTINTEGRATION_H
#define ABSTRACTINTEGRATION_H

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;
class QDesignerIntegrationInterfacePrivate;
class QDesignerResourceBrowserInterface;

#include <QObject>
#include <QScopedPointer>
#include <QStringList>
#include <QFlags>

class QVariant;
class QWidget;

namespace qdesigner_internal {
class QDesignerIntegrationPrivate;
}

class QDesignerIntegrationInterface: public QObject
{
   CS_OBJECT(QDesignerIntegrationInterface)

   CS_PROPERTY_READ(headerSuffix, headerSuffix)
   CS_PROPERTY_WRITE(headerSuffix, setHeaderSuffix)
   CS_PROPERTY_READ(headerLowercase, isHeaderLowercase)
   CS_PROPERTY_WRITE(headerLowercase, setHeaderLowercase)

 public:
   enum ResourceFileWatcherBehaviour {
      NoResourceFileWatcher,
      ReloadResourceFileSilently,
      PromptToReloadResourceFile // Default
   };

   enum FeatureFlag {
      ResourceEditorFeature = 0x1,
      SlotNavigationFeature = 0x2,
      DefaultWidgetActionFeature = 0x4,
      DefaultFeature = ResourceEditorFeature | DefaultWidgetActionFeature
   };
   using Feature = QFlags<FeatureFlag>;

   explicit QDesignerIntegrationInterface(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
   virtual ~QDesignerIntegrationInterface();

   QDesignerFormEditorInterface *core() const;

   virtual QWidget *containerWindow(QWidget *widget) const = 0;

   // Create a resource browser specific to integration. Language integration takes precedence
   virtual QDesignerResourceBrowserInterface *createResourceBrowser(QWidget *parent = nullptr) = 0;
   virtual QString headerSuffix() const = 0;
   virtual void setHeaderSuffix(const QString &headerSuffix) = 0;

   virtual bool isHeaderLowercase() const = 0;
   virtual void setHeaderLowercase(bool headerLowerCase) = 0;

   virtual Feature features() const = 0;
   bool hasFeature(Feature f) const;

   virtual ResourceFileWatcherBehaviour resourceFileWatcherBehaviour() const = 0;
   virtual void setResourceFileWatcherBehaviour(ResourceFileWatcherBehaviour behaviour) = 0;

   virtual QString contextHelpId() const = 0;

   void emitObjectNameChanged(QDesignerFormWindowInterface *formWindow, QObject *object,
      const QString &newName, const QString &oldName);

   void emitNavigateToSlot(const QString &objectName, const QString &signalSignature, const QStringList &parameterNames);
   void emitNavigateToSlot(const QString &slotSignature);

   CS_SIGNAL_1(Public, void propertyChanged(QDesignerFormWindowInterface *formWindow, const QString &name, const QVariant &value))
   CS_SIGNAL_2(propertyChanged, formWindow, name, value)

   CS_SIGNAL_1(Public, void objectNameChanged(QDesignerFormWindowInterface *formWindow, QObject *object, const QString &newName,
         const QString &oldName))
   CS_SIGNAL_2(objectNameChanged, formWindow, object, newName, oldName)

   CS_SIGNAL_1(Public, void navigateToSlot(const QString &objectName, const QString &signalSignature,
         const QStringList &parameterNames))
   CS_SIGNAL_OVERLOAD(navigateToSlot, (const QString &, const QString &, const QStringList &), objectName,
         signalSignature, parameterNames)

   CS_SIGNAL_1(Public, void navigateToSlot(const QString &slotSignature))
   CS_SIGNAL_OVERLOAD(navigateToSlot, (const QString &), slotSignature)

   CS_SLOT_1(Public, virtual void setFeatures(Feature f) = 0)
   CS_SLOT_2(setFeatures)

   CS_SLOT_1(Public, virtual void updateProperty(const QString &name, const QVariant &value, bool enableSubPropertyHandling) = 0)
   CS_SLOT_OVERLOAD(updateProperty, (const QString &, const QVariant &, bool))

   CS_SLOT_1(Public, virtual void updateProperty(const QString &name, const QVariant &value) = 0)
   CS_SLOT_OVERLOAD(updateProperty, (const QString &, const QVariant &))

   // Additional signals of designer property editor
   CS_SLOT_1(Public, virtual void resetProperty(const QString &name) = 0)
   CS_SLOT_2(resetProperty)

   CS_SLOT_1(Public, virtual void addDynamicProperty(const QString &name, const QVariant &value) = 0)
   CS_SLOT_2(addDynamicProperty)

   CS_SLOT_1(Public, virtual void removeDynamicProperty(const QString &name) = 0)
   CS_SLOT_2(removeDynamicProperty)

   CS_SLOT_1(Public, virtual void updateActiveFormWindow(QDesignerFormWindowInterface *formWindow) = 0)
   CS_SLOT_2(updateActiveFormWindow)

   CS_SLOT_1(Public, virtual void setupFormWindow(QDesignerFormWindowInterface *formWindow) = 0)
   CS_SLOT_2(setupFormWindow)

   CS_SLOT_1(Public, virtual void updateSelection() = 0)
   CS_SLOT_2(updateSelection)

   // emerald - temporary hold, plugin
   //    CS_SLOT_1(Public, virtual void updateCustomWidgetPlugins() = 0)
   //    CS_SLOT_2(updateCustomWidgetPlugins)

 private:
   QScopedPointer<QDesignerIntegrationInterfacePrivate> d;
};

class QDesignerIntegration: public QDesignerIntegrationInterface
{
   CS_OBJECT(QDesignerIntegration)

 public:
   explicit QDesignerIntegration(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
   virtual ~QDesignerIntegration();

   QString headerSuffix() const override;
   void setHeaderSuffix(const QString &headerSuffix) override;

   bool isHeaderLowercase() const override;
   void setHeaderLowercase(bool headerLowerCase) override;

   Feature features() const override;
   void setFeatures(Feature f) override;

   ResourceFileWatcherBehaviour resourceFileWatcherBehaviour() const override;
   void setResourceFileWatcherBehaviour(ResourceFileWatcherBehaviour behaviour) override;

   QWidget *containerWindow(QWidget *widget) const override;

   // Load plugins into widget database and factory

   // emerald - temporary hold, plugins
   // static void initializePlugins(QDesignerFormEditorInterface *formEditor);

   // Create a resource browser specific to integration. Language integration takes precedence
   QDesignerResourceBrowserInterface *createResourceBrowser(QWidget *parent = nullptr) override;

   QString contextHelpId() const override;

   void updateProperty(const QString &name, const QVariant &value, bool enableSubPropertyHandling) override;
   void updateProperty(const QString &name, const QVariant &value) override;

   // Additional signals of designer property editor
   void resetProperty(const QString &name) override;
   void addDynamicProperty(const QString &name, const QVariant &value) override;
   void removeDynamicProperty(const QString &name) override;

   void updateActiveFormWindow(QDesignerFormWindowInterface *formWindow) override;
   void setupFormWindow(QDesignerFormWindowInterface *formWindow) override;
   void updateSelection() override;

   // emerald - temporary hold, customer widgets
   // void updateCustomWidgetPlugins() override;

 private:
   QScopedPointer<qdesigner_internal::QDesignerIntegrationPrivate> d;
};

#endif
