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

#ifndef EDIT_PROPERTY_H
#define EDIT_PROPERTY_H

#include <edit_property_manager.h>

class QtSpinBoxFactoryPrivate;
class QtScrollBarFactoryPrivate;
class QtSliderFactoryPrivate;
class QtFontEditorFactoryPrivate;
class QtColorEditorFactoryPrivate;
class QtCheckBoxFactoryPrivate;
class QtCursorEditorFactoryPrivate;
class QtEnumEditorFactoryPrivate;
class QtDoubleSpinBoxFactoryPrivate;
class QtLineEditFactoryPrivate;
class QtDateEditFactoryPrivate;
class QtTimeEditFactoryPrivate;
class QtDateTimeEditFactoryPrivate;
class QtCharEditorFactoryPrivate;
class QtKeySequenceEditorFactoryPrivate;

class QtSpinBoxFactory : public QtAbstractEditorFactory<QtIntPropertyManager>
{
   CS_OBJECT(QtSpinBoxFactory)

 public:
   QtSpinBoxFactory(QObject *parent = nullptr);
   ~QtSpinBoxFactory();

 protected:
   void connectPropertyManager(QtIntPropertyManager *manager);
   QWidget *createEditor(QtIntPropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtIntPropertyManager *manager);

 private:
   QScopedPointer<QtSpinBoxFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtSpinBoxFactory)
   Q_DISABLE_COPY(QtSpinBoxFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotRangeChanged(QtProperty *un_named_arg1, int un_named_arg2, int un_named_arg3))
   CS_SLOT_2(slotRangeChanged)

   CS_SLOT_1(Private, void slotSingleStepChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotSingleStepChanged)

   CS_SLOT_1(Private, void slotSetValue(int un_named_arg1))
   CS_SLOT_2(slotSetValue)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtSliderFactory : public QtAbstractEditorFactory<QtIntPropertyManager>
{
   CS_OBJECT(QtSliderFactory)

 public:
   QtSliderFactory(QObject *parent = nullptr);
   ~QtSliderFactory();

 protected:
   void connectPropertyManager(QtIntPropertyManager *manager);
   QWidget *createEditor(QtIntPropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtIntPropertyManager *manager);

 private:
   QScopedPointer<QtSliderFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtSliderFactory)
   Q_DISABLE_COPY(QtSliderFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotRangeChanged(QtProperty *un_named_arg1, int un_named_arg2, int un_named_arg3))
   CS_SLOT_2(slotRangeChanged)

   CS_SLOT_1(Private, void slotSingleStepChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotSingleStepChanged)

   CS_SLOT_1(Private, void slotSetValue(int un_named_arg1))
   CS_SLOT_2(slotSetValue)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtScrollBarFactory : public QtAbstractEditorFactory<QtIntPropertyManager>
{
   CS_OBJECT(QtScrollBarFactory)

 public:
   QtScrollBarFactory(QObject *parent = nullptr);
   ~QtScrollBarFactory();

 protected:
   void connectPropertyManager(QtIntPropertyManager *manager);
   QWidget *createEditor(QtIntPropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtIntPropertyManager *manager);

 private:
   QScopedPointer<QtScrollBarFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtScrollBarFactory)
   Q_DISABLE_COPY(QtScrollBarFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotRangeChanged(QtProperty *un_named_arg1, int un_named_arg2, int un_named_arg3))
   CS_SLOT_2(slotRangeChanged)

   CS_SLOT_1(Private, void slotSingleStepChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotSingleStepChanged)

   CS_SLOT_1(Private, void slotSetValue(int un_named_arg1))
   CS_SLOT_2(slotSetValue)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtCheckBoxFactory : public QtAbstractEditorFactory<QtBoolPropertyManager>
{
   CS_OBJECT(QtCheckBoxFactory)

 public:
   QtCheckBoxFactory(QObject *parent = nullptr);
   ~QtCheckBoxFactory();

 protected:
   void connectPropertyManager(QtBoolPropertyManager *manager);
   QWidget *createEditor(QtBoolPropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtBoolPropertyManager *manager);

 private:
   QScopedPointer<QtCheckBoxFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtCheckBoxFactory)
   Q_DISABLE_COPY(QtCheckBoxFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, bool un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotSetValue(bool un_named_arg1))
   CS_SLOT_2(slotSetValue)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtDoubleSpinBoxFactory : public QtAbstractEditorFactory<QtDoublePropertyManager>
{
   CS_OBJECT(QtDoubleSpinBoxFactory)

 public:
   QtDoubleSpinBoxFactory(QObject *parent = nullptr);
   ~QtDoubleSpinBoxFactory();

 protected:
   void connectPropertyManager(QtDoublePropertyManager *manager);
   QWidget *createEditor(QtDoublePropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtDoublePropertyManager *manager);

 private:
   QScopedPointer<QtDoubleSpinBoxFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtDoubleSpinBoxFactory)
   Q_DISABLE_COPY(QtDoubleSpinBoxFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, double un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotRangeChanged(QtProperty *un_named_arg1, double un_named_arg2, double un_named_arg3))
   CS_SLOT_2(slotRangeChanged)

   CS_SLOT_1(Private, void slotSingleStepChanged(QtProperty *un_named_arg1, double un_named_arg2))
   CS_SLOT_2(slotSingleStepChanged)

   CS_SLOT_1(Private, void slotDecimalsChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotDecimalsChanged)

   CS_SLOT_1(Private, void slotSetValue(double un_named_arg1))
   CS_SLOT_2(slotSetValue)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtLineEditFactory : public QtAbstractEditorFactory<QtStringPropertyManager>
{
   CS_OBJECT(QtLineEditFactory)

 public:
   QtLineEditFactory(QObject *parent = nullptr);
   ~QtLineEditFactory();

 protected:
   void connectPropertyManager(QtStringPropertyManager *manager);
   QWidget *createEditor(QtStringPropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtStringPropertyManager *manager);

 private:
   QScopedPointer<QtLineEditFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtLineEditFactory)
   Q_DISABLE_COPY(QtLineEditFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, const QString &un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotRegExpChanged(QtProperty *un_named_arg1, const QRegularExpression &un_named_arg2))
   CS_SLOT_2(slotRegExpChanged)

   CS_SLOT_1(Private, void slotSetValue(const QString &un_named_arg1))
   CS_SLOT_2(slotSetValue)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtDateEditFactory : public QtAbstractEditorFactory<QtDatePropertyManager>
{
   CS_OBJECT(QtDateEditFactory)

 public:
   QtDateEditFactory(QObject *parent = nullptr);
   ~QtDateEditFactory();

 protected:
   void connectPropertyManager(QtDatePropertyManager *manager);
   QWidget *createEditor(QtDatePropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtDatePropertyManager *manager);

 private:
   QScopedPointer<QtDateEditFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtDateEditFactory)
   Q_DISABLE_COPY(QtDateEditFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, const QDate &un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotRangeChanged(QtProperty *un_named_arg1, const QDate &un_named_arg2, const QDate &un_named_arg3))
   CS_SLOT_2(slotRangeChanged)

   CS_SLOT_1(Private, void slotSetValue(const QDate &un_named_arg1))
   CS_SLOT_2(slotSetValue)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtTimeEditFactory : public QtAbstractEditorFactory<QtTimePropertyManager>
{
   CS_OBJECT(QtTimeEditFactory)

 public:
   QtTimeEditFactory(QObject *parent = nullptr);
   ~QtTimeEditFactory();

 protected:
   void connectPropertyManager(QtTimePropertyManager *manager);
   QWidget *createEditor(QtTimePropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtTimePropertyManager *manager);

 private:
   QScopedPointer<QtTimeEditFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtTimeEditFactory)
   Q_DISABLE_COPY(QtTimeEditFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, const QTime &un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotSetValue(const QTime &un_named_arg1))
   CS_SLOT_2(slotSetValue)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtDateTimeEditFactory : public QtAbstractEditorFactory<QtDateTimePropertyManager>
{
   CS_OBJECT(QtDateTimeEditFactory)

 public:
   QtDateTimeEditFactory(QObject *parent = nullptr);
   ~QtDateTimeEditFactory();

 protected:
   void connectPropertyManager(QtDateTimePropertyManager *manager);
   QWidget *createEditor(QtDateTimePropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtDateTimePropertyManager *manager);

 private:
   QScopedPointer<QtDateTimeEditFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtDateTimeEditFactory)
   Q_DISABLE_COPY(QtDateTimeEditFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, const QDateTime &un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotSetValue(const QDateTime &un_named_arg1))
   CS_SLOT_2(slotSetValue)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtKeySequenceEditorFactory : public QtAbstractEditorFactory<QtKeySequencePropertyManager>
{
   CS_OBJECT(QtKeySequenceEditorFactory)

 public:
   QtKeySequenceEditorFactory(QObject *parent = nullptr);
   ~QtKeySequenceEditorFactory();

 protected:
   void connectPropertyManager(QtKeySequencePropertyManager *manager);
   QWidget *createEditor(QtKeySequencePropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtKeySequencePropertyManager *manager);

 private:
   QScopedPointer<QtKeySequenceEditorFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtKeySequenceEditorFactory)
   Q_DISABLE_COPY(QtKeySequenceEditorFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, const QKeySequence &un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotSetValue(const QKeySequence &un_named_arg1))
   CS_SLOT_2(slotSetValue)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtCharEditorFactory : public QtAbstractEditorFactory<QtCharPropertyManager>
{
   CS_OBJECT(QtCharEditorFactory)

 public:
   QtCharEditorFactory(QObject *parent = nullptr);
   ~QtCharEditorFactory();

 protected:
   void connectPropertyManager(QtCharPropertyManager *manager);
   QWidget *createEditor(QtCharPropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtCharPropertyManager *manager);

 private:
   QScopedPointer<QtCharEditorFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtCharEditorFactory)
   Q_DISABLE_COPY(QtCharEditorFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, const QChar &un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotSetValue(const QChar &un_named_arg1))
   CS_SLOT_2(slotSetValue)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtEnumEditorFactory : public QtAbstractEditorFactory<QtEnumPropertyManager>
{
   CS_OBJECT(QtEnumEditorFactory)

 public:
   QtEnumEditorFactory(QObject *parent = nullptr);
   ~QtEnumEditorFactory();

 protected:
   void connectPropertyManager(QtEnumPropertyManager *manager);
   QWidget *createEditor(QtEnumPropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtEnumPropertyManager *manager);

 private:
   QScopedPointer<QtEnumEditorFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtEnumEditorFactory)
   Q_DISABLE_COPY(QtEnumEditorFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotEnumNamesChanged(QtProperty *un_named_arg1, const QStringList &un_named_arg2))
   CS_SLOT_2(slotEnumNamesChanged)

   CS_SLOT_1(Private, void slotEnumIconsChanged(QtProperty *un_named_arg1, const QMap <int, QIcon> &un_named_arg2))
   CS_SLOT_2(slotEnumIconsChanged)

   CS_SLOT_1(Private, void slotSetValue(int un_named_arg1))
   CS_SLOT_2(slotSetValue)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtCursorEditorFactory : public QtAbstractEditorFactory<QtCursorPropertyManager>
{
   CS_OBJECT(QtCursorEditorFactory)

 public:
   QtCursorEditorFactory(QObject *parent = nullptr);
   ~QtCursorEditorFactory();

 protected:
   void connectPropertyManager(QtCursorPropertyManager *manager);
   QWidget *createEditor(QtCursorPropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtCursorPropertyManager *manager);

 private:
   QScopedPointer<QtCursorEditorFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtCursorEditorFactory)
   Q_DISABLE_COPY(QtCursorEditorFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, const QCursor &un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)


   CS_SLOT_1(Private, void slotEnumChanged(QtProperty *un_named_arg1, int un_named_arg2))
   CS_SLOT_2(slotEnumChanged)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)
};

