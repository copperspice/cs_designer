/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#ifndef QTCOLORLINE_H
#define QTCOLORLINE_H

#include <QWidget>

class QtColorLine : public QWidget
{
   CS_OBJECT(QtColorLine)

   CS_PROPERTY_READ(color, color)
   CS_PROPERTY_WRITE(color, setColor)

   CS_PROPERTY_READ(indicatorSpace, indicatorSpace)
   CS_PROPERTY_WRITE(indicatorSpace, setIndicatorSpace)

   CS_PROPERTY_READ(indicatorSize, indicatorSize)
   CS_PROPERTY_WRITE(indicatorSize, setIndicatorSize)

   CS_PROPERTY_READ(flip, flip)
   CS_PROPERTY_WRITE(flip, setFlip)

   CS_PROPERTY_READ(backgroundCheckered, isBackgroundCheckered)
   CS_PROPERTY_WRITE(backgroundCheckered, setBackgroundCheckered)

   CS_PROPERTY_READ(colorComponent, colorComponent)
   CS_PROPERTY_WRITE(colorComponent, setColorComponent)

   CS_PROPERTY_READ(orientation, orientation)
   CS_PROPERTY_WRITE(orientation, setOrientation)

   CS_ENUM(ColorComponent)

 public:
   enum ColorComponent {
      Red,
      Green,
      Blue,
      Hue,
      Saturation,
      Value,
      Alpha
   };

   QSize minimumSizeHint() const override;
   QSize sizeHint() const override;

   QtColorLine(QWidget *parent = nullptr);
   ~QtColorLine();

   QColor color() const;

   void setIndicatorSize(int size);
   int indicatorSize() const;

   void setIndicatorSpace(int space);
   int indicatorSpace() const;

   void setFlip(bool flip);
   bool flip() const;

   bool isBackgroundCheckered() const;
   void setBackgroundCheckered(bool checkered);

   void setOrientation(Qt::Orientation orientation);
   Qt::Orientation orientation() const;

   void setColorComponent(ColorComponent component);
   ColorComponent colorComponent() const;

   CS_SLOT_1(Public, void setColor(const QColor &color))
   CS_SLOT_2(setColor)

   CS_SIGNAL_1(Public, void colorChanged(const QColor &color))
   CS_SIGNAL_2(colorChanged, color)

 protected:
   void resizeEvent(QResizeEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mouseDoubleClickEvent(QMouseEvent *event) override;

 private:
   QScopedPointer<class QtColorLinePrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtColorLine)
   Q_DISABLE_COPY(QtColorLine)
};

#endif
