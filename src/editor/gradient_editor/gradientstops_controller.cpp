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

#include <gradientstops_controller.h>
#include <gradientstops_model.h>
#include <ui_gradient_editor.h>

#include <QTimer>

class QtGradientStopsControllerPrivate
{
   QtGradientStopsController *q_ptr;
   Q_DECLARE_PUBLIC(QtGradientStopsController)

 public:
   typedef QMap<qreal, QColor> PositionColorMap;
   typedef QMap<qreal, QtGradientStop *> PositionStopMap;

   void slotHsvClicked();
   void slotRgbClicked();

   void slotCurrentStopChanged(QtGradientStop *stop);
   void slotStopMoved(QtGradientStop *stop, qreal newPos);
   void slotStopsSwapped(QtGradientStop *stop1, QtGradientStop *stop2);
   void slotStopChanged(QtGradientStop *stop, const QColor &newColor);
   void slotStopSelected(QtGradientStop *stop, bool selected);
   void slotStopAdded(QtGradientStop *stop);
   void slotStopRemoved(QtGradientStop *stop);
   void slotUpdatePositionSpinBox();

   void slotChangeColor(const QColor &color);
   void slotChangeHue(const QColor &color);
   void slotChangeSaturation(const QColor &color);
   void slotChangeValue(const QColor &color);
   void slotChangeAlpha(const QColor &color);
   void slotChangeHue(int color);
   void slotChangeSaturation(int color);
   void slotChangeValue(int color);
   void slotChangeAlpha(int color);
   void slotChangePosition(double value);

   void slotChangeZoom(int value);
   void slotZoomIn();
   void slotZoomOut();
   void slotZoomAll();
   void slotZoomChanged(double zoom);

   void enableCurrent(bool enable);
   void setColorSpinBoxes(const QColor &color);
   PositionColorMap stopsData(const PositionStopMap &stops) const;
   QVector<QPair<qreal, QColor>> makeGradientStops(const PositionColorMap &data) const;
   void updateZoom(double zoom);

   QtGradientStopsModel *m_model;
   QColor::Spec m_spec;

   Ui::QtGradientEditor *m_ui;
};

void QtGradientStopsControllerPrivate::enableCurrent(bool enable)
{
   m_ui->positionLabel->setEnabled(enable);
   m_ui->colorLabel->setEnabled(enable);
   m_ui->hLabel->setEnabled(enable);
   m_ui->sLabel->setEnabled(enable);
   m_ui->vLabel->setEnabled(enable);
   m_ui->aLabel->setEnabled(enable);
   m_ui->hueLabel->setEnabled(enable);
   m_ui->saturationLabel->setEnabled(enable);
   m_ui->valueLabel->setEnabled(enable);
   m_ui->alphaLabel->setEnabled(enable);

   m_ui->positionSpinBox->setEnabled(enable);
   m_ui->colorButton->setEnabled(enable);

   m_ui->hueColorLine->setEnabled(enable);
   m_ui->saturationColorLine->setEnabled(enable);
   m_ui->valueColorLine->setEnabled(enable);
   m_ui->alphaColorLine->setEnabled(enable);

   m_ui->hueSpinBox->setEnabled(enable);
   m_ui->saturationSpinBox->setEnabled(enable);
   m_ui->valueSpinBox->setEnabled(enable);
   m_ui->alphaSpinBox->setEnabled(enable);
}

QtGradientStopsControllerPrivate::PositionColorMap QtGradientStopsControllerPrivate::stopsData(const PositionStopMap &stops) const
{
   PositionColorMap data;
   auto itStop = stops.constBegin();

   while (itStop != stops.constEnd()) {
      QtGradientStop *stop = itStop.value();
      data[stop->position()] = stop->color();

      ++itStop;
   }
   return data;
}

QVector<QPair<qreal, QColor>> QtGradientStopsControllerPrivate::makeGradientStops(const PositionColorMap &data) const
{
   QVector<QPair<qreal, QColor>> stops;
   auto itData = data.constBegin();
   while (itData != data.constEnd()) {
      stops << QPair<qreal, QColor>(itData.key(), itData.value());

      ++itData;
   }
   return stops;
}

