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

#include <brush_property.h>
#include <designer_property.h>
#include <edit_property_manager.h>
#include <property_view_utils.h>

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QVariant>

static const QStringList brushStyles = {
   cs_mark_string_tr("BrushPropertyManager", "No brush"),
   cs_mark_string_tr("BrushPropertyManager", "Solid"),
   cs_mark_string_tr("BrushPropertyManager", "Dense 1"),
   cs_mark_string_tr("BrushPropertyManager", "Dense 2"),
   cs_mark_string_tr("BrushPropertyManager", "Dense 3"),
   cs_mark_string_tr("BrushPropertyManager", "Dense 4"),
   cs_mark_string_tr("BrushPropertyManager", "Dense 5"),
   cs_mark_string_tr("BrushPropertyManager", "Dense 6"),
   cs_mark_string_tr("BrushPropertyManager", "Dense 7"),
   cs_mark_string_tr("BrushPropertyManager", "Horizontal"),
   cs_mark_string_tr("BrushPropertyManager", "Vertical"),
   cs_mark_string_tr("BrushPropertyManager", "Cross"),
   cs_mark_string_tr("BrushPropertyManager", "Backward diagonal"),
   cs_mark_string_tr("BrushPropertyManager", "Forward diagonal"),
   cs_mark_string_tr("BrushPropertyManager", "Crossing diagonal"),
};

