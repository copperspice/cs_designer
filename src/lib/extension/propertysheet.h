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

#ifndef PROPERTYSHEET_H
#define PROPERTYSHEET_H

#include <extension.h>

class QVariant;

class QDesignerPropertySheetExtension
{
 public:
   virtual ~QDesignerPropertySheetExtension() {}

   virtual int count() const = 0;

   virtual int indexOf(const QString &name) const = 0;

   virtual QString propertyName(int index) const = 0;
   virtual QString propertyGroup(int index) const = 0;
   virtual void setPropertyGroup(int index, const QString &group) = 0;

   virtual bool hasReset(int index) const = 0;
   virtual bool reset(int index) = 0;

   virtual bool isVisible(int index) const = 0;
   virtual void setVisible(int index, bool b) = 0;

   virtual bool isAttribute(int index) const = 0;
   virtual void setAttribute(int index, bool b) = 0;

   virtual QVariant property(int index) const = 0;
   virtual void setProperty(int index, const QVariant &value) = 0;

   virtual bool isChanged(int index) const = 0;
   virtual void setChanged(int index, bool changed) = 0;

   virtual bool isEnabled(int index) const
   // ### Qt6 remove body, provided in Qt5 for source compatibility to Qt4.
   {
      Q_UNUSED(index);
      return true;
   }
};

CS_DECLARE_EXTENSION_INTERFACE(QDesignerPropertySheetExtension, "copperspice.com.CS.Designer.PropertySheet")

#endif // PROPERTYSHEET_H
