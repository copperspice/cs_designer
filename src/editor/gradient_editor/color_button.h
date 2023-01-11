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

#ifndef QTCOLORBUTTON_H
#define QTCOLORBUTTON_H

#include <QToolButton>

class QtColorButton : public QToolButton
{
   CS_OBJECT(QtColorButton)

   CS_PROPERTY_READ(backgroundCheckered, isBackgroundCheckered)
   CS_PROPERTY_WRITE(backgroundCheckered, setBackgroundCheckered)

 public:
   QtColorButton(QWidget *parent = nullptr);
   ~QtColorButton();

   bool isBackgroundCheckered() const;
   void setBackgroundCheckered(bool checkered);

   QColor color() const;

   CS_SLOT_1(Public, void setColor(const QColor &color))
   CS_SLOT_2(setColor)

   CS_SIGNAL_1(Public, void colorChanged(const QColor &color))
   CS_SIGNAL_2(colorChanged, color)

 protected:
   void paintEvent(QPaintEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;

   void dragEnterEvent(QDragEnterEvent *event) override;
   void dragLeaveEvent(QDragLeaveEvent *event) override;
   void dropEvent(QDropEvent *event) override;

 private:
   QScopedPointer<class QtColorButtonPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtColorButton)

   Q_DISABLE_COPY(QtColorButton)

   CS_SLOT_1(Private, void slotEditColor())
   CS_SLOT_2(slotEditColor)
};

#endif
