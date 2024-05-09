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

#ifndef MEMBERSHEET_H
#define MEMBERSHEET_H

#include <extension.h>

#include <QByteArray>
#include <QList>
#include <QString>

class QDesignerMemberSheetExtension
{
 public:
   virtual ~QDesignerMemberSheetExtension() {}

   virtual int count() const = 0;

   virtual int indexOf(const QString &name) const = 0;

   virtual QString memberName(int index) const = 0;
   virtual QString memberGroup(int index) const = 0;
   virtual void setMemberGroup(int index, const QString &group) = 0;

   virtual bool isVisible(int index) const = 0;
   virtual void setVisible(int index, bool b) = 0;

   virtual bool isSignal(int index) const = 0;
   virtual bool isSlot(int index) const = 0;

   virtual bool inheritedFromWidget(int index) const = 0;

   virtual QString declaredInClass(int index) const = 0;

   virtual QString signature(int index) const = 0;
   virtual QList<QByteArray> parameterTypes(int index) const = 0;
   virtual QList<QByteArray> parameterNames(int index) const = 0;
};

CS_DECLARE_EXTENSION_INTERFACE(QDesignerMemberSheetExtension, "copperspice.com.CS.Designer.MemberSheet")

#endif // MEMBERSHEET_H
