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

#ifndef QDESIGNER_PROPERTYSHEET_H
#define QDESIGNER_PROPERTYSHEET_H

#include <default_extensionfactory.h>
#include <dynamicpropertysheet.h>
#include <extension.h>
#include <extension_manager.h>
#include <propertysheet.h>

#include <QPair>
#include <QPointer>
#include <QVariant>

class QLayout;
class QDesignerFormEditorInterface;
class QDesignerPropertySheetPrivate;

namespace qdesigner_internal {
class DesignerPixmapCache;
class DesignerIconCache;
class FormWindowBase;
}

class QDesignerPropertySheet: public QObject, public QDesignerPropertySheetExtension, public QDesignerDynamicPropertySheetExtension
{
   CS_OBJECT(QDesignerPropertySheet)

   CS_INTERFACES(QDesignerPropertySheetExtension, QDesignerDynamicPropertySheetExtension)

 public:
   explicit QDesignerPropertySheet(QObject *object, QObject *parent = nullptr);
   virtual ~QDesignerPropertySheet();

   int indexOf(const QString &name) const override;

   int count() const override;
   QString propertyName(int index) const override;

   QString propertyGroup(int index) const override;
   void setPropertyGroup(int index, const QString &group) override;

   bool hasReset(int index) const override;
   bool reset(int index) override;

   bool isAttribute(int index) const override;
   void setAttribute(int index, bool b) override;

   bool isVisible(int index) const override;
   void setVisible(int index, bool b) override;

   QVariant property(int index) const override;
   void setProperty(int index, const QVariant &value) override;

   bool isChanged(int index) const override;

   void setChanged(int index, bool changed) override;

   bool dynamicPropertiesAllowed() const override;
   int addDynamicProperty(const QString &propertyName, const QVariant &value) override;
   bool removeDynamicProperty(int index) override;
   bool isDynamicProperty(int index) const override;
   bool canAddDynamicProperty(const QString &propertyName) const override;

   bool isDefaultDynamicProperty(int index) const;

   bool isResourceProperty(int index) const;
   QVariant defaultResourceProperty(int index) const;

   qdesigner_internal::DesignerPixmapCache *pixmapCache() const;
   void setPixmapCache(qdesigner_internal::DesignerPixmapCache *cache);
   qdesigner_internal::DesignerIconCache *iconCache() const;
   void setIconCache(qdesigner_internal::DesignerIconCache *cache);
   int createFakeProperty(const QString &propertyName, const QVariant &value = QVariant());

   bool isEnabled(int index) const override;
   QObject *object() const;

   static bool internalDynamicPropertiesEnabled();
   static void setInternalDynamicPropertiesEnabled(bool v);

 protected:
   bool isAdditionalProperty(int index) const;
   bool isFakeProperty(int index) const;
   QVariant resolvePropertyValue(int index, const QVariant &value) const;
   QVariant metaProperty(int index) const;
   void setFakeProperty(int index, const QVariant &value);
   void clearFakeProperties();

   bool isFakeLayoutProperty(int index) const;
   bool isDynamic(int index) const;
   qdesigner_internal::FormWindowBase *formWindowBase() const;
   QDesignerFormEditorInterface *core() const;

 public:
   enum PropertyType {
      PropertyNone,                                    // 0
      PropertyLayoutObjectName,
      PropertyLayoutLeftMargin,
      PropertyLayoutTopMargin,
      PropertyLayoutRightMargin,
      PropertyLayoutBottomMargin,
      PropertyLayoutSpacing,
      PropertyLayoutHorizontalSpacing,
      PropertyLayoutVerticalSpacing,
      PropertyLayoutSizeConstraint,
      PropertyLayoutFieldGrowthPolicy,                  // 10
      PropertyLayoutRowWrapPolicy,
      PropertyLayoutLabelAlignment,
      PropertyLayoutFormAlignment,
      PropertyLayoutBoxStretch,
      PropertyLayoutGridRowStretch,
      PropertyLayoutGridColumnStretch,
      PropertyLayoutGridRowMinimumHeight,
      PropertyLayoutGridColumnMinimumWidth,
      PropertyBuddy,                                    // 19
      PropertyAccessibility,
      PropertyGeometry,
      PropertyCheckable,                                // 22
      PropertyWindowTitle,
      PropertyWindowIcon,
      PropertyWindowFilePath,
      PropertyWindowOpacity,
      PropertyWindowIconText,
      PropertyWindowModality,
      PropertyWindowModified,
      PropertyStyleSheet,                                // 30
      PropertyText
   };

