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

#include <variantproperty.h>
#include <propertymanager.h>
#include <editorfactory.h>

#include <QDate>
#include <QIcon>
#include <QLocale>
#include <QVariant>

class QtEnumPropertyType
{
};

class QtFlagPropertyType
{
};

class QtGroupPropertyType
{
};

CS_DECLARE_METATYPE(QtEnumPropertyType)
CS_DECLARE_METATYPE(QtFlagPropertyType)
CS_DECLARE_METATYPE(QtGroupPropertyType)

int QtVariantPropertyManager::enumTypeId()
{
   return QVariant::typeToTypeId<QtEnumPropertyType>();
}

int QtVariantPropertyManager::flagTypeId()
{
   return QVariant::typeToTypeId<QtFlagPropertyType>();
}

int QtVariantPropertyManager::groupTypeId()
{
   return QVariant::typeToTypeId<QtGroupPropertyType>();
}

int QtVariantPropertyManager::iconMapTypeId()
{
   return QVariant::typeToTypeId<QtIconMap>();
}

typedef QMap<const QtProperty *, QtProperty *> PropertyMap;
Q_GLOBAL_STATIC(PropertyMap, propertyToWrappedProperty)

static QtProperty *wrappedProperty(QtProperty *property)
{
   return propertyToWrappedProperty()->value(property, 0);
}

class QtVariantPropertyPrivate
{
 public:
   QtVariantPropertyPrivate(QtVariantPropertyManager *m) : manager(m) {}

   QtVariantPropertyManager *manager;
};

QtVariantProperty::QtVariantProperty(QtVariantPropertyManager *manager)
   : QtProperty(manager), d_ptr(new QtVariantPropertyPrivate(manager))
{
}

QtVariantProperty::~QtVariantProperty()
{
}

QVariant QtVariantProperty::value() const
{
   return d_ptr->manager->value(this);
}

QVariant QtVariantProperty::attributeValue(const QString &attribute) const
{
   return d_ptr->manager->attributeValue(this, attribute);
}

int QtVariantProperty::valueType() const
{
   return d_ptr->manager->valueType(this);
}

int QtVariantProperty::propertyType() const
{
   return d_ptr->manager->propertyType(this);
}

void QtVariantProperty::setValue(const QVariant &value)
{
   d_ptr->manager->setValue(this, value);
}

void QtVariantProperty::setAttribute(const QString &attribute, const QVariant &value)
{
   d_ptr->manager->setAttribute(this, attribute, value);
}

class QtVariantPropertyManagerPrivate
{
   QtVariantPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtVariantPropertyManager)

 public:
   QtVariantPropertyManagerPrivate();

   bool m_creatingProperty;
   bool m_creatingSubProperties;
   bool m_destroyingSubProperties;
   int m_propertyType;

   void slotValueChanged(QtProperty *property, int val);
   void slotRangeChanged(QtProperty *property, int min, int max);
   void slotSingleStepChanged(QtProperty *property, int step);
   void slotValueChanged(QtProperty *property, double val);
   void slotRangeChanged(QtProperty *property, double min, double max);
   void slotSingleStepChanged(QtProperty *property, double step);
   void slotDecimalsChanged(QtProperty *property, int prec);
   void slotValueChanged(QtProperty *property, bool val);
   void slotValueChanged(QtProperty *property, const QString &val);
   void slotRegExpChanged(QtProperty *property, const QRegularExpression &regex);
   void slotValueChanged(QtProperty *property, const QDate &val);
   void slotRangeChanged(QtProperty *property, const QDate &min, const QDate &max);
   void slotValueChanged(QtProperty *property, const QTime &val);
   void slotValueChanged(QtProperty *property, const QDateTime &val);
   void slotValueChanged(QtProperty *property, const QKeySequence &val);
   void slotValueChanged(QtProperty *property, const QChar &val);
   void slotValueChanged(QtProperty *property, const QLocale &val);
   void slotValueChanged(QtProperty *property, const QPoint &val);
   void slotValueChanged(QtProperty *property, const QPointF &val);
   void slotValueChanged(QtProperty *property, const QSize &val);
   void slotRangeChanged(QtProperty *property, const QSize &min, const QSize &max);
   void slotValueChanged(QtProperty *property, const QSizeF &val);
   void slotRangeChanged(QtProperty *property, const QSizeF &min, const QSizeF &max);
   void slotValueChanged(QtProperty *property, const QRect &val);
   void slotConstraintChanged(QtProperty *property, const QRect &val);
   void slotValueChanged(QtProperty *property, const QRectF &val);
   void slotConstraintChanged(QtProperty *property, const QRectF &val);
   void slotValueChanged(QtProperty *property, const QColor &val);
   void slotEnumChanged(QtProperty *property, int val);
   void slotEnumNamesChanged(QtProperty *property, const QStringList &enumNames);
   void slotEnumIconsChanged(QtProperty *property, const QMap<int, QIcon> &enumIcons);
   void slotValueChanged(QtProperty *property, const QSizePolicy &val);
   void slotValueChanged(QtProperty *property, const QFont &val);
   void slotValueChanged(QtProperty *property, const QCursor &val);
   void slotFlagChanged(QtProperty *property, int val);
   void slotFlagNamesChanged(QtProperty *property, const QStringList &flagNames);
   void slotPropertyInserted(QtProperty *property, QtProperty *parent, QtProperty *after);
   void slotPropertyRemoved(QtProperty *property, QtProperty *parent);

   void valueChanged(QtProperty *property, const QVariant &val);

   int internalPropertyToType(QtProperty *property) const;
   QtVariantProperty *createSubProperty(QtVariantProperty *parent, QtVariantProperty *after,
      QtProperty *internal);
   void removeSubProperty(QtVariantProperty *property);

   QMap<int, QtAbstractPropertyManager *> m_typeToPropertyManager;
   QMap<int, QMap<QString, int>> m_typeToAttributeToAttributeType;

   QMap<const QtProperty *, QPair<QtVariantProperty *, int>> m_propertyToType;

   QMap<int, int> m_typeToValueType;


   QMap<QtProperty *, QtVariantProperty *> m_internalToProperty;

   const QString m_constraintAttribute;
   const QString m_singleStepAttribute;
   const QString m_decimalsAttribute;
   const QString m_enumIconsAttribute;
   const QString m_enumNamesAttribute;
   const QString m_flagNamesAttribute;
   const QString m_maximumAttribute;
   const QString m_minimumAttribute;
   const QString m_regexAttribute;
};

QtVariantPropertyManagerPrivate::QtVariantPropertyManagerPrivate() :
   m_constraintAttribute("constraint"),
   m_singleStepAttribute("singleStep"),
   m_decimalsAttribute("decimals"),
   m_enumIconsAttribute("enumIcons"),
   m_enumNamesAttribute("enumNames"),
   m_flagNamesAttribute("flagNames"),
   m_maximumAttribute("maximum"),
   m_minimumAttribute("minimum"),
   m_regexAttribute("regExp")
{
}

int QtVariantPropertyManagerPrivate::internalPropertyToType(QtProperty *property) const
{
   int type = 0;
   QtAbstractPropertyManager *internPropertyManager = property->propertyManager();

   if (dynamic_cast<QtIntPropertyManager *>(internPropertyManager)) {
      type = QVariant::Int;

   } else if (dynamic_cast<QtEnumPropertyManager *>(internPropertyManager)) {
      type = QtVariantPropertyManager::enumTypeId();

   } else if (dynamic_cast<QtBoolPropertyManager *>(internPropertyManager)) {
      type = QVariant::Bool;

   } else if (dynamic_cast<QtDoublePropertyManager *>(internPropertyManager)) {
      type = QVariant::Double;
   }
   return type;
}

QtVariantProperty *QtVariantPropertyManagerPrivate::createSubProperty(QtVariantProperty *parent,
      QtVariantProperty *after, QtProperty *internal)
{
   int type = internalPropertyToType(internal);

   if (! type) {
      return nullptr;
   }

   bool wasCreatingSubProperties = m_creatingSubProperties;
   m_creatingSubProperties = true;

   QtVariantProperty *varChild = q_ptr->addProperty(type, internal->propertyName());

   m_creatingSubProperties = wasCreatingSubProperties;

   varChild->setPropertyName(internal->propertyName());
   varChild->setToolTip(internal->toolTip());
   varChild->setStatusTip(internal->statusTip());
   varChild->setWhatsThis(internal->whatsThis());

   parent->insertSubProperty(varChild, after);

   m_internalToProperty[internal] = varChild;
   propertyToWrappedProperty()->insert(varChild, internal);

   return varChild;
}

void QtVariantPropertyManagerPrivate::removeSubProperty(QtVariantProperty *property)
{
   QtProperty *internChild = wrappedProperty(property);
   bool wasDestroyingSubProperties = m_destroyingSubProperties;
   m_destroyingSubProperties = true;
   delete property;
   m_destroyingSubProperties = wasDestroyingSubProperties;
   m_internalToProperty.remove(internChild);
   propertyToWrappedProperty()->remove(property);
}

void QtVariantPropertyManagerPrivate::slotPropertyInserted(QtProperty *property,
   QtProperty *parent, QtProperty *after)
{
   if (m_creatingProperty) {
      return;
   }

   QtVariantProperty *varParent = m_internalToProperty.value(parent, nullptr);
   if (! varParent) {
      return;
   }

   QtVariantProperty *varAfter = nullptr;

   if (after != nullptr) {
      varAfter = m_internalToProperty.value(after, nullptr);

      if (! varAfter) {
         return;
      }
   }

   createSubProperty(varParent, varAfter, property);
}

void QtVariantPropertyManagerPrivate::slotPropertyRemoved(QtProperty *property, QtProperty *parent)
{
   Q_UNUSED(parent)

   QtVariantProperty *varProperty = m_internalToProperty.value(property, 0);
   if (!varProperty) {
      return;
   }

   removeSubProperty(varProperty);
}

void QtVariantPropertyManagerPrivate::valueChanged(QtProperty *property, const QVariant &val)
{
   QtVariantProperty *varProp = m_internalToProperty.value(property, 0);
   if (!varProp) {
      return;
   }
   emit q_ptr->valueChanged(varProp, val);
   emit q_ptr->propertyChanged(varProp);
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, int val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotRangeChanged(QtProperty *property, int min, int max)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_minimumAttribute, QVariant(min));
      emit q_ptr->attributeChanged(varProp, m_maximumAttribute, QVariant(max));
   }
}

