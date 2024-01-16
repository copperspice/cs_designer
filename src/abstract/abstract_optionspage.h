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

#ifndef ABSTRACTOPTIONSPAGE_P_H
#define ABSTRACTOPTIONSPAGE_P_H

#include <QString>

class QWidget;

class QDesignerOptionsPageInterface
{
 public:
   virtual ~QDesignerOptionsPageInterface() {}
   virtual QString name() const = 0;
   virtual QWidget *createPage(QWidget *parent) = 0;
   virtual void apply() = 0;
   virtual void finish() = 0;
};


#endif