   enum ObjectType {
      ObjectNone,
      ObjectLabel,
      ObjectLayout,
      ObjectLayoutWidget
   };

   static ObjectType objectTypeFromObject(const QObject *o);
   static PropertyType propertyTypeFromName(const QString &name);

 protected:
   PropertyType propertyType(int index) const;
   ObjectType objectType() const;

 private:
   QDesignerPropertySheetPrivate *d;
};

/* Abstract base class for factories that register a property sheet that implements
 * both QDesignerPropertySheetExtension and QDesignerDynamicPropertySheetExtension
 * by multiple inheritance. The factory maintains ownership of
 * the extension and returns it for both id's. */

class QDesignerAbstractPropertySheetFactory: public QExtensionFactory
{
   CS_OBJECT(QDesignerAbstractPropertySheetFactory)
   CS_INTERFACES(QAbstractExtensionFactory)

 public:
   explicit QDesignerAbstractPropertySheetFactory(QExtensionManager *parent = nullptr);
   virtual ~QDesignerAbstractPropertySheetFactory();

   QObject *extension(QObject *object, const QString &iid) const override;

 private:
   CS_SLOT_1(Private, void objectDestroyed(QObject *object))
   CS_SLOT_2(objectDestroyed)

   virtual QObject *createPropertySheet(QObject *qObject, QObject *parent) const = 0;

   struct PropertySheetFactoryPrivate;
   PropertySheetFactoryPrivate *m_impl;
};

/* Convenience factory template for property sheets that implement
 * QDesignerPropertySheetExtension and QDesignerDynamicPropertySheetExtension
 * by multiple inheritance. */

template <class Object, class PropertySheet>
class QDesignerPropertySheetFactory : public QDesignerAbstractPropertySheetFactory
{
 public:
   explicit QDesignerPropertySheetFactory(QExtensionManager *parent = nullptr);

   static void registerExtension(QExtensionManager *mgr);

 private:
   // Does a  qobject_cast on  the object.
   QObject *createPropertySheet(QObject *qObject, QObject *parent) const override;
};

template <class Object, class PropertySheet> QDesignerPropertySheetFactory<Object, PropertySheet>::QDesignerPropertySheetFactory(
   QExtensionManager *parent) :
   QDesignerAbstractPropertySheetFactory(parent)
{
}

template <class Object, class PropertySheet>
QObject *QDesignerPropertySheetFactory<Object, PropertySheet>::createPropertySheet(QObject *qObject, QObject *parent) const
{
   Object *object = dynamic_cast<Object *>(qObject);
   if (!object) {
      return nullptr;
   }
   return new PropertySheet(object, parent);
}

template <class Object, class PropertySheet>
void QDesignerPropertySheetFactory<Object, PropertySheet>::registerExtension(QExtensionManager *mgr)
{
   QDesignerPropertySheetFactory *factory = new QDesignerPropertySheetFactory(mgr);
   mgr->registerExtensions(factory, CS_TYPEID(QDesignerPropertySheetExtension));
   mgr->registerExtensions(factory, CS_TYPEID(QDesignerDynamicPropertySheetExtension));
}


// Standard property sheet
typedef QDesignerPropertySheetFactory<QObject, QDesignerPropertySheet> QDesignerDefaultPropertySheetFactory;

#endif