void QtVariantPropertyManagerPrivate::slotSingleStepChanged(QtProperty *property, int step)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_singleStepAttribute, QVariant(step));
   }
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, double val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotRangeChanged(QtProperty *property, double min, double max)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_minimumAttribute, QVariant(min));
      emit q_ptr->attributeChanged(varProp, m_maximumAttribute, QVariant(max));
   }
}

void QtVariantPropertyManagerPrivate::slotSingleStepChanged(QtProperty *property, double step)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_singleStepAttribute, QVariant(step));
   }
}

void QtVariantPropertyManagerPrivate::slotDecimalsChanged(QtProperty *property, int prec)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_decimalsAttribute, QVariant(prec));
   }
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, bool val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QString &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotRegExpChanged(QtProperty *property, const QRegularExpression &regex)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_regexAttribute, QVariant(regex));
   }
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QDate &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotRangeChanged(QtProperty *property, const QDate &min, const QDate &max)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_minimumAttribute, QVariant(min));
      emit q_ptr->attributeChanged(varProp, m_maximumAttribute, QVariant(max));
   }
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QTime &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QDateTime &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QKeySequence &val)
{
   QVariant v;
   v.setValue(val);
   valueChanged(property, v);
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QChar &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QLocale &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QPoint &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QPointF &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QSize &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotRangeChanged(QtProperty *property, const QSize &min, const QSize &max)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_minimumAttribute, QVariant(min));
      emit q_ptr->attributeChanged(varProp, m_maximumAttribute, QVariant(max));
   }
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QSizeF &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotRangeChanged(QtProperty *property, const QSizeF &min, const QSizeF &max)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_minimumAttribute, QVariant(min));
      emit q_ptr->attributeChanged(varProp, m_maximumAttribute, QVariant(max));
   }
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QRect &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotConstraintChanged(QtProperty *property, const QRect &constraint)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_constraintAttribute, QVariant(constraint));
   }
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QRectF &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotConstraintChanged(QtProperty *property, const QRectF &constraint)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_constraintAttribute, QVariant(constraint));
   }
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QColor &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotEnumNamesChanged(QtProperty *property, const QStringList &enumNames)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_enumNamesAttribute, QVariant(enumNames));
   }
}

