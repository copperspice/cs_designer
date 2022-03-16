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

#ifndef VARIANTPROPERTY_H
#define VARIANTPROPERTY_H

#include <propertybrowser.h>

class QtVariantPropertyManager;

#include <QBrush>
#include <QChar>
#include <QColor>
#include <QCursor>
#include <QDateTime>
#include <QFont>
#include <QIcon>
#include <QKeySequence>
#include <QLocale>
#include <QMap>
#include <QRect>
#include <QRegularExpression>
#include <QSize>
#include <QSizePolicy>
#include <QStringList>
#include <QVariant>

using QtIconMap = QMap<int, QIcon>;

class QtVariantProperty : public QtProperty
{
 public:
   ~QtVariantProperty();

   QVariant value() const;
   QVariant attributeValue(const QString &attribute) const;
   int valueType() const;
   int propertyType() const;

   void setValue(const QVariant &value);
   void setAttribute(const QString &attribute, const QVariant &value);

 protected:
   QtVariantProperty(QtVariantPropertyManager *manager);

 private:
   friend class QtVariantPropertyManager;
   QScopedPointer<class QtVariantPropertyPrivate> d_ptr;
};

class QtVariantPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtVariantPropertyManager)

 public:
   QtVariantPropertyManager(QObject *parent = nullptr);
   ~QtVariantPropertyManager();

   virtual QtVariantProperty *addProperty(int propertyType, const QString &name = QString());

   int propertyType(const QtProperty *property) const;
   int valueType(const QtProperty *property) const;
   QtVariantProperty *variantProperty(const QtProperty *property) const;

   virtual bool isPropertyTypeSupported(int propertyType) const;
   virtual int valueType(int propertyType) const;
   virtual QStringList attributes(int propertyType) const;
   virtual int attributeType(int propertyType, const QString &attribute) const;

   virtual QVariant value(const QtProperty *property) const;
   virtual QVariant attributeValue(const QtProperty *property, const QString &attribute) const;

   static int enumTypeId();
   static int flagTypeId();
   static int groupTypeId();
   static int iconMapTypeId();

   CS_SLOT_1(Public, virtual void setValue(QtProperty *property, const QVariant &val))
   CS_SLOT_2(setValue)

   CS_SLOT_1(Public, virtual void setAttribute(QtProperty *property, const QString &attribute, const QVariant &value))
   CS_SLOT_2(setAttribute)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QVariant &val))
   CS_SIGNAL_2(valueChanged, property, val)

   CS_SIGNAL_1(Public, void attributeChanged(QtProperty *property, const QString &attribute, const QVariant &val))
   CS_SIGNAL_2(attributeChanged, property, attribute, val)

 protected:
   virtual bool hasValue(const QtProperty *property) const;
   QString valueText(const QtProperty *property) const;
   QIcon valueIcon(const QtProperty *property) const;
   virtual void initializeProperty(QtProperty *property);
   virtual void uninitializeProperty(QtProperty *property);
   virtual QtProperty *createProperty();

 private:
   QScopedPointer<class QtVariantPropertyManagerPrivate> d_ptr;

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, int))

   CS_SLOT_1(Private, void slotRangeChanged(QtProperty *un_named_arg1, int un_named_arg2, int un_named_arg3))
   CS_SLOT_OVERLOAD(slotRangeChanged, (QtProperty *, int, int))

   CS_SLOT_1(Private, void slotSingleStepChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_OVERLOAD(slotSingleStepChanged, (QtProperty *, int))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, double un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, double))

   CS_SLOT_1(Private, void slotRangeChanged(QtProperty *un_named_arg1, double un_named_arg2, double un_named_arg3))
   CS_SLOT_OVERLOAD(slotRangeChanged, (QtProperty *, double, double))

   CS_SLOT_1(Private, void slotSingleStepChanged(QtProperty *un_named_arg1, double un_named_arg2))
   CS_SLOT_OVERLOAD(slotSingleStepChanged, (QtProperty *, double))

   CS_SLOT_1(Private, void slotDecimalsChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotDecimalsChanged)

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, bool un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, bool))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QString &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *,  const QString &))

   CS_SLOT_1(Private, void slotRegExpChanged(QtProperty *un_named_arg1, const QRegularExpression &un_named_arg2))
   CS_SLOT_2(slotRegExpChanged)

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QDate &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *,  const QDate &))

   CS_SLOT_1(Private, void slotRangeChanged(QtProperty *un_named_arg1, const QDate &un_named_arg2, const QDate &un_named_arg3))
   CS_SLOT_OVERLOAD(slotRangeChanged, (QtProperty *, const QDate &, const QDate &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QTime &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QTime &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QDateTime &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QDateTime &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QKeySequence &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QKeySequence &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QChar &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QChar &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QLocale &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QLocale &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QPoint &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QPoint &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QPointF &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QPointF &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QSize &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QSize &))

   CS_SLOT_1(Private, void slotRangeChanged(QtProperty *un_named_arg1, const QSize &un_named_arg2, const QSize &un_named_arg3))
   CS_SLOT_OVERLOAD(slotRangeChanged, (QtProperty *, const QSize &, const QSize &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QSizeF &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QSizeF &))

   CS_SLOT_1(Private, void slotRangeChanged(QtProperty *un_named_arg1, const QSizeF &un_named_arg2, const QSizeF &un_named_arg3))
   CS_SLOT_OVERLOAD(slotRangeChanged, (QtProperty *, const QSizeF &, const QSizeF &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QRect &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QRect &))

   CS_SLOT_1(Private, void slotConstraintChanged(QtProperty *un_named_arg1, const QRect &un_named_arg2))
   CS_SLOT_OVERLOAD(slotConstraintChanged, (QtProperty *, const QRect &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QRectF &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QRectF &))

   CS_SLOT_1(Private, void slotConstraintChanged(QtProperty *un_named_arg1, const QRectF &un_named_arg2))
   CS_SLOT_OVERLOAD(slotConstraintChanged, (QtProperty *, const QRectF &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QColor &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QColor &))

   CS_SLOT_1(Private, void slotEnumNamesChanged(QtProperty *un_named_arg1, const QStringList &un_named_arg2))
   CS_SLOT_2(slotEnumNamesChanged)

   CS_SLOT_1(Private, void slotEnumIconsChanged(QtProperty *un_named_arg1, const QMap <int, QIcon> &un_named_arg2))
   CS_SLOT_2(slotEnumIconsChanged)

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QSizePolicy &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QSizePolicy &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QFont &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QFont &))

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *un_named_arg1, const QCursor &un_named_arg2))
   CS_SLOT_OVERLOAD(slotValueChanged, (QtProperty *, const QCursor &))

   CS_SLOT_1(Private, void slotFlagNamesChanged(QtProperty *un_named_arg1, const QStringList &un_named_arg2))
   CS_SLOT_2(slotFlagNamesChanged)

   CS_SLOT_1(Private, void slotPropertyInserted(QtProperty *un_named_arg1, QtProperty *un_named_arg2, QtProperty *un_named_arg3))
   CS_SLOT_2(slotPropertyInserted)

   CS_SLOT_1(Private, void slotPropertyRemoved(QtProperty *un_named_arg1, QtProperty *un_named_arg2))
   CS_SLOT_2(slotPropertyRemoved)

   Q_DECLARE_PRIVATE(QtVariantPropertyManager)
   Q_DISABLE_COPY(QtVariantPropertyManager)
};

class QtVariantEditorFactory : public QtAbstractEditorFactory<QtVariantPropertyManager>
{
   CS_OBJECT(QtVariantEditorFactory)

 public:
   QtVariantEditorFactory(QObject *parent = nullptr);
   ~QtVariantEditorFactory();

 protected:
   void connectPropertyManager(QtVariantPropertyManager *manager);
   QWidget *createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent);

   void disconnectPropertyManager(QtVariantPropertyManager *manager);

 private:
   QScopedPointer<class QtVariantEditorFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtVariantEditorFactory)
   Q_DISABLE_COPY(QtVariantEditorFactory)
};

CS_DECLARE_METATYPE(QtIconMap)

#endif
