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

#ifndef QTGRADIENTEDITOR_H
#define QTGRADIENTEDITOR_H

#include <QWidget>

class QtGradientEditor : public QWidget
{
   CS_OBJECT(QtGradientEditor)

   CS_PROPERTY_READ(gradient, gradient)
   CS_PROPERTY_WRITE(gradient, setGradient)

   CS_PROPERTY_READ(backgroundCheckered, isBackgroundCheckered)
   CS_PROPERTY_WRITE(backgroundCheckered, setBackgroundCheckered)

   CS_PROPERTY_READ(detailsVisible, detailsVisible)
   CS_PROPERTY_WRITE(detailsVisible, setDetailsVisible)

   CS_PROPERTY_READ(detailsButtonVisible, isDetailsButtonVisible)
   CS_PROPERTY_WRITE(detailsButtonVisible, setDetailsButtonVisible)

 public:
   QtGradientEditor(QWidget *parent = nullptr);
   ~QtGradientEditor();

   void setGradient(const QGradient &gradient);
   QGradient gradient() const;

   bool isBackgroundCheckered() const;
   void setBackgroundCheckered(bool checkered);

   bool detailsVisible() const;
   void setDetailsVisible(bool visible);

   bool isDetailsButtonVisible() const;
   void setDetailsButtonVisible(bool visible);

   QColor::Spec spec() const;
   void setSpec(QColor::Spec spec);

   CS_SIGNAL_1(Public, void gradientChanged(const QGradient &gradient))
   CS_SIGNAL_2(gradientChanged, gradient)
   CS_SIGNAL_1(Public, void aboutToShowDetails(bool details, int extenstionWidthHint))
   CS_SIGNAL_2(aboutToShowDetails, details, extenstionWidthHint)

 private:
   QScopedPointer<class QtGradientEditorPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtGradientEditor)
   Q_DISABLE_COPY(QtGradientEditor)

   CS_SLOT_1(Private, void slotGradientStopsChanged(const QVector<QPair<qreal, QColor>> &stops))
   CS_SLOT_2(slotGradientStopsChanged)

   CS_SLOT_1(Private, void slotTypeChanged(int type))
   CS_SLOT_2(slotTypeChanged)

   CS_SLOT_1(Private, void slotSpreadChanged(int type))
   CS_SLOT_2(slotSpreadChanged)

   CS_SLOT_1(Private, void slotStartLinearXChanged(double value))
   CS_SLOT_2(slotStartLinearXChanged)

   CS_SLOT_1(Private, void slotStartLinearYChanged(double value))
   CS_SLOT_2(slotStartLinearYChanged)

   CS_SLOT_1(Private, void slotEndLinearXChanged(double value))
   CS_SLOT_2(slotEndLinearXChanged)

   CS_SLOT_1(Private, void slotEndLinearYChanged(double value))
   CS_SLOT_2(slotEndLinearYChanged)

   CS_SLOT_1(Private, void slotCentralRadialXChanged(double value))
   CS_SLOT_2(slotCentralRadialXChanged)

   CS_SLOT_1(Private, void slotCentralRadialYChanged(double value))
   CS_SLOT_2(slotCentralRadialYChanged)

   CS_SLOT_1(Private, void slotFocalRadialXChanged(double value))
   CS_SLOT_2(slotFocalRadialXChanged)

   CS_SLOT_1(Private, void slotFocalRadialYChanged(double value))
   CS_SLOT_2(slotFocalRadialYChanged)

   CS_SLOT_1(Private, void slotRadiusRadialChanged(double value))
   CS_SLOT_2(slotRadiusRadialChanged)

   CS_SLOT_1(Private, void slotCentralConicalXChanged(double value))
   CS_SLOT_2(slotCentralConicalXChanged)

   CS_SLOT_1(Private, void slotCentralConicalYChanged(double value))
   CS_SLOT_2(slotCentralConicalYChanged)

   CS_SLOT_1(Private, void slotAngleConicalChanged(double value))
   CS_SLOT_2(slotAngleConicalChanged)

   CS_SLOT_1(Private, void slotDetailsChanged(bool details))
   CS_SLOT_2(slotDetailsChanged)

   CS_SLOT_1(Private, void startLinearChanged(const QPointF &un_named_arg1))
   CS_SLOT_2(startLinearChanged)

   CS_SLOT_1(Private, void endLinearChanged(const QPointF &un_named_arg1))
   CS_SLOT_2(endLinearChanged)

   CS_SLOT_1(Private, void centralRadialChanged(const QPointF &un_named_arg1))
   CS_SLOT_2(centralRadialChanged)

   CS_SLOT_1(Private, void focalRadialChanged(const QPointF &un_named_arg1))
   CS_SLOT_2(focalRadialChanged)

   CS_SLOT_1(Private, void radiusRadialChanged(qreal un_named_arg1))
   CS_SLOT_2(radiusRadialChanged)

   CS_SLOT_1(Private, void centralConicalChanged(const QPointF &un_named_arg1))
   CS_SLOT_2(centralConicalChanged)

   CS_SLOT_1(Private, void angleConicalChanged(qreal un_named_arg1))
   CS_SLOT_2(angleConicalChanged)
};

#endif