void QtGradientStopsControllerPrivate::updateZoom(double zoom)
{
   m_ui->gradientStopsWidget->setZoom(zoom);
   m_ui->zoomSpinBox->blockSignals(true);
   m_ui->zoomSpinBox->setValue(qRound(zoom * 100));
   m_ui->zoomSpinBox->blockSignals(false);

   bool zoomInEnabled = true;
   bool zoomOutEnabled = true;
   bool zoomAllEnabled = true;

   if (zoom <= 1) {
      zoomAllEnabled = false;
      zoomOutEnabled = false;
   } else if (zoom >= 100) {
      zoomInEnabled = false;
   }

   m_ui->zoomInButton->setEnabled(zoomInEnabled);
   m_ui->zoomOutButton->setEnabled(zoomOutEnabled);
   m_ui->zoomAllButton->setEnabled(zoomAllEnabled);
}

void QtGradientStopsControllerPrivate::slotHsvClicked()
{
   QString h = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "H");
   QString s = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "S");
   QString v = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "V");

   m_ui->hLabel->setText(h);
   m_ui->sLabel->setText(s);
   m_ui->vLabel->setText(v);

   h = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Hue");
   s = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Sat");
   v = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Val");

   const QString hue = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Hue");
   const QString saturation = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Saturation");
   const QString value = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Value");

   m_ui->hLabel->setToolTip(hue);
   m_ui->hueLabel->setText(h);
   m_ui->hueColorLine->setToolTip(hue);
   m_ui->hueColorLine->setColorComponent(QtColorLine::Hue);

   m_ui->sLabel->setToolTip(saturation);
   m_ui->saturationLabel->setText(s);
   m_ui->saturationColorLine->setToolTip(saturation);
   m_ui->saturationColorLine->setColorComponent(QtColorLine::Saturation);

   m_ui->vLabel->setToolTip(value);
   m_ui->valueLabel->setText(v);
   m_ui->valueColorLine->setToolTip(value);
   m_ui->valueColorLine->setColorComponent(QtColorLine::Value);

   setColorSpinBoxes(m_ui->colorButton->color());
}

void QtGradientStopsControllerPrivate::slotRgbClicked()
{
   QString r = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "R");
   QString g = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "G");
   QString b = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "B");

   m_ui->hLabel->setText(r);
   m_ui->sLabel->setText(g);
   m_ui->vLabel->setText(b);

   QString red = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Red");
   QString green = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Green");
   QString blue = QCoreApplication::translate("qdesigner_internal::QtGradientStopsController", "Blue");

   m_ui->hLabel->setToolTip(red);
   m_ui->hueLabel->setText(red);
   m_ui->hueColorLine->setToolTip(red);
   m_ui->hueColorLine->setColorComponent(QtColorLine::Red);

   m_ui->sLabel->setToolTip(green);
   m_ui->saturationLabel->setText(green);
   m_ui->saturationColorLine->setToolTip(green);
   m_ui->saturationColorLine->setColorComponent(QtColorLine::Green);

   m_ui->vLabel->setToolTip(blue);
   m_ui->valueLabel->setText(blue);
   m_ui->valueColorLine->setToolTip(blue);
   m_ui->valueColorLine->setColorComponent(QtColorLine::Blue);

   setColorSpinBoxes(m_ui->colorButton->color());
}

