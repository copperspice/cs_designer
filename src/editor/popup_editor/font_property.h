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

#ifndef FONT_PROPERTY_H
#define FONT_PROPERTY_H

class QtProperty;
class QtVariantPropertyManager;

#include <QMap>
#include <QStringList>
#include <QFont>
#include <QVariant>

namespace qdesigner_internal {

/* FontPropertyManager: A mixin for DesignerPropertyManager that manages font
 * properties. Adds an antialiasing subproperty and reset flags/mask handling
 * for the other subproperties. It also modifies the font family
 * enumeration names, which it reads from an XML mapping file that
 * contains annotations indicating the platform the font is available on. */

class FontPropertyManager
{
 public:
   FontPropertyManager();

   typedef QMap<QtProperty *, bool> ResetMap;
   typedef QMap<QString, QString> NameMap;

   // Call before QtVariantPropertyManager::initializeProperty.
   void preInitializeProperty(QtProperty *property, int type, ResetMap &resetMap);

   // Call after QtVariantPropertyManager::initializeProperty. This will trigger
   // a recursion for the sub properties
   void postInitializeProperty(QtVariantPropertyManager *vm, QtProperty *property, int type, int enumTypeId);

   bool uninitializeProperty(QtProperty *property);

   // Call from  QtPropertyManager's propertyDestroyed signal
   void slotPropertyDestroyed(QtProperty *property);

   bool resetFontSubProperty(QtVariantPropertyManager *vm, QtProperty *subProperty);

   // Call from slotValueChanged(), returns DesignerPropertyManager::ValueChangedResult
   int valueChanged(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value);

   // Call from setValue() before calling setValue() on  QtVariantPropertyManager.
   void setValue(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value);

 private:
   FontPropertyManager(const FontPropertyManager &) = delete;
   FontPropertyManager &operator=(const FontPropertyManager &) = delete;

   typedef QMap<QtProperty *, QtProperty *> PropertyToPropertyMap;
   typedef QList<QtProperty *> PropertyList;
   typedef QMap<QtProperty *, PropertyList>  PropertyToSubPropertiesMap;

   void removeAntialiasingProperty(QtProperty *);
   void updateModifiedState(QtProperty *property, const QVariant &value);
   static int antialiasingToIndex(QFont::StyleStrategy antialias);
   static QFont::StyleStrategy indexToAntialiasing(int idx);
   static unsigned fontFlag(int idx);

   PropertyToPropertyMap m_propertyToAntialiasing;
   PropertyToPropertyMap m_antialiasingToProperty;

   PropertyToSubPropertiesMap m_propertyToFontSubProperties;
   QMap<QtProperty *, int> m_fontSubPropertyToFlag;
   PropertyToPropertyMap m_fontSubPropertyToProperty;
   QtProperty *m_createdFontProperty;
   QStringList m_aliasingEnumNames;

   // Font families with Designer annotations
   QStringList m_designerFamilyNames;
   NameMap m_familyMappings;
};

}   // end namespace - qdesigner_internal


#endif // FONTPROPERTYMANAGER_H