void QtVariantPropertyManagerPrivate::slotEnumIconsChanged(QtProperty *property, const QMap<int, QIcon> &iconMap)
{
   QtVariantProperty *varProp = m_internalToProperty.value(property, 0);

   if (varProp != nullptr) {
      QVariant v;
//    v.setValue(iconMap);

//    broom - put back when QVariant operator= resolved
//    emit q_ptr->attributeChanged(varProp, m_enumIconsAttribute, v);
   }
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QSizePolicy &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QFont &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QCursor &val)
{
   valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotFlagNamesChanged(QtProperty *property, const QStringList &flagNames)
{
   if (QtVariantProperty *varProp = m_internalToProperty.value(property, 0)) {
      emit q_ptr->attributeChanged(varProp, m_flagNamesAttribute, QVariant(flagNames));
   }
}

QtVariantPropertyManager::QtVariantPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtVariantPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->m_creatingProperty = false;
   d_ptr->m_creatingSubProperties = false;
   d_ptr->m_destroyingSubProperties = false;
   d_ptr->m_propertyType = 0;

   // IntPropertyManager
   QtIntPropertyManager *intPropertyManager = new QtIntPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Int] = intPropertyManager;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::Int][d_ptr->m_minimumAttribute] = QVariant::Int;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::Int][d_ptr->m_maximumAttribute] = QVariant::Int;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::Int][d_ptr->m_singleStepAttribute] = QVariant::Int;
   d_ptr->m_typeToValueType[QVariant::Int] = QVariant::Int;

   connect(intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
      this, SLOT(slotValueChanged(QtProperty *, int)));

   connect(intPropertyManager, SIGNAL(rangeChanged(QtProperty *, int, int)),
      this, SLOT(slotRangeChanged(QtProperty *, int, int)));

   connect(intPropertyManager, SIGNAL(singleStepChanged(QtProperty *, int)),
      this, SLOT(slotSingleStepChanged(QtProperty *, int)));

   // DoublePropertyManager
   QtDoublePropertyManager *doublePropertyManager = new QtDoublePropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Double] = doublePropertyManager;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::Double][d_ptr->m_minimumAttribute]    = QVariant::Double;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::Double][d_ptr->m_maximumAttribute]    = QVariant::Double;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::Double][d_ptr->m_singleStepAttribute] = QVariant::Double;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::Double][d_ptr->m_decimalsAttribute]   = QVariant::Int;
   d_ptr->m_typeToValueType[QVariant::Double] = QVariant::Double;

   connect(doublePropertyManager, &QtDoublePropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, double>(&QtVariantPropertyManager::slotValueChanged));

   connect(doublePropertyManager, SIGNAL(rangeChanged(QtProperty *, double, double)),
      this, SLOT(slotRangeChanged(QtProperty *, double, double)));

   connect(doublePropertyManager, SIGNAL(singleStepChanged(QtProperty *, double)),
      this, SLOT(slotSingleStepChanged(QtProperty *, double)));

   connect(doublePropertyManager, SIGNAL(decimalsChanged(QtProperty *, int)),
      this, SLOT(slotDecimalsChanged(QtProperty *, int)));

   // BoolPropertyManager
   QtBoolPropertyManager *boolPropertyManager = new QtBoolPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Bool] = boolPropertyManager;
   d_ptr->m_typeToValueType[QVariant::Bool] = QVariant::Bool;

   connect(boolPropertyManager, SIGNAL(valueChanged(QtProperty *, bool)),
      this, SLOT(slotValueChanged(QtProperty *, bool)));

   // StringPropertyManager
   QtStringPropertyManager *stringPropertyManager = new QtStringPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::String] = stringPropertyManager;
   d_ptr->m_typeToValueType[QVariant::String] = QVariant::String;

   d_ptr->m_typeToAttributeToAttributeType[QVariant::String][d_ptr->m_regexAttribute] = QVariant::RegularExpression;

   connect(stringPropertyManager, &QtStringPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QString &>(&QtVariantPropertyManager::slotValueChanged));

   connect(stringPropertyManager, &QtStringPropertyManager::regExpChanged,
      this, &QtVariantPropertyManager::slotRegExpChanged);

   // DatePropertyManager
   QtDatePropertyManager *datePropertyManager = new QtDatePropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Date] = datePropertyManager;
   d_ptr->m_typeToValueType[QVariant::Date] = QVariant::Date;

   d_ptr->m_typeToAttributeToAttributeType[QVariant::Date][d_ptr->m_minimumAttribute] = QVariant::Date;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::Date][d_ptr->m_maximumAttribute] = QVariant::Date;

   connect(datePropertyManager, &QtDatePropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QDate &>(&QtVariantPropertyManager::slotValueChanged));

   connect(datePropertyManager, &QtDatePropertyManager::rangeChanged,
      this, cs_mp_cast<QtProperty *, const QDate &, const QDate &>(&QtVariantPropertyManager::slotRangeChanged));

   // TimePropertyManager
   QtTimePropertyManager *timePropertyManager = new QtTimePropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Time] = timePropertyManager;
   d_ptr->m_typeToValueType[QVariant::Time] = QVariant::Time;

   connect(timePropertyManager, &QtTimePropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QTime &>(&QtVariantPropertyManager::slotValueChanged));

   // DateTimePropertyManager
   QtDateTimePropertyManager *dateTimePropertyManager = new QtDateTimePropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::DateTime] = dateTimePropertyManager;
   d_ptr->m_typeToValueType[QVariant::DateTime] = QVariant::DateTime;

   connect(dateTimePropertyManager, &QtDateTimePropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QDateTime &>(&QtVariantPropertyManager::slotValueChanged));

   // KeySequencePropertyManager
   QtKeySequencePropertyManager *keySequencePropertyManager = new QtKeySequencePropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::KeySequence] = keySequencePropertyManager;
   d_ptr->m_typeToValueType[QVariant::KeySequence] = QVariant::KeySequence;

   connect(keySequencePropertyManager, &QtKeySequencePropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QKeySequence &>(&QtVariantPropertyManager::slotValueChanged));

   // CharPropertyManager
   QtCharPropertyManager *charPropertyManager = new QtCharPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Char] = charPropertyManager;
   d_ptr->m_typeToValueType[QVariant::Char] = QVariant::Char;

   connect(charPropertyManager, &QtCharPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QChar &>(&QtVariantPropertyManager::slotValueChanged));

   // LocalePropertyManager
   QtLocalePropertyManager *localePropertyManager = new QtLocalePropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Locale] = localePropertyManager;
   d_ptr->m_typeToValueType[QVariant::Locale] = QVariant::Locale;

   connect(localePropertyManager, &QtLocalePropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QLocale &>(&QtVariantPropertyManager::slotValueChanged));

   connect(localePropertyManager->subEnumPropertyManager(), &QtEnumPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, int>(&QtVariantPropertyManager::slotValueChanged));

   connect(localePropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));

   connect(localePropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));

   // PointPropertyManager
   QtPointPropertyManager *pointPropertyManager = new QtPointPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Point] = pointPropertyManager;
   d_ptr->m_typeToValueType[QVariant::Point] = QVariant::Point;

   connect(pointPropertyManager, &QtPointPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QPoint &>(&QtVariantPropertyManager::slotValueChanged));

   connect(pointPropertyManager->subIntPropertyManager(), &QtIntPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, int>(&QtVariantPropertyManager::slotValueChanged));

   connect(pointPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));

   connect(pointPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));

   // PointFPropertyManager
   QtPointFPropertyManager *pointFPropertyManager = new QtPointFPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::PointF] = pointFPropertyManager;
   d_ptr->m_typeToValueType[QVariant::PointF] = QVariant::PointF;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::PointF][d_ptr->m_decimalsAttribute] =
      QVariant::Int;

   connect(pointFPropertyManager, &QtPointFPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QPointF &>(&QtVariantPropertyManager::slotValueChanged));

   connect(pointFPropertyManager, SIGNAL(decimalsChanged(QtProperty *, int)),
      this, SLOT(slotDecimalsChanged(QtProperty *, int)));

   connect(pointFPropertyManager->subDoublePropertyManager(), SIGNAL(valueChanged(QtProperty *, double)),
      this, SLOT(slotValueChanged(QtProperty *, double)));

   connect(pointFPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));

   connect(pointFPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));

   // SizePropertyManager
   QtSizePropertyManager *sizePropertyManager = new QtSizePropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Size] = sizePropertyManager;
   d_ptr->m_typeToValueType[QVariant::Size] = QVariant::Size;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::Size][d_ptr->m_minimumAttribute] = QVariant::Size;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::Size][d_ptr->m_maximumAttribute] = QVariant::Size;

   connect(sizePropertyManager, &QtSizePropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QSize &>(&QtVariantPropertyManager::slotValueChanged));

   connect(sizePropertyManager, &QtSizePropertyManager::rangeChanged,
      this, cs_mp_cast<QtProperty *, const QSize &, const QSize &>(&QtVariantPropertyManager::slotRangeChanged));

   connect(sizePropertyManager->subIntPropertyManager(), SIGNAL(valueChanged(QtProperty *, int)),
      this, SLOT(slotValueChanged(QtProperty *, int)));

   connect(sizePropertyManager->subIntPropertyManager(), &QtIntPropertyManager::rangeChanged,
      this, cs_mp_cast<QtProperty *, int, int>(&QtVariantPropertyManager::slotRangeChanged));

   connect(sizePropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));

   connect(sizePropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));

   // SizeFPropertyManager
   QtSizeFPropertyManager *sizeFPropertyManager = new QtSizeFPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::SizeF] = sizeFPropertyManager;
   d_ptr->m_typeToValueType[QVariant::SizeF] = QVariant::SizeF;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::SizeF][d_ptr->m_minimumAttribute] = QVariant::SizeF;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::SizeF][d_ptr->m_maximumAttribute] = QVariant::SizeF;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::SizeF][d_ptr->m_decimalsAttribute] = QVariant::Int;

   connect(sizeFPropertyManager, &QtSizeFPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QSizeF &>(&QtVariantPropertyManager::slotValueChanged));

   connect(sizeFPropertyManager, &QtSizeFPropertyManager::rangeChanged,
      this, cs_mp_cast<QtProperty *, const QSizeF &, const QSizeF &>(&QtVariantPropertyManager::slotRangeChanged));

   connect(sizeFPropertyManager, SIGNAL(decimalsChanged(QtProperty *, int)),
      this, SLOT(slotDecimalsChanged(QtProperty *, int)));

   connect(sizeFPropertyManager->subDoublePropertyManager(), &QtDoublePropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, double>(&QtVariantPropertyManager::slotValueChanged));

   connect(sizeFPropertyManager->subDoublePropertyManager(), &QtDoublePropertyManager::rangeChanged,
      this, cs_mp_cast<QtProperty *, double, double>(&QtVariantPropertyManager::slotRangeChanged));

   connect(sizeFPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));

   connect(sizeFPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));

   // RectPropertyManager
   QtRectPropertyManager *rectPropertyManager = new QtRectPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Rect] = rectPropertyManager;
   d_ptr->m_typeToValueType[QVariant::Rect] = QVariant::Rect;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::Rect][d_ptr->m_constraintAttribute] = QVariant::Rect;

   connect(rectPropertyManager, &QtRectPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QRect &>(&QtVariantPropertyManager::slotValueChanged));

   connect(rectPropertyManager, &QtRectPropertyManager::constraintChanged,
      this, cs_mp_cast<QtProperty *, const QRect &>(&QtVariantPropertyManager::slotConstraintChanged));

   connect(rectPropertyManager->subIntPropertyManager(), &QtIntPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, int>(&QtVariantPropertyManager::slotValueChanged));

   connect(rectPropertyManager->subIntPropertyManager(), &QtIntPropertyManager::rangeChanged,
      this, cs_mp_cast<QtProperty *, int, int>(&QtVariantPropertyManager::slotRangeChanged));

   connect(rectPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));

   connect(rectPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));

   // RectFPropertyManager
   QtRectFPropertyManager *rectFPropertyManager = new QtRectFPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::RectF] = rectFPropertyManager;
   d_ptr->m_typeToValueType[QVariant::RectF] = QVariant::RectF;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::RectF][d_ptr->m_constraintAttribute] = QVariant::RectF;
   d_ptr->m_typeToAttributeToAttributeType[QVariant::RectF][d_ptr->m_decimalsAttribute] = QVariant::Int;

   connect(rectFPropertyManager, &QtRectFPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QRectF &>(&QtVariantPropertyManager::slotValueChanged));

   connect(rectFPropertyManager, &QtRectFPropertyManager::constraintChanged,
      this, cs_mp_cast<QtProperty *, const QRectF &>(&QtVariantPropertyManager::slotConstraintChanged));

   connect(rectFPropertyManager, SIGNAL(decimalsChanged(QtProperty *, int)),
      this, SLOT(slotDecimalsChanged(QtProperty *, int)));

   connect(rectFPropertyManager->subDoublePropertyManager(), SIGNAL(valueChanged(QtProperty *, double)),
      this, SLOT(slotValueChanged(QtProperty *, double)));

   connect(rectFPropertyManager->subDoublePropertyManager(), &QtDoublePropertyManager::rangeChanged,
      this, cs_mp_cast<QtProperty *, double, double>(&QtVariantPropertyManager::slotRangeChanged));

   connect(rectFPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));

   connect(rectFPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));

   // ColorPropertyManager
   QtColorPropertyManager *colorPropertyManager = new QtColorPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Color] = colorPropertyManager;
   d_ptr->m_typeToValueType[QVariant::Color] = QVariant::Color;

   connect(colorPropertyManager, &QtColorPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QColor &>(&QtVariantPropertyManager::slotValueChanged));

   connect(colorPropertyManager->subIntPropertyManager(), SIGNAL(valueChanged(QtProperty *, int)),
      this, SLOT(slotValueChanged(QtProperty *, int)));

   connect(colorPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));

   connect(colorPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
      this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));

   // EnumPropertyManager
   int enumId = enumTypeId();
   QtEnumPropertyManager *enumPropertyManager = new QtEnumPropertyManager(this);
   d_ptr->m_typeToPropertyManager[enumId] = enumPropertyManager;
   d_ptr->m_typeToValueType[enumId] = QVariant::Int;
   d_ptr->m_typeToAttributeToAttributeType[enumId][d_ptr->m_enumNamesAttribute] = QVariant::StringList;
   d_ptr->m_typeToAttributeToAttributeType[enumId][d_ptr->m_enumIconsAttribute] = iconMapTypeId();

   connect(enumPropertyManager, &QtEnumPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, int>(&QtVariantPropertyManager::slotValueChanged));

   connect(enumPropertyManager, &QtEnumPropertyManager::enumNamesChanged,
      this, &QtVariantPropertyManager::slotEnumNamesChanged);

   connect(enumPropertyManager, &QtEnumPropertyManager::enumIconsChanged,
      this, &QtVariantPropertyManager::slotEnumIconsChanged);

   // SizePolicyPropertyManager
   QtSizePolicyPropertyManager *sizePolicyPropertyManager = new QtSizePolicyPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::SizePolicy]   = sizePolicyPropertyManager;
   d_ptr->m_typeToValueType[QVariant::SizePolicy]         = QVariant::SizePolicy;

   connect(sizePolicyPropertyManager, &QtSizePolicyPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QSizePolicy &>(&QtVariantPropertyManager::slotValueChanged));

   connect(sizePolicyPropertyManager->subIntPropertyManager(), &QtIntPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, int>(&QtVariantPropertyManager::slotValueChanged));

   connect(sizePolicyPropertyManager->subIntPropertyManager(), &QtIntPropertyManager::rangeChanged,
      this, cs_mp_cast<QtProperty *, int, int>(&QtVariantPropertyManager::slotRangeChanged));

   connect(sizePolicyPropertyManager->subEnumPropertyManager(), &QtEnumPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, int>(&QtVariantPropertyManager::slotValueChanged));

   connect(sizePolicyPropertyManager->subEnumPropertyManager(), &QtEnumPropertyManager::enumNamesChanged,
      this, &QtVariantPropertyManager::slotEnumNamesChanged);

   connect(sizePolicyPropertyManager, &QtSizePolicyPropertyManager::propertyInserted,
      this, &QtVariantPropertyManager::slotPropertyInserted);

   connect(sizePolicyPropertyManager, &QtSizePolicyPropertyManager::propertyRemoved,
      this, &QtVariantPropertyManager::slotPropertyRemoved);

   // FontPropertyManager
   QtFontPropertyManager *fontPropertyManager = new QtFontPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Font] = fontPropertyManager;
   d_ptr->m_typeToValueType[QVariant::Font] = QVariant::Font;

   connect(fontPropertyManager, &QtFontPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QFont &>(&QtVariantPropertyManager::slotValueChanged));

   connect(fontPropertyManager->subIntPropertyManager(), &QtIntPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, int>(&QtVariantPropertyManager::slotValueChanged));

   connect(fontPropertyManager->subIntPropertyManager(), &QtIntPropertyManager::rangeChanged,
      this, cs_mp_cast<QtProperty *, int, int>(&QtVariantPropertyManager::slotRangeChanged));

   connect(fontPropertyManager->subEnumPropertyManager(), &QtEnumPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, int>(&QtVariantPropertyManager::slotValueChanged));

   connect(fontPropertyManager->subEnumPropertyManager(), &QtEnumPropertyManager::enumNamesChanged,
      this, &QtVariantPropertyManager::slotEnumNamesChanged);

   connect(fontPropertyManager->subBoolPropertyManager(), &QtBoolPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, bool>(&QtVariantPropertyManager::slotValueChanged));

   connect(fontPropertyManager, &QtFontPropertyManager::propertyInserted,
      this, &QtVariantPropertyManager::slotPropertyInserted);

   connect(fontPropertyManager, &QtFontPropertyManager::propertyRemoved,
      this, &QtVariantPropertyManager::slotPropertyRemoved);

   // CursorPropertyManager
   QtCursorPropertyManager *cursorPropertyManager = new QtCursorPropertyManager(this);
   d_ptr->m_typeToPropertyManager[QVariant::Cursor] = cursorPropertyManager;
   d_ptr->m_typeToValueType[QVariant::Cursor] = QVariant::Cursor;

   connect(cursorPropertyManager, &QtCursorPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, const QCursor &>(&QtVariantPropertyManager::slotValueChanged));

   // FlagPropertyManager
   int flagId = flagTypeId();
   QtFlagPropertyManager *flagPropertyManager = new QtFlagPropertyManager(this);
   d_ptr->m_typeToPropertyManager[flagId] = flagPropertyManager;
   d_ptr->m_typeToValueType[flagId] = QVariant::Int;
   d_ptr->m_typeToAttributeToAttributeType[flagId][d_ptr->m_flagNamesAttribute] = QVariant::StringList;

   connect(flagPropertyManager, &QtFlagPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, int>(&QtVariantPropertyManager::slotValueChanged));

   connect(flagPropertyManager, &QtFlagPropertyManager::flagNamesChanged,
      this, &QtVariantPropertyManager::slotFlagNamesChanged);

   connect(flagPropertyManager->subBoolPropertyManager(), &QtBoolPropertyManager::valueChanged,
      this, cs_mp_cast<QtProperty *, bool>(&QtVariantPropertyManager::slotValueChanged));

   connect(flagPropertyManager, &QtFlagPropertyManager::propertyInserted,
      this, &QtVariantPropertyManager::slotPropertyInserted);

   connect(flagPropertyManager, &QtFlagPropertyManager::propertyRemoved,
      this, &QtVariantPropertyManager::slotPropertyRemoved);

   // FlagPropertyManager
   int groupId = groupTypeId();
   QtGroupPropertyManager *groupPropertyManager = new QtGroupPropertyManager(this);
   d_ptr->m_typeToPropertyManager[groupId] = groupPropertyManager;
   d_ptr->m_typeToValueType[groupId] = QVariant::Invalid;
}

