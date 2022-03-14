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

#ifndef LAYOUTDECORATION_H
#define LAYOUTDECORATION_H

#include <extension.h>

#include <QObject>
#include <QPair>

class QPoint;
class QLayoutItem;
class QWidget;
class QRect;
class QLayout;

class QDesignerLayoutDecorationExtension
{
 public:
   enum InsertMode {
      InsertWidgetMode,
      InsertRowMode,
      InsertColumnMode
   };

   virtual ~QDesignerLayoutDecorationExtension() {}

   virtual QList<QWidget *> widgets(QLayout *layout) const = 0;

   virtual QRect itemInfo(int index) const = 0;
   virtual int indexOf(QWidget *widget) const = 0;
   virtual int indexOf(QLayoutItem *item) const = 0;

   virtual InsertMode currentInsertMode() const = 0;
   virtual int currentIndex() const = 0;
   virtual QPair<int, int> currentCell() const = 0;
   virtual void insertWidget(QWidget *widget, const QPair<int, int> &cell) = 0;
   virtual void removeWidget(QWidget *widget) = 0;

   virtual void insertRow(int row) = 0;
   virtual void insertColumn(int column) = 0;
   virtual void simplify() = 0;

   virtual int findItemAt(const QPoint &pos) const = 0;
   virtual int findItemAt(int row, int column) const = 0; // atm only for grid.

   virtual void adjustIndicator(const QPoint &pos, int index) = 0;
};

CS_DECLARE_EXTENSION_INTERFACE(QDesignerLayoutDecorationExtension, "copperspice.com.CS.Designer.LayoutDecoration")

#endif // LAYOUTDECORATION_H
