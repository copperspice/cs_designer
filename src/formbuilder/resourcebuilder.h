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

#ifndef RESOURCEBUILDER_H
#define RESOURCEBUILDER_H

#include <QList>
#include <QString>

class DomProperty;
class DomResourceIcon;

class QDir;
class QVariant;

class QResourceBuilder
{
 public:
   enum IconStateFlags {
      NormalOff = 0x1, NormalOn = 0x2, DisabledOff = 0x4, DisabledOn = 0x8,
      ActiveOff = 0x10, ActiveOn = 0x20, SelectedOff = 0x40, SelectedOn = 0x80
   };

   QResourceBuilder();
   virtual ~QResourceBuilder();

   virtual QVariant loadResource(const QDir &workingDirectory, const DomProperty *property) const;

   virtual QVariant toNativeValue(const QVariant &value) const;

   virtual DomProperty *saveResource(const QDir &workingDirectory, const QVariant &value) const;

   virtual bool isResourceProperty(const DomProperty *p) const;

   virtual bool isResourceType(const QVariant &value) const;

   static int iconStateFlags(const DomResourceIcon *resIcon);
};

#endif
