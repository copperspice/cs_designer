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

#include <designer_introspection.h>

#include <QMetaEnum>
#include <QMetaObject>
#include <QStringList>
#include <QVector>

#include <qalgorithms.h>

// emerald - remove and use cs meta system directly

namespace  {

class QDesignerMetaEnum : public QDesignerMetaEnumInterface
{
 public:
   QDesignerMetaEnum(const QMetaEnum &qEnum);

   bool isFlag() const override  {
      return m_enum.isFlag();
   }

   QString key(int index) const override {
      return m_enum.key(index);
   }

   int keyCount() const override  {
      return m_enum.keyCount();
   }

   int keyToValue(const QString &key) const override   {
      return m_enum.keyToValue(key);
   }

   int keysToValue(const QString &keys) const override {
      return m_enum.keysToValue(keys);
   }

   QString name() const override  {
      return m_name;
   }

   QString scope() const override  {
      return m_scope;
   }

   QString separator() const override;
   int value(int index) const override {
      return m_enum.value(index);
   }

   QString valueToKey(int value) const override {
      return m_enum.valueToKey(value);
   }

   QString valueToKeys(int value) const override       {
      return m_enum.valueToKeys(value);
   }

 private:
   const QMetaEnum m_enum;
   const QString m_name;
   const QString m_scope;
};

QDesignerMetaEnum::QDesignerMetaEnum(const QMetaEnum &qEnum)
   : m_enum(qEnum), m_name(m_enum.name()), m_scope(m_enum.scope())
{
}

QString QDesignerMetaEnum::separator() const
{
   static const QString rc = "::";
   return rc;
}

class QDesignerMetaProperty : public QDesignerMetaPropertyInterface
{
 public:
   QDesignerMetaProperty(const QMetaProperty &property);
   ~QDesignerMetaProperty();

   const QDesignerMetaEnumInterface *enumerator() const override {
      return m_enumerator;
   }

   Kind kind() const override {
      return m_kind;
   }

   AccessFlags accessFlags() const override {
      return m_access;
   }
   Attributes attributes(const QObject *object = nullptr) const override;

   QVariant::Type type() const override {
      return m_property.type();
   }

   QString name() const override {
      return m_name;
   }

   QString typeName() const override {
      return m_typeName;
   }

   int userType() const override {
      return m_property.userType();
   }

   bool hasSetter() const override      {
      return m_property.hasStdCppSet();
   }

   QVariant read(const QObject *object) const override {
      return m_property.read(object);
   }

   bool reset(QObject *object) const  override {
      return m_property.reset(object);
   }

   bool write(QObject *object, const QVariant &value) const override {
      return m_property.write(object, value);
   }

 private:
   const QMetaProperty m_property;
   const QString m_name;
   const QString m_typeName;

