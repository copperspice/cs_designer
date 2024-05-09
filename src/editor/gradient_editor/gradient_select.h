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

#ifndef GRADIENTVIEWDIALOG_H
#define GRADIENTVIEWDIALOG_H

#include <ui_gradient_select.h>

#include <QMap>
#include <QWidget>

class QtGradientManager;

class QtGradientSelect : public QDialog
{
   CS_OBJECT(QtGradientSelect)

 public:
   QtGradientSelect(QWidget *parent = nullptr);

   void setGradientManager(QtGradientManager *manager);
   QtGradientManager *gradientManager() const;

   static QGradient getGradient(bool *ok, QtGradientManager *manager, QWidget *parent = nullptr,
      const QString &caption = tr("Select Gradient", nullptr));

 private:
   CS_SLOT_1(Private, void slotGradientSelected(const QString &id))
   CS_SLOT_2(slotGradientSelected)

   CS_SLOT_1(Private, void slotGradientActivated(const QString &id))
   CS_SLOT_2(slotGradientActivated)

   Ui::QtGradientSelect m_ui;
};

#endif

