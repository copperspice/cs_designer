/***********************************************************************
*
* Copyright (c) 2021-2023 Barbara Geller
* Copyright (c) 2021-2023 Ansel Sermersheim
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

#ifndef DESIGNERINTROSPECTION
#define DESIGNERINTROSPECTION

#include <abstract_introspection.h>

#include <QMap>

struct QMetaObject;

class QWidget;

namespace qdesigner_internal {

// introspection with helpers to find core and meta object for an object

class QDesignerIntrospection : public QDesignerIntrospectionInterface
{
 public:
   QDesignerIntrospection();
   virtual ~QDesignerIntrospection();

   const QDesignerMetaObjectInterface *metaObject(const QObject *object) const override;

   const QDesignerMetaObjectInterface *metaObjectForQMetaObject(const QMetaObject *metaObject) const;

 private:
   typedef QMap<const QMetaObject *, QDesignerMetaObjectInterface *> MetaObjectMap;
   mutable MetaObjectMap m_metaObjectMap;
};

}

#endif
