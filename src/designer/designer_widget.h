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

#ifndef QDESIGNER_WIDGET_H
#define QDESIGNER_WIDGET_H

#include <QDialog>
#include <QLabel>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {
class FormWindowBase;
}

class QDesignerWidget : public QWidget
{
   CS_OBJECT(QDesignerWidget)

 public:
   explicit QDesignerWidget(QDesignerFormWindowInterface *formWindow, QWidget *parent = nullptr);
   virtual ~QDesignerWidget();

   QDesignerFormWindowInterface *formWindow() const;

   void updatePixmap();

   QSize minimumSizeHint() const override {
      return QWidget::minimumSizeHint().expandedTo(QSize(16, 16));
   }

 protected:
   void paintEvent(QPaintEvent *e) override;

 private:
   qdesigner_internal::FormWindowBase *m_formWindow;
};

class QDesignerDialog : public QDialog
{
   CS_OBJECT(QDesignerDialog)

 public:
   explicit QDesignerDialog(QDesignerFormWindowInterface *fw, QWidget *parent);

   QSize minimumSizeHint() const override {
      return QWidget::minimumSizeHint().expandedTo(QSize(16, 16));
   }

 protected:
   void paintEvent(QPaintEvent *e) override;

 private:
   qdesigner_internal::FormWindowBase *m_formWindow;
};

class Line : public QFrame
{
   CS_OBJECT(Line)

   CS_PROPERTY_READ(orientation, orientation)
   CS_PROPERTY_WRITE(orientation, setOrientation)

 public:
   explicit Line(QWidget *parent)
      : QFrame(parent)
   {
      setAttribute(Qt::WA_MouseNoMask);
      setFrameStyle(QFrame::HLine | QFrame::Sunken);
   }

   void setOrientation(Qt::Orientation orient) {
      setFrameShape(orient == Qt::Horizontal ? QFrame::HLine : QFrame::VLine);
   }

   Qt::Orientation orientation() const {
      return frameShape() == QFrame::HLine ? Qt::Horizontal : Qt::Vertical;
   }
};

#endif // QDESIGNER_WIDGET_H
