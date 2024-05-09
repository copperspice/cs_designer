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

#include <edit_property_manager.h>
#include <property_view_utils.h>

#include <QApplication>
#include <QDateTime>
#include <QFontDatabase>
#include <QIcon>
#include <QLabel>
#include <QLocale>
#include <QMap>
#include <QMetaEnum>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>

#include <limits.h>
#include <float.h>

template <class T, class Value>
static void setSimpleMinimumData(T *data, const Value &minVal)
{
   data->minVal = minVal;

   if (data->maxVal < data->minVal) {
      data->maxVal = data->minVal;
   }

   if (data->val < data->minVal) {
      data->val = data->minVal;
   }
}

template <class T, class Value>
static void setSimpleMaximumData(T *data, const Value &maxVal)
{
   data->maxVal = maxVal;

   if (data->minVal > data->maxVal) {
      data->minVal = data->maxVal;
   }

   if (data->val > data->maxVal) {
      data->val = data->maxVal;
   }
}

template <class T, class Value>
static void setSizeMinimumData(T *data, const Value &newMinVal)
{
   data->minVal = newMinVal;

   if (data->maxVal.width() < data->minVal.width()) {
      data->maxVal.setWidth(data->minVal.width());
   }

   if (data->maxVal.height() < data->minVal.height()) {
      data->maxVal.setHeight(data->minVal.height());
   }

   if (data->val.width() < data->minVal.width()) {
      data->val.setWidth(data->minVal.width());
   }

   if (data->val.height() < data->minVal.height()) {
      data->val.setHeight(data->minVal.height());
   }
}

template <class T, class Value>
static void setSizeMaximumData(T *data, const Value &newMaxVal)
{
   data->maxVal = newMaxVal;

   if (data->minVal.width() > data->maxVal.width()) {
      data->minVal.setWidth(data->maxVal.width());
   }

   if (data->minVal.height() > data->maxVal.height()) {
      data->minVal.setHeight(data->maxVal.height());
   }

   if (data->val.width() > data->maxVal.width()) {
      data->val.setWidth(data->maxVal.width());
   }

   if (data->val.height() > data->maxVal.height()) {
      data->val.setHeight(data->maxVal.height());
   }
}

template <class SizeValue>
static SizeValue qBoundSize(const SizeValue &minVal, const SizeValue &val, const SizeValue &maxVal)
{
   SizeValue croppedVal = val;

   if (minVal.width() > val.width()) {
      croppedVal.setWidth(minVal.width());
   } else if (maxVal.width() < val.width()) {
      croppedVal.setWidth(maxVal.width());
   }

   if (minVal.height() > val.height()) {
      croppedVal.setHeight(minVal.height());
   } else if (maxVal.height() < val.height()) {
      croppedVal.setHeight(maxVal.height());
   }

   return croppedVal;
}

// Match the exact signature of qBound for VS 6.
QSize qBound(QSize minVal, QSize val, QSize maxVal)
{
   return qBoundSize(minVal, val, maxVal);
}

QSizeF qBound(QSizeF minVal, QSizeF val, QSizeF maxVal)
{
   return qBoundSize(minVal, val, maxVal);
}

namespace {

template <class Value>
void orderBorders(Value &minVal, Value &maxVal)
{
   if (minVal > maxVal) {
      qSwap(minVal, maxVal);
   }
}

template <class Value>
static void orderSizeBorders(Value &minVal, Value &maxVal)
{
   Value fromSize = minVal;
   Value toSize = maxVal;

   if (fromSize.width() > toSize.width()) {
      fromSize.setWidth(maxVal.width());
      toSize.setWidth(minVal.width());
   }

   if (fromSize.height() > toSize.height()) {
      fromSize.setHeight(maxVal.height());
      toSize.setHeight(minVal.height());
   }

   minVal = fromSize;
   maxVal = toSize;
}

void orderBorders(QSize &minVal, QSize &maxVal)
{
   orderSizeBorders(minVal, maxVal);
}

void orderBorders(QSizeF &minVal, QSizeF &maxVal)
{
   orderSizeBorders(minVal, maxVal);
}

}   // end namespace

template <class Value, class T>
static Value getData(const QMap<const QtProperty *, T> &propertyMap,
   Value T::*data, const QtProperty *property, const Value &defaultValue = Value())
{
   auto it = propertyMap.constFind(property);

   if (it == propertyMap.constEnd()) {
      return defaultValue;
   }

   return it.value().*data;
}

template <class Value, class T>
static Value getValue(const QMap<const QtProperty *, T> &propertyMap,
   const QtProperty *property, const Value &defaultValue = Value())
{
   return getData<Value>(propertyMap, &T::val, property, defaultValue);
}

template <class Value, class T>
static Value getMinimum(const QMap<const QtProperty *, T> &propertyMap,
   const QtProperty *property, const Value &defaultValue = Value())
{
   return getData<Value>(propertyMap, &T::minVal, property, defaultValue);
}

template <class Value, class T>
static Value getMaximum(const QMap<const QtProperty *, T> &propertyMap,
   const QtProperty *property, const Value &defaultValue = Value())
{
   return getData<Value>(propertyMap, &T::maxVal, property, defaultValue);
}

template <class ValueChangeParameter, class Value, class PropertyManager>
static void setSimpleValue(QMap<const QtProperty *, Value> &propertyMap, PropertyManager *manager,
      void (PropertyManager::*propertyChangedSignal)(QtProperty *),
      void (PropertyManager::*valueChangedSignal)(QtProperty *, ValueChangeParameter),
      QtProperty *property, const Value &val)
{
   auto it = propertyMap.find(property);

   if (it == propertyMap.end()) {
      return;
   }

   if (it.value() == val) {
      return;
   }

   it.value() = val;

   emit (manager->*propertyChangedSignal)(property);
   emit (manager->*valueChangedSignal)(property, val);
}

template <class ValueChangeParameter, class PropertyManagerPrivate, class PropertyManager, class Value>
static void setValueInRange(PropertyManager *manager, PropertyManagerPrivate *managerPrivate,
      void (PropertyManager::*propertyChangedSignal)(QtProperty *),
      void (PropertyManager::*valueChangedSignal)(QtProperty *, ValueChangeParameter),
      QtProperty *property, const Value &val,
      void (PropertyManagerPrivate::*setSubPropertyValue)(QtProperty *, ValueChangeParameter))
{
   auto it = managerPrivate->m_values.find(property);

   if (it == managerPrivate->m_values.end()) {
      return;
   }

   typename PropertyManagerPrivate::Data &data = it.value();

   if (data.val == val) {
      return;
   }

   const Value oldVal = data.val;

   data.val = qBound(data.minVal, val, data.maxVal);

   if (data.val == oldVal) {
      return;
   }

   if (setSubPropertyValue) {
      (managerPrivate->*setSubPropertyValue)(property, data.val);
   }

   emit (manager->*propertyChangedSignal)(property);
   emit (manager->*valueChangedSignal)(property, data.val);
}

template <class ValueChangeParameter, class PropertyManagerPrivate, class PropertyManager, class Value>
static void setBorderValues(PropertyManager *manager, PropertyManagerPrivate *managerPrivate,
      void (PropertyManager::*propertyChangedSignal)(QtProperty *),
      void (PropertyManager::*valueChangedSignal)(QtProperty *, ValueChangeParameter),
      void (PropertyManager::*rangeChangedSignal)(QtProperty *, ValueChangeParameter, ValueChangeParameter),
      QtProperty *property, const Value &minVal, const Value &maxVal,
      void (PropertyManagerPrivate::*setSubPropertyRange)(QtProperty *,
      ValueChangeParameter, ValueChangeParameter, ValueChangeParameter))
{
   auto it = managerPrivate->m_values.find(property);

   if (it == managerPrivate->m_values.end()) {
      return;
   }

   Value fromVal = minVal;
   Value toVal   = maxVal;
   orderBorders(fromVal, toVal);

   typename PropertyManagerPrivate::Data &data = it.value();

   if (data.minVal == fromVal && data.maxVal == toVal) {
      return;
   }

   const Value oldVal = data.val;

   data.setMinimumValue(fromVal);
   data.setMaximumValue(toVal);

   emit (manager->*rangeChangedSignal)(property, data.minVal, data.maxVal);

   if (setSubPropertyRange) {
      (managerPrivate->*setSubPropertyRange)(property, data.minVal, data.maxVal, data.val);
   }

   if (data.val == oldVal) {
      return;
   }

   emit (manager->*propertyChangedSignal)(property);
   emit (manager->*valueChangedSignal)(property, data.val);
}

template <class ValueChangeParameter, class PropertyManagerPrivate, class PropertyManager, class Value, class T>
static void setBorderValue(PropertyManager *manager, PropertyManagerPrivate *managerPrivate,
      void (PropertyManager::*propertyChangedSignal)(QtProperty *),
      void (PropertyManager::*valueChangedSignal)(QtProperty *, ValueChangeParameter),
      void (PropertyManager::*rangeChangedSignal)(QtProperty *, ValueChangeParameter, ValueChangeParameter),
      QtProperty *property, Value (T::*getRangeVal)() const,
      void (T::*setRangeVal)(ValueChangeParameter), const Value &borderVal,
      void (PropertyManagerPrivate::*setSubPropertyRange)(QtProperty *,
      ValueChangeParameter, ValueChangeParameter, ValueChangeParameter))
{
   auto it = managerPrivate->m_values.find(property);

   if (it == managerPrivate->m_values.end()) {
      return;
   }

   T &data = it.value();

   if ((data.*getRangeVal)() == borderVal) {
      return;
   }

   const Value oldVal = data.val;

   (data.*setRangeVal)(borderVal);

   emit (manager->*rangeChangedSignal)(property, data.minVal, data.maxVal);

   if (setSubPropertyRange) {
      (managerPrivate->*setSubPropertyRange)(property, data.minVal, data.maxVal, data.val);
   }

   if (data.val == oldVal) {
      return;
   }

   emit (manager->*propertyChangedSignal)(property);
   emit (manager->*valueChangedSignal)(property, data.val);
}

template <class ValueChangeParameter, class PropertyManagerPrivate, class PropertyManager, class Value, class T>
static void setMinimumValue(PropertyManager *manager, PropertyManagerPrivate *managerPrivate,
   void (PropertyManager::*propertyChangedSignal)(QtProperty *),
   void (PropertyManager::*valueChangedSignal)(QtProperty *, ValueChangeParameter),
   void (PropertyManager::*rangeChangedSignal)(QtProperty *, ValueChangeParameter, ValueChangeParameter),
   QtProperty *property, const Value &minVal)
{
   void (PropertyManagerPrivate::*setSubPropertyRange)(QtProperty *,
      ValueChangeParameter, ValueChangeParameter, ValueChangeParameter) = nullptr;

   setBorderValue<ValueChangeParameter, PropertyManagerPrivate, PropertyManager, Value, T>(manager, managerPrivate,
      propertyChangedSignal, valueChangedSignal, rangeChangedSignal,
      property, &PropertyManagerPrivate::Data::minimumValue, &PropertyManagerPrivate::Data::setMinimumValue, minVal, setSubPropertyRange);
}

template <class ValueChangeParameter, class PropertyManagerPrivate, class PropertyManager, class Value, class T>
static void setMaximumValue(PropertyManager *manager, PropertyManagerPrivate *managerPrivate,
   void (PropertyManager::*propertyChangedSignal)(QtProperty *),
   void (PropertyManager::*valueChangedSignal)(QtProperty *, ValueChangeParameter),
   void (PropertyManager::*rangeChangedSignal)(QtProperty *, ValueChangeParameter, ValueChangeParameter),
   QtProperty *property, const Value &maxVal)
{
   void (PropertyManagerPrivate::*setSubPropertyRange)(QtProperty *,
      ValueChangeParameter, ValueChangeParameter, ValueChangeParameter) = nullptr;

   setBorderValue<ValueChangeParameter, PropertyManagerPrivate, PropertyManager, Value, T>(manager, managerPrivate,
      propertyChangedSignal, valueChangedSignal, rangeChangedSignal,
      property, &PropertyManagerPrivate::Data::maximumValue, &PropertyManagerPrivate::Data::setMaximumValue, maxVal, setSubPropertyRange);
}

class QtMetaEnumWrapper : public QObject
{
   CS_OBJECT(QtMetaEnumWrapper)

   CS_PROPERTY_READ(policy, policy)

 public:
   QSizePolicy::Policy policy() const {
      return QSizePolicy::Ignored;
   }

 private:
   QtMetaEnumWrapper(QObject *parent)
      : QObject(parent)
   {
   }
};

class QtMetaEnumProvider
{
 public:
   QtMetaEnumProvider();

   QStringList policyEnumNames() const {
      return m_policyEnumNames;
   }

   QStringList languageEnumNames() const {
      return m_languageEnumNames;
   }

   QStringList countryEnumNames(QLocale::Language language) const {
      return m_countryEnumNames.value(language);
   }

   QSizePolicy::Policy indexToSizePolicy(int index) const;
   int sizePolicyToIndex(QSizePolicy::Policy policy) const;

   void indexToLocale(int languageIndex, int countryIndex, QLocale::Language *language, QLocale::Country *country) const;
   void localeToIndex(QLocale::Language language, QLocale::Country country, int *languageIndex, int *countryIndex) const;

 private:
   void initLocale();

   QStringList m_policyEnumNames;
   QStringList m_languageEnumNames;
   QMap<QLocale::Language, QStringList> m_countryEnumNames;
   QMap<int, QLocale::Language> m_indexToLanguage;
   QMap<QLocale::Language, int> m_languageToIndex;
   QMap<int, QMap<int, QLocale::Country>> m_indexToCountry;
   QMap<QLocale::Language, QMap<QLocale::Country, int>> m_countryToIndex;
   QMetaEnum m_policyEnum;
};

void QtMetaEnumProvider::initLocale()
{
   QMultiMap<QString, QLocale::Language> nameToLanguage;
   QLocale::Language language = QLocale::C;

   while (language <= QLocale::LastLanguage) {
      QLocale locale(language);

      if (locale.language() == language) {
         nameToLanguage.insert(QLocale::languageToString(language), language);
      }

      language = (QLocale::Language)((uint)language + 1);
   }

   const QLocale system = QLocale::system();

   if (! nameToLanguage.contains(QLocale::languageToString(system.language()))) {
      nameToLanguage.insert(QLocale::languageToString(system.language()), system.language());
   }

   QList<QLocale::Language> languageList = nameToLanguage.values();

   for (auto langId : languageList) {
      QList<QLocale> localeList = QLocale::matchingLocales(langId, QLocale::AnyScript, QLocale::AnyCountry);

      if (localeList.isEmpty() && langId == system.language()) {
         localeList.append(system);
      }

      if (! localeList.isEmpty() && ! m_languageToIndex.contains(langId)) {

         std::sort(localeList.begin(), localeList.end(), [] (auto a, auto b) {
            return QLocale::countryToString(a.country()) < QLocale::countryToString(b.country()); });

         int langCnt = m_languageEnumNames.count();

         m_indexToLanguage[langCnt] = langId;
         m_languageToIndex[langId]  = langCnt;

         QStringList countryNames;
         int countryCnt = 0;

         for (auto locale : localeList) {
             countryNames.append(QLocale::countryToString(locale.country()));

             m_indexToCountry[langCnt][countryCnt] = locale.country();
             m_countryToIndex[langId][locale.country()] = countryCnt;

             ++countryCnt;
         }

         m_languageEnumNames.append(QLocale::languageToString(langId));
         m_countryEnumNames[langId] = countryNames;
      }
   }
}

QtMetaEnumProvider::QtMetaEnumProvider()
{
   QMetaProperty metaProp;

   int index = QtMetaEnumWrapper::staticMetaObject().indexOfProperty("policy");
   metaProp  = QtMetaEnumWrapper::staticMetaObject().property(index);

   m_policyEnum = metaProp.enumerator();

   const int keyCount = m_policyEnum.keyCount();

   for (int i = 0; i < keyCount; ++i) {
      m_policyEnumNames.append(m_policyEnum.key(i));
   }

   initLocale();
}

QSizePolicy::Policy QtMetaEnumProvider::indexToSizePolicy(int index) const
{
   return static_cast<QSizePolicy::Policy>(m_policyEnum.value(index));
}

int QtMetaEnumProvider::sizePolicyToIndex(QSizePolicy::Policy policy) const
{
   const int keyCount = m_policyEnum.keyCount();

   for (int i = 0; i < keyCount; ++i)  {
      if (indexToSizePolicy(i) == policy) {
         return i;
      }
   }

   return -1;
}

void QtMetaEnumProvider::indexToLocale(int languageIndex, int countryIndex, QLocale::Language *language,
   QLocale::Country *country) const
{
   QLocale::Language l = QLocale::C;
   QLocale::Country c  = QLocale::AnyCountry;

   if (m_indexToLanguage.contains(languageIndex)) {
      l = m_indexToLanguage[languageIndex];

      if (m_indexToCountry.contains(languageIndex) && m_indexToCountry[languageIndex].contains(countryIndex)) {
         c = m_indexToCountry[languageIndex][countryIndex];
      }
   }

   if (language) {
      *language = l;
   }

   if (country) {
      *country = c;
   }
}

void QtMetaEnumProvider::localeToIndex(QLocale::Language language, QLocale::Country country, int *languageIndex,
   int *countryIndex) const
{
   int l = -1;
   int c = -1;

   if (m_languageToIndex.contains(language)) {
      l = m_languageToIndex[language];

      if (m_countryToIndex.contains(language) && m_countryToIndex[language].contains(country)) {
         c = m_countryToIndex[language][country];
      }
   }

   if (languageIndex) {
      *languageIndex = l;
   }
   if (countryIndex) {
      *countryIndex = c;
   }
}

Q_GLOBAL_STATIC(QtMetaEnumProvider, metaEnumProvider)

QtGroupPropertyManager::QtGroupPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent)
{
}

QtGroupPropertyManager::~QtGroupPropertyManager()
{
}

bool QtGroupPropertyManager::hasValue(const QtProperty *property) const
{
   Q_UNUSED(property)
   return false;
}


void QtGroupPropertyManager::initializeProperty(QtProperty *property)
{
   Q_UNUSED(property)
}

void QtGroupPropertyManager::uninitializeProperty(QtProperty *property)
{
   Q_UNUSED(property)
}