QtVariantPropertyManager::~QtVariantPropertyManager()
{
   clear();
}

QtVariantProperty *QtVariantPropertyManager::variantProperty(const QtProperty *property) const
{
   const QMap<const QtProperty *, QPair<QtVariantProperty *, int>>::const_iterator it = d_ptr->m_propertyToType.constFind(property);
   if (it == d_ptr->m_propertyToType.constEnd()) {
      return 0;
   }
   return it.value().first;
}

bool QtVariantPropertyManager::isPropertyTypeSupported(int propertyType) const
{
   if (d_ptr->m_typeToValueType.contains(propertyType)) {
      return true;
   }
   return false;
}

QtVariantProperty *QtVariantPropertyManager::addProperty(int propertyType, const QString &name)
{
   if (!isPropertyTypeSupported(propertyType)) {
      return 0;
   }

   bool wasCreating = d_ptr->m_creatingProperty;
   d_ptr->m_creatingProperty = true;
   d_ptr->m_propertyType = propertyType;
   QtProperty *property = QtAbstractPropertyManager::addProperty(name);
   d_ptr->m_creatingProperty = wasCreating;
   d_ptr->m_propertyType = 0;

   if (!property) {
      return 0;
   }

   return variantProperty(property);
}

QVariant QtVariantPropertyManager::value(const QtProperty *property) const
{
   QtProperty *internProp = propertyToWrappedProperty()->value(property, 0);

   if (internProp == 0) {
      return QVariant();
   }

   QtAbstractPropertyManager *manager = internProp->propertyManager();

   if (QtIntPropertyManager *intManager = dynamic_cast<QtIntPropertyManager *>(manager)) {
      return intManager->value(internProp);
   } else if (QtDoublePropertyManager *doubleManager = dynamic_cast<QtDoublePropertyManager *>(manager)) {
      return doubleManager->value(internProp);
   } else if (QtBoolPropertyManager *boolManager = dynamic_cast<QtBoolPropertyManager *>(manager)) {
      return boolManager->value(internProp);
   } else if (QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(manager)) {
      return stringManager->value(internProp);
   } else if (QtDatePropertyManager *dateManager = dynamic_cast<QtDatePropertyManager *>(manager)) {
      return dateManager->value(internProp);
   } else if (QtTimePropertyManager *timeManager = dynamic_cast<QtTimePropertyManager *>(manager)) {
      return timeManager->value(internProp);
   } else if (QtDateTimePropertyManager *dateTimeManager = dynamic_cast<QtDateTimePropertyManager *>(manager)) {
      return dateTimeManager->value(internProp);

   } else if (QtKeySequencePropertyManager *keySequenceManager = dynamic_cast<QtKeySequencePropertyManager *>(manager)) {
      return QVariant::fromValue(keySequenceManager->value(internProp));

   } else if (QtCharPropertyManager *charManager = dynamic_cast<QtCharPropertyManager *>(manager)) {
      return charManager->value(internProp);
   } else if (QtLocalePropertyManager *localeManager = dynamic_cast<QtLocalePropertyManager *>(manager)) {
      return localeManager->value(internProp);
   } else if (QtPointPropertyManager *pointManager = dynamic_cast<QtPointPropertyManager *>(manager)) {
      return pointManager->value(internProp);
   } else if (QtPointFPropertyManager *pointFManager = dynamic_cast<QtPointFPropertyManager *>(manager)) {
      return pointFManager->value(internProp);
   } else if (QtSizePropertyManager *sizeManager = dynamic_cast<QtSizePropertyManager *>(manager)) {
      return sizeManager->value(internProp);
   } else if (QtSizeFPropertyManager *sizeFManager = dynamic_cast<QtSizeFPropertyManager *>(manager)) {
      return sizeFManager->value(internProp);
   } else if (QtRectPropertyManager *rectManager = dynamic_cast<QtRectPropertyManager *>(manager)) {
      return rectManager->value(internProp);
   } else if (QtRectFPropertyManager *rectFManager = dynamic_cast<QtRectFPropertyManager *>(manager)) {
      return rectFManager->value(internProp);
   } else if (QtColorPropertyManager *colorManager = dynamic_cast<QtColorPropertyManager *>(manager)) {
      return colorManager->value(internProp);
   } else if (QtEnumPropertyManager *enumManager = dynamic_cast<QtEnumPropertyManager *>(manager)) {
      return enumManager->value(internProp);
   } else if (QtSizePolicyPropertyManager *sizePolicyManager =
         dynamic_cast<QtSizePolicyPropertyManager *>(manager)) {
      return sizePolicyManager->value(internProp);
   } else if (QtFontPropertyManager *fontManager = dynamic_cast<QtFontPropertyManager *>(manager)) {
      return fontManager->value(internProp);

   } else if (QtCursorPropertyManager *cursorManager = dynamic_cast<QtCursorPropertyManager *>(manager)) {
      return cursorManager->value(internProp);

   } else if (QtFlagPropertyManager *flagManager = dynamic_cast<QtFlagPropertyManager *>(manager)) {
      return flagManager->value(internProp);
   }
   return QVariant();
}

int QtVariantPropertyManager::valueType(const QtProperty *property) const
{
   int propType = propertyType(property);
   return valueType(propType);
}

int QtVariantPropertyManager::valueType(int propertyType) const
{
   if (d_ptr->m_typeToValueType.contains(propertyType)) {
      return d_ptr->m_typeToValueType[propertyType];
   }
   return 0;
}

int QtVariantPropertyManager::propertyType(const QtProperty *property) const
{
   const QMap<const QtProperty *, QPair<QtVariantProperty *, int>>::const_iterator it = d_ptr->m_propertyToType.constFind(property);
   if (it == d_ptr->m_propertyToType.constEnd()) {
      return 0;
   }
   return it.value().second;
}

QVariant QtVariantPropertyManager::attributeValue(const QtProperty *property, const QString &attribute) const
{
   int propType = propertyType(property);
   if (!propType) {
      return QVariant();
   }

   auto it = d_ptr->m_typeToAttributeToAttributeType.find(propType);

   if (it == d_ptr->m_typeToAttributeToAttributeType.constEnd()) {
      return QVariant();
   }

   QMap<QString, int> attributes = it.value();
   auto itAttr = attributes.find(attribute);

   if (itAttr == attributes.constEnd()) {
      return QVariant();
   }

   QtProperty *internProp = propertyToWrappedProperty()->value(property, 0);
   if (internProp == 0) {
      return QVariant();
   }

   QtAbstractPropertyManager *manager = internProp->propertyManager();
   if (QtIntPropertyManager *intManager = dynamic_cast<QtIntPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_maximumAttribute) {
         return intManager->maximum(internProp);
      }
      if (attribute == d_ptr->m_minimumAttribute) {
         return intManager->minimum(internProp);
      }
      if (attribute == d_ptr->m_singleStepAttribute) {
         return intManager->singleStep(internProp);
      }
      return QVariant();

   } else if (QtDoublePropertyManager *doubleManager = dynamic_cast<QtDoublePropertyManager *>(manager)) {
      if (attribute == d_ptr->m_maximumAttribute) {
         return doubleManager->maximum(internProp);
      }
      if (attribute == d_ptr->m_minimumAttribute) {
         return doubleManager->minimum(internProp);
      }
      if (attribute == d_ptr->m_singleStepAttribute) {
         return doubleManager->singleStep(internProp);
      }
      if (attribute == d_ptr->m_decimalsAttribute) {
         return doubleManager->decimals(internProp);
      }
      return QVariant();

   } else if (QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_regexAttribute) {
         return stringManager->regExp(internProp);
      }
      return QVariant();

   } else if (QtDatePropertyManager *dateManager = dynamic_cast<QtDatePropertyManager *>(manager)) {
      if (attribute == d_ptr->m_maximumAttribute) {
         return dateManager->maximum(internProp);
      }
      if (attribute == d_ptr->m_minimumAttribute) {
         return dateManager->minimum(internProp);
      }
      return QVariant();

   } else if (QtPointFPropertyManager *pointFManager = dynamic_cast<QtPointFPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_decimalsAttribute) {
         return pointFManager->decimals(internProp);
      }
      return QVariant();

   } else if (QtSizePropertyManager *sizeManager = dynamic_cast<QtSizePropertyManager *>(manager)) {
      if (attribute == d_ptr->m_maximumAttribute) {
         return sizeManager->maximum(internProp);
      }
      if (attribute == d_ptr->m_minimumAttribute) {
         return sizeManager->minimum(internProp);
      }
      return QVariant();

   } else if (QtSizeFPropertyManager *sizeFManager = dynamic_cast<QtSizeFPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_maximumAttribute) {
         return sizeFManager->maximum(internProp);
      }
      if (attribute == d_ptr->m_minimumAttribute) {
         return sizeFManager->minimum(internProp);
      }
      if (attribute == d_ptr->m_decimalsAttribute) {
         return sizeFManager->decimals(internProp);
      }
      return QVariant();

   } else if (QtRectPropertyManager *rectManager = dynamic_cast<QtRectPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_constraintAttribute) {
         return rectManager->constraint(internProp);
      }
      return QVariant();

   } else if (QtRectFPropertyManager *rectFManager = dynamic_cast<QtRectFPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_constraintAttribute) {
         return rectFManager->constraint(internProp);
      }
      if (attribute == d_ptr->m_decimalsAttribute) {
         return rectFManager->decimals(internProp);
      }
      return QVariant();

   } else if (QtEnumPropertyManager *enumManager = dynamic_cast<QtEnumPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_enumNamesAttribute) {
         return enumManager->enumNames(internProp);
      }

      /*  broom - put back when QVariant operator= resolved

              if (attribute == d_ptr->m_enumIconsAttribute) {
                  QVariant v;
                  v.setValue(enumManager->enumIcons(internProp));
                  return v;
              }
      */

      return QVariant();

   } else if (QtFlagPropertyManager *flagManager = dynamic_cast<QtFlagPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_flagNamesAttribute) {
         return flagManager->flagNames(internProp);
      }
      return QVariant();
   }
   return QVariant();
}

