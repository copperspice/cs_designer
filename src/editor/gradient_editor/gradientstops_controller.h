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

#ifndef QTGRADIENTSTOPSCONTROLLER_H
#define QTGRADIENTSTOPSCONTROLLER_H

#include <gradientstops_model.h>

#include <QWidget>

namespace Ui {
class QtGradientEditor;
}

class QtGradientStopsController : public QObject
{
   CS_OBJECT(QtGradientStopsController)

 public:
   QtGradientStopsController(QObject *parent = nullptr);
   ~QtGradientStopsController();

   void setUi(Ui::QtGradientEditor *editor);

   void setGradientStops(const QVector<QPair<qreal, QColor>> &stops);
   QVector<QPair<qreal, QColor>> gradientStops() const;

   QColor::Spec spec() const;
   void setSpec(QColor::Spec spec);

   CS_SIGNAL_1(Public, void gradientStopsChanged(const QVector<QPair<qreal, QColor>> &stops))
   CS_SIGNAL_2(gradientStopsChanged, stops)

 private:
   QScopedPointer<class QtGradientStopsControllerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtGradientStopsController)
   Q_DISABLE_COPY(QtGradientStopsController)

   CS_SLOT_1(Private, void slotHsvClicked())
   CS_SLOT_2(slotHsvClicked)

   CS_SLOT_1(Private, void slotRgbClicked())
   CS_SLOT_2(slotRgbClicked)

   CS_SLOT_1(Private, void slotCurrentStopChanged(QtGradientStop *stop))
   CS_SLOT_2(slotCurrentStopChanged)

   CS_SLOT_1(Private, void slotStopMoved(QtGradientStop *stop, qreal newPos))
   CS_SLOT_2(slotStopMoved)

   CS_SLOT_1(Private, void slotStopsSwapped(QtGradientStop *stop1, QtGradientStop *stop2))
   CS_SLOT_2(slotStopsSwapped)

   CS_SLOT_1(Private, void slotStopChanged(QtGradientStop *stop, const QColor &newColor))
   CS_SLOT_2(slotStopChanged)

   CS_SLOT_1(Private, void slotStopSelected(QtGradientStop *stop, bool selected))
   CS_SLOT_2(slotStopSelected)

   CS_SLOT_1(Private, void slotStopAdded(QtGradientStop *stop))
   CS_SLOT_2(slotStopAdded)

   CS_SLOT_1(Private, void slotStopRemoved(QtGradientStop *stop))
   CS_SLOT_2(slotStopRemoved)

   CS_SLOT_1(Private, void slotUpdatePositionSpinBox())
   CS_SLOT_2(slotUpdatePositionSpinBox)

   CS_SLOT_1(Private, void slotChangeColor(const QColor &color))
   CS_SLOT_2(slotChangeColor)

   CS_SLOT_1(Private, void slotChangeHue(const QColor &color))
   CS_SLOT_OVERLOAD(slotChangeHue, (const QColor &))

   CS_SLOT_1(Private, void slotChangeSaturation(const QColor &color))
   CS_SLOT_OVERLOAD(slotChangeSaturation, (const QColor &))

   CS_SLOT_1(Private, void slotChangeValue(const QColor &color))
   CS_SLOT_OVERLOAD(slotChangeValue, (const QColor &))

   CS_SLOT_1(Private, void slotChangeAlpha(const QColor &color))
   CS_SLOT_OVERLOAD(slotChangeAlpha, (const QColor &))

   CS_SLOT_1(Private, void slotChangeHue(int un_named_arg1))
   CS_SLOT_OVERLOAD(slotChangeHue, (int))

   CS_SLOT_1(Private, void slotChangeSaturation(int un_named_arg1))
   CS_SLOT_OVERLOAD(slotChangeSaturation, (int))

   CS_SLOT_1(Private, void slotChangeValue(int un_named_arg1))
   CS_SLOT_OVERLOAD(slotChangeValue, (int))

   CS_SLOT_1(Private, void slotChangeAlpha(int un_named_arg1))
   CS_SLOT_OVERLOAD(slotChangeAlpha, (int))

   CS_SLOT_1(Private, void slotChangePosition(double value))
   CS_SLOT_2(slotChangePosition)

   CS_SLOT_1(Private, void slotChangeZoom(int value))
   CS_SLOT_2(slotChangeZoom)

   CS_SLOT_1(Private, void slotZoomIn())
   CS_SLOT_2(slotZoomIn)

   CS_SLOT_1(Private, void slotZoomOut())
   CS_SLOT_2(slotZoomOut)

   CS_SLOT_1(Private, void slotZoomAll())
   CS_SLOT_2(slotZoomAll)

   CS_SLOT_1(Private, void slotZoomChanged(double un_named_arg1))
   CS_SLOT_2(slotZoomChanged)
};

#endif
