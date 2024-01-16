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

#ifndef SPACER_WIDGET_H
#define SPACER_WIDGET_H

#include <QSizePolicy>
#include <QWidget>

class QDesignerFormWindowInterface;

class Spacer: public QWidget
{
   CS_OBJECT(Spacer)

   CS_PROPERTY_READ(spacerName, objectName)
   CS_PROPERTY_WRITE(spacerName, setObjectName)

   CS_PROPERTY_READ(orientation, orientation)
   CS_PROPERTY_WRITE(orientation, setOrientation)

   CS_PROPERTY_READ(sizeType, sizeType)
   CS_PROPERTY_WRITE(sizeType, setSizeType)

   CS_PROPERTY_READ(sizeHint, sizeHintProperty)
   CS_PROPERTY_WRITE(sizeHint, setSizeHintProperty)
   CS_PROPERTY_DESIGNABLE(sizeHint, true)
   CS_PROPERTY_STORED(sizeHint, true)

 public:
   Spacer(QWidget *parent = nullptr);

   QSize sizeHint() const override;

   QSize sizeHintProperty() const;
   void setSizeHintProperty(const QSize &s);

   QSizePolicy::Policy sizeType() const;
   void setSizeType(QSizePolicy::Policy t);

   Qt::Alignment alignment() const;
   Qt::Orientation orientation() const;

   void setOrientation(Qt::Orientation o);

   void setInteractiveMode(bool b) {
      m_interactive = b;
   }

   bool event(QEvent *e) override;

 protected:
   void paintEvent(QPaintEvent *e) override;
   void resizeEvent(QResizeEvent *e) override;
   void updateMask();

 private:
   bool isInLayout() const;
   void updateToolTip();

   const QSize m_SizeOffset;
   QDesignerFormWindowInterface *m_formWindow;
   Qt::Orientation m_orientation;
   bool m_interactive;

   // Cache information about 'being in layout' which is expensive to calculate.
   enum LayoutState { InLayout, OutsideLayout, UnknownLayoutState };
   mutable LayoutState m_layoutState;
   QSize m_sizeHint;
};

#endif
