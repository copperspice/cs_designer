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

#ifndef QDESIGNER_GRID_H
#define QDESIGNER_GRID_H

#include <QVariant>

class QPaintEvent;
class QPainter;
class QWidget;

namespace qdesigner_internal {

// Designer grid which is able to serialize to QVariantMap
class Grid
{
 public:
   Grid();

   bool fromVariantMap(const QVariantMap &vm);

   void addToVariantMap(QVariantMap &vm, bool forceKeys = false) const;
   QVariantMap toVariantMap(bool forceKeys = false) const;

   inline bool visible() const   {
      return m_visible;
   }
   void setVisible(bool visible) {
      m_visible = visible;
   }

   inline bool snapX() const     {
      return m_snapX;
   }
   void setSnapX(bool snap)      {
      m_snapX = snap;
   }

   inline bool snapY() const     {
      return m_snapY;
   }
   void setSnapY(bool snap)      {
      m_snapY = snap;
   }

   inline int deltaX() const     {
      return m_deltaX;
   }
   void setDeltaX(int dx)        {
      m_deltaX = dx;
   }

   inline int deltaY() const     {
      return m_deltaY;
   }
   void setDeltaY(int dy)        {
      m_deltaY = dy;
   }

   void paint(QWidget *widget, QPaintEvent *e) const;
   void paint(QPainter &p, const QWidget *widget, QPaintEvent *e) const;

   QPoint snapPoint(const QPoint &p) const;

   int widgetHandleAdjustX(int x) const;
   int widgetHandleAdjustY(int y) const;

   inline bool operator==(const Grid &rhs) const {
      return equals(rhs);
   }
   inline bool operator!=(const Grid &rhs) const {
      return !equals(rhs);
   }

 private:
   bool equals(const Grid &rhs) const;
   int snapValue(int value, int grid) const;
   bool m_visible;
   bool m_snapX;
   bool m_snapY;
   int m_deltaX;
   int m_deltaY;
};
} // namespace qdesigner_internal



#endif // QDESIGNER_GRID_H
