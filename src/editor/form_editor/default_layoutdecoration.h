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

#ifndef DEFAULT_LAYOUTDECORATION_H
#define DEFAULT_LAYOUTDECORATION_H

#include <layout_widget.h>
#include <default_extensionfactory.h>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QDesignerLayoutDecorationFactory: public QExtensionFactory
{
   CS_OBJECT(QDesignerLayoutDecorationFactory)

   CS_INTERFACES(QAbstractExtensionFactory)

 public:
   explicit QDesignerLayoutDecorationFactory(QExtensionManager *parent = nullptr);

 protected:
   QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const override;
};

}   // end namespace - qdesigner_internal


#endif
