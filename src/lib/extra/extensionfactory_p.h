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

#ifndef SHARED_EXTENSIONFACTORY_H
#define SHARED_EXTENSIONFACTORY_H

#include <default_extensionfactory.h>
#include <extension.h>
#include <extension_manager.h>

namespace qdesigner_internal {

// Extension factory for registering an extension for an object type.
template <class ExtensionInterface, class Object, class Extension>
class ExtensionFactory: public QExtensionFactory
{
 public:
   explicit ExtensionFactory(const QString &iid, QExtensionManager *parent = nullptr);

   // Convenience for registering the extension. Do not use for derived classes.
   static void registerExtension(QExtensionManager *mgr, const QString &iid);

 protected:
   QObject *createExtension(QObject *qObject, const QString &iid, QObject *parent) const override;

 private:
   // Can be overwritten to perform checks on the object.
   // Default does a qobject_cast to the desired class.
   virtual Object *checkObject(QObject *qObject) const;

   const QString m_iid;
};

template <class ExtensionInterface, class Object, class Extension> ExtensionFactory<ExtensionInterface, Object, Extension>::ExtensionFactory(
   const QString &iid, QExtensionManager *parent) :
   QExtensionFactory(parent),
   m_iid(iid)
{
}

template <class ExtensionInterface, class Object, class Extension>
Object *ExtensionFactory<ExtensionInterface, Object, Extension>::checkObject(QObject *qObject) const
{
   return dynamic_cast<Object *>(qObject);
}

template <class ExtensionInterface, class Object, class Extension>
QObject *ExtensionFactory<ExtensionInterface, Object, Extension>::createExtension(QObject *qObject, const QString &iid,
   QObject *parent) const
{
   if (iid != m_iid) {
      return 0;
   }

   Object *object = checkObject(qObject);

   if (! object) {
      return 0;
   }

   return new Extension(object, parent);
}

template <class ExtensionInterface, class Object, class Extension>
void ExtensionFactory<ExtensionInterface, Object, Extension>::registerExtension(QExtensionManager *mgr, const QString &iid)
{
   ExtensionFactory *factory = new ExtensionFactory(iid, mgr);
   mgr->registerExtensions(factory, iid);
}

}  // namespace qdesigner_internal

#endif