void QtGradientStopsControllerPrivate::setColorSpinBoxes(const QColor &color)
{
   m_ui->hueSpinBox->blockSignals(true);
   m_ui->saturationSpinBox->blockSignals(true);
   m_ui->valueSpinBox->blockSignals(true);
   m_ui->alphaSpinBox->blockSignals(true);

   if (m_ui->hsvRadioButton->isChecked()) {
      if (m_ui->hueSpinBox->maximum() != 359) {
         m_ui->hueSpinBox->setMaximum(359);
      }

      if (m_ui->hueSpinBox->value() != color.hue()) {
         m_ui->hueSpinBox->setValue(color.hue());
      }

      if (m_ui->saturationSpinBox->value() != color.saturation()) {
         m_ui->saturationSpinBox->setValue(color.saturation());
      }

      if (m_ui->valueSpinBox->value() != color.value()) {
         m_ui->valueSpinBox->setValue(color.value());
      }

   } else {
      if (m_ui->hueSpinBox->maximum() != 255) {
         m_ui->hueSpinBox->setMaximum(255);
      }

      if (m_ui->hueSpinBox->value() != color.red()) {
         m_ui->hueSpinBox->setValue(color.red());
      }

      if (m_ui->saturationSpinBox->value() != color.green()) {
         m_ui->saturationSpinBox->setValue(color.green());
      }

      if (m_ui->valueSpinBox->value() != color.blue()) {
         m_ui->valueSpinBox->setValue(color.blue());
      }
   }

   m_ui->alphaSpinBox->setValue(color.alpha());
   m_ui->hueSpinBox->blockSignals(false);
   m_ui->saturationSpinBox->blockSignals(false);
   m_ui->valueSpinBox->blockSignals(false);
   m_ui->alphaSpinBox->blockSignals(false);
}

void QtGradientStopsControllerPrivate::slotCurrentStopChanged(QtGradientStop *stop)
{
   if (!stop) {
      enableCurrent(false);
      return;
   }
   enableCurrent(true);

   QTimer::singleShot(0, q_ptr, SLOT(slotUpdatePositionSpinBox()));

   m_ui->colorButton->setColor(stop->color());
   m_ui->hueColorLine->setColor(stop->color());
   m_ui->saturationColorLine->setColor(stop->color());
   m_ui->valueColorLine->setColor(stop->color());
   m_ui->alphaColorLine->setColor(stop->color());
   setColorSpinBoxes(stop->color());
}

void QtGradientStopsControllerPrivate::slotStopMoved(QtGradientStop *stop, qreal newPos)
{
   QTimer::singleShot(0, q_ptr, SLOT(slotUpdatePositionSpinBox()));

   PositionColorMap stops = stopsData(m_model->stops());
   stops.remove(stop->position());
   stops[newPos] = stop->color();

   QVector<QPair<qreal, QColor>> gradStops = makeGradientStops(stops);
   emit q_ptr->gradientStopsChanged(gradStops);
}

void QtGradientStopsControllerPrivate::slotStopsSwapped(QtGradientStop *stop1, QtGradientStop *stop2)
{
   QTimer::singleShot(0, q_ptr, SLOT(slotUpdatePositionSpinBox()));

   PositionColorMap stops = stopsData(m_model->stops());
   const qreal pos1 = stop1->position();
   const qreal pos2 = stop2->position();
   stops[pos1] = stop2->color();
   stops[pos2] = stop1->color();

   QVector<QPair<qreal, QColor>> gradStops = makeGradientStops(stops);
   emit q_ptr->gradientStopsChanged(gradStops);
}

void QtGradientStopsControllerPrivate::slotStopAdded(QtGradientStop *stop)
{
   PositionColorMap stops = stopsData(m_model->stops());
   stops[stop->position()] = stop->color();

   QVector<QPair<qreal, QColor>> gradStops = makeGradientStops(stops);
   emit q_ptr->gradientStopsChanged(gradStops);
}

void QtGradientStopsControllerPrivate::slotStopRemoved(QtGradientStop *stop)
{
   PositionColorMap stops = stopsData(m_model->stops());
   stops.remove(stop->position());

   QVector<QPair<qreal, QColor>> gradStops = makeGradientStops(stops);
   emit q_ptr->gradientStopsChanged(gradStops);
}

