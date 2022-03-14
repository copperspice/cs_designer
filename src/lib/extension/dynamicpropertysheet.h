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

#ifndef DYNAMICPROPERTYSHEET_H
#define DYNAMICPROPERTYSHEET_H

#include <extension.h>

#include <QString>

class QDesignerDynamicPropertySheetExtension
{
 public:
   virtual ~QDesignerDynamicPropertySheetExtension() {
   }

   virtual bool dynamicPropertiesAllowed() const = 0;
   virtual int addDynamicProperty(const QString &propertyName, const QVariant &value) = 0;
   virtual bool removeDynamicProperty(int index) = 0;
   virtual bool isDynamicProperty(int index) const = 0;
   virtual bool canAddDynamicProperty(const QString &propertyName) const = 0;
};

CS_DECLARE_EXTENSION_INTERFACE(QDesignerDynamicPropertySheetExtension, "copperspice.com.CS.Designer.DynamicPropertySheet")

#endif
