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

#ifndef GRADIENTUTILS_H
#define GRADIENTUTILS_H

#include <QGradient>
#include <QPainter>

class QtGradientManager;

class QtGradientUtils
{
 public:
   static QString styleSheetCode(const QGradient &gradient);
   // utils methods, they could be outside of this class
   static QString saveState(const QtGradientManager *manager);
   static void restoreState(QtGradientManager *manager, const QString &state);

   static QPixmap gradientPixmap(const QGradient &gradient, const QSize &size = QSize(64, 64), bool checkeredBackground = false);
};

#endif
