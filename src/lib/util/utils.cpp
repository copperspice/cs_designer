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

#include <QMessageBox>
#include <QObject>
#include <QWidget>

bool compareMetaObject(const QMetaObject &a, const QMetaObject &b)
{
   bool retval = false;

   if (a.className() == b.className()) {
      retval = true;
   }

   return retval;
}

void csMsg(QWidget *parent, const QString &title, const QString &msg)
{
   QMessageBox msgB(parent);

   if (parent != nullptr) {
      msgB.setWindowModality(Qt::WindowModal);
   }

   msgB.setWindowTitle(title);
   msgB.setWindowIcon(QIcon(":/resources/designer.png"));

   msgB.setText(msg);
   msgB.exec();
}

void csError(QString title, QString msg)
{
   QMessageBox msgB;

   msgB.setWindowTitle(title);
   msgB.setWindowIcon(QIcon(":/resources/designer.png"));

   msgB.setIcon(QMessageBox::Warning);
   msgB.setText(msg);
   msgB.exec();
}

void csWarning(const QString &msg)
{
   qWarning("CS Designer (Warning) : %s", csPrintable(msg));
}

