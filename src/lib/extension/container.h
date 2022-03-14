/***********************************************************************
*
* Copyright (c) 2021-2022 Barbara Geller
* Copyright (c) 2021-2022 Ansel Sermersheim
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

#ifndef CONTAINER_H
#define CONTAINER_H

#include <extension.h>

#include <QObject>

class QWidget;

class QDesignerContainerExtension
{
 public:
   virtual ~QDesignerContainerExtension() {}

   virtual int count() const = 0;
   virtual QWidget *widget(int index) const = 0;

   virtual int currentIndex() const = 0;
   virtual void setCurrentIndex(int index) = 0;

   virtual void addWidget(QWidget *widget) = 0;
   virtual void insertWidget(int index, QWidget *widget) = 0;
   virtual void remove(int index) = 0;

   virtual bool canAddWidget() const = 0;
   virtual bool canRemove(int index) const = 0;
};

CS_DECLARE_EXTENSION_INTERFACE(QDesignerContainerExtension, "copperspice.com.CS.Designer.Container")

#endif