void QtGradientStopsControllerPrivate::slotStopChanged(QtGradientStop *stop, const QColor &newColor)
{
   if (m_model->currentStop() == stop) {
      m_ui->colorButton->setColor(newColor);
      m_ui->hueColorLine->setColor(newColor);
      m_ui->saturationColorLine->setColor(newColor);
      m_ui->valueColorLine->setColor(newColor);
      m_ui->alphaColorLine->setColor(newColor);
      setColorSpinBoxes(newColor);
   }

   PositionColorMap stops = stopsData(m_model->stops());
   stops[stop->position()] = newColor;

   QVector<QPair<qreal, QColor>> gradStops = makeGradientStops(stops);
   emit q_ptr->gradientStopsChanged(gradStops);
}

void QtGradientStopsControllerPrivate::slotStopSelected(QtGradientStop *stop, bool selected)
{
   Q_UNUSED(stop)
   Q_UNUSED(selected)
   QTimer::singleShot(0, q_ptr, SLOT(slotUpdatePositionSpinBox()));
}

void QtGradientStopsControllerPrivate::slotUpdatePositionSpinBox()
{
   QtGradientStop *current = m_model->currentStop();
   if (!current) {
      return;
   }

   qreal min = 0.0;
   qreal max = 1.0;
   const qreal pos = current->position();

   QtGradientStop *first = m_model->firstSelected();
   QtGradientStop *last = m_model->lastSelected();

   if (first && last) {
      const qreal minPos = pos - first->position() - 0.0004999;
      const qreal maxPos = pos + 1.0 - last->position() + 0.0004999;

      if (max > maxPos) {
         max = maxPos;
      }
      if (min < minPos) {
         min = minPos;
      }

      if (first->position() == 0.0) {
         min = pos;
      }
      if (last->position() == 1.0) {
         max = pos;
      }
   }

   const int spinMin = qRound(m_ui->positionSpinBox->minimum() * 1000);
   const int spinMax = qRound(m_ui->positionSpinBox->maximum() * 1000);

   const int newMin = qRound(min * 1000);
   const int newMax = qRound(max * 1000);

   m_ui->positionSpinBox->blockSignals(true);
   if (spinMin != newMin || spinMax != newMax) {
      m_ui->positionSpinBox->setRange((double)newMin / 1000, (double)newMax / 1000);
   }
   if (m_ui->positionSpinBox->value() != pos) {
      m_ui->positionSpinBox->setValue(pos);
   }
   m_ui->positionSpinBox->blockSignals(false);
}

void QtGradientStopsControllerPrivate::slotChangeColor(const QColor &color)
{
   QtGradientStop *stop = m_model->currentStop();
   if (!stop) {
      return;
   }
   m_model->changeStop(stop, color);
   QList<QtGradientStop *> stops = m_model->selectedStops();
   QListIterator<QtGradientStop *> itStop(stops);
   while (itStop.hasNext()) {
      QtGradientStop *s = itStop.next();
      if (s != stop) {
         m_model->changeStop(s, color);
      }
   }
}

void QtGradientStopsControllerPrivate::slotChangeHue(const QColor &color)
{
   QtGradientStop *stop = m_model->currentStop();
   if (!stop) {
      return;
   }
   m_model->changeStop(stop, color);
   QList<QtGradientStop *> stops = m_model->selectedStops();
   QListIterator<QtGradientStop *> itStop(stops);
   while (itStop.hasNext()) {
      QtGradientStop *s = itStop.next();
      if (s != stop) {
         QColor c = s->color();
         if (m_ui->hsvRadioButton->isChecked()) {
            c.setHsvF(color.hueF(), c.saturationF(), c.valueF(), c.alphaF());
         } else {
            c.setRgbF(color.redF(), c.greenF(), c.blueF(), c.alphaF());
         }
         m_model->changeStop(s, c);
      }
   }
}

void QtGradientStopsControllerPrivate::slotChangeHue(int color)
{
   QColor c = m_ui->hueColorLine->color();
   if (m_ui->hsvRadioButton->isChecked()) {
      c.setHsvF((qreal)color / 360.0, c.saturationF(), c.valueF(), c.alphaF());
   } else {
      c.setRed(color);
   }
   slotChangeHue(c);
}