   Kind m_kind;
   AccessFlags m_access;
   Attributes m_defaultAttributes;
   QDesignerMetaEnumInterface *m_enumerator;
};

QDesignerMetaProperty::QDesignerMetaProperty(const QMetaProperty &property)
   : m_property(property), m_name(m_property.name()),
     m_typeName(m_property.typeName()), m_kind(OtherKind), m_enumerator(nullptr)
{
   if (m_property.isFlagType() || m_property.isEnumType()) {
      const QMetaEnum metaEnum = m_property.enumerator();
      Q_ASSERT(metaEnum.isValid());

      m_enumerator = new QDesignerMetaEnum(metaEnum);
   }

   // kind
   if (m_property.isFlagType()) {
      m_kind = FlagKind;

   } else if (m_property.isEnumType()) {
      m_kind = EnumKind;
   }

   // flags and attributes
   if (m_property.isReadable()) {
      m_access |= ReadAccess;
   }

   if (m_property.isWritable()) {
      m_access |= WriteAccess;
   }

   if (m_property.isResettable()) {
      m_access |= ResetAccess;
   }

   if (m_property.isDesignable()) {
      m_defaultAttributes |= DesignableAttribute;
   }

   if (m_property.isScriptable()) {
      m_defaultAttributes |= ScriptableAttribute;
   }

   if (m_property.isStored()) {
      m_defaultAttributes |= StoredAttribute;
   }

   if (m_property.isUser()) {
      m_defaultAttributes |= UserAttribute;
   }
}

QDesignerMetaProperty::~QDesignerMetaProperty()
{
   delete m_enumerator;
}

QDesignerMetaProperty::Attributes QDesignerMetaProperty::attributes(const QObject *object) const
{
   if (! object) {
      return m_defaultAttributes;
   }

   Attributes rc;

   if (m_property.isDesignable(object)) {
      rc |= DesignableAttribute;
   }

   if (m_property.isScriptable(object)) {
      rc |= ScriptableAttribute;
   }

   if (m_property.isStored(object)) {
      rc |= StoredAttribute;
   }

   if (m_property.isUser(object)) {
      rc |= UserAttribute;
   }

   return rc;
}

class QDesignerMetaMethod : public QDesignerMetaMethodInterface
{
 public:
   QDesignerMetaMethod(const QMetaMethod &method);

   Access access() const override {
      return m_access;
   }

   MethodType methodType() const override {
      return m_methodType;
   }

   QStringList parameterNames() const override {
      return m_parameterNames;
   }

   QStringList parameterTypes() const override {
      return m_parameterTypes;
   }

   QString signature() const override {
      return m_signature;
   }

   QString normalizedSignature() const override {
      return m_normalizedSignature;
   }

   QString tag() const override {
      return m_tag;
   }

   QString typeName() const override {
      return m_typeName;
   }

 private:
   Access m_access;
   MethodType m_methodType;

   const QStringList m_parameterNames;
   const QStringList m_parameterTypes;
   const QString m_signature;
   const QString m_normalizedSignature;
   const QString m_tag;
   const QString m_typeName;
};

QDesignerMetaMethod::QDesignerMetaMethod(const QMetaMethod &method)
   : m_parameterNames(method.parameterNames()),
     m_parameterTypes(method.parameterTypes()),
     m_signature(method.methodSignature()),
     m_normalizedSignature(QMetaObject::normalizedSignature(method.methodSignature())),
     m_tag(method.tag()), m_typeName(method.typeName())
{
   switch (method.access()) {
      case QMetaMethod::Public:
         m_access = Public;
         break;

      case QMetaMethod::Protected:
         m_access = Protected;
         break;

      case QMetaMethod::Private:
         m_access = Private;
         break;

   }

   switch (method.methodType()) {
      case QMetaMethod::Constructor:
         m_methodType = Constructor;
         break;

      case QMetaMethod::Method:
         m_methodType = Method;
         break;

      case QMetaMethod::Signal:
         m_methodType = Signal;
         break;

      case QMetaMethod::Slot:
         m_methodType = Slot;
         break;
   }
}

class QDesignerMetaObject : public QDesignerMetaObjectInterface
{
 public:
   QDesignerMetaObject(const qdesigner_internal::QDesignerIntrospection *introspection, const QMetaObject *metaObject);
   ~QDesignerMetaObject();

   QString className() const override {
      return m_className;
   }

   const QDesignerMetaEnumInterface *enumerator(int index) const override {
      return m_enumerators[index];
   }

   int enumeratorCount() const override {
      return m_enumerators.size();
   }

   int enumeratorOffset() const override {
      return m_metaObject->enumeratorOffset();
   }

   int indexOfEnumerator(const QString &name) const override {
      return m_metaObject->indexOfEnumerator(name);
   }

   int indexOfMethod(const QString &method) const override {
      return m_metaObject->indexOfMethod(method);
   }

   int indexOfProperty(const QString &name) const override {
      return m_metaObject->indexOfProperty(name);
   }

