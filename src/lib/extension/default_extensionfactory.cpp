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

#include <default_extensionfactory.h>
#include <extension_manager.h>

#include <qpointer.h>
#include <QDebug>

QExtensionFactory::QExtensionFactory(QExtensionManager *parent)
   : QObject(parent)
{
}

QObject *QExtensionFactory::extension(QObject *object, const QString &iid) const
{
   if (!object) {
      return nullptr;
   }

   const IdObjectKey key = qMakePair(iid, object);

   ExtensionMap::iterator it = m_extensions.find(key);
   if (it == m_extensions.end()) {
      if (QObject *ext = createExtension(object, iid, const_cast<QExtensionFactory *>(this))) {
         connect(ext, &QObject::destroyed, this, &QExtensionFactory::objectDestroyed);
         it = m_extensions.insert(key, ext);
      }
   }

   if (!m_extended.contains(object)) {
      connect(object, &QObject::destroyed, this, &QExtensionFactory::objectDestroyed);
      m_extended.insert(object, true);
   }

   if (it == m_extensions.end()) {
      return nullptr;
   }

   return it.value();
}

void QExtensionFactory::objectDestroyed(QObject *object)
{
   QMutableMapIterator< IdObjectKey, QObject *> it(m_extensions);
   while (it.hasNext()) {
      it.next();

      QObject *o = it.key().second;
      if (o == object || object == it.value()) {
         it.remove();
      }
   }

   m_extended.remove(object);
}

QObject *QExtensionFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
   Q_UNUSED(object);
   Q_UNUSED(iid);
   Q_UNUSED(parent);

   return nullptr;
}

QExtensionManager *QExtensionFactory::extensionManager() const
{
   return static_cast<QExtensionManager *>(parent());
}