void QtGradientStopsControllerPrivate::slotChangeSaturation(const QColor &color)
{
   QtGradientStop *stop = m_model->currentStop();
   if (!stop) {
      return;
   }
   m_model->changeStop(stop, color);
   QList<QtGradientStop *> stops = m_model->selectedStops();
   QListIterator<QtGradientStop *> itStop(stops);
   while (itStop.hasNext()) {
      QtGradientStop *s = itStop.next();
      if (s != stop) {
         QColor c = s->color();
         if (m_ui->hsvRadioButton->isChecked()) {
            c.setHsvF(c.hueF(), color.saturationF(), c.valueF(), c.alphaF());
            int hue = c.hue();
            if (hue == 360 || hue == -1) {
               c.setHsvF(0.0, c.saturationF(), c.valueF(), c.alphaF());
            }
         } else {
            c.setRgbF(c.redF(), color.greenF(), c.blueF(), c.alphaF());
         }
         m_model->changeStop(s, c);
      }
   }
}

void QtGradientStopsControllerPrivate::slotChangeSaturation(int color)
{
   QColor c = m_ui->saturationColorLine->color();
   if (m_ui->hsvRadioButton->isChecked()) {
      c.setHsvF(c.hueF(), (qreal)color / 255, c.valueF(), c.alphaF());
   } else {
      c.setGreen(color);
   }
   slotChangeSaturation(c);
}

void QtGradientStopsControllerPrivate::slotChangeValue(const QColor &color)
{
   QtGradientStop *stop = m_model->currentStop();
   if (!stop) {
      return;
   }
   m_model->changeStop(stop, color);
   QList<QtGradientStop *> stops = m_model->selectedStops();
   QListIterator<QtGradientStop *> itStop(stops);
   while (itStop.hasNext()) {
      QtGradientStop *s = itStop.next();
      if (s != stop) {
         QColor c = s->color();
         if (m_ui->hsvRadioButton->isChecked()) {
            c.setHsvF(c.hueF(), c.saturationF(), color.valueF(), c.alphaF());
            int hue = c.hue();
            if (hue == 360 || hue == -1) {
               c.setHsvF(0.0, c.saturationF(), c.valueF(), c.alphaF());
            }
         } else {
            c.setRgbF(c.redF(), c.greenF(), color.blueF(), c.alphaF());
         }
         m_model->changeStop(s, c);
      }
   }
}

void QtGradientStopsControllerPrivate::slotChangeValue(int color)
{
   QColor c = m_ui->valueColorLine->color();
   if (m_ui->hsvRadioButton->isChecked()) {
      c.setHsvF(c.hueF(), c.saturationF(), (qreal)color / 255, c.alphaF());
   } else {
      c.setBlue(color);
   }
   slotChangeValue(c);
}

void QtGradientStopsControllerPrivate::slotChangeAlpha(const QColor &color)
{
   QtGradientStop *stop = m_model->currentStop();
   if (!stop) {
      return;
   }
   m_model->changeStop(stop, color);
   QList<QtGradientStop *> stops = m_model->selectedStops();
   QListIterator<QtGradientStop *> itStop(stops);
   while (itStop.hasNext()) {
      QtGradientStop *s = itStop.next();
      if (s != stop) {
         QColor c = s->color();
         if (m_ui->hsvRadioButton->isChecked()) {
            c.setHsvF(c.hueF(), c.saturationF(), c.valueF(), color.alphaF());
            int hue = c.hue();
            if (hue == 360 || hue == -1) {
               c.setHsvF(0.0, c.saturationF(), c.valueF(), c.alphaF());
            }
         } else {
            c.setRgbF(c.redF(), c.greenF(), c.blueF(), color.alphaF());
         }
         m_model->changeStop(s, c);
      }
   }
}

void QtGradientStopsControllerPrivate::slotChangeAlpha(int color)
{
   QColor c = m_ui->alphaColorLine->color();
   if (m_ui->hsvRadioButton->isChecked()) {
      c.setHsvF(c.hueF(), c.saturationF(), c.valueF(), (qreal)color / 255);
   } else {
      c.setAlpha(color);
   }
   slotChangeAlpha(c);
}

