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

#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <designer_propertysheet.h>
#include <extension.h>
#include <layout_info.h>

#include <spacer_widget_p.h>

#include <QEvent>
#include <QLayout>
#include <QPainter>

// The Spacer widget is Designer representation of  QLayoutItem.
// It uses QLayoutItem's sizeHint property as QWidget
// sizeHint and the QLayoutItem's sizeType property as QWidget size policy.
// If it is not within a layout, it adds a margin (m_SizeOffset) around it
// to avoid being shrunk to an invisible state when the sizeHint is reset to 0,0
// and enables sizeHandle-resizing. In a layout, however, this m_SizeOffset
// should not be applied for  pixel-exact design.

Spacer::Spacer(QWidget *parent)
   : QWidget(parent),
     m_SizeOffset(3, 3), // A small offset to ensure the spacer is still visible when reset to size 0,0
     m_orientation(Qt::Vertical), m_interactive(true), m_layoutState(UnknownLayoutState),
     m_sizeHint(0, 0)
{
   setAttribute(Qt::WA_MouseNoMask);
   m_formWindow = QDesignerFormWindowInterface::findFormWindow(this);
   setSizeType(QSizePolicy::Expanding);
}

bool Spacer::event(QEvent *e)
{
   switch (e->type()) {
      case QEvent::ToolTip:
         updateToolTip(); // Tooltip includes size, so, refresh on demand
         break;
      case QEvent::ParentChange: // Cache information about 'being in layout' which is expensive to calculate.
         m_layoutState = UnknownLayoutState;
         break;
      default:
         break;
   }
   return QWidget::event(e);
}

bool Spacer::isInLayout() const
{
   if (m_layoutState == UnknownLayoutState) {
      m_layoutState = OutsideLayout;
      if (m_formWindow)
         if (const QWidget *parent = parentWidget())
            if (qdesigner_internal::LayoutInfo::managedLayoutType(m_formWindow->core(), parent) != qdesigner_internal::LayoutInfo::NoLayout) {
               m_layoutState = InLayout;
            }
   }
   return m_layoutState == InLayout;
}

void Spacer::paintEvent(QPaintEvent *)
{
   // Only draw spacers when we're editting widgets
   if (m_formWindow != nullptr && m_formWindow->currentTool() != 0) {
      return;
   }

   QPainter p(this);
   p.setPen(Qt::blue);

   const int w = width();
   const int h = height();

   if (w * h == 0) {
      return;
   }

   if (w <= m_SizeOffset.width() || h <= m_SizeOffset.height()) {
      const int lw = w - 1;
      const int lh = h - 1;
      switch (m_orientation) {
         case Qt::Horizontal:
            p.drawLine(0,  0, 0,  lh);
            p.drawLine(lw, 0, lw, lh);
            break;
         case Qt::Vertical:
            p.drawLine(0, 0,  lw, 0);
            p.drawLine(0, lh, lw, lh);
            break;
      }
      return;
   }
   if (m_orientation == Qt::Horizontal) {
      const int dist = 3;
      const int amplitude = qMin(3, h / 3);
      const int base = h / 2;
      int i = 0;
      p.setPen(Qt::white);
      for (i = 0; i < w / 3 + 2; ++i) {
         p.drawLine(i * dist, base - amplitude, i * dist + dist / 2, base + amplitude);
      }
      p.setPen(Qt::blue);
      for (i = 0; i < w / 3 + 2; ++i) {
         p.drawLine(i * dist + dist / 2, base + amplitude, i * dist + dist, base - amplitude);
      }
      const int y = h / 2;
      p.drawLine(0, y - 10, 0, y + 10);
      p.drawLine(w - 1, y - 10, w - 1, y + 10);
   } else {
      const int dist = 3;
      const int amplitude = qMin(3, w / 3);
      const int base = w / 2;
      int i = 0;
      p.setPen(Qt::white);
      for (i = 0; i < h / 3 + 2; ++i) {
         p.drawLine(base - amplitude, i * dist, base + amplitude, i * dist + dist / 2);
      }
      p.setPen(Qt::blue);
      for (i = 0; i < h / 3 + 2; ++i) {
         p.drawLine(base + amplitude, i * dist + dist / 2, base - amplitude, i * dist + dist);
      }
      const int x = w / 2;
      p.drawLine(x - 10, 0, x + 10, 0);
      p.drawLine(x - 10, h - 1, x + 10, h - 1);
   }
}