class QtColorEditorFactory : public QtAbstractEditorFactory<QtColorPropertyManager>
{
   CS_OBJECT(QtColorEditorFactory)

 public:
   QtColorEditorFactory(QObject *parent = nullptr);
   ~QtColorEditorFactory();

 protected:
   void connectPropertyManager(QtColorPropertyManager *manager);
   QWidget *createEditor(QtColorPropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtColorPropertyManager *manager);

 private:
   QScopedPointer<QtColorEditorFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtColorEditorFactory)
   Q_DISABLE_COPY(QtColorEditorFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, const QColor &un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)

   CS_SLOT_1(Private, void slotSetValue(const QColor &un_named_arg1))
   CS_SLOT_2(slotSetValue)
};

class QtFontEditorFactory : public QtAbstractEditorFactory<QtFontPropertyManager>
{
   CS_OBJECT(QtFontEditorFactory)

 public:
   QtFontEditorFactory(QObject *parent = nullptr);
   ~QtFontEditorFactory();

 protected:
   void connectPropertyManager(QtFontPropertyManager *manager);
   QWidget *createEditor(QtFontPropertyManager *manager, QtProperty *property, QWidget *parent);
   void disconnectPropertyManager(QtFontPropertyManager *manager);

 private:
   QScopedPointer<QtFontEditorFactoryPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtFontEditorFactory)
   Q_DISABLE_COPY(QtFontEditorFactory)

   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *un_named_arg1, const QFont &un_named_arg2))
   CS_SLOT_2(slotPropertyChanged)

   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *un_named_arg1))
   CS_SLOT_2(slotEditorDestroyed)

   CS_SLOT_1(Private, void slotSetValue(const QFont &un_named_arg1))
   CS_SLOT_2(slotSetValue)
};

#endif
