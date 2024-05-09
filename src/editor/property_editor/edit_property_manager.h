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

#ifndef EDIT_PROPERTY_MANAGER_H
#define EDIT_PROPERTY_MANAGER_H

#include <property_view.h>

#include <QDateTime>
#include <QLocale>

class QtBoolPropertyManagerPrivate;
class QtCharPropertyManagerPrivate;
class QtColorPropertyManagerPrivate;
class QtCursorPropertyManagerPrivate;
class QtDatePropertyManagerPrivate;
class QtDateTimePropertyManagerPrivate;
class QtDoublePropertyManagerPrivate;
class QtEnumPropertyManager;
class QtEnumPropertyManagerPrivate;
class QtFlagPropertyManagerPrivate;
class QtFontPropertyManagerPrivate;
class QtIntPropertyManagerPrivate;
class QtKeySequencePropertyManagerPrivate;
class QtLocalePropertyManagerPrivate;
class QtPointFPropertyManagerPrivate;
class QtPointPropertyManagerPrivate;
class QtRectFPropertyManagerPrivate;
class QtRectPropertyManagerPrivate;
class QtSizeFPropertyManagerPrivate;
class QtSizePolicyPropertyManagerPrivate;
class QtSizePropertyManagerPrivate;
class QtStringPropertyManagerPrivate;
class QtTimePropertyManagerPrivate;

class QtGroupPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtGroupPropertyManager)

 public:
   QtGroupPropertyManager(QObject *parent = nullptr);
   ~QtGroupPropertyManager();

 protected:
   bool hasValue(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;
};

class QtIntPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtIntPropertyManager)

 public:
   QtIntPropertyManager(QObject *parent = nullptr);
   ~QtIntPropertyManager();

   int value(const QtProperty *property) const;
   int minimum(const QtProperty *property) const;
   int maximum(const QtProperty *property) const;
   int singleStep(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, int val))
   CS_SLOT_2(setValue)

   CS_SLOT_1(Public, void setMinimum(QtProperty *property, int minVal))
   CS_SLOT_2(setMinimum)

   CS_SLOT_1(Public, void setMaximum(QtProperty *property, int maxVal))
   CS_SLOT_2(setMaximum)

   CS_SLOT_1(Public, void setRange(QtProperty *property, int minVal, int maxVal))
   CS_SLOT_2(setRange)

   CS_SLOT_1(Public, void setSingleStep(QtProperty *property, int step))
   CS_SLOT_2(setSingleStep)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, int val))
   CS_SIGNAL_2(valueChanged, property, val)

   CS_SIGNAL_1(Public, void rangeChanged(QtProperty *property, int minVal, int maxVal))
   CS_SIGNAL_2(rangeChanged, property, minVal, maxVal)

   CS_SIGNAL_1(Public, void singleStepChanged(QtProperty *property, int step))
   CS_SIGNAL_2(singleStepChanged, property, step)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtIntPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtIntPropertyManager)
   Q_DISABLE_COPY(QtIntPropertyManager)
};

class QtBoolPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtBoolPropertyManager)

 public:
   QtBoolPropertyManager(QObject *parent = nullptr);
   ~QtBoolPropertyManager();

   bool value(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, bool val))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, bool val))
   CS_SIGNAL_2(valueChanged, property, val)

 protected:
   QString valueText(const QtProperty *property) const override;
   QIcon valueIcon(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtBoolPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtBoolPropertyManager)
   Q_DISABLE_COPY(QtBoolPropertyManager)
};

class QtDoublePropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtDoublePropertyManager)

 public:
   QtDoublePropertyManager(QObject *parent = nullptr);
   ~QtDoublePropertyManager();

   double value(const QtProperty *property) const;
   double minimum(const QtProperty *property) const;
   double maximum(const QtProperty *property) const;
   double singleStep(const QtProperty *property) const;
   int decimals(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, double val))
   CS_SLOT_2(setValue)
   CS_SLOT_1(Public, void setMinimum(QtProperty *property, double minVal))
   CS_SLOT_2(setMinimum)
   CS_SLOT_1(Public, void setMaximum(QtProperty *property, double maxVal))
   CS_SLOT_2(setMaximum)
   CS_SLOT_1(Public, void setRange(QtProperty *property, double minVal, double maxVal))
   CS_SLOT_2(setRange)
   CS_SLOT_1(Public, void setSingleStep(QtProperty *property, double step))
   CS_SLOT_2(setSingleStep)
   CS_SLOT_1(Public, void setDecimals(QtProperty *property, int prec))
   CS_SLOT_2(setDecimals)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, double val))
   CS_SIGNAL_2(valueChanged, property, val)
   CS_SIGNAL_1(Public, void rangeChanged(QtProperty *property, double minVal, double maxVal))
   CS_SIGNAL_2(rangeChanged, property, minVal, maxVal)
   CS_SIGNAL_1(Public, void singleStepChanged(QtProperty *property, double step))
   CS_SIGNAL_2(singleStepChanged, property, step)
   CS_SIGNAL_1(Public, void decimalsChanged(QtProperty *property, int prec))
   CS_SIGNAL_2(decimalsChanged, property, prec)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtDoublePropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtDoublePropertyManager)
   Q_DISABLE_COPY(QtDoublePropertyManager)
};

class QtStringPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtStringPropertyManager)

 public:
   QtStringPropertyManager(QObject *parent = nullptr);
   ~QtStringPropertyManager();

   QString value(const QtProperty *property) const;
   QRegularExpression regExp(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QString &val))
   CS_SLOT_2(setValue)

   CS_SLOT_1(Public, void setRegExp(QtProperty *property, const QRegularExpression &regExp))
   CS_SLOT_2(setRegExp)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QString &val))
   CS_SIGNAL_2(valueChanged, property, val)

   CS_SIGNAL_1(Public, void regExpChanged(QtProperty *property, const QRegularExpression &regExp))
   CS_SIGNAL_2(regExpChanged, property, regExp)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtStringPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtStringPropertyManager)
   Q_DISABLE_COPY(QtStringPropertyManager)
};

class QtDatePropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtDatePropertyManager)

 public:
   QtDatePropertyManager(QObject *parent = nullptr);
   ~QtDatePropertyManager();

   QDate value(const QtProperty *property) const;
   QDate minimum(const QtProperty *property) const;
   QDate maximum(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QDate &val))
   CS_SLOT_2(setValue)
   CS_SLOT_1(Public, void setMinimum(QtProperty *property, const QDate &minVal))
   CS_SLOT_2(setMinimum)
   CS_SLOT_1(Public, void setMaximum(QtProperty *property, const QDate &maxVal))
   CS_SLOT_2(setMaximum)
   CS_SLOT_1(Public, void setRange(QtProperty *property, const QDate &minVal, const QDate &maxVal))
   CS_SLOT_2(setRange)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QDate &val))
   CS_SIGNAL_2(valueChanged, property, val)

   CS_SIGNAL_1(Public, void rangeChanged(QtProperty *property, const QDate &minVal, const QDate &maxVal))
   CS_SIGNAL_2(rangeChanged, property, minVal, maxVal)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtDatePropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtDatePropertyManager)
   Q_DISABLE_COPY(QtDatePropertyManager)
};

class QtTimePropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtTimePropertyManager)

 public:
   QtTimePropertyManager(QObject *parent = nullptr);
   ~QtTimePropertyManager();

   QTime value(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QTime &val))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QTime &val))
   CS_SIGNAL_2(valueChanged, property, val)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtTimePropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtTimePropertyManager)
   Q_DISABLE_COPY(QtTimePropertyManager)
};

class QtDateTimePropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtDateTimePropertyManager)

 public:
   QtDateTimePropertyManager(QObject *parent = nullptr);
   ~QtDateTimePropertyManager();

   QDateTime value(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QDateTime &val))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QDateTime &val))
   CS_SIGNAL_2(valueChanged, property, val)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtDateTimePropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtDateTimePropertyManager)
   Q_DISABLE_COPY(QtDateTimePropertyManager)
};

class QtKeySequencePropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtKeySequencePropertyManager)

 public:
   QtKeySequencePropertyManager(QObject *parent = nullptr);
   ~QtKeySequencePropertyManager();

   QKeySequence value(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QKeySequence &val))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QKeySequence &val))
   CS_SIGNAL_2(valueChanged, property, val)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtKeySequencePropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtKeySequencePropertyManager)
   Q_DISABLE_COPY(QtKeySequencePropertyManager)
};

class QtCharPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtCharPropertyManager)

 public:
   QtCharPropertyManager(QObject *parent = nullptr);
   ~QtCharPropertyManager();

   QChar value(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QChar &val))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QChar &val))
   CS_SIGNAL_2(valueChanged, property, val)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtCharPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtCharPropertyManager)
   Q_DISABLE_COPY(QtCharPropertyManager)
};

class QtLocalePropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtLocalePropertyManager)

 public:
   QtLocalePropertyManager(QObject *parent = nullptr);
   ~QtLocalePropertyManager();

   QtEnumPropertyManager *subEnumPropertyManager() const;

   QLocale value(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QLocale &val))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QLocale &val))
   CS_SIGNAL_2(valueChanged, property, val)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtLocalePropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtLocalePropertyManager)
   Q_DISABLE_COPY(QtLocalePropertyManager)

   CS_SLOT_1(Private, void slotEnumChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotEnumChanged)

   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDestroyed)
};

class QtPointPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtPointPropertyManager)

 public:
   QtPointPropertyManager(QObject *parent = nullptr);
   ~QtPointPropertyManager();

   QtIntPropertyManager *subIntPropertyManager() const;

   QPoint value(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QPoint &val))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QPoint &val))
   CS_SIGNAL_2(valueChanged, property, val)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtPointPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtPointPropertyManager)
   Q_DISABLE_COPY(QtPointPropertyManager)

   CS_SLOT_1(Private, void slotIntChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotIntChanged)

   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDestroyed)
};

class QtPointFPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtPointFPropertyManager)

 public:
   QtPointFPropertyManager(QObject *parent = nullptr);
   ~QtPointFPropertyManager();

   QtDoublePropertyManager *subDoublePropertyManager() const;

   QPointF value(const QtProperty *property) const;
   int decimals(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QPointF &val))
   CS_SLOT_2(setValue)
   CS_SLOT_1(Public, void setDecimals(QtProperty *property, int prec))
   CS_SLOT_2(setDecimals)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QPointF &val))
   CS_SIGNAL_2(valueChanged, property, val)
   CS_SIGNAL_1(Public, void decimalsChanged(QtProperty *property, int prec))
   CS_SIGNAL_2(decimalsChanged, property, prec)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtPointFPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtPointFPropertyManager)
   Q_DISABLE_COPY(QtPointFPropertyManager)

   CS_SLOT_1(Private, void slotDoubleChanged(QtProperty *un_named_arg1, double un_named_arg2))
   CS_SLOT_2(slotDoubleChanged)

   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDestroyed)
};

class QtSizePropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtSizePropertyManager)

 public:
   QtSizePropertyManager(QObject *parent = nullptr);
   ~QtSizePropertyManager();

   QtIntPropertyManager *subIntPropertyManager() const;

   QSize value(const QtProperty *property) const;
   QSize minimum(const QtProperty *property) const;
   QSize maximum(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QSize &val))
   CS_SLOT_2(setValue)
   CS_SLOT_1(Public, void setMinimum(QtProperty *property, const QSize &minVal))
   CS_SLOT_2(setMinimum)
   CS_SLOT_1(Public, void setMaximum(QtProperty *property, const QSize &maxVal))
   CS_SLOT_2(setMaximum)
   CS_SLOT_1(Public, void setRange(QtProperty *property, const QSize &minVal, const QSize &maxVal))
   CS_SLOT_2(setRange)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QSize &val))
   CS_SIGNAL_2(valueChanged, property, val)
   CS_SIGNAL_1(Public, void rangeChanged(QtProperty *property, const QSize &minVal, const QSize &maxVal))
   CS_SIGNAL_2(rangeChanged, property, minVal, maxVal)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtSizePropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtSizePropertyManager)
   Q_DISABLE_COPY(QtSizePropertyManager)

   CS_SLOT_1(Private, void slotIntChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotIntChanged)

   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDestroyed)
};

class QtSizeFPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtSizeFPropertyManager)

 public:
   QtSizeFPropertyManager(QObject *parent = nullptr);
   ~QtSizeFPropertyManager();

   QtDoublePropertyManager *subDoublePropertyManager() const;

   QSizeF value(const QtProperty *property) const;
   QSizeF minimum(const QtProperty *property) const;
   QSizeF maximum(const QtProperty *property) const;
   int decimals(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QSizeF &val))
   CS_SLOT_2(setValue)
   CS_SLOT_1(Public, void setMinimum(QtProperty *property, const QSizeF &minVal))
   CS_SLOT_2(setMinimum)
   CS_SLOT_1(Public, void setMaximum(QtProperty *property, const QSizeF &maxVal))
   CS_SLOT_2(setMaximum)
   CS_SLOT_1(Public, void setRange(QtProperty *property, const QSizeF &minVal, const QSizeF &maxVal))
   CS_SLOT_2(setRange)
   CS_SLOT_1(Public, void setDecimals(QtProperty *property, int prec))
   CS_SLOT_2(setDecimals)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QSizeF &val))
   CS_SIGNAL_2(valueChanged, property, val)
   CS_SIGNAL_1(Public, void rangeChanged(QtProperty *property, const QSizeF &minVal, const QSizeF &maxVal))
   CS_SIGNAL_2(rangeChanged, property, minVal, maxVal)
   CS_SIGNAL_1(Public, void decimalsChanged(QtProperty *property, int prec))
   CS_SIGNAL_2(decimalsChanged, property, prec)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtSizeFPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtSizeFPropertyManager)
   Q_DISABLE_COPY(QtSizeFPropertyManager)

   CS_SLOT_1(Private, void slotDoubleChanged(QtProperty *un_named_arg1, double un_named_arg2))
   CS_SLOT_2(slotDoubleChanged)

   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDestroyed)
};

class QtRectPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtRectPropertyManager)

 public:
   QtRectPropertyManager(QObject *parent = nullptr);
   ~QtRectPropertyManager();

   QtIntPropertyManager *subIntPropertyManager() const;

   QRect value(const QtProperty *property) const;
   QRect constraint(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QRect &val))
   CS_SLOT_2(setValue)
   CS_SLOT_1(Public, void setConstraint(QtProperty *property, const QRect &constraint))
   CS_SLOT_2(setConstraint)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QRect &val))
   CS_SIGNAL_2(valueChanged, property, val)
   CS_SIGNAL_1(Public, void constraintChanged(QtProperty *property, const QRect &constraint))
   CS_SIGNAL_2(constraintChanged, property, constraint)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtRectPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtRectPropertyManager)
   Q_DISABLE_COPY(QtRectPropertyManager)

   CS_SLOT_1(Private, void slotIntChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotIntChanged)

   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDestroyed)
};

class QtRectFPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtRectFPropertyManager)

 public:
   QtRectFPropertyManager(QObject *parent = nullptr);
   ~QtRectFPropertyManager();

   QtDoublePropertyManager *subDoublePropertyManager() const;

   QRectF value(const QtProperty *property) const;
   QRectF constraint(const QtProperty *property) const;
   int decimals(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QRectF &val))
   CS_SLOT_2(setValue)
   CS_SLOT_1(Public, void setConstraint(QtProperty *property, const QRectF &constraint))
   CS_SLOT_2(setConstraint)
   CS_SLOT_1(Public, void setDecimals(QtProperty *property, int prec))
   CS_SLOT_2(setDecimals)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QRectF &val))
   CS_SIGNAL_2(valueChanged, property, val)
   CS_SIGNAL_1(Public, void constraintChanged(QtProperty *property, const QRectF &constraint))
   CS_SIGNAL_2(constraintChanged, property, constraint)
   CS_SIGNAL_1(Public, void decimalsChanged(QtProperty *property, int prec))
   CS_SIGNAL_2(decimalsChanged, property, prec)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtRectFPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtRectFPropertyManager)
   Q_DISABLE_COPY(QtRectFPropertyManager)

   CS_SLOT_1(Private, void slotDoubleChanged(QtProperty *un_named_arg1, double un_named_arg2))
   CS_SLOT_2(slotDoubleChanged)

   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDestroyed)
};

class QtEnumPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtEnumPropertyManager)

 public:
   QtEnumPropertyManager(QObject *parent = nullptr);
   ~QtEnumPropertyManager();

   int value(const QtProperty *property) const;
   QStringList enumNames(const QtProperty *property) const;
   QMap<int, QIcon> enumIcons(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, int val))
   CS_SLOT_2(setValue)
   CS_SLOT_1(Public, void setEnumNames(QtProperty *property, const QStringList &names))
   CS_SLOT_2(setEnumNames)
   CS_SLOT_1(Public, void setEnumIcons(QtProperty *property, const QMap <int, QIcon> &icons))
   CS_SLOT_2(setEnumIcons)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, int val))
   CS_SIGNAL_2(valueChanged, property, val)
   CS_SIGNAL_1(Public, void enumNamesChanged(QtProperty *property, const QStringList &names))
   CS_SIGNAL_2(enumNamesChanged, property, names)
   CS_SIGNAL_1(Public, void enumIconsChanged(QtProperty *property, const QMap <int, QIcon> &icons))
   CS_SIGNAL_2(enumIconsChanged, property, icons)

 protected:
   QString valueText(const QtProperty *property) const override;
   QIcon valueIcon(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtEnumPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtEnumPropertyManager)
   Q_DISABLE_COPY(QtEnumPropertyManager)
};

class QtFlagPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtFlagPropertyManager)

 public:
   QtFlagPropertyManager(QObject *parent = nullptr);
   ~QtFlagPropertyManager();

   QtBoolPropertyManager *subBoolPropertyManager() const;

   int value(const QtProperty *property) const;
   QStringList flagNames(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, int val))
   CS_SLOT_2(setValue)
   CS_SLOT_1(Public, void setFlagNames(QtProperty *property, const QStringList &names))
   CS_SLOT_2(setFlagNames)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, int val))
   CS_SIGNAL_2(valueChanged, property, val)
   CS_SIGNAL_1(Public, void flagNamesChanged(QtProperty *property, const QStringList &names))
   CS_SIGNAL_2(flagNamesChanged, property, names)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtFlagPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtFlagPropertyManager)
   Q_DISABLE_COPY(QtFlagPropertyManager)

   CS_SLOT_1(Private, void slotBoolChanged(QtProperty *un_named_arg1, bool un_named_arg2))
   CS_SLOT_2(slotBoolChanged)

   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDestroyed)
};

class QtSizePolicyPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtSizePolicyPropertyManager)

 public:
   QtSizePolicyPropertyManager(QObject *parent = nullptr);
   ~QtSizePolicyPropertyManager();

   QtIntPropertyManager *subIntPropertyManager() const;
   QtEnumPropertyManager *subEnumPropertyManager() const;

   QSizePolicy value(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QSizePolicy &val))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QSizePolicy &val))
   CS_SIGNAL_2(valueChanged, property, val)

 protected:
   QString valueText(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtSizePolicyPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtSizePolicyPropertyManager)
   Q_DISABLE_COPY(QtSizePolicyPropertyManager)

   CS_SLOT_1(Private, void slotIntChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotIntChanged)

   CS_SLOT_1(Private, void slotEnumChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotEnumChanged)

   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDestroyed)
};

class QtFontPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtFontPropertyManager)

 public:
   QtFontPropertyManager(QObject *parent = nullptr);
   ~QtFontPropertyManager();

   QtIntPropertyManager *subIntPropertyManager() const;
   QtEnumPropertyManager *subEnumPropertyManager() const;
   QtBoolPropertyManager *subBoolPropertyManager() const;

   QFont value(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QFont &val))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QFont &val))
   CS_SIGNAL_2(valueChanged, property, val)

 protected:
   QString valueText(const QtProperty *property) const override;
   QIcon valueIcon(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtFontPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtFontPropertyManager)
   Q_DISABLE_COPY(QtFontPropertyManager)

   CS_SLOT_1(Private, void slotIntChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotIntChanged)

   CS_SLOT_1(Private, void slotEnumChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotEnumChanged)

   CS_SLOT_1(Private, void slotBoolChanged(QtProperty *un_named_arg1, bool un_named_arg2))
   CS_SLOT_2(slotBoolChanged)

   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDestroyed)

   CS_SLOT_1(Private, void slotFontDatabaseChanged())
   CS_SLOT_2(slotFontDatabaseChanged)

   CS_SLOT_1(Private, void slotFontDatabaseDelayedChange())
   CS_SLOT_2(slotFontDatabaseDelayedChange)
};

class QtColorPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtColorPropertyManager)

 public:
   QtColorPropertyManager(QObject *parent = nullptr);
   ~QtColorPropertyManager();

   QtIntPropertyManager *subIntPropertyManager() const;

   QColor value(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QColor &val))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QColor &val))
   CS_SIGNAL_2(valueChanged, property, val)

 protected:
   QString valueText(const QtProperty *property) const override;
   QIcon valueIcon(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtColorPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtColorPropertyManager)
   Q_DISABLE_COPY(QtColorPropertyManager)

   CS_SLOT_1(Private, void slotIntChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotIntChanged)

   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDestroyed)
};

class QtCursorPropertyManager : public QtAbstractPropertyManager
{
   CS_OBJECT(QtCursorPropertyManager)

 public:
   QtCursorPropertyManager(QObject *parent = nullptr);
   ~QtCursorPropertyManager();

   QCursor value(const QtProperty *property) const;

   CS_SLOT_1(Public, void setValue(QtProperty *property, const QCursor &val))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QCursor &val))
   CS_SIGNAL_2(valueChanged, property, val)

 protected:
   QString valueText(const QtProperty *property) const override;
   QIcon valueIcon(const QtProperty *property) const override;
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;

 private:
   QScopedPointer<QtCursorPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtCursorPropertyManager)
   Q_DISABLE_COPY(QtCursorPropertyManager)
};

#endif