class QtIntPropertyManagerPrivate
{
 public:
   struct Data {
      Data()
         : val(0), minVal(-INT_MAX), maxVal(INT_MAX), singleStep(1)
      {
      }

      int val;
      int minVal;
      int maxVal;
      int singleStep;

      int minimumValue() const {
         return minVal;
      }

      int maximumValue() const {
         return maxVal;
      }

      void setMinimumValue(int newMinVal) {
         setSimpleMinimumData(this, newMinVal);
      }

      void setMaximumValue(int newMaxVal) {
         setSimpleMaximumData(this, newMaxVal);
      }
   };

   QMap<const QtProperty *, Data> m_values;

 private:
   QtIntPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtIntPropertyManager)
};

QtIntPropertyManager::QtIntPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtIntPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;
}

QtIntPropertyManager::~QtIntPropertyManager()
{
   clear();
}

int QtIntPropertyManager::value(const QtProperty *property) const
{
   return getValue<int>(d_ptr->m_values, property, 0);
}

int QtIntPropertyManager::minimum(const QtProperty *property) const
{
   return getMinimum<int>(d_ptr->m_values, property, 0);
}

int QtIntPropertyManager::maximum(const QtProperty *property) const
{
   return getMaximum<int>(d_ptr->m_values, property, 0);
}

int QtIntPropertyManager::singleStep(const QtProperty *property) const
{
   return getData<int>(d_ptr->m_values, &QtIntPropertyManagerPrivate::Data::singleStep, property, 0);
}

QString QtIntPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   return QString::number(it.value().val);
}

void QtIntPropertyManager::setValue(QtProperty *property, int val)
{
   void (QtIntPropertyManagerPrivate::*setSubPropertyValue)(QtProperty *, int) = nullptr;

   setValueInRange<int, QtIntPropertyManagerPrivate, QtIntPropertyManager, int>(this, d_ptr.data(),
      &QtIntPropertyManager::propertyChanged,
      &QtIntPropertyManager::valueChanged,
      property, val, setSubPropertyValue);
}

void QtIntPropertyManager::setMinimum(QtProperty *property, int minVal)
{
   setMinimumValue<int, QtIntPropertyManagerPrivate, QtIntPropertyManager, int, QtIntPropertyManagerPrivate::Data>(this, d_ptr.data(),
      &QtIntPropertyManager::propertyChanged,
      &QtIntPropertyManager::valueChanged,
      &QtIntPropertyManager::rangeChanged,
      property, minVal);
}

void QtIntPropertyManager::setMaximum(QtProperty *property, int maxVal)
{
   setMaximumValue<int, QtIntPropertyManagerPrivate, QtIntPropertyManager, int, QtIntPropertyManagerPrivate::Data>(this, d_ptr.data(),
      &QtIntPropertyManager::propertyChanged,
      &QtIntPropertyManager::valueChanged,
      &QtIntPropertyManager::rangeChanged,
      property, maxVal);
}

void QtIntPropertyManager::setRange(QtProperty *property, int minVal, int maxVal)
{
   void (QtIntPropertyManagerPrivate::*setSubPropertyRange)(QtProperty *, int, int, int) = nullptr;
   setBorderValues<int, QtIntPropertyManagerPrivate, QtIntPropertyManager, int>(this, d_ptr.data(),
      &QtIntPropertyManager::propertyChanged,
      &QtIntPropertyManager::valueChanged,
      &QtIntPropertyManager::rangeChanged,
      property, minVal, maxVal, setSubPropertyRange);
}

void QtIntPropertyManager::setSingleStep(QtProperty *property, int step)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtIntPropertyManagerPrivate::Data data = it.value();

   if (step < 0) {
      step = 0;
   }

   if (data.singleStep == step) {
      return;
   }

   data.singleStep = step;

   it.value() = data;

   emit singleStepChanged(property, data.singleStep);
}

void QtIntPropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QtIntPropertyManagerPrivate::Data();
}

void QtIntPropertyManager::uninitializeProperty(QtProperty *property)
{
   d_ptr->m_values.remove(property);
}

class QtDoublePropertyManagerPrivate
{
 public:
   struct Data {
      Data()
         : val(0), minVal(-DBL_MAX), maxVal(DBL_MAX), singleStep(1), decimals(2)
      {
      }

      double val;
      double minVal;
      double maxVal;
      double singleStep;
      int decimals;

      double minimumValue() const {
         return minVal;
      }
      double maximumValue() const {
         return maxVal;
      }
      void setMinimumValue(double newMinVal) {
         setSimpleMinimumData(this, newMinVal);
      }
      void setMaximumValue(double newMaxVal) {
         setSimpleMaximumData(this, newMaxVal);
      }
   };

   QMap<const QtProperty *, Data> m_values;

 private:
   QtDoublePropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtDoublePropertyManager)
};

QtDoublePropertyManager::QtDoublePropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtDoublePropertyManagerPrivate)
{
   d_ptr->q_ptr = this;
}

QtDoublePropertyManager::~QtDoublePropertyManager()
{
   clear();
}

double QtDoublePropertyManager::value(const QtProperty *property) const
{
   return getValue<double>(d_ptr->m_values, property, 0.0);
}

double QtDoublePropertyManager::minimum(const QtProperty *property) const
{
   return getMinimum<double>(d_ptr->m_values, property, 0.0);
}

double QtDoublePropertyManager::maximum(const QtProperty *property) const
{
   return getMaximum<double>(d_ptr->m_values, property, 0.0);
}

double QtDoublePropertyManager::singleStep(const QtProperty *property) const
{
   return getData<double>(d_ptr->m_values, &QtDoublePropertyManagerPrivate::Data::singleStep, property, 0);
}

int QtDoublePropertyManager::decimals(const QtProperty *property) const
{
   return getData<int>(d_ptr->m_values, &QtDoublePropertyManagerPrivate::Data::decimals, property, 0);
}

QString QtDoublePropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   return QString::number(it.value().val, 'f', it.value().decimals);
}

void QtDoublePropertyManager::setValue(QtProperty *property, double val)
{
   void (QtDoublePropertyManagerPrivate::*setSubPropertyValue)(QtProperty *, double) = nullptr;

   setValueInRange<double, QtDoublePropertyManagerPrivate, QtDoublePropertyManager, double>(this, d_ptr.data(),
      &QtDoublePropertyManager::propertyChanged, &QtDoublePropertyManager::valueChanged,
      property, val, setSubPropertyValue);
}

void QtDoublePropertyManager::setSingleStep(QtProperty *property, double step)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtDoublePropertyManagerPrivate::Data data = it.value();

   if (step < 0) {
      step = 0;
   }

   if (data.singleStep == step) {
      return;
   }

   data.singleStep = step;

   it.value() = data;

   emit singleStepChanged(property, data.singleStep);
}

void QtDoublePropertyManager::setDecimals(QtProperty *property, int prec)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtDoublePropertyManagerPrivate::Data data = it.value();

   if (prec > 13) {
      prec = 13;
   } else if (prec < 0) {
      prec = 0;
   }

   if (data.decimals == prec) {
      return;
   }

   data.decimals = prec;

   it.value() = data;

   emit decimalsChanged(property, data.decimals);
}

void QtDoublePropertyManager::setMinimum(QtProperty *property, double minVal)
{
   setMinimumValue<double, QtDoublePropertyManagerPrivate, QtDoublePropertyManager, double,
         QtDoublePropertyManagerPrivate::Data>(this, d_ptr.data(),
         &QtDoublePropertyManager::propertyChanged, &QtDoublePropertyManager::valueChanged,
         &QtDoublePropertyManager::rangeChanged, property, minVal);
}

void QtDoublePropertyManager::setMaximum(QtProperty *property, double maxVal)
{
   setMaximumValue<double, QtDoublePropertyManagerPrivate, QtDoublePropertyManager, double,
         QtDoublePropertyManagerPrivate::Data>(this, d_ptr.data(),
         &QtDoublePropertyManager::propertyChanged, &QtDoublePropertyManager::valueChanged,
         &QtDoublePropertyManager::rangeChanged, property, maxVal);
}

void QtDoublePropertyManager::setRange(QtProperty *property, double minVal, double maxVal)
{
   void (QtDoublePropertyManagerPrivate::*setSubPropertyRange)(QtProperty *, double, double, double) = nullptr;

   setBorderValues<double, QtDoublePropertyManagerPrivate, QtDoublePropertyManager,
         double>(this, d_ptr.data(), &QtDoublePropertyManager::propertyChanged,
         &QtDoublePropertyManager::valueChanged, &QtDoublePropertyManager::rangeChanged,
         property, minVal, maxVal, setSubPropertyRange);
}

void QtDoublePropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QtDoublePropertyManagerPrivate::Data();
}

void QtDoublePropertyManager::uninitializeProperty(QtProperty *property)
{
   d_ptr->m_values.remove(property);
}

class QtStringPropertyManagerPrivate
{
 public:
   struct Data {
      Data()
         : regex("*", QPatternOption::ExactMatchOption | QPatternOption::WildcardOption) {
      }

      QString val;
      QRegularExpression regex;
   };

   QMap<const QtProperty *, Data> m_values;

 private:
   QtStringPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtStringPropertyManager)
};

QtStringPropertyManager::QtStringPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtStringPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;
}

QtStringPropertyManager::~QtStringPropertyManager()
{
   clear();
}

QString QtStringPropertyManager::value(const QtProperty *property) const
{
   return getValue<QString>(d_ptr->m_values, property);
}

QRegularExpression QtStringPropertyManager::regExp(const QtProperty *property) const
{
   return getData<QRegularExpression>(d_ptr->m_values,
         &QtStringPropertyManagerPrivate::Data::regex, property, QRegularExpression());
}

QString QtStringPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   return it.value().val;
}

void QtStringPropertyManager::setValue(QtProperty *property, const QString &val)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtStringPropertyManagerPrivate::Data data = it.value();

   if (data.val == val) {
      return;
   }

   if (data.regex.isValid())  {
      auto match = data.regex.match(val);

      if (! match.hasMatch()) {
         return;
      }
   }

   data.val = val;

   it.value() = data;

   emit propertyChanged(property);
   emit valueChanged(property, data.val);
}

void QtStringPropertyManager::setRegExp(QtProperty *property, const QRegularExpression &regex)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtStringPropertyManagerPrivate::Data data = it.value() ;

   if (data.regex.pattern() == regex.pattern()) {
      return;
   }

   data.regex = regex;
   it.value() = data;

   emit regExpChanged(property, data.regex);
}

void QtStringPropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QtStringPropertyManagerPrivate::Data();
}

void QtStringPropertyManager::uninitializeProperty(QtProperty *property)
{
   d_ptr->m_values.remove(property);
}

static QIcon drawCheckBox(bool value)
{
   QStyleOptionButton opt;
   opt.state |= value ? QStyle::State_On : QStyle::State_Off;
   opt.state |= QStyle::State_Enabled;
   const QStyle *style = QApplication::style();

   // Figure out size of an indicator and make sure it is not scaled down in a list view item
   // by making the pixmap as big as a list view icon and centering the indicator in it.
   // (if it is smaller, it can't be helped)

   const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &opt);
   const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight, &opt);
   const int listViewIconSize = indicatorWidth;
   const int pixmapWidth = indicatorWidth;
   const int pixmapHeight = qMax(indicatorHeight, listViewIconSize);

   opt.rect = QRect(0, 0, indicatorWidth, indicatorHeight);
   QPixmap pixmap = QPixmap(pixmapWidth, pixmapHeight);

   pixmap.fill(Qt::transparent);
   {
      // Center?
      const int xoff = (pixmapWidth  > indicatorWidth)  ? (pixmapWidth  - indicatorWidth)  / 2 : 0;
      const int yoff = (pixmapHeight > indicatorHeight) ? (pixmapHeight - indicatorHeight) / 2 : 0;
      QPainter painter(&pixmap);
      painter.translate(xoff, yoff);
      style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &painter);
   }

   return QIcon(pixmap);
}

class QtBoolPropertyManagerPrivate
{
 public:
   QtBoolPropertyManagerPrivate();

   QMap<const QtProperty *, bool> m_values;
   const QIcon m_checkedIcon;
   const QIcon m_uncheckedIcon;

 private:
   QtBoolPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtBoolPropertyManager)
};

QtBoolPropertyManagerPrivate::QtBoolPropertyManagerPrivate()
   : m_checkedIcon(drawCheckBox(true)), m_uncheckedIcon(drawCheckBox(false))
{
}

QtBoolPropertyManager::QtBoolPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtBoolPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;
}

QtBoolPropertyManager::~QtBoolPropertyManager()
{
   clear();
}

bool QtBoolPropertyManager::value(const QtProperty *property) const
{
   return d_ptr->m_values.value(property, false);
}

QString QtBoolPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   static const QString trueText  = tr("True");
   static const QString falseText = tr("False");

   return it.value() ? trueText : falseText;
}

QIcon QtBoolPropertyManager::valueIcon(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QIcon();
   }

   return it.value() ? d_ptr->m_checkedIcon : d_ptr->m_uncheckedIcon;
}

void QtBoolPropertyManager::setValue(QtProperty *property, bool val)
{
   setSimpleValue<bool, bool, QtBoolPropertyManager>(d_ptr->m_values, this,
      &QtBoolPropertyManager::propertyChanged,
      &QtBoolPropertyManager::valueChanged, property, val);
}

void QtBoolPropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = false;
}

void QtBoolPropertyManager::uninitializeProperty(QtProperty *property)
{
   d_ptr->m_values.remove(property);
}

class QtDatePropertyManagerPrivate
{
 public:
   explicit QtDatePropertyManagerPrivate(QtDatePropertyManager *q);

   struct Data {
      Data()
         : val(QDate::currentDate()), minVal(QDate(1752, 9, 14)), maxVal(QDate(7999, 12, 31))
      {
      }

      QDate val;
      QDate minVal;
      QDate maxVal;
      QDate minimumValue() const {
         return minVal;
      }
      QDate maximumValue() const {
         return maxVal;
      }
      void setMinimumValue(const QDate &newMinVal) {
         setSimpleMinimumData(this, newMinVal);
      }
      void setMaximumValue(const QDate &newMaxVal) {
         setSimpleMaximumData(this, newMaxVal);
      }
   };

   QString m_format;
   QMap<const QtProperty *, Data> m_values;

 private:
   QtDatePropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtDatePropertyManager)
};

QtDatePropertyManagerPrivate::QtDatePropertyManagerPrivate(QtDatePropertyManager *q)
   : m_format(QtPropertyBrowserUtils::dateFormat()), q_ptr(q)
{
}

QtDatePropertyManager::QtDatePropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtDatePropertyManagerPrivate(this))
{
}

QtDatePropertyManager::~QtDatePropertyManager()
{
   clear();
}

QDate QtDatePropertyManager::value(const QtProperty *property) const
{
   return getValue<QDate>(d_ptr->m_values, property);
}

QDate QtDatePropertyManager::minimum(const QtProperty *property) const
{
   return getMinimum<QDate>(d_ptr->m_values, property);
}

QDate QtDatePropertyManager::maximum(const QtProperty *property) const
{
   return getMaximum<QDate>(d_ptr->m_values, property);
}

QString QtDatePropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }
   return it.value().val.toString(d_ptr->m_format);
}

void QtDatePropertyManager::setValue(QtProperty *property, const QDate &val)
{
   void (QtDatePropertyManagerPrivate::*setSubPropertyValue)(QtProperty *, const QDate &) = nullptr;

   setValueInRange<const QDate &, QtDatePropertyManagerPrivate, QtDatePropertyManager,
         const QDate>(this, d_ptr.data(), &QtDatePropertyManager::propertyChanged,
         &QtDatePropertyManager::valueChanged, property, val, setSubPropertyValue);
}

void QtDatePropertyManager::setMinimum(QtProperty *property, const QDate &minVal)
{
   setMinimumValue<const QDate &, QtDatePropertyManagerPrivate, QtDatePropertyManager,
         QDate, QtDatePropertyManagerPrivate::Data>(this, d_ptr.data(),
         &QtDatePropertyManager::propertyChanged, &QtDatePropertyManager::valueChanged,
         &QtDatePropertyManager::rangeChanged, property, minVal);
}

void QtDatePropertyManager::setMaximum(QtProperty *property, const QDate &maxVal)
{
   setMaximumValue<const QDate &, QtDatePropertyManagerPrivate, QtDatePropertyManager,
         QDate, QtDatePropertyManagerPrivate::Data>(this, d_ptr.data(),
         &QtDatePropertyManager::propertyChanged, &QtDatePropertyManager::valueChanged,
         &QtDatePropertyManager::rangeChanged, property, maxVal);
}

void QtDatePropertyManager::setRange(QtProperty *property, const QDate &minVal, const QDate &maxVal)
{
   void (QtDatePropertyManagerPrivate::*setSubPropertyRange)(QtProperty *, const QDate &,
         const QDate &, const QDate &) = nullptr;

   setBorderValues<const QDate &, QtDatePropertyManagerPrivate, QtDatePropertyManager, QDate>(this, d_ptr.data(),
         &QtDatePropertyManager::propertyChanged, &QtDatePropertyManager::valueChanged,
         &QtDatePropertyManager::rangeChanged, property, minVal, maxVal, setSubPropertyRange);
}

void QtDatePropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QtDatePropertyManagerPrivate::Data();
}

void QtDatePropertyManager::uninitializeProperty(QtProperty *property)
{
   d_ptr->m_values.remove(property);
}


class QtTimePropertyManagerPrivate
{
 public:
   explicit QtTimePropertyManagerPrivate(QtTimePropertyManager *q);

   const QString m_format;
   QMap<const QtProperty *, QTime> m_values;

 private:
   QtTimePropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtTimePropertyManager)
};

QtTimePropertyManagerPrivate::QtTimePropertyManagerPrivate(QtTimePropertyManager *q)
   : m_format(QtPropertyBrowserUtils::timeFormat()), q_ptr(q)
{
}

QtTimePropertyManager::QtTimePropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtTimePropertyManagerPrivate(this))
{
}

QtTimePropertyManager::~QtTimePropertyManager()
{
   clear();
}

QTime QtTimePropertyManager::value(const QtProperty *property) const
{
   return d_ptr->m_values.value(property, QTime());
}

QString QtTimePropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   return it.value().toString(d_ptr->m_format);
}