void QtGradientStopsControllerPrivate::slotChangePosition(double value)
{
   QtGradientStop *stop = m_model->currentStop();
   if (!stop) {
      return;
   }

   m_model->moveStops(value);
}

void QtGradientStopsControllerPrivate::slotChangeZoom(int value)
{
   updateZoom(value / 100.0);
}

void QtGradientStopsControllerPrivate::slotZoomIn()
{
   double newZoom = m_ui->gradientStopsWidget->zoom() * 2;
   if (newZoom > 100) {
      newZoom = 100;
   }
   updateZoom(newZoom);
}

void QtGradientStopsControllerPrivate::slotZoomOut()
{
   double newZoom = m_ui->gradientStopsWidget->zoom() / 2;
   if (newZoom < 1) {
      newZoom = 1;
   }
   updateZoom(newZoom);
}

void QtGradientStopsControllerPrivate::slotZoomAll()
{
   updateZoom(1);
}

void QtGradientStopsControllerPrivate::slotZoomChanged(double zoom)
{
   updateZoom(zoom);
}

QtGradientStopsController::QtGradientStopsController(QObject *parent)
   : QObject(parent), d_ptr(new QtGradientStopsControllerPrivate())
{
   d_ptr->q_ptr = this;

   d_ptr->m_spec = QColor::Hsv;
}

void QtGradientStopsController::setUi(Ui::QtGradientEditor *ui)
{
   d_ptr->m_ui = ui;

   d_ptr->m_ui->hueColorLine->setColorComponent(QtColorLine::Hue);
   d_ptr->m_ui->saturationColorLine->setColorComponent(QtColorLine::Saturation);
   d_ptr->m_ui->valueColorLine->setColorComponent(QtColorLine::Value);
   d_ptr->m_ui->alphaColorLine->setColorComponent(QtColorLine::Alpha);

   d_ptr->m_model = new QtGradientStopsModel(this);
   d_ptr->m_ui->gradientStopsWidget->setGradientStopsModel(d_ptr->m_model);
   connect(d_ptr->m_model, SIGNAL(currentStopChanged(QtGradientStop *)),
      this, SLOT(slotCurrentStopChanged(QtGradientStop *)));
   connect(d_ptr->m_model, SIGNAL(stopMoved(QtGradientStop *, qreal)),
      this, SLOT(slotStopMoved(QtGradientStop *, qreal)));
   connect(d_ptr->m_model, SIGNAL(stopsSwapped(QtGradientStop *, QtGradientStop *)),
      this, SLOT(slotStopsSwapped(QtGradientStop *, QtGradientStop *)));
   connect(d_ptr->m_model, SIGNAL(stopChanged(QtGradientStop *, QColor)),
      this, SLOT(slotStopChanged(QtGradientStop *, QColor)));
   connect(d_ptr->m_model, SIGNAL(stopSelected(QtGradientStop *, bool)),
      this, SLOT(slotStopSelected(QtGradientStop *, bool)));
   connect(d_ptr->m_model, SIGNAL(stopAdded(QtGradientStop *)),
      this, SLOT(slotStopAdded(QtGradientStop *)));
   connect(d_ptr->m_model, SIGNAL(stopRemoved(QtGradientStop *)),
      this, SLOT(slotStopRemoved(QtGradientStop *)));

   connect(d_ptr->m_ui->hueColorLine, SIGNAL(colorChanged(QColor)),
      this, SLOT(slotChangeHue(QColor)));
   connect(d_ptr->m_ui->saturationColorLine, SIGNAL(colorChanged(QColor)),
      this, SLOT(slotChangeSaturation(QColor)));
   connect(d_ptr->m_ui->valueColorLine, SIGNAL(colorChanged(QColor)),
      this, SLOT(slotChangeValue(QColor)));
   connect(d_ptr->m_ui->alphaColorLine, SIGNAL(colorChanged(QColor)),
      this, SLOT(slotChangeAlpha(QColor)));
   connect(d_ptr->m_ui->colorButton, SIGNAL(colorChanged(QColor)),
      this, SLOT(slotChangeColor(QColor)));

   connect(d_ptr->m_ui->hueSpinBox, SIGNAL(valueChanged(int)),
      this, SLOT(slotChangeHue(int)));
   connect(d_ptr->m_ui->saturationSpinBox, SIGNAL(valueChanged(int)),
      this, SLOT(slotChangeSaturation(int)));
   connect(d_ptr->m_ui->valueSpinBox, SIGNAL(valueChanged(int)),
      this, SLOT(slotChangeValue(int)));
   connect(d_ptr->m_ui->alphaSpinBox, SIGNAL(valueChanged(int)),
      this, SLOT(slotChangeAlpha(int)));

   connect(d_ptr->m_ui->positionSpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(slotChangePosition(double)));

   connect(d_ptr->m_ui->zoomSpinBox, SIGNAL(valueChanged(int)),
      this, SLOT(slotChangeZoom(int)));
   connect(d_ptr->m_ui->zoomInButton, SIGNAL(clicked()),
      this, SLOT(slotZoomIn()));
   connect(d_ptr->m_ui->zoomOutButton, SIGNAL(clicked()),
      this, SLOT(slotZoomOut()));
   connect(d_ptr->m_ui->zoomAllButton, SIGNAL(clicked()),
      this, SLOT(slotZoomAll()));
   connect(d_ptr->m_ui->gradientStopsWidget, SIGNAL(zoomChanged(double)),
      this, SLOT(slotZoomChanged(double)));

   connect(d_ptr->m_ui->hsvRadioButton, SIGNAL(clicked()),
      this, SLOT(slotHsvClicked()));
   connect(d_ptr->m_ui->rgbRadioButton, SIGNAL(clicked()),
      this, SLOT(slotRgbClicked()));

   d_ptr->enableCurrent(false);
   d_ptr->m_ui->zoomInButton->setIcon(QIcon(":/resources/gradient_editor/images/zoomin.png"));
   d_ptr->m_ui->zoomOutButton->setIcon(QIcon(":/resources/gradient_editor/images/zoomout.png"));
   d_ptr->updateZoom(1);
}