QStringList QtVariantPropertyManager::attributes(int propertyType) const
{
   auto it = d_ptr->m_typeToAttributeToAttributeType.find(propertyType);

   if (it == d_ptr->m_typeToAttributeToAttributeType.constEnd()) {
      return QStringList();
   }

   return it.value().keys();
}

int QtVariantPropertyManager::attributeType(int propertyType, const QString &attribute) const
{
   auto it = d_ptr->m_typeToAttributeToAttributeType.find(propertyType);

   if (it == d_ptr->m_typeToAttributeToAttributeType.constEnd()) {
      return 0;
   }

   auto attributes = it.value();
   auto itAttr = attributes.find(attribute);

   if (itAttr == attributes.constEnd()) {
      return 0;
   }
   return itAttr.value();
}

void QtVariantPropertyManager::setValue(QtProperty *property, const QVariant &val)
{
   int propType = val.userType();
   if (!propType) {
      return;
   }

   int valType = valueType(property);

   if (propType != valType && !val.canConvert(static_cast<QVariant::Type>(valType))) {
      return;
   }

   QtProperty *internProp = propertyToWrappedProperty()->value(property, 0);
   if (internProp == 0) {
      return;
   }

   QtAbstractPropertyManager *manager = internProp->propertyManager();
   if (QtIntPropertyManager *intManager = dynamic_cast<QtIntPropertyManager *>(manager)) {
      intManager->setValue(internProp, val.value<int>());
      return;
   } else if (QtDoublePropertyManager *doubleManager = dynamic_cast<QtDoublePropertyManager *>(manager)) {
      doubleManager->setValue(internProp, val.value<double>());
      return;
   } else if (QtBoolPropertyManager *boolManager = dynamic_cast<QtBoolPropertyManager *>(manager)) {
      boolManager->setValue(internProp, val.value<bool>());
      return;
   } else if (QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(manager)) {
      stringManager->setValue(internProp, val.value<QString>());
      return;
   } else if (QtDatePropertyManager *dateManager = dynamic_cast<QtDatePropertyManager *>(manager)) {
      dateManager->setValue(internProp, val.value<QDate>());
      return;
   } else if (QtTimePropertyManager *timeManager = dynamic_cast<QtTimePropertyManager *>(manager)) {
      timeManager->setValue(internProp, val.value<QTime>());
      return;
   } else if (QtDateTimePropertyManager *dateTimeManager = dynamic_cast<QtDateTimePropertyManager *>(manager)) {
      dateTimeManager->setValue(internProp, val.value<QDateTime>());
      return;
   } else if (QtKeySequencePropertyManager *keySequenceManager = dynamic_cast<QtKeySequencePropertyManager *>(manager)) {
      keySequenceManager->setValue(internProp, val.value<QKeySequence>());
      return;
   } else if (QtCharPropertyManager *charManager = dynamic_cast<QtCharPropertyManager *>(manager)) {
      charManager->setValue(internProp, val.value<QChar>());
      return;
   } else if (QtLocalePropertyManager *localeManager = dynamic_cast<QtLocalePropertyManager *>(manager)) {
      localeManager->setValue(internProp, val.value<QLocale>());
      return;
   } else if (QtPointPropertyManager *pointManager = dynamic_cast<QtPointPropertyManager *>(manager)) {
      pointManager->setValue(internProp, val.value<QPoint>());
      return;
   } else if (QtPointFPropertyManager *pointFManager = dynamic_cast<QtPointFPropertyManager *>(manager)) {
      pointFManager->setValue(internProp, val.value<QPointF>());
      return;
   } else if (QtSizePropertyManager *sizeManager = dynamic_cast<QtSizePropertyManager *>(manager)) {
      sizeManager->setValue(internProp, val.value<QSize>());
      return;
   } else if (QtSizeFPropertyManager *sizeFManager = dynamic_cast<QtSizeFPropertyManager *>(manager)) {
      sizeFManager->setValue(internProp, val.value<QSizeF>());
      return;
   } else if (QtRectPropertyManager *rectManager = dynamic_cast<QtRectPropertyManager *>(manager)) {
      rectManager->setValue(internProp, val.value<QRect>());
      return;
   } else if (QtRectFPropertyManager *rectFManager = dynamic_cast<QtRectFPropertyManager *>(manager)) {
      rectFManager->setValue(internProp, val.value<QRectF>());
      return;
   } else if (QtColorPropertyManager *colorManager = dynamic_cast<QtColorPropertyManager *>(manager)) {
      colorManager->setValue(internProp, val.value<QColor>());
      return;
   } else if (QtEnumPropertyManager *enumManager = dynamic_cast<QtEnumPropertyManager *>(manager)) {
      enumManager->setValue(internProp, val.value<int>());
      return;

   } else if (QtSizePolicyPropertyManager *sizePolicyManager =
         dynamic_cast<QtSizePolicyPropertyManager *>(manager)) {
      sizePolicyManager->setValue(internProp, val.value<QSizePolicy>());
      return;

   } else if (QtFontPropertyManager *fontManager = dynamic_cast<QtFontPropertyManager *>(manager)) {
      fontManager->setValue(internProp, val.value<QFont>());
      return;

   } else if (QtCursorPropertyManager *cursorManager = dynamic_cast<QtCursorPropertyManager *>(manager)) {
      cursorManager->setValue(internProp, val.value<QCursor>());
      return;

   } else if (QtFlagPropertyManager *flagManager = dynamic_cast<QtFlagPropertyManager *>(manager)) {
      flagManager->setValue(internProp, val.value<int>());
      return;
   }
}

void QtVariantPropertyManager::setAttribute(QtProperty *property,
      const QString &attribute, const QVariant &value)
{
   QVariant oldAttr = attributeValue(property, attribute);

   if (! oldAttr.isValid()) {
      return;
   }

   int attrType = value.userType();
   if (! attrType) {
      return;
   }

   if (attrType != attributeType(propertyType(property), attribute) &&
         ! value.canConvert((QVariant::Type)attrType)) {
      return;
   }

   QtProperty *internProp = propertyToWrappedProperty()->value(property, 0);
   if (internProp == nullptr) {
      return;
   }

   QtAbstractPropertyManager *manager = internProp->propertyManager();

   if (QtIntPropertyManager *intManager = dynamic_cast<QtIntPropertyManager *>(manager)) {

      if (attribute == d_ptr->m_maximumAttribute) {
         intManager->setMaximum(internProp, value.value<int>());

      } else if (attribute == d_ptr->m_minimumAttribute) {
         intManager->setMinimum(internProp, value.value<int>());

      } else if (attribute == d_ptr->m_singleStepAttribute) {
         intManager->setSingleStep(internProp, value.value<int>());
      }

      return;

   } else if (QtDoublePropertyManager *doubleManager = dynamic_cast<QtDoublePropertyManager *>(manager)) {
      if (attribute == d_ptr->m_maximumAttribute) {
         doubleManager->setMaximum(internProp, value.value<double>());
      }
      if (attribute == d_ptr->m_minimumAttribute) {
         doubleManager->setMinimum(internProp, value.value<double>());
      }
      if (attribute == d_ptr->m_singleStepAttribute) {
         doubleManager->setSingleStep(internProp, value.value<double>());
      }
      if (attribute == d_ptr->m_decimalsAttribute) {
         doubleManager->setDecimals(internProp, value.value<int>());
      }
      return;

   } else if (QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_regexAttribute) {
         stringManager->setRegExp(internProp, value.value<QRegularExpression>());
      }
      return;

   } else if (QtDatePropertyManager *dateManager = dynamic_cast<QtDatePropertyManager *>(manager)) {
      if (attribute == d_ptr->m_maximumAttribute) {
         dateManager->setMaximum(internProp, value.value<QDate>());
      }
      if (attribute == d_ptr->m_minimumAttribute) {
         dateManager->setMinimum(internProp, value.value<QDate>());
      }
      return;
   } else if (QtPointFPropertyManager *pointFManager = dynamic_cast<QtPointFPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_decimalsAttribute) {
         pointFManager->setDecimals(internProp, value.value<int>());
      }
      return;
   } else if (QtSizePropertyManager *sizeManager = dynamic_cast<QtSizePropertyManager *>(manager)) {
      if (attribute == d_ptr->m_maximumAttribute) {
         sizeManager->setMaximum(internProp, value.value<QSize>());
      }
      if (attribute == d_ptr->m_minimumAttribute) {
         sizeManager->setMinimum(internProp, value.value<QSize>());
      }
      return;
   } else if (QtSizeFPropertyManager *sizeFManager = dynamic_cast<QtSizeFPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_maximumAttribute) {
         sizeFManager->setMaximum(internProp, value.value<QSizeF>());
      }

      if (attribute == d_ptr->m_minimumAttribute) {
         sizeFManager->setMinimum(internProp, value.value<QSizeF>());
      }

      if (attribute == d_ptr->m_decimalsAttribute) {
         sizeFManager->setDecimals(internProp, value.value<int>());
      }
      return;

   } else if (QtRectPropertyManager *rectManager = dynamic_cast<QtRectPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_constraintAttribute) {
         rectManager->setConstraint(internProp, value.value<QRect>());
      }
      return;

   } else if (QtRectFPropertyManager *rectFManager = dynamic_cast<QtRectFPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_constraintAttribute) {
         rectFManager->setConstraint(internProp, value.value<QRectF>());
      }

      if (attribute == d_ptr->m_decimalsAttribute) {
         rectFManager->setDecimals(internProp, value.value<int>());
      }

      return;

   } else if (QtEnumPropertyManager *enumManager = dynamic_cast<QtEnumPropertyManager *>(manager)) {

      if (attribute == d_ptr->m_enumNamesAttribute) {
         enumManager->setEnumNames(internProp, value.value<QStringList>());
      }

      if (attribute == d_ptr->m_enumIconsAttribute) {
//       broom - put back when QVariant operator= resolved
//       enumManager->setEnumIcons(internProp, value.value<QtIconMap>());
      }

      return;

   } else if (QtFlagPropertyManager *flagManager = dynamic_cast<QtFlagPropertyManager *>(manager)) {
      if (attribute == d_ptr->m_flagNamesAttribute) {
         flagManager->setFlagNames(internProp, value.value<QStringList>());
      }

      return;
   }
}

