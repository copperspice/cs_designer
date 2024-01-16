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

#ifndef QTGRADIENTSTOPSWIDGET_H
#define QTGRADIENTSTOPSWIDGET_H

#include <gradientstops_model.h>

class QtGradientStopsWidgetPrivate;

#include <QAbstractScrollArea>

class QtGradientStopsWidget : public QAbstractScrollArea
{
   CS_OBJECT(QtGradientStopsWidget)

   CS_PROPERTY_READ(backgroundCheckered, isBackgroundCheckered)
   CS_PROPERTY_WRITE(backgroundCheckered, setBackgroundCheckered)

 public:
   QtGradientStopsWidget(QWidget *parent = nullptr);
   ~QtGradientStopsWidget();

   QSize minimumSizeHint() const override;
   QSize sizeHint() const override;

   bool isBackgroundCheckered() const;
   void setBackgroundCheckered(bool checkered);

   void setGradientStopsModel(QtGradientStopsModel *model);

   void setZoom(double zoom);
   double zoom() const;

   CS_SIGNAL_1(Public, void zoomChanged(double zoom))
   CS_SIGNAL_2(zoomChanged, zoom)

 protected:
   void paintEvent(QPaintEvent *e) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseReleaseEvent(QMouseEvent *e) override;
   void mouseMoveEvent(QMouseEvent *e) override;
   void mouseDoubleClickEvent(QMouseEvent *e) override;
   void keyPressEvent(QKeyEvent *e) override;
   void focusInEvent(QFocusEvent *e) override;
   void focusOutEvent(QFocusEvent *e) override;
   void contextMenuEvent(QContextMenuEvent *e) override;
   void wheelEvent(QWheelEvent *e) override;

   void dragEnterEvent(QDragEnterEvent *event) override;
   void dragMoveEvent(QDragMoveEvent *event) override;
   void dragLeaveEvent(QDragLeaveEvent *event) override;
   void dropEvent(QDropEvent *event) override;

 private:
   QScopedPointer<QtGradientStopsWidgetPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtGradientStopsWidget)
   Q_DISABLE_COPY(QtGradientStopsWidget)

   CS_SLOT_1(Private, void slotStopAdded(QtGradientStop *stop))
   CS_SLOT_2(slotStopAdded)

   CS_SLOT_1(Private, void slotStopRemoved(QtGradientStop *stop))
   CS_SLOT_2(slotStopRemoved)

   CS_SLOT_1(Private, void slotStopMoved(QtGradientStop *stop, qreal newPos))
   CS_SLOT_2(slotStopMoved)

   CS_SLOT_1(Private, void slotStopsSwapped(QtGradientStop *stop1, QtGradientStop *stop2))
   CS_SLOT_2(slotStopsSwapped)

   CS_SLOT_1(Private, void slotStopChanged(QtGradientStop *stop, const QColor &newColor))
   CS_SLOT_2(slotStopChanged)

   CS_SLOT_1(Private, void slotStopSelected(QtGradientStop *stop, bool selected))
   CS_SLOT_2(slotStopSelected)

   CS_SLOT_1(Private, void slotCurrentStopChanged(QtGradientStop *stop))
   CS_SLOT_2(slotCurrentStopChanged)

   CS_SLOT_1(Private, void slotNewStop())
   CS_SLOT_2(slotNewStop)

   CS_SLOT_1(Private, void slotDelete())
   CS_SLOT_2(slotDelete)

   CS_SLOT_1(Private, void slotFlipAll())
   CS_SLOT_2(slotFlipAll)

   CS_SLOT_1(Private, void slotSelectAll())
   CS_SLOT_2(slotSelectAll)

   CS_SLOT_1(Private, void slotZoomIn())
   CS_SLOT_2(slotZoomIn)

   CS_SLOT_1(Private, void slotZoomOut())
   CS_SLOT_2(slotZoomOut)

   CS_SLOT_1(Private, void slotResetZoom())
   CS_SLOT_2(slotResetZoom)
};

#endif
