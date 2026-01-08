/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#ifndef ABSTRACTMETAOBJECT_H
#define ABSTRACTMETAOBJECT_H

#include <QFlags>
#include <QVariant>

class QDesignerMetaEnumInterface
{
 public:
   QDesignerMetaEnumInterface();
   virtual ~QDesignerMetaEnumInterface();

   virtual bool isFlag() const = 0;
   virtual QString key(int index) const = 0;
   virtual int keyCount() const = 0;
   virtual int keyToValue(const QString &key) const = 0;
   virtual int keysToValue(const QString &keys) const = 0;
   virtual QString name() const = 0;
   virtual QString scope() const = 0;
   virtual QString separator() const = 0;
   virtual int value(int index) const = 0;
   virtual QString valueToKey(int value) const = 0;
   virtual QString valueToKeys(int value) const  = 0;
};

class QDesignerMetaPropertyInterface
{
 public:
   enum Kind {
      EnumKind,
      FlagKind,
      OtherKind
   };

   enum AccessFlag {
      ReadAccess  = 0x0001,
      WriteAccess = 0x0002,
      ResetAccess = 0x0004
   };

   enum Attribute {
      DesignableAttribute = 0x0001,
      ScriptableAttribute = 0x0002,
      StoredAttribute     = 0x0004,
      UserAttribute       = 0x0008
   };

   using Attributes  = QFlags<Attribute>;
   using AccessFlags = QFlags<AccessFlag>;

   QDesignerMetaPropertyInterface();
   virtual ~QDesignerMetaPropertyInterface();

   virtual const QDesignerMetaEnumInterface *enumerator() const = 0;

   virtual Kind kind() const = 0;
   virtual AccessFlags accessFlags() const = 0;
   virtual Attributes attributes(const QObject *object = nullptr) const = 0;

   virtual QVariant::Type type() const = 0;
   virtual QString name() const = 0;
   virtual QString typeName() const = 0;
   virtual int userType() const = 0;
   virtual bool hasSetter() const = 0;

   virtual QVariant read(const QObject *object) const = 0;
   virtual bool reset(QObject *object) const = 0;
   virtual bool write(QObject *object, const QVariant &value) const  = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDesignerMetaPropertyInterface::AccessFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDesignerMetaPropertyInterface::Attributes)

class QDesignerMetaMethodInterface
{
 public:
   QDesignerMetaMethodInterface();
   virtual ~QDesignerMetaMethodInterface();

   enum MethodType { Method, Signal, Slot, Constructor };
   enum Access { Private, Protected, Public };

   virtual Access access() const  = 0;
   virtual MethodType methodType() const = 0;
   virtual QStringList parameterNames() const = 0;
   virtual QStringList parameterTypes() const = 0;
   virtual QString signature() const = 0;
   virtual QString normalizedSignature() const = 0;
   virtual QString tag() const = 0;
   virtual QString typeName() const  = 0;
};

class QDesignerMetaObjectInterface
{
 public:
   QDesignerMetaObjectInterface();
   virtual ~QDesignerMetaObjectInterface();

   virtual QString className() const = 0;
   virtual const QDesignerMetaEnumInterface *enumerator(int index) const = 0;
   virtual int enumeratorCount() const = 0;
   virtual int enumeratorOffset() const = 0;

   virtual int indexOfEnumerator(const QString &name) const = 0;
   virtual int indexOfMethod(const QString &method) const = 0;
   virtual int indexOfProperty(const QString &name) const = 0;
   virtual int indexOfSignal(const QString &signal) const = 0;
   virtual int indexOfSlot(const QString &slot) const = 0;

   virtual const QDesignerMetaMethodInterface *method(int index) const = 0;
   virtual int methodCount() const = 0;
   virtual int methodOffset() const = 0;

   virtual const  QDesignerMetaPropertyInterface *property(int index) const = 0;
   virtual int propertyCount() const = 0;
   virtual int propertyOffset() const = 0;

   virtual const QDesignerMetaObjectInterface *superClass() const = 0;
   virtual const QDesignerMetaPropertyInterface *userProperty() const  = 0;
};

class QDesignerIntrospectionInterface
{
 public:
   QDesignerIntrospectionInterface();
   virtual ~QDesignerIntrospectionInterface();

   virtual const QDesignerMetaObjectInterface *metaObject(const QObject *object) const = 0;
};

#endif