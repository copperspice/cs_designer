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

#ifndef ABSTRACTRESOURCEBROWSER_H
#define ABSTRACTRESOURCEBROWSER_H

#include <QWidget>

class QDesignerResourceBrowserInterface: public QWidget
{
   CS_OBJECT(QDesignerResourceBrowserInterface)

 public:
   explicit QDesignerResourceBrowserInterface(QWidget *parent = nullptr);
   virtual ~QDesignerResourceBrowserInterface();

   virtual void setCurrentPath(const QString &filePath) = 0;
   virtual QString currentPath() const = 0;

   CS_SIGNAL_1(Public, void currentPathChanged(const QString &filePath))
   CS_SIGNAL_2(currentPathChanged, filePath)
   CS_SIGNAL_1(Public, void pathActivated(const QString &filePath))
   CS_SIGNAL_2(pathActivated, filePath)
};

#endif