void QtTimePropertyManager::setValue(QtProperty *property, const QTime &val)
{
   setSimpleValue<const QTime &, QTime, QtTimePropertyManager>(d_ptr->m_values, this,
      &QtTimePropertyManager::propertyChanged,
      &QtTimePropertyManager::valueChanged,
      property, val);
}

/*!
    \reimp
*/
void QtTimePropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QTime::currentTime();
}

/*!
    \reimp
*/
void QtTimePropertyManager::uninitializeProperty(QtProperty *property)
{
   d_ptr->m_values.remove(property);
}

// QtDateTimePropertyManager

class QtDateTimePropertyManagerPrivate
{
 public:
   explicit QtDateTimePropertyManagerPrivate(QtDateTimePropertyManager *q);

   const QString m_format;
   QMap<const QtProperty *, QDateTime> m_values;

 private:
   QtDateTimePropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtDateTimePropertyManager)
};

QtDateTimePropertyManagerPrivate::QtDateTimePropertyManagerPrivate(QtDateTimePropertyManager *q)
   : m_format(QtPropertyBrowserUtils::dateTimeFormat()), q_ptr(q)
{
}

QtDateTimePropertyManager::QtDateTimePropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtDateTimePropertyManagerPrivate(this))
{
}

QtDateTimePropertyManager::~QtDateTimePropertyManager()
{
   clear();
}

QDateTime QtDateTimePropertyManager::value(const QtProperty *property) const
{
   return d_ptr->m_values.value(property, QDateTime());
}

QString QtDateTimePropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }
   return it.value().toString(d_ptr->m_format);
}

void QtDateTimePropertyManager::setValue(QtProperty *property, const QDateTime &val)
{
   setSimpleValue<const QDateTime &, QDateTime, QtDateTimePropertyManager>(d_ptr->m_values, this,
      &QtDateTimePropertyManager::propertyChanged,
      &QtDateTimePropertyManager::valueChanged,
      property, val);
}

void QtDateTimePropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QDateTime::currentDateTime();
}

void QtDateTimePropertyManager::uninitializeProperty(QtProperty *property)
{
   d_ptr->m_values.remove(property);
}

class QtKeySequencePropertyManagerPrivate
{
 public:
   QString m_format;
   QMap<const QtProperty *, QKeySequence> m_values;

 private:
   QtKeySequencePropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtKeySequencePropertyManager)
};

QtKeySequencePropertyManager::QtKeySequencePropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtKeySequencePropertyManagerPrivate)
{
   d_ptr->q_ptr = this;
}

QtKeySequencePropertyManager::~QtKeySequencePropertyManager()
{
   clear();
}

QKeySequence QtKeySequencePropertyManager::value(const QtProperty *property) const
{
   return d_ptr->m_values.value(property, QKeySequence());
}

QString QtKeySequencePropertyManager::valueText(const QtProperty *property) const
{
   auto  it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   return it.value().toString(QKeySequence::NativeText);
}

void QtKeySequencePropertyManager::setValue(QtProperty *property, const QKeySequence &val)
{
   setSimpleValue<const QKeySequence &, QKeySequence, QtKeySequencePropertyManager>(d_ptr->m_values, this,
      &QtKeySequencePropertyManager::propertyChanged,
      &QtKeySequencePropertyManager::valueChanged,
      property, val);
}

void QtKeySequencePropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QKeySequence();
}

void QtKeySequencePropertyManager::uninitializeProperty(QtProperty *property)
{
   d_ptr->m_values.remove(property);
}


class QtCharPropertyManagerPrivate
{
 public:
   QMap<const QtProperty *, QChar> m_values;

 private:
   QtCharPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtCharPropertyManager)
};

QtCharPropertyManager::QtCharPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtCharPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;
}

QtCharPropertyManager::~QtCharPropertyManager()
{
   clear();
}

QChar QtCharPropertyManager::value(const QtProperty *property) const
{
   return d_ptr->m_values.value(property, QChar());
}

QString QtCharPropertyManager::valueText(const QtProperty *property) const
{
   QString retval;

   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return retval;
   }

   const QChar c = it.value();

   return c.isNull() ? retval : QString(c);
}

void QtCharPropertyManager::setValue(QtProperty *property, const QChar &val)
{
   setSimpleValue<const QChar &, QChar, QtCharPropertyManager>(d_ptr->m_values, this,
      &QtCharPropertyManager::propertyChanged,
      &QtCharPropertyManager::valueChanged,
      property, val);
}

void QtCharPropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QChar();
}

void QtCharPropertyManager::uninitializeProperty(QtProperty *property)
{
   d_ptr->m_values.remove(property);
}

class QtLocalePropertyManagerPrivate
{
 public:
   QtLocalePropertyManagerPrivate();

   void slotEnumChanged(QtProperty *property, int value);
   void slotPropertyDestroyed(QtProperty *property);

   QMap<const QtProperty *, QLocale> m_values;

   QtEnumPropertyManager *m_enumPropertyManager;

   QMap<const QtProperty *, QtProperty *> m_propertyToLanguage;
   QMap<const QtProperty *, QtProperty *> m_propertyToCountry;

   QMap<const QtProperty *, QtProperty *> m_languageToProperty;
   QMap<const QtProperty *, QtProperty *> m_countryToProperty;

 private:
   QtLocalePropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtLocalePropertyManager)
};

QtLocalePropertyManagerPrivate::QtLocalePropertyManagerPrivate()
{
}

void QtLocalePropertyManagerPrivate::slotEnumChanged(QtProperty *property, int value)
{
   if (QtProperty *langProp = m_languageToProperty.value(property, nullptr)) {
      const QLocale loc     = m_values[langProp];

      QLocale::Language newLanguage = loc.language();
      QLocale::Country newCountry   = loc.country();

      metaEnumProvider()->indexToLocale(value, 0, &newLanguage, nullptr);

      QLocale newLoc(newLanguage, newCountry);
      q_ptr->setValue(langProp, newLoc);

   } else if (QtProperty *countryProp = m_countryToProperty.value(property, nullptr)) {
      const QLocale loc = m_values[countryProp];

      QLocale::Language newLanguage = loc.language();
      QLocale::Country newCountry   = loc.country();

      metaEnumProvider()->indexToLocale(m_enumPropertyManager->value(m_propertyToLanguage.value(countryProp)),
            value, &newLanguage, &newCountry);

      QLocale newLoc(newLanguage, newCountry);
      q_ptr->setValue(countryProp, newLoc);
   }
}

void QtLocalePropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
   if (QtProperty *langSubProp = m_languageToProperty.value(property, nullptr)) {
      m_propertyToLanguage[langSubProp] = nullptr;
      m_languageToProperty.remove(property);

   } else if (QtProperty *countrySubProp = m_countryToProperty.value(property, nullptr)) {
      m_propertyToCountry[countrySubProp] = nullptr;
      m_countryToProperty.remove(property);
   }
}

QtLocalePropertyManager::QtLocalePropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtLocalePropertyManagerPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->m_enumPropertyManager = new QtEnumPropertyManager(this);

   connect(d_ptr->m_enumPropertyManager, &QtEnumPropertyManager::valueChanged,
         this, &QtLocalePropertyManager::slotEnumChanged);

   connect(d_ptr->m_enumPropertyManager, &QtEnumPropertyManager::propertyDestroyed,
         this, &QtLocalePropertyManager::slotPropertyDestroyed);
}

QtLocalePropertyManager::~QtLocalePropertyManager()
{
   clear();
}

QtEnumPropertyManager *QtLocalePropertyManager::subEnumPropertyManager() const
{
   return d_ptr->m_enumPropertyManager;
}

QLocale QtLocalePropertyManager::value(const QtProperty *property) const
{
   return d_ptr->m_values.value(property, QLocale());
}

QString QtLocalePropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   const QLocale loc = it.value();

   int langIdx    = 0;
   int countryIdx = 0;

   const QtMetaEnumProvider *me = metaEnumProvider();
   me->localeToIndex(loc.language(), loc.country(), &langIdx, &countryIdx);

   if (langIdx < 0) {
      qWarning("QtLocalePropertyManager::valueText: Unknown language %d", loc.language());
      return tr("<Invalid>");
   }

   const QString languageName = me->languageEnumNames().at(langIdx);
   if (countryIdx < 0) {
      qWarning("QtLocalePropertyManager::valueText: Unknown country %d for %s", loc.country(), csPrintable(languageName));
      return languageName;
   }

   const QString countryName = me->countryEnumNames(loc.language()).at(countryIdx);

   return tr("%1, %2").formatArgs(languageName, countryName);
}

void QtLocalePropertyManager::setValue(QtProperty *property, const QLocale &val)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   const QLocale loc = it.value();

   if (loc == val) {
      return;
   }

   it.value() = val;

   int langIdx = 0;
   int countryIdx = 0;
   metaEnumProvider()->localeToIndex(val.language(), val.country(), &langIdx, &countryIdx);

   if (loc.language() != val.language()) {
      d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToLanguage.value(property), langIdx);
      d_ptr->m_enumPropertyManager->setEnumNames(d_ptr->m_propertyToCountry.value(property),
         metaEnumProvider()->countryEnumNames(val.language()));
   }

   d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToCountry.value(property), countryIdx);

   emit propertyChanged(property);
   emit valueChanged(property, val);
}

void QtLocalePropertyManager::initializeProperty(QtProperty *property)
{
   QLocale val;
   d_ptr->m_values[property] = val;

   int langIdx = 0;
   int countryIdx = 0;
   metaEnumProvider()->localeToIndex(val.language(), val.country(), &langIdx, &countryIdx);

   QtProperty *languageProp = d_ptr->m_enumPropertyManager->addProperty();
   languageProp->setPropertyName(tr("Language"));
   d_ptr->m_enumPropertyManager->setEnumNames(languageProp, metaEnumProvider()->languageEnumNames());
   d_ptr->m_enumPropertyManager->setValue(languageProp, langIdx);
   d_ptr->m_propertyToLanguage[property]     = languageProp;
   d_ptr->m_languageToProperty[languageProp] = property;
   property->addSubProperty(languageProp);

   QtProperty *countryProp = d_ptr->m_enumPropertyManager->addProperty();
   countryProp->setPropertyName(tr("Country"));
   d_ptr->m_enumPropertyManager->setEnumNames(countryProp, metaEnumProvider()->countryEnumNames(val.language()));
   d_ptr->m_enumPropertyManager->setValue(countryProp, countryIdx);
   d_ptr->m_propertyToCountry[property] = countryProp;
   d_ptr->m_countryToProperty[countryProp] = property;
   property->addSubProperty(countryProp);
}

void QtLocalePropertyManager::uninitializeProperty(QtProperty *property)
{
   QtProperty *languageProp = d_ptr->m_propertyToLanguage[property];
   if (languageProp) {
      d_ptr->m_languageToProperty.remove(languageProp);
      delete languageProp;
   }
   d_ptr->m_propertyToLanguage.remove(property);

   QtProperty *countryProp = d_ptr->m_propertyToCountry[property];
   if (countryProp) {
      d_ptr->m_countryToProperty.remove(countryProp);
      delete countryProp;
   }
   d_ptr->m_propertyToCountry.remove(property);

   d_ptr->m_values.remove(property);
}

class QtPointPropertyManagerPrivate
{
 public:
   void slotIntChanged(QtProperty *property, int value);
   void slotPropertyDestroyed(QtProperty *property);

   QMap<const QtProperty *, QPoint> m_values;

   QtIntPropertyManager *m_intPropertyManager;

   QMap<const QtProperty *, QtProperty *> m_propertyToX;
   QMap<const QtProperty *, QtProperty *> m_propertyToY;

   QMap<const QtProperty *, QtProperty *> m_xToProperty;
   QMap<const QtProperty *, QtProperty *> m_yToProperty;

 private:
   QtPointPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtPointPropertyManager)
};

void QtPointPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
   if (QtProperty *xprop = m_xToProperty.value(property, nullptr)) {
      QPoint p = m_values[xprop];
      p.setX(value);
      q_ptr->setValue(xprop, p);

   } else if (QtProperty *yprop = m_yToProperty.value(property, nullptr)) {
      QPoint p = m_values[yprop];
      p.setY(value);
      q_ptr->setValue(yprop, p);
   }
}

void QtPointPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
   if (QtProperty *xPointProp = m_xToProperty.value(property, nullptr)) {
      m_propertyToX[xPointProp] = nullptr;
      m_xToProperty.remove(property);

   } else if (QtProperty *yPointProp = m_yToProperty.value(property, nullptr)) {
      m_propertyToY[yPointProp] = nullptr;
      m_yToProperty.remove(property);
   }
}

QtPointPropertyManager::QtPointPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtPointPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);

   connect(d_ptr->m_intPropertyManager, &QtIntPropertyManager::valueChanged,
         this, &QtPointPropertyManager::slotIntChanged);

   connect(d_ptr->m_intPropertyManager, &QtIntPropertyManager::propertyDestroyed,
         this, &QtPointPropertyManager::slotPropertyDestroyed);
}

QtPointPropertyManager::~QtPointPropertyManager()
{
   clear();
}

QtIntPropertyManager *QtPointPropertyManager::subIntPropertyManager() const
{
   return d_ptr->m_intPropertyManager;
}


QPoint QtPointPropertyManager::value(const QtProperty *property) const
{
   return d_ptr->m_values.value(property, QPoint());
}

QString QtPointPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   const QPoint v = it.value();
   return tr("(%1, %2)").formatArg(QString::number(v.x()))
      .formatArg(QString::number(v.y()));
}

void QtPointPropertyManager::setValue(QtProperty *property, const QPoint &val)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   if (it.value() == val) {
      return;
   }

   it.value() = val;
   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToX[property], val.x());
   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToY[property], val.y());

   emit propertyChanged(property);
   emit valueChanged(property, val);
}

void QtPointPropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QPoint(0, 0);

   QtProperty *xProp = d_ptr->m_intPropertyManager->addProperty();
   xProp->setPropertyName(tr("X"));
   d_ptr->m_intPropertyManager->setValue(xProp, 0);
   d_ptr->m_propertyToX[property] = xProp;
   d_ptr->m_xToProperty[xProp] = property;
   property->addSubProperty(xProp);

   QtProperty *yProp = d_ptr->m_intPropertyManager->addProperty();
   yProp->setPropertyName(tr("Y"));
   d_ptr->m_intPropertyManager->setValue(yProp, 0);
   d_ptr->m_propertyToY[property] = yProp;
   d_ptr->m_yToProperty[yProp] = property;
   property->addSubProperty(yProp);
}

void QtPointPropertyManager::uninitializeProperty(QtProperty *property)
{
   QtProperty *xProp = d_ptr->m_propertyToX[property];
   if (xProp) {
      d_ptr->m_xToProperty.remove(xProp);
      delete xProp;
   }
   d_ptr->m_propertyToX.remove(property);

   QtProperty *yProp = d_ptr->m_propertyToY[property];
   if (yProp) {
      d_ptr->m_yToProperty.remove(yProp);
      delete yProp;
   }
   d_ptr->m_propertyToY.remove(property);

   d_ptr->m_values.remove(property);
}

class QtPointFPropertyManagerPrivate
{
 public:
   struct Data {
      Data()
         : decimals(2)
      {
      }

      QPointF val;
      int decimals;
   };

   void slotDoubleChanged(QtProperty *property, double value);
   void slotPropertyDestroyed(QtProperty *property);

   QMap<const QtProperty *, Data>  m_values;

   QtDoublePropertyManager *m_doublePropertyManager;

   QMap<const QtProperty *, QtProperty *> m_propertyToX;
   QMap<const QtProperty *, QtProperty *> m_propertyToY;

   QMap<const QtProperty *, QtProperty *> m_xToProperty;
   QMap<const QtProperty *, QtProperty *> m_yToProperty;

 private:
   QtPointFPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtPointFPropertyManager)
};

void QtPointFPropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
   if (QtProperty *xProp = m_xToProperty.value(property, nullptr)) {
      QPointF p = m_values[xProp].val;
      p.setX(value);
      q_ptr->setValue(xProp, p);

   } else if (QtProperty *yProp = m_yToProperty.value(property, nullptr)) {
      QPointF p = m_values[yProp].val;
      p.setY(value);
      q_ptr->setValue(yProp, p);
   }
}

void QtPointFPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
   if (QtProperty *xPointProp  = m_xToProperty.value(property, nullptr)) {
      m_propertyToX[xPointProp] = nullptr;
      m_xToProperty.remove(property);

   } else if (QtProperty *yPointProp = m_yToProperty.value(property, nullptr)) {
      m_propertyToY[yPointProp] = nullptr;
      m_yToProperty.remove(property);
   }
}

QtPointFPropertyManager::QtPointFPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtPointFPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);

   connect(d_ptr->m_doublePropertyManager, &QtDoublePropertyManager::valueChanged,
         this, &QtPointFPropertyManager::slotDoubleChanged);

   connect(d_ptr->m_doublePropertyManager, &QtDoublePropertyManager::propertyDestroyed,
         this, &QtPointFPropertyManager::slotPropertyDestroyed);
}

QtPointFPropertyManager::~QtPointFPropertyManager()
{
   clear();
}

QtDoublePropertyManager *QtPointFPropertyManager::subDoublePropertyManager() const
{
   return d_ptr->m_doublePropertyManager;
}

QPointF QtPointFPropertyManager::value(const QtProperty *property) const
{
   return getValue<QPointF>(d_ptr->m_values, property);
}

int QtPointFPropertyManager::decimals(const QtProperty *property) const
{
   return getData<int>(d_ptr->m_values, &QtPointFPropertyManagerPrivate::Data::decimals, property, 0);
}

QString QtPointFPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   const QPointF v = it.value().val;
   const int dec =  it.value().decimals;

   return tr("(%1, %2)").formatArg(QString::number(v.x(), 'f', dec))
      .formatArg(QString::number(v.y(), 'f', dec));
}

void QtPointFPropertyManager::setValue(QtProperty *property, const QPointF &val)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   if (it.value().val == val) {
      return;
   }

   it.value().val = val;
   d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToX[property], val.x());
   d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToY[property], val.y());

   emit propertyChanged(property);
   emit valueChanged(property, val);
}

void QtPointFPropertyManager::setDecimals(QtProperty *property, int prec)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtPointFPropertyManagerPrivate::Data data = it.value();

   if (prec > 13) {
      prec = 13;
   } else if (prec < 0) {
      prec = 0;
   }

   if (data.decimals == prec) {
      return;
   }

   data.decimals = prec;
   d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToX[property], prec);
   d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToY[property], prec);

   it.value() = data;

   emit decimalsChanged(property, data.decimals);
}

