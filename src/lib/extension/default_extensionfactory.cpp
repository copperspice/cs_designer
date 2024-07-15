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

#include <QPointer>

QExtensionFactory::QExtensionFactory(QExtensionManager *parent)
   : QObject(parent)
{
}

QObject *QExtensionFactory::extension(QObject *object, const QString &iid) const
{
   if (! object) {
      return nullptr;
   }

   const QPair<QString, QObject *> key = qMakePair(iid, object);

   auto iter = m_extensions.find(key);

   if (iter == m_extensions.end()) {
      if (QObject *ext = createExtension(object, iid, const_cast<QExtensionFactory *>(this))) {
         connect(ext, &QObject::destroyed, this, &QExtensionFactory::objectDestroyed);
         iter = m_extensions.insert(key, ext);
      }
   }

   if (! m_extended.contains(object)) {
      connect(object, &QObject::destroyed, this, &QExtensionFactory::objectDestroyed);
      m_extended.insert(object, true);
   }

   if (iter == m_extensions.end()) {
      return nullptr;
   }

   return iter.value();
}

void QExtensionFactory::objectDestroyed(QObject *object)
{
   QMutableMapIterator< QPair<QString, QObject *>, QObject *> iter(m_extensions);

   while (iter.hasNext()) {
      iter.next();

      QObject *obj = iter.key().second;

      if (obj == object || object == iter.value()) {
         iter.remove();
      }
   }

   m_extended.remove(object);
}

QObject *QExtensionFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
   (void) object;
   (void) iid;
   (void) parent;

   return nullptr;
}

QExtensionManager *QExtensionFactory::extensionManager() const
{
   return static_cast<QExtensionManager *>(parent());
}
