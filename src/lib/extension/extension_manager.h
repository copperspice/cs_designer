/***********************************************************************
*
* Copyright (c) 2021-2025 Barbara Geller
* Copyright (c) 2021-2025 Ansel Sermersheim
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

#ifndef QEXTENSIONMANAGER_H
#define QEXTENSIONMANAGER_H

#include <extension.h>

#include <QList>
#include <QHash>

class QExtensionManager : public QObject, public QAbstractExtensionManager
{
   CS_OBJECT_MULTIPLE(QExtensionManager, QObject)
   CS_INTERFACES(QAbstractExtensionManager)

 public:
   explicit QExtensionManager(QObject *parent = nullptr);
   ~QExtensionManager();

   void registerExtensions(QAbstractExtensionFactory *factory, const QString &iid = QString()) override;
   void unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid = QString()) override;

   QObject *extension(QObject *object, const QString &iid) const override;

 private:
   typedef QList<QAbstractExtensionFactory *> FactoryList;
   typedef QHash<QString, FactoryList> FactoryMap;
   FactoryMap m_extensions;
   FactoryList m_globalExtension;
};

#endif