/*!
    \reimp
*/
void QtPointFPropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QtPointFPropertyManagerPrivate::Data();

   QtProperty *xProp = d_ptr->m_doublePropertyManager->addProperty();
   xProp->setPropertyName(tr("X"));
   d_ptr->m_doublePropertyManager->setDecimals(xProp, decimals(property));
   d_ptr->m_doublePropertyManager->setValue(xProp, 0);
   d_ptr->m_propertyToX[property] = xProp;
   d_ptr->m_xToProperty[xProp] = property;
   property->addSubProperty(xProp);

   QtProperty *yProp = d_ptr->m_doublePropertyManager->addProperty();
   yProp->setPropertyName(tr("Y"));
   d_ptr->m_doublePropertyManager->setDecimals(yProp, decimals(property));
   d_ptr->m_doublePropertyManager->setValue(yProp, 0);
   d_ptr->m_propertyToY[property] = yProp;
   d_ptr->m_yToProperty[yProp] = property;
   property->addSubProperty(yProp);
}

/*!
    \reimp
*/
void QtPointFPropertyManager::uninitializeProperty(QtProperty *property)
{
   QtProperty *xProp = d_ptr->m_propertyToX[property];
   if (xProp) {
      d_ptr->m_xToProperty.remove(xProp);
      delete xProp;
   }
   d_ptr->m_propertyToX.remove(property);

   QtProperty *yProp = d_ptr->m_propertyToY[property];
   if (yProp) {
      d_ptr->m_yToProperty.remove(yProp);
      delete yProp;
   }
   d_ptr->m_propertyToY.remove(property);

   d_ptr->m_values.remove(property);
}

class QtSizePropertyManagerPrivate
{
 public:
   void slotIntChanged(QtProperty *property, int value);
   void slotPropertyDestroyed(QtProperty *property);
   void setValue(QtProperty *property, const QSize &val);
   void setRange(QtProperty *property, const QSize &minVal, const QSize &maxVal, const QSize &val);

   struct Data {
      Data()
         : val(QSize(0, 0)), minVal(QSize(0, 0)), maxVal(QSize(INT_MAX, INT_MAX))
      {
      }

      QSize val;
      QSize minVal;
      QSize maxVal;

      QSize minimumValue() const {
         return minVal;
      }

      QSize maximumValue() const {
         return maxVal;
      }

      void setMinimumValue(const QSize &newMinVal) {
         setSizeMinimumData(this, newMinVal);
      }

      void setMaximumValue(const QSize &newMaxVal) {
         setSizeMaximumData(this, newMaxVal);
      }
   };

   QMap<const QtProperty *, Data> m_values;

   QtIntPropertyManager *m_intPropertyManager;

   QMap<const QtProperty *, QtProperty *> m_propertyToW;
   QMap<const QtProperty *, QtProperty *> m_propertyToH;

   QMap<const QtProperty *, QtProperty *> m_wToProperty;
   QMap<const QtProperty *, QtProperty *> m_hToProperty;

 private:
   QtSizePropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtSizePropertyManager)
};

void QtSizePropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
   if (QtProperty *wProp = m_wToProperty.value(property, nullptr)) {
      QSize s = m_values[wProp].val;
      s.setWidth(value);
      q_ptr->setValue(wProp, s);

   } else if (QtProperty *hProp = m_hToProperty.value(property, nullptr)) {
      QSize s = m_values[hProp].val;
      s.setHeight(value);
      q_ptr->setValue(hProp, s);
   }
}

void QtSizePropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
   if (QtProperty *wPointProp = m_wToProperty.value(property, nullptr)) {
      m_propertyToW[wPointProp] = nullptr;
      m_wToProperty.remove(property);

   } else if (QtProperty *hPointProp = m_hToProperty.value(property, nullptr)) {
      m_propertyToH[hPointProp] = nullptr;
      m_hToProperty.remove(property);
   }
}

void QtSizePropertyManagerPrivate::setValue(QtProperty *property, const QSize &val)
{
   m_intPropertyManager->setValue(m_propertyToW.value(property), val.width());
   m_intPropertyManager->setValue(m_propertyToH.value(property), val.height());
}

void QtSizePropertyManagerPrivate::setRange(QtProperty *property,
   const QSize &minVal, const QSize &maxVal, const QSize &val)
{
   QtProperty *wProperty = m_propertyToW.value(property);
   QtProperty *hProperty = m_propertyToH.value(property);
   m_intPropertyManager->setRange(wProperty, minVal.width(), maxVal.width());
   m_intPropertyManager->setValue(wProperty, val.width());
   m_intPropertyManager->setRange(hProperty, minVal.height(), maxVal.height());
   m_intPropertyManager->setValue(hProperty, val.height());
}

QtSizePropertyManager::QtSizePropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtSizePropertyManagerPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);

   connect(d_ptr->m_intPropertyManager, &QtIntPropertyManager::valueChanged,
         this, &QtSizePropertyManager::slotIntChanged);

   connect(d_ptr->m_intPropertyManager, &QtIntPropertyManager::propertyDestroyed,
         this, &QtSizePropertyManager::slotPropertyDestroyed);
}

QtSizePropertyManager::~QtSizePropertyManager()
{
   clear();
}

QtIntPropertyManager *QtSizePropertyManager::subIntPropertyManager() const
{
   return d_ptr->m_intPropertyManager;
}

QSize QtSizePropertyManager::value(const QtProperty *property) const
{
   return getValue<QSize>(d_ptr->m_values, property);
}

QSize QtSizePropertyManager::minimum(const QtProperty *property) const
{
   return getMinimum<QSize>(d_ptr->m_values, property);
}

QSize QtSizePropertyManager::maximum(const QtProperty *property) const
{
   return getMaximum<QSize>(d_ptr->m_values, property);
}

QString QtSizePropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   const QSize v = it.value().val;

   return tr("%1 x %2").formatArg(QString::number(v.width()))
      .formatArg(QString::number(v.height()));
}

void QtSizePropertyManager::setValue(QtProperty *property, const QSize &val)
{
   setValueInRange<const QSize &, QtSizePropertyManagerPrivate, QtSizePropertyManager, const QSize>(this, d_ptr.data(),
      &QtSizePropertyManager::propertyChanged,
      &QtSizePropertyManager::valueChanged,
      property, val, &QtSizePropertyManagerPrivate::setValue);
}

/*!
    Sets the minimum size value for the given \a property to \a minVal.

    When setting the minimum size value, the maximum and current
    values are adjusted if necessary (ensuring that the size range
    remains valid and that the current value is within the range).

    \sa minimum(), setRange(), rangeChanged()
*/
void QtSizePropertyManager::setMinimum(QtProperty *property, const QSize &minVal)
{
   setBorderValue<const QSize &, QtSizePropertyManagerPrivate, QtSizePropertyManager, QSize, QtSizePropertyManagerPrivate::Data>(this,
      d_ptr.data(),
      &QtSizePropertyManager::propertyChanged,
      &QtSizePropertyManager::valueChanged,
      &QtSizePropertyManager::rangeChanged,
      property,
      &QtSizePropertyManagerPrivate::Data::minimumValue,
      &QtSizePropertyManagerPrivate::Data::setMinimumValue,
      minVal, &QtSizePropertyManagerPrivate::setRange);
}

/*!
    Sets the maximum size value for the given \a property to \a maxVal.

    When setting the maximum size value, the minimum and current
    values are adjusted if necessary (ensuring that the size range
    remains valid and that the current value is within the range).

    \sa maximum(), setRange(), rangeChanged()
*/
void QtSizePropertyManager::setMaximum(QtProperty *property, const QSize &maxVal)
{
   setBorderValue<const QSize &, QtSizePropertyManagerPrivate, QtSizePropertyManager, QSize, QtSizePropertyManagerPrivate::Data>(this,
      d_ptr.data(),
      &QtSizePropertyManager::propertyChanged,
      &QtSizePropertyManager::valueChanged,
      &QtSizePropertyManager::rangeChanged,
      property,
      &QtSizePropertyManagerPrivate::Data::maximumValue,
      &QtSizePropertyManagerPrivate::Data::setMaximumValue,
      maxVal, &QtSizePropertyManagerPrivate::setRange);
}

/*!
    \fn void QtSizePropertyManager::setRange(QtProperty *property, const QSize &minimum, const QSize &maximum)

    Sets the range of valid values.

    This is a convenience function defining the range of valid values
    in one go; setting the \a minimum and \a maximum values for the
    given \a property with a single function call.

    When setting a new range, the current value is adjusted if
    necessary (ensuring that the value remains within the range).

    \sa setMinimum(), setMaximum(), rangeChanged()
*/
void QtSizePropertyManager::setRange(QtProperty *property, const QSize &minVal, const QSize &maxVal)
{
   setBorderValues<const QSize &, QtSizePropertyManagerPrivate, QtSizePropertyManager, QSize>(this, d_ptr.data(),
      &QtSizePropertyManager::propertyChanged,
      &QtSizePropertyManager::valueChanged,
      &QtSizePropertyManager::rangeChanged,
      property, minVal, maxVal, &QtSizePropertyManagerPrivate::setRange);
}

/*!
    \reimp
*/
void QtSizePropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QtSizePropertyManagerPrivate::Data();

   QtProperty *wProp = d_ptr->m_intPropertyManager->addProperty();
   wProp->setPropertyName(tr("Width"));
   d_ptr->m_intPropertyManager->setValue(wProp, 0);
   d_ptr->m_intPropertyManager->setMinimum(wProp, 0);
   d_ptr->m_propertyToW[property] = wProp;
   d_ptr->m_wToProperty[wProp] = property;
   property->addSubProperty(wProp);

   QtProperty *hProp = d_ptr->m_intPropertyManager->addProperty();
   hProp->setPropertyName(tr("Height"));
   d_ptr->m_intPropertyManager->setValue(hProp, 0);
   d_ptr->m_intPropertyManager->setMinimum(hProp, 0);
   d_ptr->m_propertyToH[property] = hProp;
   d_ptr->m_hToProperty[hProp] = property;
   property->addSubProperty(hProp);
}

/*!
    \reimp
*/
void QtSizePropertyManager::uninitializeProperty(QtProperty *property)
{
   QtProperty *wProp = d_ptr->m_propertyToW[property];
   if (wProp) {
      d_ptr->m_wToProperty.remove(wProp);
      delete wProp;
   }
   d_ptr->m_propertyToW.remove(property);

   QtProperty *hProp = d_ptr->m_propertyToH[property];
   if (hProp) {
      d_ptr->m_hToProperty.remove(hProp);
      delete hProp;
   }
   d_ptr->m_propertyToH.remove(property);

   d_ptr->m_values.remove(property);
}

class QtSizeFPropertyManagerPrivate
{
 public:
   void slotDoubleChanged(QtProperty *property, double value);
   void slotPropertyDestroyed(QtProperty *property);
   void setValue(QtProperty *property, const QSizeF &val);
   void setRange(QtProperty *property, const QSizeF &minVal, const QSizeF &maxVal, const QSizeF &val);

   struct Data {
      Data()
         : val(QSizeF(0, 0)), minVal(QSizeF(0, 0)), maxVal(QSizeF(INT_MAX, INT_MAX)), decimals(2)
      {
      }

      QSizeF val;
      QSizeF minVal;
      QSizeF maxVal;
      int decimals;

      QSizeF minimumValue() const {
         return minVal;
      }
      QSizeF maximumValue() const {
         return maxVal;
      }
      void setMinimumValue(const QSizeF &newMinVal) {
         setSizeMinimumData(this, newMinVal);
      }
      void setMaximumValue(const QSizeF &newMaxVal) {
         setSizeMaximumData(this, newMaxVal);
      }
   };

   QMap<const QtProperty *, Data> m_values;

   QtDoublePropertyManager *m_doublePropertyManager;

   QMap<const QtProperty *, QtProperty *> m_propertyToW;
   QMap<const QtProperty *, QtProperty *> m_propertyToH;

   QMap<const QtProperty *, QtProperty *> m_wToProperty;
   QMap<const QtProperty *, QtProperty *> m_hToProperty;

 private:
   QtSizeFPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtSizeFPropertyManager)
};

void QtSizeFPropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
   if (QtProperty *wProp = m_wToProperty.value(property, nullptr)) {
      QSizeF s = m_values[wProp].val;
      s.setWidth(value);
      q_ptr->setValue(wProp, s);

   } else if (QtProperty *hProp = m_hToProperty.value(property, nullptr)) {
      QSizeF s = m_values[hProp].val;
      s.setHeight(value);
      q_ptr->setValue(hProp, s);
   }
}

void QtSizeFPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
   if (QtProperty *wPointProp = m_wToProperty.value(property, nullptr)) {
      m_propertyToW[wPointProp] = nullptr;
      m_wToProperty.remove(property);

   } else if (QtProperty *hPointProp = m_hToProperty.value(property, nullptr)) {
      m_propertyToH[hPointProp] = nullptr;
      m_hToProperty.remove(property);
   }
}

void QtSizeFPropertyManagerPrivate::setValue(QtProperty *property, const QSizeF &val)
{
   m_doublePropertyManager->setValue(m_propertyToW.value(property), val.width());
   m_doublePropertyManager->setValue(m_propertyToH.value(property), val.height());
}

void QtSizeFPropertyManagerPrivate::setRange(QtProperty *property,
   const QSizeF &minVal, const QSizeF &maxVal, const QSizeF &val)
{
   m_doublePropertyManager->setRange(m_propertyToW[property], minVal.width(), maxVal.width());
   m_doublePropertyManager->setValue(m_propertyToW[property], val.width());
   m_doublePropertyManager->setRange(m_propertyToH[property], minVal.height(), maxVal.height());
   m_doublePropertyManager->setValue(m_propertyToH[property], val.height());
}

QtSizeFPropertyManager::QtSizeFPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtSizeFPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);

   connect(d_ptr->m_doublePropertyManager, &QtDoublePropertyManager::valueChanged,
         this, &QtSizeFPropertyManager::slotDoubleChanged);

   connect(d_ptr->m_doublePropertyManager, &QtDoublePropertyManager::propertyDestroyed,
         this, &QtSizeFPropertyManager::slotPropertyDestroyed);
}

QtSizeFPropertyManager::~QtSizeFPropertyManager()
{
   clear();
}

QtDoublePropertyManager *QtSizeFPropertyManager::subDoublePropertyManager() const
{
   return d_ptr->m_doublePropertyManager;
}

QSizeF QtSizeFPropertyManager::value(const QtProperty *property) const
{
   return getValue<QSizeF>(d_ptr->m_values, property);
}

int QtSizeFPropertyManager::decimals(const QtProperty *property) const
{
   return getData<int>(d_ptr->m_values, &QtSizeFPropertyManagerPrivate::Data::decimals, property, 0);
}

QSizeF QtSizeFPropertyManager::minimum(const QtProperty *property) const
{
   return getMinimum<QSizeF>(d_ptr->m_values, property);
}

QSizeF QtSizeFPropertyManager::maximum(const QtProperty *property) const
{
   return getMaximum<QSizeF>(d_ptr->m_values, property);
}

QString QtSizeFPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   const QSizeF v = it.value().val;
   const int dec = it.value().decimals;

   return tr("%1 x %2").formatArg(QString::number(v.width(), 'f', dec))
      .formatArg(QString::number(v.height(), 'f', dec));
}

void QtSizeFPropertyManager::setValue(QtProperty *property, const QSizeF &val)
{
   setValueInRange<const QSizeF &, QtSizeFPropertyManagerPrivate, QtSizeFPropertyManager, QSizeF>(this, d_ptr.data(),
      &QtSizeFPropertyManager::propertyChanged,
      &QtSizeFPropertyManager::valueChanged,
      property, val, &QtSizeFPropertyManagerPrivate::setValue);
}

void QtSizeFPropertyManager::setDecimals(QtProperty *property, int prec)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtSizeFPropertyManagerPrivate::Data data = it.value();

   if (prec > 13) {
      prec = 13;
   } else if (prec < 0) {
      prec = 0;
   }

   if (data.decimals == prec) {
      return;
   }

   data.decimals = prec;
   d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToW[property], prec);
   d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToH[property], prec);

   it.value() = data;

   emit decimalsChanged(property, data.decimals);
}

void QtSizeFPropertyManager::setMinimum(QtProperty *property, const QSizeF &minVal)
{
   setBorderValue<const QSizeF &, QtSizeFPropertyManagerPrivate, QtSizeFPropertyManager, QSizeF, QtSizeFPropertyManagerPrivate::Data>
   (this, d_ptr.data(),
      &QtSizeFPropertyManager::propertyChanged,
      &QtSizeFPropertyManager::valueChanged,
      &QtSizeFPropertyManager::rangeChanged,
      property,
      &QtSizeFPropertyManagerPrivate::Data::minimumValue,
      &QtSizeFPropertyManagerPrivate::Data::setMinimumValue,
      minVal, &QtSizeFPropertyManagerPrivate::setRange);
}

void QtSizeFPropertyManager::setMaximum(QtProperty *property, const QSizeF &maxVal)
{
   setBorderValue<const QSizeF &, QtSizeFPropertyManagerPrivate, QtSizeFPropertyManager, QSizeF, QtSizeFPropertyManagerPrivate::Data>
   (this, d_ptr.data(),
      &QtSizeFPropertyManager::propertyChanged,
      &QtSizeFPropertyManager::valueChanged,
      &QtSizeFPropertyManager::rangeChanged,
      property,
      &QtSizeFPropertyManagerPrivate::Data::maximumValue,
      &QtSizeFPropertyManagerPrivate::Data::setMaximumValue,
      maxVal, &QtSizeFPropertyManagerPrivate::setRange);
}

void QtSizeFPropertyManager::setRange(QtProperty *property, const QSizeF &minVal, const QSizeF &maxVal)
{
   setBorderValues<const QSizeF &, QtSizeFPropertyManagerPrivate, QtSizeFPropertyManager, QSizeF>(this, d_ptr.data(),
      &QtSizeFPropertyManager::propertyChanged,
      &QtSizeFPropertyManager::valueChanged,
      &QtSizeFPropertyManager::rangeChanged,
      property, minVal, maxVal, &QtSizeFPropertyManagerPrivate::setRange);
}

void QtSizeFPropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QtSizeFPropertyManagerPrivate::Data();

   QtProperty *wProp = d_ptr->m_doublePropertyManager->addProperty();
   wProp->setPropertyName(tr("Width"));
   d_ptr->m_doublePropertyManager->setDecimals(wProp, decimals(property));
   d_ptr->m_doublePropertyManager->setValue(wProp, 0);
   d_ptr->m_doublePropertyManager->setMinimum(wProp, 0);
   d_ptr->m_propertyToW[property] = wProp;
   d_ptr->m_wToProperty[wProp] = property;
   property->addSubProperty(wProp);

   QtProperty *hProp = d_ptr->m_doublePropertyManager->addProperty();
   hProp->setPropertyName(tr("Height"));
   d_ptr->m_doublePropertyManager->setDecimals(hProp, decimals(property));
   d_ptr->m_doublePropertyManager->setValue(hProp, 0);
   d_ptr->m_doublePropertyManager->setMinimum(hProp, 0);
   d_ptr->m_propertyToH[property] = hProp;
   d_ptr->m_hToProperty[hProp] = property;
   property->addSubProperty(hProp);
}

void QtSizeFPropertyManager::uninitializeProperty(QtProperty *property)
{
   QtProperty *wProp = d_ptr->m_propertyToW[property];
   if (wProp) {
      d_ptr->m_wToProperty.remove(wProp);
      delete wProp;
   }
   d_ptr->m_propertyToW.remove(property);

   QtProperty *hProp = d_ptr->m_propertyToH[property];
   if (hProp) {
      d_ptr->m_hToProperty.remove(hProp);
      delete hProp;
   }
   d_ptr->m_propertyToH.remove(property);

   d_ptr->m_values.remove(property);
}

class QtRectPropertyManagerPrivate
{
 public:
   struct Data {
      Data()
         : val(0, 0, 0, 0)
      {
      }

      QRect val;
      QRect constraint;
   };

   void slotIntChanged(QtProperty *property, int value);
   void slotPropertyDestroyed(QtProperty *property);
   void setConstraint(QtProperty *property, const QRect &constraint, const QRect &val);

   QMap<const QtProperty *, Data> m_values;

   QtIntPropertyManager *m_intPropertyManager;

   QMap<const QtProperty *, QtProperty *> m_propertyToX;
   QMap<const QtProperty *, QtProperty *> m_propertyToY;
   QMap<const QtProperty *, QtProperty *> m_propertyToW;
   QMap<const QtProperty *, QtProperty *> m_propertyToH;

   QMap<const QtProperty *, QtProperty *> m_xToProperty;
   QMap<const QtProperty *, QtProperty *> m_yToProperty;
   QMap<const QtProperty *, QtProperty *> m_wToProperty;
   QMap<const QtProperty *, QtProperty *> m_hToProperty;

 private:
   QtRectPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtRectPropertyManager)
};

void QtRectPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
   if (QtProperty *xProp = m_xToProperty.value(property, nullptr)) {
      QRect r = m_values[xProp].val;
      r.moveLeft(value);
      q_ptr->setValue(xProp, r);

   } else if (QtProperty *yProp = m_yToProperty.value(property)) {
      QRect r = m_values[yProp].val;
      r.moveTop(value);
      q_ptr->setValue(yProp, r);

   } else if (QtProperty *wProp = m_wToProperty.value(property, nullptr)) {
      Data data = m_values[wProp];
      QRect r = data.val;
      r.setWidth(value);

      if (! data.constraint.isNull() && data.constraint.x() + data.constraint.width() < r.x() + r.width()) {
         r.moveLeft(data.constraint.left() + data.constraint.width() - r.width());
      }
      q_ptr->setValue(wProp, r);

   } else if (QtProperty *hProp = m_hToProperty.value(property, nullptr)) {
      Data data = m_values[hProp];
      QRect r = data.val;
      r.setHeight(value);

      if (! data.constraint.isNull() && data.constraint.y() + data.constraint.height() < r.y() + r.height()) {
         r.moveTop(data.constraint.top() + data.constraint.height() - r.height());
      }
      q_ptr->setValue(hProp, r);
   }
}

void QtRectPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
   if (QtProperty *xPointProp = m_xToProperty.value(property, nullptr)) {
      m_propertyToX[xPointProp] = nullptr;
      m_xToProperty.remove(property);

   } else if (QtProperty *yPointProp = m_yToProperty.value(property, nullptr)) {
      m_propertyToY[yPointProp] = nullptr;
      m_yToProperty.remove(property);

   } else if (QtProperty *wPointProp = m_wToProperty.value(property, nullptr)) {
      m_propertyToW[wPointProp] = nullptr;
      m_wToProperty.remove(property);

   } else if (QtProperty *hPointProp = m_hToProperty.value(property, nullptr)) {
      m_propertyToH[hPointProp] = nullptr;
      m_hToProperty.remove(property);
   }
}

void QtRectPropertyManagerPrivate::setConstraint(QtProperty *property,
   const QRect &constraint, const QRect &val)
{
   const bool isNull = constraint.isNull();

   const int left   = isNull ? INT_MIN : constraint.left();
   const int right  = isNull ? INT_MAX : constraint.left() + constraint.width();
   const int top    = isNull ? INT_MIN : constraint.top();
   const int bottom = isNull ? INT_MAX : constraint.top() + constraint.height();
   const int width  = isNull ? INT_MAX : constraint.width();
   const int height = isNull ? INT_MAX : constraint.height();

   m_intPropertyManager->setRange(m_propertyToX[property], left, right);
   m_intPropertyManager->setRange(m_propertyToY[property], top, bottom);
   m_intPropertyManager->setRange(m_propertyToW[property], 0, width);
   m_intPropertyManager->setRange(m_propertyToH[property], 0, height);

   m_intPropertyManager->setValue(m_propertyToX[property], val.x());
   m_intPropertyManager->setValue(m_propertyToY[property], val.y());
   m_intPropertyManager->setValue(m_propertyToW[property], val.width());
   m_intPropertyManager->setValue(m_propertyToH[property], val.height());
}

QtRectPropertyManager::QtRectPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtRectPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);

   connect(d_ptr->m_intPropertyManager, &QtIntPropertyManager::valueChanged,
      this, &QtRectPropertyManager::slotIntChanged);

   connect(d_ptr->m_intPropertyManager, &QtIntPropertyManager::propertyDestroyed,
      this, &QtRectPropertyManager::slotPropertyDestroyed);
}

QtRectPropertyManager::~QtRectPropertyManager()
{
   clear();
}

QtIntPropertyManager *QtRectPropertyManager::subIntPropertyManager() const
{
   return d_ptr->m_intPropertyManager;
}

QRect QtRectPropertyManager::value(const QtProperty *property) const
{
   return getValue<QRect>(d_ptr->m_values, property);
}

QRect QtRectPropertyManager::constraint(const QtProperty *property) const
{
   return getData<QRect>(d_ptr->m_values, &QtRectPropertyManagerPrivate::Data::constraint, property, QRect());
}

QString QtRectPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   const QRect v = it.value().val;

   return tr("[(%1, %2), %3 x %4]").formatArg(QString::number(v.x()))
      .formatArg(QString::number(v.y()))
      .formatArg(QString::number(v.width()))
      .formatArg(QString::number(v.height()));
}

void QtRectPropertyManager::setValue(QtProperty *property, const QRect &val)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtRectPropertyManagerPrivate::Data data = it.value();

   QRect newRect = val.normalized();
   if (! data.constraint.isNull() && !data.constraint.contains(newRect)) {
      const QRect r1 = data.constraint;
      const QRect r2 = newRect;

      newRect.setLeft(qMax(r1.left(), r2.left()));
      newRect.setRight(qMin(r1.right(), r2.right()));
      newRect.setTop(qMax(r1.top(), r2.top()));
      newRect.setBottom(qMin(r1.bottom(), r2.bottom()));

      if (newRect.width() < 0 || newRect.height() < 0) {
         return;
      }
   }

   if (data.val == newRect) {
      return;
   }

   data.val = newRect;

   it.value() = data;
   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToX[property], newRect.x());
   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToY[property], newRect.y());
   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToW[property], newRect.width());
   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToH[property], newRect.height());

   emit propertyChanged(property);
   emit valueChanged(property, data.val);
}

void QtRectPropertyManager::setConstraint(QtProperty *property, const QRect &constraint)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtRectPropertyManagerPrivate::Data data = it.value();

   QRect newConstraint = constraint.normalized();
   if (data.constraint == newConstraint) {
      return;
   }

   const QRect oldVal = data.val;

   data.constraint = newConstraint;

   if (!data.constraint.isNull() && !data.constraint.contains(oldVal)) {
      QRect r1 = data.constraint;
      QRect r2 = data.val;

      if (r2.width() > r1.width()) {
         r2.setWidth(r1.width());
      }
      if (r2.height() > r1.height()) {
         r2.setHeight(r1.height());
      }
      if (r2.left() < r1.left()) {
         r2.moveLeft(r1.left());
      } else if (r2.right() > r1.right()) {
         r2.moveRight(r1.right());
      }
      if (r2.top() < r1.top()) {
         r2.moveTop(r1.top());
      } else if (r2.bottom() > r1.bottom()) {
         r2.moveBottom(r1.bottom());
      }

      data.val = r2;
   }

   it.value() = data;

   emit constraintChanged(property, data.constraint);

   d_ptr->setConstraint(property, data.constraint, data.val);

   if (data.val == oldVal) {
      return;
   }

   emit propertyChanged(property);
   emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtRectPropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QtRectPropertyManagerPrivate::Data();

   QtProperty *xProp = d_ptr->m_intPropertyManager->addProperty();
   xProp->setPropertyName(tr("X"));
   d_ptr->m_intPropertyManager->setValue(xProp, 0);
   d_ptr->m_propertyToX[property] = xProp;
   d_ptr->m_xToProperty[xProp] = property;
   property->addSubProperty(xProp);

   QtProperty *yProp = d_ptr->m_intPropertyManager->addProperty();
   yProp->setPropertyName(tr("Y"));
   d_ptr->m_intPropertyManager->setValue(yProp, 0);
   d_ptr->m_propertyToY[property] = yProp;
   d_ptr->m_yToProperty[yProp] = property;
   property->addSubProperty(yProp);

   QtProperty *wProp = d_ptr->m_intPropertyManager->addProperty();
   wProp->setPropertyName(tr("Width"));
   d_ptr->m_intPropertyManager->setValue(wProp, 0);
   d_ptr->m_intPropertyManager->setMinimum(wProp, 0);
   d_ptr->m_propertyToW[property] = wProp;
   d_ptr->m_wToProperty[wProp] = property;
   property->addSubProperty(wProp);

   QtProperty *hProp = d_ptr->m_intPropertyManager->addProperty();
   hProp->setPropertyName(tr("Height"));
   d_ptr->m_intPropertyManager->setValue(hProp, 0);
   d_ptr->m_intPropertyManager->setMinimum(hProp, 0);
   d_ptr->m_propertyToH[property] = hProp;
   d_ptr->m_hToProperty[hProp] = property;
   property->addSubProperty(hProp);
}

/*!
    \reimp
*/
void QtRectPropertyManager::uninitializeProperty(QtProperty *property)
{
   QtProperty *xProp = d_ptr->m_propertyToX[property];
   if (xProp) {
      d_ptr->m_xToProperty.remove(xProp);
      delete xProp;
   }
   d_ptr->m_propertyToX.remove(property);

   QtProperty *yProp = d_ptr->m_propertyToY[property];
   if (yProp) {
      d_ptr->m_yToProperty.remove(yProp);
      delete yProp;
   }
   d_ptr->m_propertyToY.remove(property);

   QtProperty *wProp = d_ptr->m_propertyToW[property];
   if (wProp) {
      d_ptr->m_wToProperty.remove(wProp);
      delete wProp;
   }
   d_ptr->m_propertyToW.remove(property);

   QtProperty *hProp = d_ptr->m_propertyToH[property];
   if (hProp) {
      d_ptr->m_hToProperty.remove(hProp);
      delete hProp;
   }
   d_ptr->m_propertyToH.remove(property);

   d_ptr->m_values.remove(property);
}

// QtRectFPropertyManager

class QtRectFPropertyManagerPrivate
{
 public:
   struct Data {
      Data()
         : val(0, 0, 0, 0), decimals(2)
      {
      }

      QRectF val;
      QRectF constraint;
      int decimals;
   };

   void slotDoubleChanged(QtProperty *property, double value);
   void slotPropertyDestroyed(QtProperty *property);
   void setConstraint(QtProperty *property, const QRectF &constraint, const QRectF &val);

   QMap<const QtProperty *, Data> m_values;

   QtDoublePropertyManager *m_doublePropertyManager;

   QMap<const QtProperty *, QtProperty *> m_propertyToX;
   QMap<const QtProperty *, QtProperty *> m_propertyToY;
   QMap<const QtProperty *, QtProperty *> m_propertyToW;
   QMap<const QtProperty *, QtProperty *> m_propertyToH;

   QMap<const QtProperty *, QtProperty *> m_xToProperty;
   QMap<const QtProperty *, QtProperty *> m_yToProperty;
   QMap<const QtProperty *, QtProperty *> m_wToProperty;
   QMap<const QtProperty *, QtProperty *> m_hToProperty;

 private:
   QtRectFPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtRectFPropertyManager)
};

void QtRectFPropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
   if (QtProperty *xProp = m_xToProperty.value(property, nullptr)) {
      QRectF r = m_values[xProp].val;
      r.moveLeft(value);
      q_ptr->setValue(xProp, r);

   } else if (QtProperty *yProp = m_yToProperty.value(property, nullptr)) {
      QRectF r = m_values[yProp].val;
      r.moveTop(value);
      q_ptr->setValue(yProp, r);

   } else if (QtProperty *wProp = m_wToProperty.value(property, nullptr)) {
      Data data = m_values[wProp];
      QRectF r = data.val;
      r.setWidth(value);

      if (!data.constraint.isNull() && data.constraint.x() + data.constraint.width() < r.x() + r.width()) {
         r.moveLeft(data.constraint.left() + data.constraint.width() - r.width());
      }
      q_ptr->setValue(wProp, r);

   } else if (QtProperty *hProp = m_hToProperty.value(property, nullptr)) {
      Data data = m_values[hProp];
      QRectF r = data.val;
      r.setHeight(value);

      if (!data.constraint.isNull() && data.constraint.y() + data.constraint.height() < r.y() + r.height()) {
         r.moveTop(data.constraint.top() + data.constraint.height() - r.height());
      }
      q_ptr->setValue(hProp, r);
   }
}

void QtRectFPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
   if (QtProperty *xPointProp = m_xToProperty.value(property, nullptr)) {
      m_propertyToX[xPointProp] = nullptr;
      m_xToProperty.remove(property);

   } else if (QtProperty *yPointProp = m_yToProperty.value(property, nullptr)) {
      m_propertyToY[yPointProp] = nullptr;
      m_yToProperty.remove(property);

   } else if (QtProperty *wPointProp = m_wToProperty.value(property, nullptr)) {
      m_propertyToW[wPointProp] = nullptr;
      m_wToProperty.remove(property);

   } else if (QtProperty *hPointProp = m_hToProperty.value(property, nullptr)) {
      m_propertyToH[hPointProp] = nullptr;
      m_hToProperty.remove(property);
   }
}

void QtRectFPropertyManagerPrivate::setConstraint(QtProperty *property,
   const QRectF &constraint, const QRectF &val)
{
   const bool isNull  = constraint.isNull();
   const float left   = isNull ? FLT_MIN : constraint.left();
   const float right  = isNull ? FLT_MAX : constraint.left() + constraint.width();
   const float top    = isNull ? FLT_MIN : constraint.top();
   const float bottom = isNull ? FLT_MAX : constraint.top() + constraint.height();
   const float width  = isNull ? FLT_MAX : constraint.width();
   const float height = isNull ? FLT_MAX : constraint.height();

   m_doublePropertyManager->setRange(m_propertyToX[property], left, right);
   m_doublePropertyManager->setRange(m_propertyToY[property], top, bottom);
   m_doublePropertyManager->setRange(m_propertyToW[property], 0, width);
   m_doublePropertyManager->setRange(m_propertyToH[property], 0, height);

   m_doublePropertyManager->setValue(m_propertyToX[property], val.x());
   m_doublePropertyManager->setValue(m_propertyToY[property], val.y());
   m_doublePropertyManager->setValue(m_propertyToW[property], val.width());
   m_doublePropertyManager->setValue(m_propertyToH[property], val.height());
}

QtRectFPropertyManager::QtRectFPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtRectFPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);

   connect(d_ptr->m_doublePropertyManager, &QtDoublePropertyManager::valueChanged,
      this, &QtRectFPropertyManager::slotDoubleChanged);

   connect(d_ptr->m_doublePropertyManager, &QtDoublePropertyManager::propertyDestroyed,
      this, &QtRectFPropertyManager::slotPropertyDestroyed);
}

QtRectFPropertyManager::~QtRectFPropertyManager()
{
   clear();
}

QtDoublePropertyManager *QtRectFPropertyManager::subDoublePropertyManager() const
{
   return d_ptr->m_doublePropertyManager;
}

QRectF QtRectFPropertyManager::value(const QtProperty *property) const
{
   return getValue<QRectF>(d_ptr->m_values, property);
}

int QtRectFPropertyManager::decimals(const QtProperty *property) const
{
   return getData<int>(d_ptr->m_values, &QtRectFPropertyManagerPrivate::Data::decimals, property, 0);
}

QRectF QtRectFPropertyManager::constraint(const QtProperty *property) const
{
   return getData<QRectF>(d_ptr->m_values, &QtRectFPropertyManagerPrivate::Data::constraint, property, QRect());
}

QString QtRectFPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   const QRectF v = it.value().val;
   const int dec = it.value().decimals;

   return QString(tr("[(%1, %2), %3 x %4]").formatArg(QString::number(v.x(), 'f', dec))
         .formatArg(QString::number(v.y(), 'f', dec))
         .formatArg(QString::number(v.width(), 'f', dec))
         .formatArg(QString::number(v.height(), 'f', dec)));
}

