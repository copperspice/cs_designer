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

#ifndef BRUSH_PROPERTY_H
#define BRUSH_PROPERTY_H

class QtProperty;
class QtVariantPropertyManager;

#include <QMap>
#include <QBrush>
#include <QIcon>
#include <QString>
#include <QVariant>

namespace qdesigner_internal {

// BrushPropertyManager: A mixin for DesignerPropertyManager that manages brush properties.

class BrushPropertyManager
{
   BrushPropertyManager(const BrushPropertyManager &);
   BrushPropertyManager &operator=(const BrushPropertyManager &);

 public:
   BrushPropertyManager();

   void initializeProperty(QtVariantPropertyManager *vm, QtProperty *property, int enumTypeId);
   bool uninitializeProperty(QtProperty *property);

   // Call from slotValueChanged().
   int valueChanged(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value);
   int setValue(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value);

   bool valueText(const QtProperty *property, QString *text) const;
   bool valueIcon(const QtProperty *property, QIcon *icon) const;
   bool value(const QtProperty *property, QVariant *v) const;

   // Call from  QtPropertyManager's propertyDestroyed signal
   void slotPropertyDestroyed(QtProperty *property);

 private:
   static int brushStyleToIndex(Qt::BrushStyle st);
   static Qt::BrushStyle brushStyleIndexToStyle(int brushStyleIndex);
   static QString brushStyleIndexToString(int brushStyleIndex);

   typedef QMap<int, QIcon> EnumIndexIconMap;
   static const EnumIndexIconMap &brushStyleIcons();

   typedef QMap<QtProperty *, QtProperty *> PropertyToPropertyMap;
   PropertyToPropertyMap m_brushPropertyToStyleSubProperty;
   PropertyToPropertyMap m_brushPropertyToColorSubProperty;
   PropertyToPropertyMap m_brushStyleSubPropertyToProperty;
   PropertyToPropertyMap m_brushColorSubPropertyToProperty;

   typedef QMap<QtProperty *, QBrush> PropertyBrushMap;
   PropertyBrushMap m_brushValues;
};

}   // end namespace

#endif