bool QtVariantPropertyManager::hasValue(const QtProperty *property) const
{
   if (propertyType(property) == groupTypeId()) {
      return false;
   }

   return true;
}

QString QtVariantPropertyManager::valueText(const QtProperty *property) const
{
   const QtProperty *internProp = propertyToWrappedProperty()->value(property, 0);
   return internProp ? internProp->valueText() : QString();
}

QIcon QtVariantPropertyManager::valueIcon(const QtProperty *property) const
{
   const QtProperty *internProp = propertyToWrappedProperty()->value(property, 0);
   return internProp ? internProp->valueIcon() : QIcon();
}

void QtVariantPropertyManager::initializeProperty(QtProperty *property)
{
   QtVariantProperty *varProp = variantProperty(property);
   if (!varProp) {
      return;
   }

   auto it = d_ptr->m_typeToPropertyManager.find(d_ptr->m_propertyType);

   if (it != d_ptr->m_typeToPropertyManager.constEnd()) {
      QtProperty *internProp = 0;
      if (!d_ptr->m_creatingSubProperties) {
         QtAbstractPropertyManager *manager = it.value();
         internProp = manager->addProperty();
         d_ptr->m_internalToProperty[internProp] = varProp;
      }
      propertyToWrappedProperty()->insert(varProp, internProp);
      if (internProp) {
         QList<QtProperty *> children = internProp->subProperties();
         QListIterator<QtProperty *> itChild(children);
         QtVariantProperty *lastProperty = 0;
         while (itChild.hasNext()) {
            QtVariantProperty *prop = d_ptr->createSubProperty(varProp, lastProperty, itChild.next());
            lastProperty = prop ? prop : lastProperty;
         }
      }
   }
}

void QtVariantPropertyManager::uninitializeProperty(QtProperty *property)
{
   const QMap<const QtProperty *, QPair<QtVariantProperty *, int>>::iterator type_it = d_ptr->m_propertyToType.find(property);
   if (type_it == d_ptr->m_propertyToType.end()) {
      return;
   }

   PropertyMap::iterator it = propertyToWrappedProperty()->find(property);
   if (it != propertyToWrappedProperty()->end()) {
      QtProperty *internProp = it.value();
      if (internProp) {
         d_ptr->m_internalToProperty.remove(internProp);
         if (!d_ptr->m_destroyingSubProperties) {
            delete internProp;
         }
      }
      propertyToWrappedProperty()->erase(it);
   }
   d_ptr->m_propertyToType.erase(type_it);
}

QtProperty *QtVariantPropertyManager::createProperty()
{
   if (!d_ptr->m_creatingProperty) {
      return 0;
   }

   QtVariantProperty *property = new QtVariantProperty(this);
   d_ptr->m_propertyToType.insert(property, qMakePair(property, d_ptr->m_propertyType));

   return property;
}

class QtVariantEditorFactoryPrivate
{
   QtVariantEditorFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtVariantEditorFactory)

 public:
   QtSpinBoxFactory           *m_spinBoxFactory;
   QtDoubleSpinBoxFactory     *m_doubleSpinBoxFactory;
   QtCheckBoxFactory          *m_checkBoxFactory;
   QtLineEditFactory          *m_lineEditFactory;
   QtDateEditFactory          *m_dateEditFactory;
   QtTimeEditFactory          *m_timeEditFactory;
   QtDateTimeEditFactory      *m_dateTimeEditFactory;
   QtKeySequenceEditorFactory *m_keySequenceEditorFactory;
   QtCharEditorFactory        *m_charEditorFactory;
   QtEnumEditorFactory        *m_comboBoxFactory;
   QtCursorEditorFactory      *m_cursorEditorFactory;
   QtColorEditorFactory       *m_colorEditorFactory;
   QtFontEditorFactory        *m_fontEditorFactory;

   QMap<QtAbstractEditorFactoryBase *, int> m_factoryToType;
   QMap<int, QtAbstractEditorFactoryBase *> m_typeToFactory;
};

QtVariantEditorFactory::QtVariantEditorFactory(QObject *parent)
   : QtAbstractEditorFactory<QtVariantPropertyManager>(parent), d_ptr(new QtVariantEditorFactoryPrivate())
{
   d_ptr->q_ptr = this;

   d_ptr->m_spinBoxFactory = new QtSpinBoxFactory(this);
   d_ptr->m_factoryToType[d_ptr->m_spinBoxFactory] = QVariant::Int;
   d_ptr->m_typeToFactory[QVariant::Int] = d_ptr->m_spinBoxFactory;

   d_ptr->m_doubleSpinBoxFactory = new QtDoubleSpinBoxFactory(this);
   d_ptr->m_factoryToType[d_ptr->m_doubleSpinBoxFactory] = QVariant::Double;
   d_ptr->m_typeToFactory[QVariant::Double] = d_ptr->m_doubleSpinBoxFactory;

   d_ptr->m_checkBoxFactory = new QtCheckBoxFactory(this);
   d_ptr->m_factoryToType[d_ptr->m_checkBoxFactory] = QVariant::Bool;
   d_ptr->m_typeToFactory[QVariant::Bool] = d_ptr->m_checkBoxFactory;

   d_ptr->m_lineEditFactory = new QtLineEditFactory(this);
   d_ptr->m_factoryToType[d_ptr->m_lineEditFactory] = QVariant::String;
   d_ptr->m_typeToFactory[QVariant::String] = d_ptr->m_lineEditFactory;

   d_ptr->m_dateEditFactory = new QtDateEditFactory(this);
   d_ptr->m_factoryToType[d_ptr->m_dateEditFactory] = QVariant::Date;
   d_ptr->m_typeToFactory[QVariant::Date] = d_ptr->m_dateEditFactory;

   d_ptr->m_timeEditFactory = new QtTimeEditFactory(this);
   d_ptr->m_factoryToType[d_ptr->m_timeEditFactory] = QVariant::Time;
   d_ptr->m_typeToFactory[QVariant::Time] = d_ptr->m_timeEditFactory;

   d_ptr->m_dateTimeEditFactory = new QtDateTimeEditFactory(this);
   d_ptr->m_factoryToType[d_ptr->m_dateTimeEditFactory] = QVariant::DateTime;
   d_ptr->m_typeToFactory[QVariant::DateTime] = d_ptr->m_dateTimeEditFactory;

   d_ptr->m_keySequenceEditorFactory = new QtKeySequenceEditorFactory(this);
   d_ptr->m_factoryToType[d_ptr->m_keySequenceEditorFactory] = QVariant::KeySequence;
   d_ptr->m_typeToFactory[QVariant::KeySequence] = d_ptr->m_keySequenceEditorFactory;

   d_ptr->m_charEditorFactory = new QtCharEditorFactory(this);
   d_ptr->m_factoryToType[d_ptr->m_charEditorFactory] = QVariant::Char;
   d_ptr->m_typeToFactory[QVariant::Char] = d_ptr->m_charEditorFactory;

   d_ptr->m_cursorEditorFactory = new QtCursorEditorFactory(this);
   d_ptr->m_factoryToType[d_ptr->m_cursorEditorFactory] = QVariant::Cursor;
   d_ptr->m_typeToFactory[QVariant::Cursor] = d_ptr->m_cursorEditorFactory;

   d_ptr->m_colorEditorFactory = new QtColorEditorFactory(this);
   d_ptr->m_factoryToType[d_ptr->m_colorEditorFactory] = QVariant::Color;
   d_ptr->m_typeToFactory[QVariant::Color] = d_ptr->m_colorEditorFactory;

   d_ptr->m_fontEditorFactory = new QtFontEditorFactory(this);
   d_ptr->m_factoryToType[d_ptr->m_fontEditorFactory] = QVariant::Font;
   d_ptr->m_typeToFactory[QVariant::Font] = d_ptr->m_fontEditorFactory;

   d_ptr->m_comboBoxFactory = new QtEnumEditorFactory(this);
   const int enumId = QtVariantPropertyManager::enumTypeId();
   d_ptr->m_factoryToType[d_ptr->m_comboBoxFactory] = enumId;
   d_ptr->m_typeToFactory[enumId] = d_ptr->m_comboBoxFactory;
}