void QtRectFPropertyManager::setValue(QtProperty *property, const QRectF &val)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtRectFPropertyManagerPrivate::Data data = it.value();

   QRectF newRect = val.normalized();
   if (!data.constraint.isNull() && !data.constraint.contains(newRect)) {
      const QRectF r1 = data.constraint;
      const QRectF r2 = newRect;
      newRect.setLeft(qMax(r1.left(), r2.left()));
      newRect.setRight(qMin(r1.right(), r2.right()));
      newRect.setTop(qMax(r1.top(), r2.top()));
      newRect.setBottom(qMin(r1.bottom(), r2.bottom()));
      if (newRect.width() < 0 || newRect.height() < 0) {
         return;
      }
   }

   if (data.val == newRect) {
      return;
   }

   data.val = newRect;

   it.value() = data;
   d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToX[property], newRect.x());
   d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToY[property], newRect.y());
   d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToW[property], newRect.width());
   d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToH[property], newRect.height());

   emit propertyChanged(property);
   emit valueChanged(property, data.val);
}

void QtRectFPropertyManager::setConstraint(QtProperty *property, const QRectF &constraint)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtRectFPropertyManagerPrivate::Data data = it.value();

   QRectF newConstraint = constraint.normalized();
   if (data.constraint == newConstraint) {
      return;
   }

   const QRectF oldVal = data.val;

   data.constraint = newConstraint;

   if (!data.constraint.isNull() && !data.constraint.contains(oldVal)) {
      QRectF r1 = data.constraint;
      QRectF r2 = data.val;

      if (r2.width() > r1.width()) {
         r2.setWidth(r1.width());
      }
      if (r2.height() > r1.height()) {
         r2.setHeight(r1.height());
      }
      if (r2.left() < r1.left()) {
         r2.moveLeft(r1.left());
      } else if (r2.right() > r1.right()) {
         r2.moveRight(r1.right());
      }
      if (r2.top() < r1.top()) {
         r2.moveTop(r1.top());
      } else if (r2.bottom() > r1.bottom()) {
         r2.moveBottom(r1.bottom());
      }

      data.val = r2;
   }

   it.value() = data;

   emit constraintChanged(property, data.constraint);

   d_ptr->setConstraint(property, data.constraint, data.val);

   if (data.val == oldVal) {
      return;
   }

   emit propertyChanged(property);
   emit valueChanged(property, data.val);
}

void QtRectFPropertyManager::setDecimals(QtProperty *property, int prec)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtRectFPropertyManagerPrivate::Data data = it.value();

   if (prec > 13) {
      prec = 13;
   } else if (prec < 0) {
      prec = 0;
   }

   if (data.decimals == prec) {
      return;
   }

   data.decimals = prec;
   d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToX[property], prec);
   d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToY[property], prec);
   d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToW[property], prec);
   d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToH[property], prec);

   it.value() = data;

   emit decimalsChanged(property, data.decimals);
}

/*!
    \reimp
*/
void QtRectFPropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QtRectFPropertyManagerPrivate::Data();

   QtProperty *xProp = d_ptr->m_doublePropertyManager->addProperty();
   xProp->setPropertyName(tr("X"));
   d_ptr->m_doublePropertyManager->setDecimals(xProp, decimals(property));
   d_ptr->m_doublePropertyManager->setValue(xProp, 0);
   d_ptr->m_propertyToX[property] = xProp;
   d_ptr->m_xToProperty[xProp] = property;
   property->addSubProperty(xProp);

   QtProperty *yProp = d_ptr->m_doublePropertyManager->addProperty();
   yProp->setPropertyName(tr("Y"));
   d_ptr->m_doublePropertyManager->setDecimals(yProp, decimals(property));
   d_ptr->m_doublePropertyManager->setValue(yProp, 0);
   d_ptr->m_propertyToY[property] = yProp;
   d_ptr->m_yToProperty[yProp] = property;
   property->addSubProperty(yProp);

   QtProperty *wProp = d_ptr->m_doublePropertyManager->addProperty();
   wProp->setPropertyName(tr("Width"));
   d_ptr->m_doublePropertyManager->setDecimals(wProp, decimals(property));
   d_ptr->m_doublePropertyManager->setValue(wProp, 0);
   d_ptr->m_doublePropertyManager->setMinimum(wProp, 0);
   d_ptr->m_propertyToW[property] = wProp;
   d_ptr->m_wToProperty[wProp] = property;
   property->addSubProperty(wProp);

   QtProperty *hProp = d_ptr->m_doublePropertyManager->addProperty();
   hProp->setPropertyName(tr("Height"));
   d_ptr->m_doublePropertyManager->setDecimals(hProp, decimals(property));
   d_ptr->m_doublePropertyManager->setValue(hProp, 0);
   d_ptr->m_doublePropertyManager->setMinimum(hProp, 0);
   d_ptr->m_propertyToH[property] = hProp;
   d_ptr->m_hToProperty[hProp] = property;
   property->addSubProperty(hProp);
}

/*!
    \reimp
*/
void QtRectFPropertyManager::uninitializeProperty(QtProperty *property)
{
   QtProperty *xProp = d_ptr->m_propertyToX[property];
   if (xProp) {
      d_ptr->m_xToProperty.remove(xProp);
      delete xProp;
   }
   d_ptr->m_propertyToX.remove(property);

   QtProperty *yProp = d_ptr->m_propertyToY[property];
   if (yProp) {
      d_ptr->m_yToProperty.remove(yProp);
      delete yProp;
   }
   d_ptr->m_propertyToY.remove(property);

   QtProperty *wProp = d_ptr->m_propertyToW[property];
   if (wProp) {
      d_ptr->m_wToProperty.remove(wProp);
      delete wProp;
   }
   d_ptr->m_propertyToW.remove(property);

   QtProperty *hProp = d_ptr->m_propertyToH[property];
   if (hProp) {
      d_ptr->m_hToProperty.remove(hProp);
      delete hProp;
   }
   d_ptr->m_propertyToH.remove(property);

   d_ptr->m_values.remove(property);
}

class QtEnumPropertyManagerPrivate
{
 public:
   struct Data {
      Data()
         : val(-1)
      {
      }

      int val;
      QStringList enumNames;
      QMap<int, QIcon> enumIcons;
   };

   QMap<const QtProperty *, Data> m_values;

 private:
   QtEnumPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtEnumPropertyManager)
};

QtEnumPropertyManager::QtEnumPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtEnumPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;
}

QtEnumPropertyManager::~QtEnumPropertyManager()
{
   clear();
}

int QtEnumPropertyManager::value(const QtProperty *property) const
{
   return getValue<int>(d_ptr->m_values, property, -1);
}

QStringList QtEnumPropertyManager::enumNames(const QtProperty *property) const
{
   return getData<QStringList>(d_ptr->m_values, &QtEnumPropertyManagerPrivate::Data::enumNames, property, QStringList());
}

QMap<int, QIcon> QtEnumPropertyManager::enumIcons(const QtProperty *property) const
{
   return getData<QMap<int, QIcon>>(d_ptr->m_values, &QtEnumPropertyManagerPrivate::Data::enumIcons, property, QMap<int, QIcon>());
}

QString QtEnumPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   const QtEnumPropertyManagerPrivate::Data &data = it.value();

   const int v = data.val;
   if (v >= 0 && v < data.enumNames.count()) {
      return data.enumNames.at(v);
   }

   return QString();
}

QIcon QtEnumPropertyManager::valueIcon(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QIcon();
   }

   const QtEnumPropertyManagerPrivate::Data &data = it.value();

   const int v = data.val;
   return data.enumIcons.value(v);
}

void QtEnumPropertyManager::setValue(QtProperty *property, int val)
{
   auto it = d_ptr->m_values.find(property);
   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtEnumPropertyManagerPrivate::Data data = it.value();

   if (val >= data.enumNames.count()) {
      return;
   }

   if (val < 0 && data.enumNames.count() > 0) {
      return;
   }

   if (val < 0) {
      val = -1;
   }

   if (data.val == val) {
      return;
   }

   data.val = val;

   it.value() = data;

   emit propertyChanged(property);
   emit valueChanged(property, data.val);
}

void QtEnumPropertyManager::setEnumNames(QtProperty *property, const QStringList &enumNames)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtEnumPropertyManagerPrivate::Data data = it.value();

   QStringList list = enumNames;

   // sort the list of ennum values
   // std::sort(list.begin(), list.end());

   if (data.enumNames == list) {
      return;
   }

   data.enumNames = std::move(list);
   data.val = -1;

   if (data.enumNames.count() > 0) {
      data.val = 0;
   }

   it.value() = data;

   emit enumNamesChanged(property, data.enumNames);
   emit propertyChanged(property);
   emit valueChanged(property, data.val);
}

void QtEnumPropertyManager::setEnumIcons(QtProperty *property, const QMap<int, QIcon> &enumIcons)
{
   auto it = d_ptr->m_values.find(property);
   if (it == d_ptr->m_values.end()) {
      return;
   }

   it.value().enumIcons = enumIcons;

   emit enumIconsChanged(property, it.value().enumIcons);
   emit propertyChanged(property);
}

void QtEnumPropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values.insert(property, QtEnumPropertyManagerPrivate::Data());
}

void QtEnumPropertyManager::uninitializeProperty(QtProperty *property)
{
   d_ptr->m_values.remove(property);
}

class QtFlagPropertyManagerPrivate
{
 public:
   struct Data {
      Data()
         : val(-1)
      {
      }

      int val;
      QStringList flagNames;
   };

   void slotBoolChanged(QtProperty *property, bool value);
   void slotPropertyDestroyed(QtProperty *property);

   QMap<const QtProperty *, Data> m_values;

   QtBoolPropertyManager *m_boolPropertyManager;

   QMap<const QtProperty *, QList<QtProperty *>> m_propertyToFlags;
   QMap<const QtProperty *, QtProperty *> m_flagToProperty;

 private:
   QtFlagPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtFlagPropertyManager)
};

void QtFlagPropertyManagerPrivate::slotBoolChanged(QtProperty *property, bool value)
{
   QtProperty *prop = m_flagToProperty.value(property, nullptr);
   if (prop == nullptr) {
      return;
   }

   QListIterator<QtProperty *> itProp(m_propertyToFlags[prop]);
   int level = 0;

   while (itProp.hasNext()) {
      QtProperty *p = itProp.next();

      if (p == property) {
         int v = m_values[prop].val;

         if (value) {
            v |= (1 << level);
         } else {
            v &= ~(1 << level);
         }
         q_ptr->setValue(prop, v);
         return;
      }
      level++;
   }
}

void QtFlagPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
   QtProperty *flagProperty = m_flagToProperty.value(property, nullptr);
   if (flagProperty == nullptr) {
      return;
   }

   m_propertyToFlags[flagProperty].replace(m_propertyToFlags[flagProperty].indexOf(property), nullptr);
   m_flagToProperty.remove(property);
}

QtFlagPropertyManager::QtFlagPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtFlagPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->m_boolPropertyManager = new QtBoolPropertyManager(this);

   connect(d_ptr->m_boolPropertyManager, &QtBoolPropertyManager::valueChanged,
      this, &QtFlagPropertyManager::slotBoolChanged);

   connect(d_ptr->m_boolPropertyManager, &QtBoolPropertyManager::propertyDestroyed,
      this, &QtFlagPropertyManager::slotPropertyDestroyed);
}

QtFlagPropertyManager::~QtFlagPropertyManager()
{
   clear();
}

QtBoolPropertyManager *QtFlagPropertyManager::subBoolPropertyManager() const
{
   return d_ptr->m_boolPropertyManager;
}

int QtFlagPropertyManager::value(const QtProperty *property) const
{
   return getValue<int>(d_ptr->m_values, property, 0);
}

QStringList QtFlagPropertyManager::flagNames(const QtProperty *property) const
{
   return getData<QStringList>(d_ptr->m_values, &QtFlagPropertyManagerPrivate::Data::flagNames, property, QStringList());
}

QString QtFlagPropertyManager::valueText(const QtProperty *property) const
{
   auto iterFind = d_ptr->m_values.constFind(property);

   if (iterFind == d_ptr->m_values.constEnd()) {
      return QString();
   }

   const QtFlagPropertyManagerPrivate::Data &data = iterFind.value();

   QString str;

   int level = 0;
   const QChar bar = QChar('|');

   for (const auto &item : data.flagNames) {

      if (data.val & (1 << level)) {
         if (! str.isEmpty()) {
            str += bar;
         }

         str += item;
      }

      ++level;
   }

   return str;
}

void QtFlagPropertyManager::setValue(QtProperty *property, int val)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtFlagPropertyManagerPrivate::Data data = it.value();

   if (data.val == val) {
      return;
   }

   if (val > (1 << data.flagNames.count()) - 1) {
      return;
   }

   if (val < 0) {
      return;
   }

   data.val = val;

   it.value() = data;

   QListIterator<QtProperty *> itProp(d_ptr->m_propertyToFlags[property]);
   int level = 0;

   while (itProp.hasNext()) {
      QtProperty *prop = itProp.next();

      if (prop != nullptr) {
         d_ptr->m_boolPropertyManager->setValue(prop, val & (1 << level));
      }

      ++level;
   }

   emit propertyChanged(property);
   emit valueChanged(property, data.val);
}

void QtFlagPropertyManager::setFlagNames(QtProperty *property, const QStringList &flagNames)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   QtFlagPropertyManagerPrivate::Data data = it.value();

   if (data.flagNames == flagNames) {
      return;
   }

   data.flagNames = flagNames;
   data.val = 0;

   it.value() = data;

   QListIterator<QtProperty *> itProp(d_ptr->m_propertyToFlags[property]);

   while (itProp.hasNext()) {
      QtProperty *prop = itProp.next();

      if (prop) {
         delete prop;
         d_ptr->m_flagToProperty.remove(prop);
      }
   }
   d_ptr->m_propertyToFlags[property].clear();

   QStringListIterator itFlag(flagNames);

   while (itFlag.hasNext()) {
      const QString flagName = itFlag.next();

      QtProperty *prop = d_ptr->m_boolPropertyManager->addProperty();
      prop->setPropertyName(flagName);
      property->addSubProperty(prop);
      d_ptr->m_propertyToFlags[property].append(prop);
      d_ptr->m_flagToProperty[prop] = property;
   }

   emit flagNamesChanged(property, data.flagNames);
   emit propertyChanged(property);
   emit valueChanged(property, data.val);
}

void QtFlagPropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QtFlagPropertyManagerPrivate::Data();

   d_ptr->m_propertyToFlags[property] = QList<QtProperty *>();
}

void QtFlagPropertyManager::uninitializeProperty(QtProperty *property)
{
   QListIterator<QtProperty *> itProp(d_ptr->m_propertyToFlags[property]);

   while (itProp.hasNext()) {
      QtProperty *prop = itProp.next();

      if (prop) {
         delete prop;
         d_ptr->m_flagToProperty.remove(prop);
      }
   }
   d_ptr->m_propertyToFlags.remove(property);

   d_ptr->m_values.remove(property);
}

// QtSizePolicyPropertyManager

class QtSizePolicyPropertyManagerPrivate
{
 public:
   QtSizePolicyPropertyManagerPrivate();

   void slotIntChanged(QtProperty *property, int value);
   void slotEnumChanged(QtProperty *property, int value);
   void slotPropertyDestroyed(QtProperty *property);

   QMap<const QtProperty *, QSizePolicy> m_values;

   QtIntPropertyManager *m_intPropertyManager;
   QtEnumPropertyManager *m_enumPropertyManager;

   QMap<const QtProperty *, QtProperty *> m_propertyToHPolicy;
   QMap<const QtProperty *, QtProperty *> m_propertyToVPolicy;
   QMap<const QtProperty *, QtProperty *> m_propertyToHStretch;
   QMap<const QtProperty *, QtProperty *> m_propertyToVStretch;

   QMap<const QtProperty *, QtProperty *> m_hPolicyToProperty;
   QMap<const QtProperty *, QtProperty *> m_vPolicyToProperty;
   QMap<const QtProperty *, QtProperty *> m_hStretchToProperty;
   QMap<const QtProperty *, QtProperty *> m_vStretchToProperty;

 private:
   QtSizePolicyPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtSizePolicyPropertyManager)
};

QtSizePolicyPropertyManagerPrivate::QtSizePolicyPropertyManagerPrivate()
{
}

void QtSizePolicyPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
   if (QtProperty *hProp = m_hStretchToProperty.value(property, nullptr)) {
      QSizePolicy sp = m_values[hProp];
      sp.setHorizontalStretch(value);
      q_ptr->setValue(hProp, sp);

   } else if (QtProperty *vProp = m_vStretchToProperty.value(property, nullptr)) {
      QSizePolicy sp = m_values[vProp];
      sp.setVerticalStretch(value);
      q_ptr->setValue(vProp, sp);
   }
}

void QtSizePolicyPropertyManagerPrivate::slotEnumChanged(QtProperty *property, int value)
{
   if (QtProperty *hProp = m_hPolicyToProperty.value(property, nullptr)) {
      QSizePolicy sp = m_values[hProp];
      sp.setHorizontalPolicy(metaEnumProvider()->indexToSizePolicy(value));
      q_ptr->setValue(hProp, sp);

   } else if (QtProperty *vProp = m_vPolicyToProperty.value(property, nullptr)) {
      QSizePolicy sp = m_values[vProp];
      sp.setVerticalPolicy(metaEnumProvider()->indexToSizePolicy(value));
      q_ptr->setValue(vProp, sp);
   }
}

void QtSizePolicyPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
   if (QtProperty *hPointProp = m_hStretchToProperty.value(property, nullptr)) {
      m_propertyToHStretch[hPointProp] = nullptr;
      m_hStretchToProperty.remove(property);

   } else if (QtProperty *vPointProp = m_vStretchToProperty.value(property, nullptr)) {
      m_propertyToVStretch[vPointProp] = nullptr;
      m_vStretchToProperty.remove(property);

   } else if (QtProperty *hPolicyPointProp = m_hPolicyToProperty.value(property, nullptr)) {
      m_propertyToHPolicy[hPolicyPointProp] = nullptr;
      m_hPolicyToProperty.remove(property);

   } else if (QtProperty *vPolicyPointProp = m_vPolicyToProperty.value(property, nullptr)) {
      m_propertyToVPolicy[vPolicyPointProp] = nullptr;
      m_vPolicyToProperty.remove(property);
   }
}