namespace qdesigner_internal {

static void clearBrushIcons();

BrushPropertyManager::BrushPropertyManager()
{
}

int BrushPropertyManager::brushStyleToIndex(Qt::BrushStyle st)
{
   switch (st) {
      case Qt::NoBrush:
         return 0;

      case Qt::SolidPattern:
         return 1;

      case Qt::Dense1Pattern:
         return 2;

      case Qt::Dense2Pattern:
         return 3;

      case Qt::Dense3Pattern:
         return 4;

      case Qt::Dense4Pattern:
         return 5;

      case Qt::Dense5Pattern:
         return 6;

      case Qt::Dense6Pattern:
         return 7;

      case Qt::Dense7Pattern:
         return 8;

      case Qt::HorPattern:
         return 9;

      case Qt::VerPattern:
         return 10;

      case Qt::CrossPattern:
         return 11;

      case Qt::BDiagPattern:
         return 12;

      case Qt::FDiagPattern:
         return 13;

      case Qt::DiagCrossPattern:
         return 14;

      default:
         break;
   }

   return 0;
}

Qt::BrushStyle BrushPropertyManager::brushStyleIndexToStyle(int brushStyleIndex)
{
   switch (brushStyleIndex) {
      case  0:
         return Qt::NoBrush;
      case  1:
         return Qt::SolidPattern;
      case  2:
         return Qt::Dense1Pattern;
      case  3:
         return Qt::Dense2Pattern;
      case  4:
         return Qt::Dense3Pattern;
      case  5:
         return Qt::Dense4Pattern;
      case  6:
         return Qt::Dense5Pattern;
      case  7:
         return Qt::Dense6Pattern;
      case  8:
         return Qt::Dense7Pattern;
      case  9:
         return Qt::HorPattern;
      case 10:
         return Qt::VerPattern;
      case 11:
         return Qt::CrossPattern;
      case 12:
         return Qt::BDiagPattern;
      case 13:
         return Qt::FDiagPattern;
      case 14:
         return Qt::DiagCrossPattern;
   }

   return Qt::NoBrush;
}

namespace {

class EnumIndexIconMap : public QMap<int, QIcon>
{
 public:
   EnumIndexIconMap() {
      qAddPostRoutine(&clearBrushIcons);
   }
};

}

Q_GLOBAL_STATIC(EnumIndexIconMap, brushIcons)

static void clearBrushIcons()
{
   brushIcons()->clear();
}

static QString brushTr(const QString &text)
{
   return QCoreApplication::translate("BrushPropertyManager", text);
}

const BrushPropertyManager::EnumIndexIconMap &BrushPropertyManager::brushStyleIcons()
{
   // Create a map of icons for the brush style editor
   if (brushIcons()->empty()) {
      const int brushStyleCount = std::size(brushStyles);

      QBrush brush(Qt::black);
      const QIcon solidIcon = QtPropertyBrowserUtils::brushValueIcon(brush);

      for (int i = 0; i < brushStyleCount; i++) {
         const Qt::BrushStyle style = brushStyleIndexToStyle(i);
         brush.setStyle(style);
         brushIcons()->insert(i, QtPropertyBrowserUtils::brushValueIcon(brush));
      }
   }

   return *(brushIcons());
}

QString BrushPropertyManager::brushStyleIndexToString(int brushStyleIndex)
{
   const int brushStyleCount = std::size(brushStyles);

   if (brushStyleIndex < brushStyleCount) {
      return brushTr(brushStyles[brushStyleIndex]);

   } else {
      return QString();

   }
}

void BrushPropertyManager::initializeProperty(QtVariantPropertyManager *vm, QtProperty *property, int enumTypeId)
{
   m_brushValues.insert(property, QBrush());

   // style
   QtVariantProperty *styleSubProperty = vm->addProperty(enumTypeId, brushTr("Style"));
   property->addSubProperty(styleSubProperty);

   QStringList styles;

   for (QString item : brushStyles) {
      styles.push_back(brushTr(item));
   }

   styleSubProperty->setAttribute("enumNames", styles);

//   broom - put back when QVariant operator= resolved
//   styleSubProperty->setAttribute("enumIcons", QVariant::fromValue(brushStyleIcons()));

   m_brushPropertyToStyleSubProperty.insert(property, styleSubProperty);
   m_brushStyleSubPropertyToProperty.insert(styleSubProperty, property);

   // color
   QtVariantProperty *colorSubProperty = vm->addProperty(QVariant::Color, brushTr("Color"));
   property->addSubProperty(colorSubProperty);
   m_brushPropertyToColorSubProperty.insert(property, colorSubProperty);
   m_brushColorSubPropertyToProperty.insert(colorSubProperty, property);
}

bool BrushPropertyManager::uninitializeProperty(QtProperty *property)
{
   auto brit = m_brushValues.find(property); // Brushes

   if (brit == m_brushValues.end()) {
      return false;
   }

   m_brushValues.erase(brit);

   // style
   PropertyToPropertyMap::iterator subit = m_brushPropertyToStyleSubProperty.find(property);

   if (subit != m_brushPropertyToStyleSubProperty.end()) {
      QtProperty *styleProp = subit.value();
      m_brushStyleSubPropertyToProperty.remove(styleProp);
      m_brushPropertyToStyleSubProperty.erase(subit);
      delete styleProp;
   }

   // color
   subit = m_brushPropertyToColorSubProperty.find(property);
   if (subit != m_brushPropertyToColorSubProperty.end()) {
      QtProperty *colorProp = subit.value();
      m_brushColorSubPropertyToProperty.remove(colorProp);
      m_brushPropertyToColorSubProperty.erase(subit);
      delete colorProp;
   }
   return true;
}

void BrushPropertyManager::slotPropertyDestroyed(QtProperty *property)
{
   PropertyToPropertyMap::iterator subit = m_brushStyleSubPropertyToProperty.find(property);
   if (subit != m_brushStyleSubPropertyToProperty.end()) {
      m_brushPropertyToStyleSubProperty[subit.value()] = nullptr;
      m_brushStyleSubPropertyToProperty.erase(subit);
   }

   subit = m_brushColorSubPropertyToProperty.find(property);
   if (subit != m_brushColorSubPropertyToProperty.end()) {
      m_brushPropertyToColorSubProperty[subit.value()] = nullptr;
      m_brushColorSubPropertyToProperty.erase(subit);
   }
}

int BrushPropertyManager::valueChanged(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value)
{
   switch (value.type()) {
      case QVariant::Int: // Style subproperty?
         if (QtProperty *brushProperty = m_brushStyleSubPropertyToProperty.value(property, nullptr)) {
            const QBrush oldValue = m_brushValues.value(brushProperty);

            QBrush newBrush = oldValue;
            const int index = value.toInt();
            newBrush.setStyle(brushStyleIndexToStyle(index));

            if (newBrush == oldValue) {
               return DesignerPropertyManager::Unchanged;
            }
            vm->variantProperty(brushProperty)->setValue(newBrush);
            return DesignerPropertyManager::Changed;
         }
         break;

      case QVariant::Color: // Color  subproperty?
         if (QtProperty *brushProperty = m_brushColorSubPropertyToProperty.value(property, nullptr)) {
            const QBrush oldValue = m_brushValues.value(brushProperty);

            QBrush newBrush = oldValue;
            newBrush.setColor(value.value<QColor>());

            if (newBrush == oldValue) {
               return DesignerPropertyManager::Unchanged;
            }
            vm->variantProperty(brushProperty)->setValue(newBrush);
            return DesignerPropertyManager::Changed;
         }
         break;

      default:
         break;
   }
   return DesignerPropertyManager::NoMatch;
}

int BrushPropertyManager::setValue(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value)
{
   if (value.type() != QVariant::Brush) {
      return DesignerPropertyManager::NoMatch;
   }

   const PropertyBrushMap::iterator brit = m_brushValues.find(property);

   if (brit == m_brushValues.end()) {
      return DesignerPropertyManager::NoMatch;
   }

   const QBrush newBrush = value.value<QBrush>();

   if (newBrush == brit.value()) {
      return DesignerPropertyManager::Unchanged;
   }
   brit.value() = newBrush;
   if (QtProperty *styleProperty = m_brushPropertyToStyleSubProperty.value(property)) {
      vm->variantProperty(styleProperty)->setValue(brushStyleToIndex(newBrush.style()));
   }
   if (QtProperty *colorProperty = m_brushPropertyToColorSubProperty.value(property)) {
      vm->variantProperty(colorProperty)->setValue(newBrush.color());
   }

   return DesignerPropertyManager::Changed;
}

bool BrushPropertyManager::valueText(const QtProperty *property, QString *text) const
{
   const PropertyBrushMap::const_iterator brit = m_brushValues.constFind(const_cast<QtProperty *>(property));
   if (brit == m_brushValues.constEnd()) {
      return false;
   }

   const QBrush &brush = brit.value();
   const QString styleName = brushStyleIndexToString(brushStyleToIndex(brush.style()));

   *text = brushTr("[%1, %2]").formatArg(styleName).formatArg(QtPropertyBrowserUtils::colorValueText(brush.color()));

   return true;
}

bool BrushPropertyManager::valueIcon(const QtProperty *property, QIcon *icon) const
{
   auto brit = m_brushValues.constFind(const_cast<QtProperty *>(property));

   if (brit == m_brushValues.constEnd()) {
      return false;
   }

   *icon = QtPropertyBrowserUtils::brushValueIcon(brit.value());

   return true;
}

bool BrushPropertyManager::value(const QtProperty *property, QVariant *v) const
{
   auto brit = m_brushValues.constFind(const_cast<QtProperty *>(property));

   if (brit == m_brushValues.constEnd()) {
      return false;
   }

   v->setValue(brit.value());

   return true;
}

}

