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

#ifndef UILIBPROPERTIES_H
#define UILIBPROPERTIES_H

#include <formbuilderextra.h>
#include <utils.h>

class QAbstractFormBuilder;
class DomProperty;

#include <QObject>
#include <QMetaProperty>
#include <QLocale>
#include <QCoreApplication>
#include <QWidget>

DomProperty *variantToDomProperty(QAbstractFormBuilder *abstractFormBuilder,
   const QMetaObject &meta, const QString &propertyName, const QVariant &value);

QVariant domPropertyToVariant(const DomProperty *property);

QVariant domPropertyToVariant(QAbstractFormBuilder *abstractFormBuilder,
   const QMetaObject &meta, const DomProperty *property);

// This class exists to provide meta information for enumerations only
class QAbstractFormBuilderGadget: public QWidget
{
   CS_OBJECT(QAbstractFormBuilderGadget)

   CS_PROPERTY_READ(itemFlags,          fakeItemFlags)
   CS_PROPERTY_READ(checkState,         fakeCheckState)
   CS_PROPERTY_READ(textAlignment,      fakeAlignment)
   CS_PROPERTY_READ(orientation,        fakeOrientation)
   CS_PROPERTY_READ(sizeType,           fakeSizeType)
   CS_PROPERTY_READ(colorRole,          fakeColorRole)
   CS_PROPERTY_READ(colorGroup,         fakeColorGroup)
   CS_PROPERTY_READ(styleStrategy,      fakeStyleStrategy)
   CS_PROPERTY_READ(cursorShape,        fakeCursorShape)
   CS_PROPERTY_READ(brushStyle,         fakeBrushStyle)
   CS_PROPERTY_READ(toolBarArea,        fakeToolBarArea)
   CS_PROPERTY_READ(gradientType,       fakeGradientType)
   CS_PROPERTY_READ(gradientSpread,     fakeGradientSpread)
   CS_PROPERTY_READ(gradientCoordinate, fakeGradientCoordinate)
   CS_PROPERTY_READ(language,           fakeLanguage)
   CS_PROPERTY_READ(country,            fakeCountry)

 public:
   QAbstractFormBuilderGadget() {
      Q_ASSERT(0);
   }

   Qt::Orientation fakeOrientation() const {
      Q_ASSERT(0);
      return Qt::Horizontal;
   }

   QSizePolicy::Policy fakeSizeType() const {
      Q_ASSERT(0);
      return QSizePolicy::Expanding;
   }

   QPalette::ColorGroup fakeColorGroup() const {
      Q_ASSERT(0);
      return static_cast<QPalette::ColorGroup>(0);
   }

   QPalette::ColorRole fakeColorRole() const {
      Q_ASSERT(0);
      return static_cast<QPalette::ColorRole>(0);
   }

   QFont::StyleStrategy fakeStyleStrategy() const {
      Q_ASSERT(0);
      return QFont::PreferDefault;
   }

   Qt::CursorShape fakeCursorShape() const {
      Q_ASSERT(0);
      return Qt::ArrowCursor;
   }

   Qt::BrushStyle fakeBrushStyle() const {
      Q_ASSERT(0);
      return Qt::NoBrush;
   }

   Qt::ToolBarArea fakeToolBarArea() const {
      Q_ASSERT(0);
      return Qt::NoToolBarArea;
   }

   QGradient::Type fakeGradientType() const {
      Q_ASSERT(0);
      return QGradient::NoGradient;
   }

   QGradient::Spread fakeGradientSpread() const  {
      Q_ASSERT(0);
      return QGradient::PadSpread;
   }

   QGradient::CoordinateMode fakeGradientCoordinate() const  {
      Q_ASSERT(0);
      return QGradient::LogicalMode;
   }

   QLocale::Language fakeLanguage() const {
      Q_ASSERT(0);
      return QLocale::C;
   }

   QLocale::Country fakeCountry() const {
      Q_ASSERT(0);
      return QLocale::AnyCountry;
   }

   Qt::ItemFlags fakeItemFlags() const {
      Q_ASSERT(0);
      return Qt::NoItemFlags;
   }

   Qt::CheckState fakeCheckState() const {
      Q_ASSERT(0);
      return Qt::Unchecked;
   }

   Qt::Alignment fakeAlignment() const {
      Q_ASSERT(0);
      return Qt::AlignLeft;
   }
};

// Convert key to value for a given QMetaEnum
template <class E>
inline E enumKeyToValue(const QMetaEnum &metaEnum, const QString &key, const E * = nullptr)
{
   int val = metaEnum.keyToValue(key);

   if (val == -1) {
      csWarning(QCoreApplication::translate("QFormBuilder",
            "The enumeration value '%1' is invalid, default value '%2' will be used instead.")
            .formatArg(key).formatArg(metaEnum.key(0)));

      val = metaEnum.value(0);
   }

   return static_cast<E>(val);
}

// Convert keys to value for a given QMetaEnum
template <class E>
inline E enumKeysToValue(const QMetaEnum &metaEnum, const QString &keys, const E * = nullptr)
{
   int val = metaEnum.keysToValue(keys);

   if (val == -1) {
      csWarning(QCoreApplication::translate("QFormBuilder",
            "The flag value '%1' is invalid, zero will be used instead.").formatArg(keys));
      val = 0;
   }

   return static_cast<E>(QFlag(val));
}

// Access meta enumeration object of a qobject
template <class T>
inline QMetaEnum metaEnum(const QString &name, const T * = nullptr)
{
   const int index = T::staticMetaObject().indexOfProperty(name);
   Q_ASSERT(index != -1);

   return T::staticMetaObject().property(index).enumerator();
}

// Convert key to value for enumeration by name
template <class T, class E>
inline E enumKeyOfObjectToValue(const QString &enumName, const QString &key, const T * = nullptr, const E * = nullptr)
{
   const QMetaEnum me = metaEnum<T>(enumName);
   return enumKeyToValue<E>(me, key);
}

#endif
