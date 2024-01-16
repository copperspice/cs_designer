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

#include <designer_property.h>
#include <designer_utils.h>
#include <edit_property_manager.h>
#include <font_property.h>
#include <property_view_utils.h>

#include <QCoreApplication>
#include <QVariant>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>

namespace qdesigner_internal {

static const char *aliasingC[] = {
   cs_mark_tr("FontPropertyManager", "PreferDefault"),
   cs_mark_tr("FontPropertyManager", "NoAntialias"),
   cs_mark_tr("FontPropertyManager", "PreferAntialias")
};

FontPropertyManager::FontPropertyManager()
   : m_createdFontProperty(nullptr)
{
   const int nameCount = sizeof(aliasingC) / sizeof(const char *);

   for (int  i = 0; i < nameCount; i++) {
      m_aliasingEnumNames.push_back(QCoreApplication::translate("FontPropertyManager", aliasingC[i]));
   }
}

void FontPropertyManager::preInitializeProperty(QtProperty *property,
   int type,
   ResetMap &resetMap)
{
   if (m_createdFontProperty) {
      PropertyToSubPropertiesMap::iterator it = m_propertyToFontSubProperties.find(m_createdFontProperty);
      if (it == m_propertyToFontSubProperties.end()) {
         it = m_propertyToFontSubProperties.insert(m_createdFontProperty, PropertyList());
      }
      const int index = it.value().size();
      m_fontSubPropertyToFlag.insert(property, index);
      it.value().push_back(property);
      m_fontSubPropertyToProperty[property] = m_createdFontProperty;
      resetMap[property] = true;
   }

   if (type == QVariant::Font) {
      m_createdFontProperty = property;
   }
}

// Map the font family names to display names retrieved from the XML configuration
static QStringList designerFamilyNames(QStringList families, const FontPropertyManager::NameMap &nm)
{
   if (nm.empty()) {
      return families;
   }

   const FontPropertyManager::NameMap::const_iterator ncend = nm.constEnd();
   const QStringList::iterator end = families.end();
   for (QStringList::iterator it = families.begin(); it != end; ++it) {
      const FontPropertyManager::NameMap::const_iterator nit = nm.constFind(*it);
      if (nit != ncend) {
         *it = nit.value();
      }
   }
   return families;
}

void FontPropertyManager::postInitializeProperty(QtVariantPropertyManager *vm,
      QtProperty *property, int type, int enumTypeId)
{
   if (type != QVariant::Font) {
      return;
   }

   // This will cause a recursion
   QtVariantProperty *antialiasing = vm->addProperty(enumTypeId, QCoreApplication::translate("FontPropertyManager", "Antialiasing"));

   QVariant data = vm->variantProperty(property)->value();
   const QFont font = data.value<QFont>();

   antialiasing->setAttribute(QString("enumNames"), m_aliasingEnumNames);
   antialiasing->setValue(antialiasingToIndex(font.styleStrategy()));
   property->addSubProperty(antialiasing);

   m_propertyToAntialiasing[property] = antialiasing;
   m_antialiasingToProperty[antialiasing] = property;

   // Fiddle family names
   if (! m_familyMappings.empty()) {
      const PropertyToSubPropertiesMap::iterator it = m_propertyToFontSubProperties.find(m_createdFontProperty);
      QtVariantProperty *familyProperty = vm->variantProperty(it.value().front());
      const QString enumNamesAttribute = QString("enumNames");
      QStringList plainFamilyNames = familyProperty->attributeValue(enumNamesAttribute).toStringList();

      // Did someone load fonts or something?
      if (m_designerFamilyNames.size() != plainFamilyNames.size()) {
         m_designerFamilyNames = designerFamilyNames(plainFamilyNames, m_familyMappings);
      }
      familyProperty->setAttribute(enumNamesAttribute, m_designerFamilyNames);
   }

   // Next
   m_createdFontProperty = nullptr;
}

bool FontPropertyManager::uninitializeProperty(QtProperty *property)
{
   const PropertyToPropertyMap::iterator ait =  m_propertyToAntialiasing.find(property);

   if (ait != m_propertyToAntialiasing.end()) {
      QtProperty *antialiasing = ait.value();
      m_antialiasingToProperty.remove(antialiasing);
      m_propertyToAntialiasing.erase(ait);
      delete antialiasing;
   }

   PropertyToSubPropertiesMap::iterator sit = m_propertyToFontSubProperties.find(property);
   if (sit == m_propertyToFontSubProperties.end()) {
      return false;
   }

   m_propertyToFontSubProperties.erase(sit);
   m_fontSubPropertyToFlag.remove(property);
   m_fontSubPropertyToProperty.remove(property);

   return true;
}

void FontPropertyManager::slotPropertyDestroyed(QtProperty *property)
{
   removeAntialiasingProperty(property);
}

void FontPropertyManager::removeAntialiasingProperty(QtProperty *property)
{
   const PropertyToPropertyMap::iterator ait =  m_antialiasingToProperty.find(property);
   if (ait == m_antialiasingToProperty.end()) {
      return;
   }

   m_propertyToAntialiasing[ait.value()] = nullptr;
   m_antialiasingToProperty.erase(ait);
}

bool FontPropertyManager::resetFontSubProperty(QtVariantPropertyManager *vm, QtProperty *property)
{
   const PropertyToPropertyMap::iterator it = m_fontSubPropertyToProperty.find(property);
   if (it == m_fontSubPropertyToProperty.end()) {
      return false;
   }

   QtVariantProperty *fontProperty = vm->variantProperty(it.value());

   QVariant v = fontProperty->value();

   QFont font = v.value<QFont>();
   unsigned mask = font.resolve();
   const unsigned flag = fontFlag(m_fontSubPropertyToFlag.value(property));

   mask &= ~flag;
   font.resolve(mask);
   v.setValue(font);
   fontProperty->setValue(v);
   return true;
}

int FontPropertyManager::antialiasingToIndex(QFont::StyleStrategy antialias)
{
   switch (antialias) {
      case QFont::PreferDefault:
         return 0;

      case QFont::NoAntialias:
         return 1;

      case QFont::PreferAntialias:
         return 2;

      default:
         break;
   }

   return 0;
}

QFont::StyleStrategy FontPropertyManager::indexToAntialiasing(int idx)
{
   switch (idx) {
      case 0:
         return QFont::PreferDefault;
      case 1:
         return QFont::NoAntialias;
      case 2:
         return QFont::PreferAntialias;
   }
   return QFont::PreferDefault;
}

unsigned FontPropertyManager::fontFlag(int idx)
{
   switch (idx) {
      case 0:
         return QFont::FamilyResolved;
      case 1:
         return QFont::SizeResolved;
      case 2:
         return QFont::WeightResolved;
      case 3:
         return QFont::StyleResolved;
      case 4:
         return QFont::UnderlineResolved;
      case 5:
         return QFont::StrikeOutResolved;
      case 6:
         return QFont::KerningResolved;
      case 7:
         return QFont::StyleStrategyResolved;
   }

   return 0;
}

int FontPropertyManager::valueChanged(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value)
{
   QtProperty *antialiasingProperty = m_antialiasingToProperty.value(property, nullptr);

   if (! antialiasingProperty) {
      if (m_propertyToFontSubProperties.contains(property)) {
         updateModifiedState(property, value);
      }
      return DesignerPropertyManager::NoMatch;
   }

   QtVariantProperty *fontProperty = vm->variantProperty(antialiasingProperty);
   const QFont::StyleStrategy newValue = indexToAntialiasing(value.toInt());

   QVariant data = fontProperty->value();
   QFont font = data.value<QFont>();

   const QFont::StyleStrategy oldValue = font.styleStrategy();
   if (newValue == oldValue) {
      return DesignerPropertyManager::Unchanged;
   }

   font.setStyleStrategy(newValue);
   fontProperty->setValue(QVariant::fromValue(font));
   return DesignerPropertyManager::Changed;
}

void FontPropertyManager::updateModifiedState(QtProperty *property, const QVariant &value)
{
   const PropertyToSubPropertiesMap::iterator it = m_propertyToFontSubProperties.find(property);
   if (it == m_propertyToFontSubProperties.end()) {
      return;
   }

   const PropertyList &subProperties = it.value();

   QFont font = value.value<QFont>();
   const unsigned mask = font.resolve();

   const int count = subProperties.size();
   for (int index = 0; index < count; index++) {
      const unsigned flag = fontFlag(index);
      subProperties.at(index)->setModified(mask & flag);
   }
}

void FontPropertyManager::setValue(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value)
{
   updateModifiedState(property, value);

   if (QtProperty *antialiasingProperty = m_propertyToAntialiasing.value(property, nullptr)) {
      QtVariantProperty *antialiasing = vm->variantProperty(antialiasingProperty);

      if (antialiasing) {
         QFont font = value.value<QFont>();
         antialiasing->setValue(antialiasingToIndex(font.styleStrategy()));
      }
   }
}

}   // end namespace