QtVariantEditorFactory::~QtVariantEditorFactory()
{
}

void QtVariantEditorFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
   QList<QtIntPropertyManager *> intPropertyManagers = manager->findChildren<QtIntPropertyManager *>();
   QListIterator<QtIntPropertyManager *> itInt(intPropertyManagers);
   while (itInt.hasNext()) {
      d_ptr->m_spinBoxFactory->addPropertyManager(itInt.next());
   }

   QList<QtDoublePropertyManager *> doublePropertyManagers = manager->findChildren<QtDoublePropertyManager *>();
   QListIterator<QtDoublePropertyManager *> itDouble(doublePropertyManagers);
   while (itDouble.hasNext()) {
      d_ptr->m_doubleSpinBoxFactory->addPropertyManager(itDouble.next());
   }

   QList<QtBoolPropertyManager *> boolPropertyManagers = manager->findChildren<QtBoolPropertyManager *>();
   QListIterator<QtBoolPropertyManager *> itBool(boolPropertyManagers);
   while (itBool.hasNext()) {
      d_ptr->m_checkBoxFactory->addPropertyManager(itBool.next());
   }

   QList<QtStringPropertyManager *> stringPropertyManagers = manager->findChildren<QtStringPropertyManager *>();
   QListIterator<QtStringPropertyManager *> itString(stringPropertyManagers);
   while (itString.hasNext()) {
      d_ptr->m_lineEditFactory->addPropertyManager(itString.next());
   }

   QList<QtDatePropertyManager *> datePropertyManagers = manager->findChildren<QtDatePropertyManager *>();
   QListIterator<QtDatePropertyManager *> itDate(datePropertyManagers);
   while (itDate.hasNext()) {
      d_ptr->m_dateEditFactory->addPropertyManager(itDate.next());
   }

   QList<QtTimePropertyManager *> timePropertyManagers = manager->findChildren<QtTimePropertyManager *>();
   QListIterator<QtTimePropertyManager *> itTime(timePropertyManagers);
   while (itTime.hasNext()) {
      d_ptr->m_timeEditFactory->addPropertyManager(itTime.next());
   }

   QList<QtDateTimePropertyManager *> dateTimePropertyManagers = manager->findChildren<QtDateTimePropertyManager *>();
   QListIterator<QtDateTimePropertyManager *> itDateTime(dateTimePropertyManagers);
   while (itDateTime.hasNext()) {
      d_ptr->m_dateTimeEditFactory->addPropertyManager(itDateTime.next());
   }

   QList<QtKeySequencePropertyManager *> keySequencePropertyManagers = manager->findChildren<QtKeySequencePropertyManager *>();
   QListIterator<QtKeySequencePropertyManager *> itKeySequence(keySequencePropertyManagers);
   while (itKeySequence.hasNext()) {
      d_ptr->m_keySequenceEditorFactory->addPropertyManager(itKeySequence.next());
   }

   QList<QtCharPropertyManager *> charPropertyManagers = manager->findChildren<QtCharPropertyManager *>();
   QListIterator<QtCharPropertyManager *> itChar(charPropertyManagers);
   while (itChar.hasNext()) {
      d_ptr->m_charEditorFactory->addPropertyManager(itChar.next());
   }

   QList<QtLocalePropertyManager *> localePropertyManagers = manager->findChildren<QtLocalePropertyManager *>();
   QListIterator<QtLocalePropertyManager *> itLocale(localePropertyManagers);
   while (itLocale.hasNext()) {
      d_ptr->m_comboBoxFactory->addPropertyManager(itLocale.next()->subEnumPropertyManager());
   }

   QList<QtPointPropertyManager *> pointPropertyManagers = manager->findChildren<QtPointPropertyManager *>();
   QListIterator<QtPointPropertyManager *> itPoint(pointPropertyManagers);
   while (itPoint.hasNext()) {
      d_ptr->m_spinBoxFactory->addPropertyManager(itPoint.next()->subIntPropertyManager());
   }

   QList<QtPointFPropertyManager *> pointFPropertyManagers = manager->findChildren<QtPointFPropertyManager *>();
   QListIterator<QtPointFPropertyManager *> itPointF(pointFPropertyManagers);
   while (itPointF.hasNext()) {
      d_ptr->m_doubleSpinBoxFactory->addPropertyManager(itPointF.next()->subDoublePropertyManager());
   }

   QList<QtSizePropertyManager *> sizePropertyManagers = manager->findChildren<QtSizePropertyManager *>();
   QListIterator<QtSizePropertyManager *> itSize(sizePropertyManagers);
   while (itSize.hasNext()) {
      d_ptr->m_spinBoxFactory->addPropertyManager(itSize.next()->subIntPropertyManager());
   }

   QList<QtSizeFPropertyManager *> sizeFPropertyManagers = manager->findChildren<QtSizeFPropertyManager *>();
   QListIterator<QtSizeFPropertyManager *> itSizeF(sizeFPropertyManagers);
   while (itSizeF.hasNext()) {
      d_ptr->m_doubleSpinBoxFactory->addPropertyManager(itSizeF.next()->subDoublePropertyManager());
   }

   QList<QtRectPropertyManager *> rectPropertyManagers = manager->findChildren<QtRectPropertyManager *>();
   QListIterator<QtRectPropertyManager *> itRect(rectPropertyManagers);
   while (itRect.hasNext()) {
      d_ptr->m_spinBoxFactory->addPropertyManager(itRect.next()->subIntPropertyManager());
   }

   QList<QtRectFPropertyManager *> rectFPropertyManagers = manager->findChildren<QtRectFPropertyManager *>();
   QListIterator<QtRectFPropertyManager *> itRectF(rectFPropertyManagers);
   while (itRectF.hasNext()) {
      d_ptr->m_doubleSpinBoxFactory->addPropertyManager(itRectF.next()->subDoublePropertyManager());
   }

   QList<QtColorPropertyManager *> colorPropertyManagers = manager->findChildren<QtColorPropertyManager *>();
   QListIterator<QtColorPropertyManager *> itColor(colorPropertyManagers);
   while (itColor.hasNext()) {
      QtColorPropertyManager *manager = itColor.next();
      d_ptr->m_colorEditorFactory->addPropertyManager(manager);
      d_ptr->m_spinBoxFactory->addPropertyManager(manager->subIntPropertyManager());
   }

   QList<QtEnumPropertyManager *> enumPropertyManagers = manager->findChildren<QtEnumPropertyManager *>();
   QListIterator<QtEnumPropertyManager *> itEnum(enumPropertyManagers);
   while (itEnum.hasNext()) {
      d_ptr->m_comboBoxFactory->addPropertyManager(itEnum.next());
   }

   QList<QtSizePolicyPropertyManager *> sizePolicyPropertyManagers = manager->findChildren<QtSizePolicyPropertyManager *>();
   QListIterator<QtSizePolicyPropertyManager *> itSizePolicy(sizePolicyPropertyManagers);
   while (itSizePolicy.hasNext()) {
      QtSizePolicyPropertyManager *manager = itSizePolicy.next();
      d_ptr->m_spinBoxFactory->addPropertyManager(manager->subIntPropertyManager());
      d_ptr->m_comboBoxFactory->addPropertyManager(manager->subEnumPropertyManager());
   }

   QList<QtFontPropertyManager *> fontPropertyManagers = manager->findChildren<QtFontPropertyManager *>();
   QListIterator<QtFontPropertyManager *> itFont(fontPropertyManagers);
   while (itFont.hasNext()) {
      QtFontPropertyManager *manager = itFont.next();
      d_ptr->m_fontEditorFactory->addPropertyManager(manager);
      d_ptr->m_spinBoxFactory->addPropertyManager(manager->subIntPropertyManager());
      d_ptr->m_comboBoxFactory->addPropertyManager(manager->subEnumPropertyManager());
      d_ptr->m_checkBoxFactory->addPropertyManager(manager->subBoolPropertyManager());
   }

   QList<QtCursorPropertyManager *> cursorPropertyManagers = manager->findChildren<QtCursorPropertyManager *>();
   QListIterator<QtCursorPropertyManager *> itCursor(cursorPropertyManagers);
   while (itCursor.hasNext()) {
      d_ptr->m_cursorEditorFactory->addPropertyManager(itCursor.next());
   }

   QList<QtFlagPropertyManager *> flagPropertyManagers = manager->findChildren<QtFlagPropertyManager *>();
   QListIterator<QtFlagPropertyManager *> itFlag(flagPropertyManagers);
   while (itFlag.hasNext()) {
      d_ptr->m_checkBoxFactory->addPropertyManager(itFlag.next()->subBoolPropertyManager());
   }
}

QWidget *QtVariantEditorFactory::createEditor(QtVariantPropertyManager *manager, QtProperty *property,
   QWidget *parent)
{
   const int propType = manager->propertyType(property);
   QtAbstractEditorFactoryBase *factory = d_ptr->m_typeToFactory.value(propType, 0);
   if (!factory) {
      return 0;
   }
   return factory->createEditor(wrappedProperty(property), parent);
}

void QtVariantEditorFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
{
   QList<QtIntPropertyManager *> intPropertyManagers = manager->findChildren<QtIntPropertyManager *>();
   QListIterator<QtIntPropertyManager *> itInt(intPropertyManagers);
   while (itInt.hasNext()) {
      d_ptr->m_spinBoxFactory->removePropertyManager(itInt.next());
   }

   QList<QtDoublePropertyManager *> doublePropertyManagers = manager->findChildren<QtDoublePropertyManager *>();
   QListIterator<QtDoublePropertyManager *> itDouble(doublePropertyManagers);
   while (itDouble.hasNext()) {
      d_ptr->m_doubleSpinBoxFactory->removePropertyManager(itDouble.next());
   }

   QList<QtBoolPropertyManager *> boolPropertyManagers = manager->findChildren<QtBoolPropertyManager *>();
   QListIterator<QtBoolPropertyManager *> itBool(boolPropertyManagers);
   while (itBool.hasNext()) {
      d_ptr->m_checkBoxFactory->removePropertyManager(itBool.next());
   }

   QList<QtStringPropertyManager *> stringPropertyManagers = manager->findChildren<QtStringPropertyManager *>();
   QListIterator<QtStringPropertyManager *> itString(stringPropertyManagers);
   while (itString.hasNext()) {
      d_ptr->m_lineEditFactory->removePropertyManager(itString.next());
   }

   QList<QtDatePropertyManager *> datePropertyManagers = manager->findChildren<QtDatePropertyManager *>();
   QListIterator<QtDatePropertyManager *> itDate(datePropertyManagers);
   while (itDate.hasNext()) {
      d_ptr->m_dateEditFactory->removePropertyManager(itDate.next());
   }

   QList<QtTimePropertyManager *> timePropertyManagers = manager->findChildren<QtTimePropertyManager *>();
   QListIterator<QtTimePropertyManager *> itTime(timePropertyManagers);
   while (itTime.hasNext()) {
      d_ptr->m_timeEditFactory->removePropertyManager(itTime.next());
   }

   QList<QtDateTimePropertyManager *> dateTimePropertyManagers = manager->findChildren<QtDateTimePropertyManager *>();
   QListIterator<QtDateTimePropertyManager *> itDateTime(dateTimePropertyManagers);
   while (itDateTime.hasNext()) {
      d_ptr->m_dateTimeEditFactory->removePropertyManager(itDateTime.next());
   }

   QList<QtKeySequencePropertyManager *> keySequencePropertyManagers = manager->findChildren<QtKeySequencePropertyManager *>();
   QListIterator<QtKeySequencePropertyManager *> itKeySequence(keySequencePropertyManagers);
   while (itKeySequence.hasNext()) {
      d_ptr->m_keySequenceEditorFactory->removePropertyManager(itKeySequence.next());
   }

   QList<QtCharPropertyManager *> charPropertyManagers = manager->findChildren<QtCharPropertyManager *>();
   QListIterator<QtCharPropertyManager *> itChar(charPropertyManagers);
   while (itChar.hasNext()) {
      d_ptr->m_charEditorFactory->removePropertyManager(itChar.next());
   }

   QList<QtLocalePropertyManager *> localePropertyManagers = manager->findChildren<QtLocalePropertyManager *>();
   QListIterator<QtLocalePropertyManager *> itLocale(localePropertyManagers);
   while (itLocale.hasNext()) {
      d_ptr->m_comboBoxFactory->removePropertyManager(itLocale.next()->subEnumPropertyManager());
   }

   QList<QtPointPropertyManager *> pointPropertyManagers = manager->findChildren<QtPointPropertyManager *>();
   QListIterator<QtPointPropertyManager *> itPoint(pointPropertyManagers);
   while (itPoint.hasNext()) {
      d_ptr->m_spinBoxFactory->removePropertyManager(itPoint.next()->subIntPropertyManager());
   }

   QList<QtPointFPropertyManager *> pointFPropertyManagers = manager->findChildren<QtPointFPropertyManager *>();
   QListIterator<QtPointFPropertyManager *> itPointF(pointFPropertyManagers);
   while (itPointF.hasNext()) {
      d_ptr->m_doubleSpinBoxFactory->removePropertyManager(itPointF.next()->subDoublePropertyManager());
   }

   QList<QtSizePropertyManager *> sizePropertyManagers = manager->findChildren<QtSizePropertyManager *>();
   QListIterator<QtSizePropertyManager *> itSize(sizePropertyManagers);
   while (itSize.hasNext()) {
      d_ptr->m_spinBoxFactory->removePropertyManager(itSize.next()->subIntPropertyManager());
   }

   QList<QtSizeFPropertyManager *> sizeFPropertyManagers = manager->findChildren<QtSizeFPropertyManager *>();
   QListIterator<QtSizeFPropertyManager *> itSizeF(sizeFPropertyManagers);
   while (itSizeF.hasNext()) {
      d_ptr->m_doubleSpinBoxFactory->removePropertyManager(itSizeF.next()->subDoublePropertyManager());
   }

   QList<QtRectPropertyManager *> rectPropertyManagers = manager->findChildren<QtRectPropertyManager *>();
   QListIterator<QtRectPropertyManager *> itRect(rectPropertyManagers);
   while (itRect.hasNext()) {
      d_ptr->m_spinBoxFactory->removePropertyManager(itRect.next()->subIntPropertyManager());
   }

   QList<QtRectFPropertyManager *> rectFPropertyManagers = manager->findChildren<QtRectFPropertyManager *>();
   QListIterator<QtRectFPropertyManager *> itRectF(rectFPropertyManagers);
   while (itRectF.hasNext()) {
      d_ptr->m_doubleSpinBoxFactory->removePropertyManager(itRectF.next()->subDoublePropertyManager());
   }

   QList<QtColorPropertyManager *> colorPropertyManagers = manager->findChildren<QtColorPropertyManager *>();
   QListIterator<QtColorPropertyManager *> itColor(colorPropertyManagers);
   while (itColor.hasNext()) {
      QtColorPropertyManager *manager = itColor.next();
      d_ptr->m_colorEditorFactory->removePropertyManager(manager);
      d_ptr->m_spinBoxFactory->removePropertyManager(manager->subIntPropertyManager());
   }

   QList<QtEnumPropertyManager *> enumPropertyManagers = manager->findChildren<QtEnumPropertyManager *>();
   QListIterator<QtEnumPropertyManager *> itEnum(enumPropertyManagers);
   while (itEnum.hasNext()) {
      d_ptr->m_comboBoxFactory->removePropertyManager(itEnum.next());
   }

   QList<QtSizePolicyPropertyManager *> sizePolicyPropertyManagers = manager->findChildren<QtSizePolicyPropertyManager *>();
   QListIterator<QtSizePolicyPropertyManager *> itSizePolicy(sizePolicyPropertyManagers);
   while (itSizePolicy.hasNext()) {
      QtSizePolicyPropertyManager *manager = itSizePolicy.next();
      d_ptr->m_spinBoxFactory->removePropertyManager(manager->subIntPropertyManager());
      d_ptr->m_comboBoxFactory->removePropertyManager(manager->subEnumPropertyManager());
   }

   QList<QtFontPropertyManager *> fontPropertyManagers = manager->findChildren<QtFontPropertyManager *>();
   QListIterator<QtFontPropertyManager *> itFont(fontPropertyManagers);
   while (itFont.hasNext()) {
      QtFontPropertyManager *manager = itFont.next();
      d_ptr->m_fontEditorFactory->removePropertyManager(manager);
      d_ptr->m_spinBoxFactory->removePropertyManager(manager->subIntPropertyManager());
      d_ptr->m_comboBoxFactory->removePropertyManager(manager->subEnumPropertyManager());
      d_ptr->m_checkBoxFactory->removePropertyManager(manager->subBoolPropertyManager());
   }

   QList<QtCursorPropertyManager *> cursorPropertyManagers = manager->findChildren<QtCursorPropertyManager *>();
   QListIterator<QtCursorPropertyManager *> itCursor(cursorPropertyManagers);
   while (itCursor.hasNext()) {
      d_ptr->m_cursorEditorFactory->removePropertyManager(itCursor.next());
   }

   QList<QtFlagPropertyManager *> flagPropertyManagers = manager->findChildren<QtFlagPropertyManager *>();
   QListIterator<QtFlagPropertyManager *> itFlag(flagPropertyManagers);

   while (itFlag.hasNext()) {
      d_ptr->m_checkBoxFactory->removePropertyManager(itFlag.next()->subBoolPropertyManager());
   }
}

// private slots
void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotRangeChanged(QtProperty *un_named_arg1, int un_named_arg2, int un_named_arg3)
{
   Q_D(QtVariantPropertyManager);
   d->slotRangeChanged(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QtVariantPropertyManager::slotSingleStepChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotSingleStepChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, double un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotRangeChanged(QtProperty *un_named_arg1, double un_named_arg2, double un_named_arg3)
{
   Q_D(QtVariantPropertyManager);
   d->slotRangeChanged(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QtVariantPropertyManager::slotSingleStepChanged(QtProperty *un_named_arg1, double un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotSingleStepChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotDecimalsChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotDecimalsChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, bool un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QString &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotRegExpChanged(QtProperty *un_named_arg1, const QRegularExpression &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotRegExpChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QDate &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotRangeChanged(QtProperty *un_named_arg1, const QDate &un_named_arg2, const QDate &un_named_arg3)
{
   Q_D(QtVariantPropertyManager);
   d->slotRangeChanged(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QTime &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QDateTime &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QKeySequence &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QChar &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QLocale &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QPoint &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QPointF &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QSize &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotRangeChanged(QtProperty *un_named_arg1, const QSize &un_named_arg2, const QSize &un_named_arg3)
{
   Q_D(QtVariantPropertyManager);
   d->slotRangeChanged(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QSizeF &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotRangeChanged(QtProperty *un_named_arg1, const QSizeF &un_named_arg2, const QSizeF &un_named_arg3)
{
   Q_D(QtVariantPropertyManager);
   d->slotRangeChanged(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QRect &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotConstraintChanged(QtProperty *un_named_arg1, const QRect &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotConstraintChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QRectF &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotConstraintChanged(QtProperty *un_named_arg1, const QRectF &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotConstraintChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QColor &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotEnumNamesChanged(QtProperty *un_named_arg1, const QStringList &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotEnumNamesChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotEnumIconsChanged(QtProperty *un_named_arg1, const QMap <int, QIcon> &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotEnumIconsChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QSizePolicy &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QFont &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotValueChanged(QtProperty *un_named_arg1, const QCursor &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotValueChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotFlagNamesChanged(QtProperty *un_named_arg1, const QStringList &un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotFlagNamesChanged(un_named_arg1, un_named_arg2);
}

void QtVariantPropertyManager::slotPropertyInserted(QtProperty *un_named_arg1, QtProperty *un_named_arg2, QtProperty *un_named_arg3)
{
   Q_D(QtVariantPropertyManager);
   d->slotPropertyInserted(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QtVariantPropertyManager::slotPropertyRemoved(QtProperty *un_named_arg1, QtProperty *un_named_arg2)
{
   Q_D(QtVariantPropertyManager);
   d->slotPropertyRemoved(un_named_arg1, un_named_arg2);
}