QtSizePolicyPropertyManager::QtSizePolicyPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtSizePolicyPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);

   connect(d_ptr->m_intPropertyManager, &QtIntPropertyManager::valueChanged,
         this, &QtSizePolicyPropertyManager::slotIntChanged);

   connect(d_ptr->m_intPropertyManager, &QtIntPropertyManager::propertyDestroyed,
         this, &QtSizePolicyPropertyManager::slotPropertyDestroyed);

   d_ptr->m_enumPropertyManager = new QtEnumPropertyManager(this);

   connect(d_ptr->m_enumPropertyManager, &QtEnumPropertyManager::valueChanged,
         this, &QtSizePolicyPropertyManager::slotEnumChanged);

   connect(d_ptr->m_enumPropertyManager, &QtEnumPropertyManager::propertyDestroyed,
         this, &QtSizePolicyPropertyManager::slotPropertyDestroyed);
}

QtSizePolicyPropertyManager::~QtSizePolicyPropertyManager()
{
   clear();
}

QtIntPropertyManager *QtSizePolicyPropertyManager::subIntPropertyManager() const
{
   return d_ptr->m_intPropertyManager;
}

QtEnumPropertyManager *QtSizePolicyPropertyManager::subEnumPropertyManager() const
{
   return d_ptr->m_enumPropertyManager;
}

QSizePolicy QtSizePolicyPropertyManager::value(const QtProperty *property) const
{
   return d_ptr->m_values.value(property, QSizePolicy());
}

QString QtSizePolicyPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   const QSizePolicy sp = it.value();
   const QtMetaEnumProvider *mep = metaEnumProvider();
   const int hIndex = mep->sizePolicyToIndex(sp.horizontalPolicy());
   const int vIndex = mep->sizePolicyToIndex(sp.verticalPolicy());

   //! Unknown size policy on reading invalid uic3 files
   const QString hPolicy = hIndex != -1 ? mep->policyEnumNames().at(hIndex) : tr("<Invalid>");
   const QString vPolicy = vIndex != -1 ? mep->policyEnumNames().at(vIndex) : tr("<Invalid>");

   const QString str = tr("[%1, %2, %3, %4]").formatArg(hPolicy).formatArg(vPolicy)
         .formatArg(sp.horizontalStretch()).formatArg(sp.verticalStretch());

   return str;
}

void QtSizePolicyPropertyManager::setValue(QtProperty *property, const QSizePolicy &val)
{
   auto  it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   if (it.value() == val) {
      return;
   }

   it.value() = val;

   d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToHPolicy[property],
      metaEnumProvider()->sizePolicyToIndex(val.horizontalPolicy()));
   d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToVPolicy[property],
      metaEnumProvider()->sizePolicyToIndex(val.verticalPolicy()));
   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToHStretch[property],
      val.horizontalStretch());
   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToVStretch[property],
      val.verticalStretch());

   emit propertyChanged(property);
   emit valueChanged(property, val);
}


void QtSizePolicyPropertyManager::initializeProperty(QtProperty *property)
{
   QSizePolicy val;
   d_ptr->m_values[property] = val;

   QtProperty *hPolicyProp = d_ptr->m_enumPropertyManager->addProperty();
   hPolicyProp->setPropertyName(tr("Horizontal Policy"));
   d_ptr->m_enumPropertyManager->setEnumNames(hPolicyProp, metaEnumProvider()->policyEnumNames());
   d_ptr->m_enumPropertyManager->setValue(hPolicyProp,
      metaEnumProvider()->sizePolicyToIndex(val.horizontalPolicy()));
   d_ptr->m_propertyToHPolicy[property] = hPolicyProp;
   d_ptr->m_hPolicyToProperty[hPolicyProp] = property;
   property->addSubProperty(hPolicyProp);

   QtProperty *vPolicyProp = d_ptr->m_enumPropertyManager->addProperty();
   vPolicyProp->setPropertyName(tr("Vertical Policy"));
   d_ptr->m_enumPropertyManager->setEnumNames(vPolicyProp, metaEnumProvider()->policyEnumNames());
   d_ptr->m_enumPropertyManager->setValue(vPolicyProp,
      metaEnumProvider()->sizePolicyToIndex(val.verticalPolicy()));
   d_ptr->m_propertyToVPolicy[property] = vPolicyProp;
   d_ptr->m_vPolicyToProperty[vPolicyProp] = property;
   property->addSubProperty(vPolicyProp);

   QtProperty *hStretchProp = d_ptr->m_intPropertyManager->addProperty();
   hStretchProp->setPropertyName(tr("Horizontal Stretch"));
   d_ptr->m_intPropertyManager->setValue(hStretchProp, val.horizontalStretch());
   d_ptr->m_intPropertyManager->setRange(hStretchProp, 0, 0xff);
   d_ptr->m_propertyToHStretch[property] = hStretchProp;
   d_ptr->m_hStretchToProperty[hStretchProp] = property;
   property->addSubProperty(hStretchProp);

   QtProperty *vStretchProp = d_ptr->m_intPropertyManager->addProperty();
   vStretchProp->setPropertyName(tr("Vertical Stretch"));
   d_ptr->m_intPropertyManager->setValue(vStretchProp, val.verticalStretch());
   d_ptr->m_intPropertyManager->setRange(vStretchProp, 0, 0xff);
   d_ptr->m_propertyToVStretch[property] = vStretchProp;
   d_ptr->m_vStretchToProperty[vStretchProp] = property;
   property->addSubProperty(vStretchProp);

}

/*!
    \reimp
*/
void QtSizePolicyPropertyManager::uninitializeProperty(QtProperty *property)
{
   QtProperty *hPolicyProp = d_ptr->m_propertyToHPolicy[property];
   if (hPolicyProp) {
      d_ptr->m_hPolicyToProperty.remove(hPolicyProp);
      delete hPolicyProp;
   }
   d_ptr->m_propertyToHPolicy.remove(property);

   QtProperty *vPolicyProp = d_ptr->m_propertyToVPolicy[property];
   if (vPolicyProp) {
      d_ptr->m_vPolicyToProperty.remove(vPolicyProp);
      delete vPolicyProp;
   }
   d_ptr->m_propertyToVPolicy.remove(property);

   QtProperty *hStretchProp = d_ptr->m_propertyToHStretch[property];
   if (hStretchProp) {
      d_ptr->m_hStretchToProperty.remove(hStretchProp);
      delete hStretchProp;
   }
   d_ptr->m_propertyToHStretch.remove(property);

   QtProperty *vStretchProp = d_ptr->m_propertyToVStretch[property];
   if (vStretchProp) {
      d_ptr->m_vStretchToProperty.remove(vStretchProp);
      delete vStretchProp;
   }
   d_ptr->m_propertyToVStretch.remove(property);

   d_ptr->m_values.remove(property);
}

// QtFontPropertyManagerPrivate has a mechanism for reacting
// to QApplication::fontDatabaseChanged() which is emitted
// when someone loads an application font. The signals are compressed
// using a timer with interval 0, which then causes the family
// enumeration manager to re-set its strings and index values
// for each property.

Q_GLOBAL_STATIC(QFontDatabase, fontDatabase)

class QtFontPropertyManagerPrivate
{
 public:
   QtFontPropertyManagerPrivate();

   void slotIntChanged(QtProperty *property, int value);
   void slotEnumChanged(QtProperty *property, int value);
   void slotBoolChanged(QtProperty *property, bool value);
   void slotPropertyDestroyed(QtProperty *property);
   void slotFontDatabaseChanged();
   void slotFontDatabaseDelayedChange();

   QStringList m_familyNames;
   QMap<const QtProperty *, QFont> m_values;

   QtIntPropertyManager *m_intPropertyManager;
   QtEnumPropertyManager *m_enumPropertyManager;
   QtBoolPropertyManager *m_boolPropertyManager;

   QMap<const QtProperty *, QtProperty *> m_propertyToFamily;
   QMap<const QtProperty *, QtProperty *> m_propertyToPointSize;
   QMap<const QtProperty *, QtProperty *> m_propertyToBold;
   QMap<const QtProperty *, QtProperty *> m_propertyToItalic;
   QMap<const QtProperty *, QtProperty *> m_propertyToUnderline;
   QMap<const QtProperty *, QtProperty *> m_propertyToStrikeOut;
   QMap<const QtProperty *, QtProperty *> m_propertyToKerning;

   QMap<const QtProperty *, QtProperty *> m_familyToProperty;
   QMap<const QtProperty *, QtProperty *> m_pointSizeToProperty;
   QMap<const QtProperty *, QtProperty *> m_boldToProperty;
   QMap<const QtProperty *, QtProperty *> m_italicToProperty;
   QMap<const QtProperty *, QtProperty *> m_underlineToProperty;
   QMap<const QtProperty *, QtProperty *> m_strikeOutToProperty;
   QMap<const QtProperty *, QtProperty *> m_kerningToProperty;

   bool m_settingValue;
   QTimer *m_fontDatabaseChangeTimer;

 private:
   QtFontPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtFontPropertyManager)
};

QtFontPropertyManagerPrivate::QtFontPropertyManagerPrivate()
   : m_settingValue(false), m_fontDatabaseChangeTimer(nullptr)
{
}

void QtFontPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
   if (m_settingValue) {
      return;
   }
   if (QtProperty *prop = m_pointSizeToProperty.value(property, nullptr)) {
      QFont f = m_values[prop];
      f.setPointSize(value);
      q_ptr->setValue(prop, f);
   }
}

void QtFontPropertyManagerPrivate::slotEnumChanged(QtProperty *property, int value)
{
   if (m_settingValue) {
      return;
   }

   if (QtProperty *prop = m_familyToProperty.value(property, nullptr)) {
      QFont f = m_values[prop];
      f.setFamily(m_familyNames.at(value));
      q_ptr->setValue(prop, f);
   }
}

void QtFontPropertyManagerPrivate::slotBoolChanged(QtProperty *property, bool value)
{
   if (m_settingValue) {
      return;
   }

   if (QtProperty *boldProp = m_boldToProperty.value(property, nullptr)) {
      QFont fontBold = m_values[boldProp];
      fontBold.setBold(value);
      q_ptr->setValue(boldProp, fontBold);

   } else if (QtProperty *italicProp = m_italicToProperty.value(property, nullptr)) {
      QFont fontItalic = m_values[italicProp];
      fontItalic.setItalic(value);
      q_ptr->setValue(italicProp, fontItalic);

   } else if (QtProperty *underlineProp = m_underlineToProperty.value(property, nullptr)) {
      QFont fontUnderLine = m_values[underlineProp];
      fontUnderLine.setUnderline(value);
      q_ptr->setValue(underlineProp, fontUnderLine);

   } else if (QtProperty *strikeOutProp = m_strikeOutToProperty.value(property, nullptr)) {
      QFont fontStrikOut = m_values[strikeOutProp];
      fontStrikOut.setStrikeOut(value);
      q_ptr->setValue(strikeOutProp, fontStrikOut);

   } else if (QtProperty *kerningProp = m_kerningToProperty.value(property, nullptr)) {
      QFont fontKerning = m_values[kerningProp];
      fontKerning.setKerning(value);
      q_ptr->setValue(kerningProp, fontKerning);
   }
}

void QtFontPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
   if (QtProperty *sizePointProp = m_pointSizeToProperty.value(property, nullptr)) {
      m_propertyToPointSize[sizePointProp] = nullptr;
      m_pointSizeToProperty.remove(property);

   } else if (QtProperty *familyPointProp = m_familyToProperty.value(property, nullptr)) {
      m_propertyToFamily[familyPointProp] = nullptr;
      m_familyToProperty.remove(property);

   } else if (QtProperty *boldPointProp = m_boldToProperty.value(property, nullptr)) {
      m_propertyToBold[boldPointProp] = nullptr;
      m_boldToProperty.remove(property);

   } else if (QtProperty *italicPointProp = m_italicToProperty.value(property, nullptr)) {
      m_propertyToItalic[italicPointProp] = nullptr;
      m_italicToProperty.remove(property);

   } else if (QtProperty *pointProp = m_underlineToProperty.value(property, nullptr)) {
      m_propertyToUnderline[pointProp] = nullptr;
      m_underlineToProperty.remove(property);

   } else if (QtProperty *strikeOutPointProp = m_strikeOutToProperty.value(property, nullptr)) {
      m_propertyToStrikeOut[strikeOutPointProp] = nullptr;
      m_strikeOutToProperty.remove(property);

   } else if (QtProperty *kerningPointProp = m_kerningToProperty.value(property, nullptr)) {
      m_propertyToKerning[kerningPointProp] = nullptr;
      m_kerningToProperty.remove(property);
   }
}

void QtFontPropertyManagerPrivate::slotFontDatabaseChanged()
{
   if (!m_fontDatabaseChangeTimer) {
      m_fontDatabaseChangeTimer = new QTimer(q_ptr);
      m_fontDatabaseChangeTimer->setInterval(0);
      m_fontDatabaseChangeTimer->setSingleShot(true);

      QObject::connect(m_fontDatabaseChangeTimer, &QTimer::timeout,
            q_ptr, &QtFontPropertyManager::slotFontDatabaseDelayedChange);
   }

   if (! m_fontDatabaseChangeTimer->isActive()) {
      m_fontDatabaseChangeTimer->start();
   }
}

void QtFontPropertyManagerPrivate::slotFontDatabaseDelayedChange()
{
   // rescan available font names
   const QStringList oldFamilies = m_familyNames;
   m_familyNames = fontDatabase()->families();

   // Adapt all existing properties
   if (! m_propertyToFamily.empty()) {
      auto cend = m_propertyToFamily.constEnd();

      for (auto it = m_propertyToFamily.constBegin(); it != cend; ++it) {
         QtProperty *familyProp = it.value();

         const int oldIdx = m_enumPropertyManager->value(familyProp);
         int newIdx = m_familyNames.indexOf(oldFamilies.at(oldIdx));

         if (newIdx < 0) {
            newIdx = 0;
         }

         m_enumPropertyManager->setEnumNames(familyProp, m_familyNames);
         m_enumPropertyManager->setValue(familyProp, newIdx);
      }
   }
}

QtFontPropertyManager::QtFontPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtFontPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;

   QObject::connect(qApp, &QApplication::fontDatabaseChanged,
         this, &QtFontPropertyManager::slotFontDatabaseChanged);

   d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);

   connect(d_ptr->m_intPropertyManager, &QtIntPropertyManager::valueChanged,
         this, &QtFontPropertyManager::slotIntChanged);

   connect(d_ptr->m_intPropertyManager, &QtIntPropertyManager::propertyDestroyed,
         this, &QtFontPropertyManager::slotPropertyDestroyed);

   d_ptr->m_enumPropertyManager = new QtEnumPropertyManager(this);

   connect(d_ptr->m_enumPropertyManager, &QtEnumPropertyManager::valueChanged,
         this, &QtFontPropertyManager::slotEnumChanged);

   connect(d_ptr->m_enumPropertyManager, &QtEnumPropertyManager::propertyDestroyed,
         this, &QtFontPropertyManager::slotPropertyDestroyed);

   d_ptr->m_boolPropertyManager = new QtBoolPropertyManager(this);

   connect(d_ptr->m_boolPropertyManager, &QtBoolPropertyManager::valueChanged,
         this, &QtFontPropertyManager::slotBoolChanged);

   connect(d_ptr->m_boolPropertyManager, &QtBoolPropertyManager::propertyDestroyed,
         this, &QtFontPropertyManager::slotPropertyDestroyed);
}

QtFontPropertyManager::~QtFontPropertyManager()
{
   clear();
}

QtIntPropertyManager *QtFontPropertyManager::subIntPropertyManager() const
{
   return d_ptr->m_intPropertyManager;
}

QtEnumPropertyManager *QtFontPropertyManager::subEnumPropertyManager() const
{
   return d_ptr->m_enumPropertyManager;
}

QtBoolPropertyManager *QtFontPropertyManager::subBoolPropertyManager() const
{
   return d_ptr->m_boolPropertyManager;
}

QFont QtFontPropertyManager::value(const QtProperty *property) const
{
   return d_ptr->m_values.value(property, QFont());
}

QString QtFontPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   return QtPropertyBrowserUtils::fontValueText(it.value());
}

QIcon QtFontPropertyManager::valueIcon(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QIcon();
   }

   return QtPropertyBrowserUtils::fontValueIcon(it.value());
}

void QtFontPropertyManager::setValue(QtProperty *property, const QFont &val)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   const QFont oldVal = it.value();
   if (oldVal == val && oldVal.resolve() == val.resolve()) {
      return;
   }

   it.value() = val;

   int idx = d_ptr->m_familyNames.indexOf(val.family());
   if (idx == -1) {
      idx = 0;
   }

   bool settingValue = d_ptr->m_settingValue;
   d_ptr->m_settingValue = true;
   d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToFamily[property], idx);
   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToPointSize[property], val.pointSize());
   d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToBold[property], val.bold());
   d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToItalic[property], val.italic());
   d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToUnderline[property], val.underline());
   d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToStrikeOut[property], val.strikeOut());
   d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToKerning[property], val.kerning());
   d_ptr->m_settingValue = settingValue;

   emit propertyChanged(property);
   emit valueChanged(property, val);
}