void Spacer::resizeEvent(QResizeEvent *e)
{
   QWidget::resizeEvent(e);
   // When resized by widget handle dragging after a reset (QSize(0, 0)):
   // Mark the property as changed (geometry and sizeHint are in sync except for 'changed').
   if (m_formWindow) {
      const QSize oldSize = e->oldSize();
      if (oldSize.isNull() || oldSize.width() <= m_SizeOffset.width() || oldSize.height() <= m_SizeOffset.height())
         if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(m_formWindow->core()->extensionManager(),
                  this)) {
            sheet->setChanged(sheet->indexOf(QString("sizeHint")), true);
         }
   }

   updateMask();

   if (!m_interactive) {
      return;
   }

   if (!isInLayout()) { // Allow size-handle resize only if not in layout
      const QSize currentSize = size();
      if (currentSize.width() >= m_SizeOffset.width() && currentSize.height() >= m_SizeOffset.height()) {
         m_sizeHint = currentSize - m_SizeOffset;
      }
   }
}

void Spacer::updateMask()
{
   QRegion r(rect());
   const int w = width();
   const int h = height();
   if (w > 1 && h > 1) {
      if (m_orientation == Qt::Horizontal) {
         const int amplitude = qMin(3, h / 3);
         const int base = h / 2;
         r = r.subtracted(QRect(1, 0, w - 2, base - amplitude));
         r = r.subtracted(QRect(1, base + amplitude, w - 2, h - base - amplitude));
      } else {
         const int amplitude = qMin(3, w / 3);
         const int base = w / 2;
         r = r.subtracted(QRect(0, 1, base - amplitude, h - 2));
         r = r.subtracted(QRect(base + amplitude, 1, w - base - amplitude, h - 2));
      }
   }
   setMask(r);
}

void Spacer::setSizeType(QSizePolicy::Policy t)
{
   const QSizePolicy sizeP = m_orientation == Qt::Vertical ? QSizePolicy(QSizePolicy::Minimum, t) : QSizePolicy(t, QSizePolicy::Minimum);
   setSizePolicy(sizeP);
}


QSizePolicy::Policy Spacer::sizeType() const
{
   return m_orientation == Qt::Vertical ? sizePolicy().verticalPolicy() : sizePolicy().horizontalPolicy();
}

Qt::Alignment Spacer::alignment() const
{
   // For grid layouts
   return m_orientation == Qt::Vertical ?  Qt::AlignHCenter : Qt::AlignVCenter;
}

QSize Spacer::sizeHint() const
{
   return isInLayout() ? m_sizeHint : m_sizeHint + m_SizeOffset;
}

QSize Spacer::sizeHintProperty() const
{
   return m_sizeHint;
}

void Spacer::setSizeHintProperty(const QSize &s)
{
   m_sizeHint = s;

   if (!isInLayout()) { // Visible resize only if not in layout
      resize(s + m_SizeOffset);
   }

   updateGeometry();
}

Qt::Orientation Spacer::orientation() const
{
   return m_orientation;
}

void Spacer::setOrientation(Qt::Orientation o)
{
   if (m_orientation == o) {
      return;
   }

   const QSizePolicy::Policy st = sizeType(); // flip size type
   m_orientation = o;
   setSizeType(st);

   if (m_interactive) {
      m_sizeHint = QSize(m_sizeHint.height(), m_sizeHint.width());
      if (!isInLayout()) {
         resize(m_sizeHint + m_SizeOffset);
      }
   }

   updateMask();
   update();
   updateGeometry();
}

void Spacer::updateToolTip()
{
   const QString format = m_orientation == Qt::Horizontal ? tr("Horizontal Spacer '%1', %2 x %3") : tr("Vertical Spacer '%1', %2 x %3");
   QString msg = format.formatArg(objectName()).formatArg(m_sizeHint.width()).formatArg(m_sizeHint.height());
   setToolTip(msg);
}