QtGradientStopsController::~QtGradientStopsController()
{
}

void QtGradientStopsController::setGradientStops(const QVector<QPair<qreal, QColor>> &stops)
{
   d_ptr->m_model->clear();

   QVectorIterator<QPair<qreal, QColor>> it(stops);
   QtGradientStop *first = nullptr;

   while (it.hasNext()) {
      QPair<qreal, QColor> pair = it.next();
      QtGradientStop *stop = d_ptr->m_model->addStop(pair.first, pair.second);
      if (!first) {
         first = stop;
      }
   }

   if (first) {
      d_ptr->m_model->setCurrentStop(first);
   }
}

QVector<QPair<qreal, QColor>> QtGradientStopsController::gradientStops() const
{
   QVector<QPair<qreal, QColor>> stops;
   QList<QtGradientStop *> stopsList = d_ptr->m_model->stops().values();
   QListIterator<QtGradientStop *> itStop(stopsList);
   while (itStop.hasNext()) {
      QtGradientStop *stop = itStop.next();
      stops << QPair<qreal, QColor>(stop->position(), stop->color());
   }
   return stops;
}

QColor::Spec QtGradientStopsController::spec() const
{
   return d_ptr->m_spec;
}

void QtGradientStopsController::setSpec(QColor::Spec spec)
{
   if (d_ptr->m_spec == spec) {
      return;
   }

   d_ptr->m_spec = spec;
   if (d_ptr->m_spec == QColor::Rgb) {
      d_ptr->m_ui->rgbRadioButton->setChecked(true);
      d_ptr->slotRgbClicked();
   } else {
      d_ptr->m_ui->hsvRadioButton->setChecked(true);
      d_ptr->slotHsvClicked();
   }
}

void QtGradientStopsController::slotHsvClicked()
{
   Q_D(QtGradientStopsController);
   d->slotHsvClicked();
}

