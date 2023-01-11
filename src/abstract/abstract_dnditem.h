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

#ifndef ABSTRACTDNDITEM_H
#define ABSTRACTDNDITEM_H

class DomUI;

class QWidget;
class QPoint;

class QDesignerDnDItemInterface
{
 public:
   enum DropType { MoveDrop, CopyDrop };

   QDesignerDnDItemInterface() {}
   virtual ~QDesignerDnDItemInterface() {}

   virtual DomUI *domUi() const = 0;
   virtual QWidget *widget() const = 0;
   virtual QWidget *decoration() const = 0;
   virtual QPoint hotSpot() const = 0;
   virtual DropType type() const = 0;
   virtual QWidget *source() const = 0;
};

#endif // ABSTRACTDNDITEM_H
