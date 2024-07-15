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

#ifndef QTGRADIENTDIALOG_H
#define QTGRADIENTDIALOG_H

#include <QDialog>

class QtGradientDialog : public QDialog
{
   CS_OBJECT(QtGradientDialog)

   CS_PROPERTY_READ(gradient, gradient)
   CS_PROPERTY_WRITE(gradient, setGradient)

   CS_PROPERTY_READ(backgroundCheckered, isBackgroundCheckered)
   CS_PROPERTY_WRITE(backgroundCheckered, setBackgroundCheckered)

   CS_PROPERTY_READ(detailsVisible, detailsVisible)
   CS_PROPERTY_WRITE(detailsVisible, setDetailsVisible)

   CS_PROPERTY_READ(detailsButtonVisible, isDetailsButtonVisible)
   CS_PROPERTY_WRITE(detailsButtonVisible, setDetailsButtonVisible)

 public:
   QtGradientDialog(QWidget *parent = nullptr);
   ~QtGradientDialog();

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

   static QGradient getGradient(bool *ok, const QGradient &initial, QWidget *parent = nullptr, const QString &caption = QString());
   static QGradient getGradient(bool *ok, QWidget *parent = nullptr, const QString &caption = QString());

 private:
   QScopedPointer<class QtGradientDialogPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtGradientDialog)
   Q_DISABLE_COPY(QtGradientDialog)

   CS_SLOT_1(Private, void slotAboutToShowDetails(bool details, int extensionWidthHint))
   CS_SLOT_2(slotAboutToShowDetails)
};

#endif