void QtGradientStopsController::slotRgbClicked()
{
   Q_D(QtGradientStopsController);
   d->slotRgbClicked();
}

void QtGradientStopsController::slotCurrentStopChanged(QtGradientStop *stop)
{
   Q_D(QtGradientStopsController);
   d->slotCurrentStopChanged(stop);
}

void QtGradientStopsController::slotStopMoved(QtGradientStop *stop, qreal newPos)
{
   Q_D(QtGradientStopsController);
   d->slotStopMoved(stop, newPos);
}

void QtGradientStopsController::slotStopsSwapped(QtGradientStop *stop1, QtGradientStop *stop2)
{
   Q_D(QtGradientStopsController);
   d->slotStopsSwapped(stop1, stop2);
}

void QtGradientStopsController::slotStopChanged(QtGradientStop *stop, const QColor &newColor)
{
   Q_D(QtGradientStopsController);
   d->slotStopChanged(stop, newColor);
}

void QtGradientStopsController::slotStopSelected(QtGradientStop *stop, bool selected)
{
   Q_D(QtGradientStopsController);
   d->slotStopSelected(stop, selected);
}

void QtGradientStopsController::slotStopAdded(QtGradientStop *stop)
{
   Q_D(QtGradientStopsController);
   d->slotStopAdded(stop);
}

void QtGradientStopsController::slotStopRemoved(QtGradientStop *stop)
{
   Q_D(QtGradientStopsController);
   d->slotStopRemoved(stop);
}

void QtGradientStopsController::slotUpdatePositionSpinBox()
{
   Q_D(QtGradientStopsController);
   d->slotUpdatePositionSpinBox();
}

void QtGradientStopsController::slotChangeColor(const QColor &color)
{
   Q_D(QtGradientStopsController);
   d->slotChangeColor(color);
}

void QtGradientStopsController::slotChangeHue(const QColor &color)
{
   Q_D(QtGradientStopsController);
   d->slotChangeHue(color);
}

void QtGradientStopsController::slotChangeSaturation(const QColor &color)
{
   Q_D(QtGradientStopsController);
   d->slotChangeSaturation(color);
}

void QtGradientStopsController::slotChangeValue(const QColor &color)
{
   Q_D(QtGradientStopsController);
   d->slotChangeValue(color);
}

void QtGradientStopsController::slotChangeAlpha(const QColor &color)
{
   Q_D(QtGradientStopsController);
   d->slotChangeAlpha(color);
}

void QtGradientStopsController::slotChangeHue(int un_named_arg1)
{
   Q_D(QtGradientStopsController);
   d->slotChangeHue(un_named_arg1);
}

void QtGradientStopsController::slotChangeSaturation(int un_named_arg1)
{
   Q_D(QtGradientStopsController);
   d->slotChangeSaturation(un_named_arg1);
}

void QtGradientStopsController::slotChangeValue(int un_named_arg1)
{
   Q_D(QtGradientStopsController);
   d->slotChangeValue(un_named_arg1);
}

void QtGradientStopsController::slotChangeAlpha(int un_named_arg1)
{
   Q_D(QtGradientStopsController);
   d->slotChangeAlpha(un_named_arg1);
}

void QtGradientStopsController::slotChangePosition(double value)
{
   Q_D(QtGradientStopsController);
   d->slotChangePosition(value);
}

void QtGradientStopsController::slotChangeZoom(int value)
{
   Q_D(QtGradientStopsController);
   d->slotChangeZoom(value);
}

void QtGradientStopsController::slotZoomIn()
{
   Q_D(QtGradientStopsController);
   d->slotZoomIn();
}

void QtGradientStopsController::slotZoomOut()
{
   Q_D(QtGradientStopsController);
   d->slotZoomOut();
}

void QtGradientStopsController::slotZoomAll()
{
   Q_D(QtGradientStopsController);
   d->slotZoomAll();
}

void QtGradientStopsController::slotZoomChanged(double un_named_arg1)
{
   Q_D(QtGradientStopsController);
   d->slotZoomChanged(un_named_arg1);
}