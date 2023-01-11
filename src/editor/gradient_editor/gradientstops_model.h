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

#ifndef QTGRADIENTSTOPSMODEL_H
#define QTGRADIENTSTOPSMODEL_H

class QtGradientStopsModel;

#include <QObject>
#include <QMap>

class QColor;

class QtGradientStop
{
 public:
   qreal position() const;
   QColor color() const;
   QtGradientStopsModel *gradientModel() const;

 private:
   QtGradientStop(QtGradientStopsModel *model = nullptr);
   ~QtGradientStop();

   void setColor(const QColor &color);
   void setPosition(qreal position);
   friend class QtGradientStopsModel;
   QScopedPointer<class QtGradientStopPrivate> d_ptr;
};

class QtGradientStopsModel : public QObject
{
   CS_OBJECT(QtGradientStopsModel)

 public:
   typedef QMap<qreal, QtGradientStop *> PositionStopMap;

   QtGradientStopsModel(QObject *parent = nullptr);
   ~QtGradientStopsModel();

   PositionStopMap stops() const;
   QtGradientStop *at(qreal pos) const;
   QColor color(qreal pos) const; // calculated between points
   QList<QtGradientStop *> selectedStops() const;
   QtGradientStop *currentStop() const;
   bool isSelected(QtGradientStop *stop) const;
   QtGradientStop *firstSelected() const;
   QtGradientStop *lastSelected() const;
   QtGradientStopsModel *clone() const;

   QtGradientStop *addStop(qreal pos, const QColor &color);
   void removeStop(QtGradientStop *stop);
   void moveStop(QtGradientStop *stop, qreal newPos);
   void swapStops(QtGradientStop *stop1, QtGradientStop *stop2);
   void changeStop(QtGradientStop *stop, const QColor &newColor);
   void selectStop(QtGradientStop *stop, bool select);
   void setCurrentStop(QtGradientStop *stop);

   void moveStops(double newPosition); // moves current stop to newPos and all selected stops are moved accordingly
   void clear();
   void clearSelection();
   void flipAll();
   void selectAll();
   void deleteStops();

   CS_SIGNAL_1(Public, void stopAdded(QtGradientStop *stop))
   CS_SIGNAL_2(stopAdded, stop)

   CS_SIGNAL_1(Public, void stopRemoved(QtGradientStop *stop))
   CS_SIGNAL_2(stopRemoved, stop)

   CS_SIGNAL_1(Public, void stopMoved(QtGradientStop *stop, qreal newPos))
   CS_SIGNAL_2(stopMoved, stop, newPos)

   CS_SIGNAL_1(Public, void stopsSwapped(QtGradientStop *stop1, QtGradientStop *stop2))
   CS_SIGNAL_2(stopsSwapped, stop1, stop2)

   CS_SIGNAL_1(Public, void stopChanged(QtGradientStop *stop, const QColor &newColor))
   CS_SIGNAL_2(stopChanged, stop, newColor)

   CS_SIGNAL_1(Public, void stopSelected(QtGradientStop *stop, bool selected))
   CS_SIGNAL_2(stopSelected, stop, selected)

   CS_SIGNAL_1(Public, void currentStopChanged(QtGradientStop *stop))
   CS_SIGNAL_2(currentStopChanged, stop)

 private:
   QScopedPointer<class QtGradientStopsModelPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtGradientStopsModel)
   Q_DISABLE_COPY(QtGradientStopsModel)
};

#endif
