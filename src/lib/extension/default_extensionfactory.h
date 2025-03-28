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

#ifndef DEFAULT_EXTENSIONFACTORY_H
#define DEFAULT_EXTENSIONFACTORY_H

#include <extension.h>

#include <QHash>
#include <QMap>
#include <QPair>

class QExtensionManager;

class QExtensionFactory : public QObject, public QAbstractExtensionFactory
{
   CS_OBJECT(QExtensionFactory)
   CS_INTERFACES(QAbstractExtensionFactory)

 public:
   explicit QExtensionFactory(QExtensionManager *parent = nullptr);

   QObject *extension(QObject *object, const QString &iid) const override;
   QExtensionManager *extensionManager() const;

 protected:
   virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;

 private:
   CS_SLOT_1(Private, void objectDestroyed(QObject *object))
   CS_SLOT_2(objectDestroyed)

   mutable QMap< QPair<QString, QObject *>, QObject *> m_extensions;
   mutable QHash<QObject *, bool> m_extended;
};


#endif // DEFAULT_EXTENSIONFACTORY_H
