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

#include <extension_manager.h>

QExtensionManager::QExtensionManager(QObject *parent)
   : QObject(parent)
{
}

QExtensionManager::~QExtensionManager()
{
}

void QExtensionManager::registerExtensions(QAbstractExtensionFactory *factory, const QString &iid)
{
   if (iid.isEmpty()) {
      m_globalExtension.prepend(factory);
      return;
   }

   FactoryMap::iterator it = m_extensions.find(iid);
   if (it == m_extensions.end()) {
      it = m_extensions.insert(iid, FactoryList());
   }

   it.value().prepend(factory);
}

void QExtensionManager::unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid)
{
   if (iid.isEmpty()) {
      m_globalExtension.removeAll(factory);
      return;
   }

   const FactoryMap::iterator it = m_extensions.find(iid);
   if (it == m_extensions.end()) {
      return;
   }

   FactoryList &factories = it.value();
   factories.removeAll(factory);

   if (factories.isEmpty()) {
      m_extensions.erase(it);
   }
}

QObject *QExtensionManager::extension(QObject *object, const QString &iid) const
{
   auto it = m_extensions.constFind(iid);

   if (it != m_extensions.constEnd()) {
      auto fcend = it.value().constEnd();

      for (auto fit = it.value().constBegin(); fit != fcend; ++fit) {
         if (QObject *ext = (*fit)->extension(object, iid)) {
            return ext;
         }
      }
   }

   auto gfcend = m_globalExtension.constEnd();

   for (auto git = m_globalExtension.constBegin(); git != gfcend; ++git) {
      if (QObject *ext = (*git)->extension(object, iid)) {
         return ext;
      }
   }

   return nullptr;
}