   int indexOfSignal(const QString &signal) const override {
      return m_metaObject->indexOfSignal(signal);
   }

   int indexOfSlot(const QString &slot) const override {
      return m_metaObject->indexOfSlot(slot);
   }

   const QDesignerMetaMethodInterface *method(int index) const override {
      return m_methods[index];
   }

   int methodCount() const override {
      return m_methods.size();
   }

   int methodOffset() const override {
      return m_metaObject->methodOffset();
   }

   const QDesignerMetaPropertyInterface *property(int index) const override {
      return m_properties[index];
   }

   int propertyCount() const override {
      return m_properties.size();
   }

   int propertyOffset() const override {
      return m_metaObject->propertyOffset();
   }

   const QDesignerMetaObjectInterface *superClass() const override;

   const QDesignerMetaPropertyInterface *userProperty() const override {
      return m_userProperty;
   }

 private:
   const QString m_className;
   const qdesigner_internal::QDesignerIntrospection *m_introspection;
   const QMetaObject *m_metaObject;

   typedef QVector<QDesignerMetaEnumInterface *> Enumerators;
   Enumerators m_enumerators;

   typedef QVector<QDesignerMetaMethodInterface *> Methods;
   Methods m_methods;

   typedef QVector<QDesignerMetaPropertyInterface *> Properties;
   Properties m_properties;

   QDesignerMetaPropertyInterface *m_userProperty;
};

QDesignerMetaObject::QDesignerMetaObject(const qdesigner_internal::QDesignerIntrospection *introspection,
      const QMetaObject *metaObject)
   :  m_className(metaObject->className()), m_introspection(introspection),
      m_metaObject(metaObject), m_userProperty(nullptr)
{
   const int numEnumerators = metaObject->enumeratorCount();
   m_enumerators.reserve(numEnumerators);

   for (int i = 0; i < numEnumerators; i++) {
      m_enumerators.push_back(new QDesignerMetaEnum(metaObject->enumerator(i)));
   }

   const int numMethods = metaObject->methodCount();
   m_methods.reserve(numMethods);
   for (int i = 0; i < numMethods; i++) {
      m_methods.push_back(new QDesignerMetaMethod(metaObject->method(i)));
   }

   const int numProperties = metaObject->propertyCount();
   m_properties.reserve(numProperties);
   for (int i = 0; i < numProperties; i++) {
      m_properties.push_back(new QDesignerMetaProperty(metaObject->property(i)));
   }

   const QMetaProperty userProperty = metaObject->userProperty();
   if (userProperty.isValid()) {
      m_userProperty = new QDesignerMetaProperty(userProperty);
   }
}

QDesignerMetaObject::~QDesignerMetaObject()
{
   qDeleteAll(m_enumerators);
   qDeleteAll(m_methods);
   qDeleteAll(m_properties);
   delete m_userProperty;
}

const QDesignerMetaObjectInterface *QDesignerMetaObject::superClass() const
{
   const QMetaObject *qSuperClass = m_metaObject->superClass();
   if (!qSuperClass) {
      return nullptr;
   }
   return m_introspection->metaObjectForQMetaObject(qSuperClass);
}
}

namespace qdesigner_internal {

QDesignerIntrospection::QDesignerIntrospection()
{
}

QDesignerIntrospection::~QDesignerIntrospection()
{
   qDeleteAll(m_metaObjectMap.values());
}

const QDesignerMetaObjectInterface *QDesignerIntrospection::metaObject(const QObject *object) const
{
   return metaObjectForQMetaObject(object->metaObject());
}

const QDesignerMetaObjectInterface *QDesignerIntrospection::metaObjectForQMetaObject(const QMetaObject *metaObject) const
{
   MetaObjectMap::iterator it = m_metaObjectMap.find(metaObject);
   if (it == m_metaObjectMap.end()) {
      it = m_metaObjectMap.insert(metaObject, new QDesignerMetaObject(this, metaObject));
   }
   return it.value();
}

}