void QtFontPropertyManager::initializeProperty(QtProperty *property)
{
   QFont val;
   d_ptr->m_values[property] = val;

   QtProperty *familyProp = d_ptr->m_enumPropertyManager->addProperty();
   familyProp->setPropertyName(tr("Family"));
   if (d_ptr->m_familyNames.empty()) {
      d_ptr->m_familyNames = fontDatabase()->families();
   }
   d_ptr->m_enumPropertyManager->setEnumNames(familyProp, d_ptr->m_familyNames);
   int idx = d_ptr->m_familyNames.indexOf(val.family());
   if (idx == -1) {
      idx = 0;
   }
   d_ptr->m_enumPropertyManager->setValue(familyProp, idx);
   d_ptr->m_propertyToFamily[property] = familyProp;
   d_ptr->m_familyToProperty[familyProp] = property;
   property->addSubProperty(familyProp);

   QtProperty *pointSizeProp = d_ptr->m_intPropertyManager->addProperty();
   pointSizeProp->setPropertyName(tr("Point Size"));
   d_ptr->m_intPropertyManager->setValue(pointSizeProp, val.pointSize());
   d_ptr->m_intPropertyManager->setMinimum(pointSizeProp, 1);
   d_ptr->m_propertyToPointSize[property] = pointSizeProp;
   d_ptr->m_pointSizeToProperty[pointSizeProp] = property;
   property->addSubProperty(pointSizeProp);

   QtProperty *boldProp = d_ptr->m_boolPropertyManager->addProperty();
   boldProp->setPropertyName(tr("Bold"));
   d_ptr->m_boolPropertyManager->setValue(boldProp, val.bold());
   d_ptr->m_propertyToBold[property] = boldProp;
   d_ptr->m_boldToProperty[boldProp] = property;
   property->addSubProperty(boldProp);

   QtProperty *italicProp = d_ptr->m_boolPropertyManager->addProperty();
   italicProp->setPropertyName(tr("Italic"));
   d_ptr->m_boolPropertyManager->setValue(italicProp, val.italic());
   d_ptr->m_propertyToItalic[property] = italicProp;
   d_ptr->m_italicToProperty[italicProp] = property;
   property->addSubProperty(italicProp);

   QtProperty *underlineProp = d_ptr->m_boolPropertyManager->addProperty();
   underlineProp->setPropertyName(tr("Underline"));
   d_ptr->m_boolPropertyManager->setValue(underlineProp, val.underline());
   d_ptr->m_propertyToUnderline[property] = underlineProp;
   d_ptr->m_underlineToProperty[underlineProp] = property;
   property->addSubProperty(underlineProp);

   QtProperty *strikeOutProp = d_ptr->m_boolPropertyManager->addProperty();
   strikeOutProp->setPropertyName(tr("Strikeout"));
   d_ptr->m_boolPropertyManager->setValue(strikeOutProp, val.strikeOut());
   d_ptr->m_propertyToStrikeOut[property] = strikeOutProp;
   d_ptr->m_strikeOutToProperty[strikeOutProp] = property;
   property->addSubProperty(strikeOutProp);

   QtProperty *kerningProp = d_ptr->m_boolPropertyManager->addProperty();
   kerningProp->setPropertyName(tr("Kerning"));
   d_ptr->m_boolPropertyManager->setValue(kerningProp, val.kerning());
   d_ptr->m_propertyToKerning[property] = kerningProp;
   d_ptr->m_kerningToProperty[kerningProp] = property;
   property->addSubProperty(kerningProp);
}

void QtFontPropertyManager::uninitializeProperty(QtProperty *property)
{
   QtProperty *familyProp = d_ptr->m_propertyToFamily[property];
   if (familyProp) {
      d_ptr->m_familyToProperty.remove(familyProp);
      delete familyProp;
   }
   d_ptr->m_propertyToFamily.remove(property);

   QtProperty *pointSizeProp = d_ptr->m_propertyToPointSize[property];
   if (pointSizeProp) {
      d_ptr->m_pointSizeToProperty.remove(pointSizeProp);
      delete pointSizeProp;
   }
   d_ptr->m_propertyToPointSize.remove(property);

   QtProperty *boldProp = d_ptr->m_propertyToBold[property];
   if (boldProp) {
      d_ptr->m_boldToProperty.remove(boldProp);
      delete boldProp;
   }
   d_ptr->m_propertyToBold.remove(property);

   QtProperty *italicProp = d_ptr->m_propertyToItalic[property];
   if (italicProp) {
      d_ptr->m_italicToProperty.remove(italicProp);
      delete italicProp;
   }
   d_ptr->m_propertyToItalic.remove(property);

   QtProperty *underlineProp = d_ptr->m_propertyToUnderline[property];
   if (underlineProp) {
      d_ptr->m_underlineToProperty.remove(underlineProp);
      delete underlineProp;
   }
   d_ptr->m_propertyToUnderline.remove(property);

   QtProperty *strikeOutProp = d_ptr->m_propertyToStrikeOut[property];
   if (strikeOutProp) {
      d_ptr->m_strikeOutToProperty.remove(strikeOutProp);
      delete strikeOutProp;
   }
   d_ptr->m_propertyToStrikeOut.remove(property);

   QtProperty *kerningProp = d_ptr->m_propertyToKerning[property];
   if (kerningProp) {
      d_ptr->m_kerningToProperty.remove(kerningProp);
      delete kerningProp;
   }
   d_ptr->m_propertyToKerning.remove(property);

   d_ptr->m_values.remove(property);
}

class QtColorPropertyManagerPrivate
{
 public:
   void slotIntChanged(QtProperty *property, int value);
   void slotPropertyDestroyed(QtProperty *property);

   QMap<const QtProperty *, QColor> m_values;

   QtIntPropertyManager *m_intPropertyManager;

   QMap<const QtProperty *, QtProperty *> m_propertyToR;
   QMap<const QtProperty *, QtProperty *> m_propertyToG;
   QMap<const QtProperty *, QtProperty *> m_propertyToB;
   QMap<const QtProperty *, QtProperty *> m_propertyToA;

   QMap<const QtProperty *, QtProperty *> m_rToProperty;
   QMap<const QtProperty *, QtProperty *> m_gToProperty;
   QMap<const QtProperty *, QtProperty *> m_bToProperty;
   QMap<const QtProperty *, QtProperty *> m_aToProperty;

 private:
   QtColorPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtColorPropertyManager)
};

void QtColorPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
   if (QtProperty *rProp = m_rToProperty.value(property, nullptr)) {
      QColor c = m_values[rProp];
      c.setRed(value);
      q_ptr->setValue(rProp, c);

   } else if (QtProperty *gProp = m_gToProperty.value(property, nullptr)) {
      QColor c = m_values[gProp];
      c.setGreen(value);
      q_ptr->setValue(gProp, c);

   } else if (QtProperty *bProp = m_bToProperty.value(property, nullptr)) {
      QColor c = m_values[bProp];
      c.setBlue(value);
      q_ptr->setValue(bProp, c);

   } else if (QtProperty *aProp = m_aToProperty.value(property, nullptr)) {
      QColor c = m_values[aProp];
      c.setAlpha(value);
      q_ptr->setValue(aProp, c);
   }
}

void QtColorPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
   if (QtProperty *rPointProp = m_rToProperty.value(property, nullptr)) {
      m_propertyToR[rPointProp] = nullptr;
      m_rToProperty.remove(property);

   } else if (QtProperty *gPointProp = m_gToProperty.value(property, nullptr)) {
      m_propertyToG[gPointProp] = nullptr;
      m_gToProperty.remove(property);

   } else if (QtProperty *bPointProp = m_bToProperty.value(property, nullptr)) {
      m_propertyToB[bPointProp] = nullptr;
      m_bToProperty.remove(property);

   } else if (QtProperty *aPointProp = m_aToProperty.value(property, nullptr)) {
      m_propertyToA[aPointProp] = nullptr;
      m_aToProperty.remove(property);
   }
}

QtColorPropertyManager::QtColorPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtColorPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);

   connect(d_ptr->m_intPropertyManager, &QtIntPropertyManager::valueChanged,
      this, &QtColorPropertyManager::slotIntChanged);

   connect(d_ptr->m_intPropertyManager, &QtIntPropertyManager::propertyDestroyed,
      this, &QtColorPropertyManager::slotPropertyDestroyed);
}

QtColorPropertyManager::~QtColorPropertyManager()
{
   clear();
}

QtIntPropertyManager *QtColorPropertyManager::subIntPropertyManager() const
{
   return d_ptr->m_intPropertyManager;
}

QColor QtColorPropertyManager::value(const QtProperty *property) const
{
   return d_ptr->m_values.value(property, QColor());
}

QString QtColorPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   return QtPropertyBrowserUtils::colorValueText(it.value());
}

QIcon QtColorPropertyManager::valueIcon(const QtProperty *property) const
{
   auto  it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QIcon();
   }
   return QtPropertyBrowserUtils::brushValueIcon(QBrush(it.value()));
}

void QtColorPropertyManager::setValue(QtProperty *property, const QColor &val)
{
   auto  it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   if (it.value() == val) {
      return;
   }

   it.value() = val;

   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToR[property], val.red());
   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToG[property], val.green());
   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToB[property], val.blue());
   d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToA[property], val.alpha());

   emit propertyChanged(property);
   emit valueChanged(property, val);
}

void QtColorPropertyManager::initializeProperty(QtProperty *property)
{
   QColor val;
   d_ptr->m_values[property] = val;

   QtProperty *rProp = d_ptr->m_intPropertyManager->addProperty();
   rProp->setPropertyName(tr("Red"));
   d_ptr->m_intPropertyManager->setValue(rProp, val.red());
   d_ptr->m_intPropertyManager->setRange(rProp, 0, 0xFF);
   d_ptr->m_propertyToR[property] = rProp;
   d_ptr->m_rToProperty[rProp] = property;
   property->addSubProperty(rProp);

   QtProperty *gProp = d_ptr->m_intPropertyManager->addProperty();
   gProp->setPropertyName(tr("Green"));
   d_ptr->m_intPropertyManager->setValue(gProp, val.green());
   d_ptr->m_intPropertyManager->setRange(gProp, 0, 0xFF);
   d_ptr->m_propertyToG[property] = gProp;
   d_ptr->m_gToProperty[gProp] = property;
   property->addSubProperty(gProp);

   QtProperty *bProp = d_ptr->m_intPropertyManager->addProperty();
   bProp->setPropertyName(tr("Blue"));
   d_ptr->m_intPropertyManager->setValue(bProp, val.blue());
   d_ptr->m_intPropertyManager->setRange(bProp, 0, 0xFF);
   d_ptr->m_propertyToB[property] = bProp;
   d_ptr->m_bToProperty[bProp] = property;
   property->addSubProperty(bProp);

   QtProperty *aProp = d_ptr->m_intPropertyManager->addProperty();
   aProp->setPropertyName(tr("Alpha"));
   d_ptr->m_intPropertyManager->setValue(aProp, val.alpha());
   d_ptr->m_intPropertyManager->setRange(aProp, 0, 0xFF);
   d_ptr->m_propertyToA[property] = aProp;
   d_ptr->m_aToProperty[aProp] = property;
   property->addSubProperty(aProp);
}

void QtColorPropertyManager::uninitializeProperty(QtProperty *property)
{
   QtProperty *rProp = d_ptr->m_propertyToR[property];
   if (rProp) {
      d_ptr->m_rToProperty.remove(rProp);
      delete rProp;
   }
   d_ptr->m_propertyToR.remove(property);

   QtProperty *gProp = d_ptr->m_propertyToG[property];
   if (gProp) {
      d_ptr->m_gToProperty.remove(gProp);
      delete gProp;
   }
   d_ptr->m_propertyToG.remove(property);

   QtProperty *bProp = d_ptr->m_propertyToB[property];
   if (bProp) {
      d_ptr->m_bToProperty.remove(bProp);
      delete bProp;
   }
   d_ptr->m_propertyToB.remove(property);

   QtProperty *aProp = d_ptr->m_propertyToA[property];
   if (aProp) {
      d_ptr->m_aToProperty.remove(aProp);
      delete aProp;
   }
   d_ptr->m_propertyToA.remove(property);

   d_ptr->m_values.remove(property);
}

// Make sure icons are removed as soon as QApplication is destroyed, otherwise,
// handles are leaked on X11.
static void clearCursorDatabase();

namespace {

struct CursorDatabase : public QtCursorDatabase {
   CursorDatabase() {
      qAddPostRoutine(clearCursorDatabase);
   }
};

}   // end namespace

Q_GLOBAL_STATIC(QtCursorDatabase, cursorDatabase)

static void clearCursorDatabase()
{
   cursorDatabase()->clear();
}

class QtCursorPropertyManagerPrivate
{
 public:
   QMap<const QtProperty *, QCursor> m_values;

 private:
   QtCursorPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtCursorPropertyManager)
};

QtCursorPropertyManager::QtCursorPropertyManager(QObject *parent)
   : QtAbstractPropertyManager(parent), d_ptr(new QtCursorPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;
}

QtCursorPropertyManager::~QtCursorPropertyManager()
{
   clear();
}

QCursor QtCursorPropertyManager::value(const QtProperty *property) const
{
   return d_ptr->m_values.value(property, QCursor());
}

QString QtCursorPropertyManager::valueText(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QString();
   }

   return cursorDatabase()->cursorToShapeName(it.value());
}

QIcon QtCursorPropertyManager::valueIcon(const QtProperty *property) const
{
   auto it = d_ptr->m_values.constFind(property);

   if (it == d_ptr->m_values.constEnd()) {
      return QIcon();
   }

   return cursorDatabase()->cursorToShapeIcon(it.value());
}

void QtCursorPropertyManager::setValue(QtProperty *property, const QCursor &value)
{
   auto it = d_ptr->m_values.find(property);

   if (it == d_ptr->m_values.end()) {
      return;
   }

   if (it.value().shape() == value.shape() && value.shape() != Qt::BitmapCursor) {
      return;
   }

   it.value() = value;

   emit propertyChanged(property);
   emit valueChanged(property, value);
}


void QtCursorPropertyManager::initializeProperty(QtProperty *property)
{
   d_ptr->m_values[property] = QCursor();
}

void QtCursorPropertyManager::uninitializeProperty(QtProperty *property)
{
   d_ptr->m_values.remove(property);
}

// private slots

void QtSizePropertyManager::slotIntChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtSizePropertyManager);
   d->slotIntChanged(un_named_arg1, un_named_arg2);
}

void QtSizePropertyManager::slotPropertyDestroyed(QtProperty *un_named_arg1)
{
   Q_D(QtSizePropertyManager);
   d->slotPropertyDestroyed(un_named_arg1);
}

void QtLocalePropertyManager::slotEnumChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtLocalePropertyManager);
   d->slotEnumChanged(un_named_arg1, un_named_arg2);
}

void QtLocalePropertyManager::slotPropertyDestroyed(QtProperty *un_named_arg1)
{
   Q_D(QtLocalePropertyManager);
   d->slotPropertyDestroyed(un_named_arg1);
}

void QtPointPropertyManager::slotIntChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtPointPropertyManager);
   d->slotIntChanged(un_named_arg1, un_named_arg2);
}

void QtPointPropertyManager::slotPropertyDestroyed(QtProperty *un_named_arg1)
{
   Q_D(QtPointPropertyManager);
   d->slotPropertyDestroyed(un_named_arg1);
}

void QtPointFPropertyManager::slotDoubleChanged(QtProperty *un_named_arg1, double un_named_arg2)
{
   Q_D(QtPointFPropertyManager);
   d->slotDoubleChanged(un_named_arg1, un_named_arg2);
}

void QtPointFPropertyManager::slotPropertyDestroyed(QtProperty *un_named_arg1)
{
   Q_D(QtPointFPropertyManager);
   d->slotPropertyDestroyed(un_named_arg1);
}

void QtRectPropertyManager::slotIntChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtRectPropertyManager);
   d->slotIntChanged(un_named_arg1, un_named_arg2);
}

void QtRectPropertyManager::slotPropertyDestroyed(QtProperty *un_named_arg1)
{
   Q_D(QtRectPropertyManager);
   d->slotPropertyDestroyed(un_named_arg1);
}

void QtRectFPropertyManager::slotDoubleChanged(QtProperty *un_named_arg1, double un_named_arg2)
{
   Q_D(QtRectFPropertyManager);
   d->slotDoubleChanged(un_named_arg1, un_named_arg2);
}

void QtRectFPropertyManager::slotPropertyDestroyed(QtProperty *un_named_arg1)
{
   Q_D(QtRectFPropertyManager);
   d->slotPropertyDestroyed(un_named_arg1);
}

void QtFlagPropertyManager::slotBoolChanged(QtProperty *un_named_arg1, bool un_named_arg2)
{
   Q_D(QtFlagPropertyManager);
   d->slotBoolChanged(un_named_arg1, un_named_arg2);
}

void QtFlagPropertyManager::slotPropertyDestroyed(QtProperty *un_named_arg1)
{
   Q_D(QtFlagPropertyManager);
   d->slotPropertyDestroyed(un_named_arg1);
}

void QtSizePolicyPropertyManager::slotIntChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtSizePolicyPropertyManager);
   d->slotIntChanged(un_named_arg1, un_named_arg2);
}

void QtSizePolicyPropertyManager::slotEnumChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtSizePolicyPropertyManager);
   d->slotEnumChanged(un_named_arg1, un_named_arg2);
}

void QtSizePolicyPropertyManager::slotPropertyDestroyed(QtProperty *un_named_arg1)
{
   Q_D(QtSizePolicyPropertyManager);
   d->slotPropertyDestroyed(un_named_arg1);
}

void QtFontPropertyManager::slotIntChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtFontPropertyManager);
   d->slotIntChanged(un_named_arg1, un_named_arg2);
}

void QtFontPropertyManager::slotEnumChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtFontPropertyManager);
   d->slotEnumChanged(un_named_arg1, un_named_arg2);
}

void QtFontPropertyManager::slotBoolChanged(QtProperty *un_named_arg1, bool un_named_arg2)
{
   Q_D(QtFontPropertyManager);
   d->slotBoolChanged(un_named_arg1, un_named_arg2);
}

void QtFontPropertyManager::slotPropertyDestroyed(QtProperty *un_named_arg1)
{
   Q_D(QtFontPropertyManager);
   d->slotPropertyDestroyed(un_named_arg1);
}

void QtFontPropertyManager::slotFontDatabaseChanged()
{
   Q_D(QtFontPropertyManager);
   d->slotFontDatabaseChanged();
}

void QtFontPropertyManager::slotFontDatabaseDelayedChange()
{
   Q_D(QtFontPropertyManager);
   d->slotFontDatabaseDelayedChange();
}

void QtColorPropertyManager::slotIntChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtColorPropertyManager);
   d->slotIntChanged(un_named_arg1, un_named_arg2);
}

void QtColorPropertyManager::slotPropertyDestroyed(QtProperty *un_named_arg1)
{
   Q_D(QtColorPropertyManager);
   d->slotPropertyDestroyed(un_named_arg1);
}

void QtSizeFPropertyManager::slotDoubleChanged(QtProperty *un_named_arg1, double un_named_arg2)
{
   Q_D(QtSizeFPropertyManager);
   d->slotDoubleChanged(un_named_arg1, un_named_arg2);
}

void QtSizeFPropertyManager::slotPropertyDestroyed(QtProperty *un_named_arg1)
{
   Q_D(QtSizeFPropertyManager);
   d->slotPropertyDestroyed(un_named_arg1);
}
