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

#ifndef QTGRADIENTWIDGET_H
#define QTGRADIENTWIDGET_H

#include <QWidget>

class QtGradientWidget : public QWidget
{
   CS_OBJECT(QtGradientWidget)

   CS_PROPERTY_READ(backgroundCheckered, isBackgroundCheckered)
   CS_PROPERTY_WRITE(backgroundCheckered, setBackgroundCheckered)

 public:
   QtGradientWidget(QWidget *parent = nullptr);
   ~QtGradientWidget();

   QSize minimumSizeHint() const override;
   QSize sizeHint() const override;
   int heightForWidth(int w) const override;

   bool isBackgroundCheckered() const;
   void setBackgroundCheckered(bool checkered);

   QVector<QPair<qreal, QColor>> gradientStops() const;

   void setGradientType(QGradient::Type type);
   QGradient::Type gradientType() const;

   void setGradientSpread(QGradient::Spread spread);
   QGradient::Spread gradientSpread() const;

   void setStartLinear(const QPointF &point);
   QPointF startLinear() const;

   void setEndLinear(const QPointF &point);
   QPointF endLinear() const;

   void setCentralRadial(const QPointF &point);
   QPointF centralRadial() const;

   void setFocalRadial(const QPointF &point);
   QPointF focalRadial() const;

   void setRadiusRadial(qreal radius);
   qreal radiusRadial() const;

   void setCentralConical(const QPointF &point);
   QPointF centralConical() const;

   void setAngleConical(qreal angle);
   qreal angleConical() const;

   CS_SLOT_1(Public, void setGradientStops(const QVector<QPair<qreal, QColor>> &stops))
   CS_SLOT_2(setGradientStops)

   CS_SIGNAL_1(Public, void startLinearChanged(const QPointF &point))
   CS_SIGNAL_2(startLinearChanged, point)
   CS_SIGNAL_1(Public, void endLinearChanged(const QPointF &point))
   CS_SIGNAL_2(endLinearChanged, point)
   CS_SIGNAL_1(Public, void centralRadialChanged(const QPointF &point))
   CS_SIGNAL_2(centralRadialChanged, point)
   CS_SIGNAL_1(Public, void focalRadialChanged(const QPointF &point))
   CS_SIGNAL_2(focalRadialChanged, point)
   CS_SIGNAL_1(Public, void radiusRadialChanged(qreal radius))
   CS_SIGNAL_2(radiusRadialChanged, radius)
   CS_SIGNAL_1(Public, void centralConicalChanged(const QPointF &point))
   CS_SIGNAL_2(centralConicalChanged, point)
   CS_SIGNAL_1(Public, void angleConicalChanged(qreal angle))
   CS_SIGNAL_2(angleConicalChanged, angle)

 protected:
   void paintEvent(QPaintEvent *e) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseReleaseEvent(QMouseEvent *e) override;
   void mouseMoveEvent(QMouseEvent *e) override;
   void mouseDoubleClickEvent(QMouseEvent *e) override;

 private:
   QScopedPointer<class QtGradientWidgetPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtGradientWidget)
   Q_DISABLE_COPY(QtGradientWidget)
};

#endif
