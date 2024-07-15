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

#ifndef EXTENSION_H
#define EXTENSION_H

#include <QString>
#include <QObject>

class QAbstractExtensionFactory
{
 public:
   virtual ~QAbstractExtensionFactory()
   { }

   virtual QObject *extension(QObject *object, const QString &iid) const = 0;
};

CS_DECLARE_INTERFACE(QAbstractExtensionFactory, "copperspice.com.CS.QAbstractExtensionFactory")

class QAbstractExtensionManager
{
 public:
   virtual ~QAbstractExtensionManager() {}

   virtual void registerExtensions(QAbstractExtensionFactory *factory, const QString &iid) = 0;
   virtual void unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid) = 0;

   virtual QObject *extension(QObject *object, const QString &iid) const = 0;
};

CS_DECLARE_INTERFACE(QAbstractExtensionManager, "copperspice.com.CS.QAbstractExtensionManager")


template <class T>
inline T qt_extension(QAbstractExtensionManager *, QObject *)
{
   return nullptr;
}


#define CS_TYPEID(IFace) QString(IFace##_iid)

#define CS_DECLARE_EXTENSION_INTERFACE(IFace, IId) \
const QString IFace##_iid = IId; \
CS_DECLARE_INTERFACE(IFace, IId) \
template <> \
inline IFace *qt_extension<IFace *>(QAbstractExtensionManager *manager, QObject *object) \
{ \
  QObject *extension = manager->extension(object, CS_TYPEID(IFace)); \
  return dynamic_cast<IFace *>(extension); \
}

#endif
