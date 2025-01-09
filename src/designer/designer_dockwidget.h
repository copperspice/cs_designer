/***********************************************************************
*
* Copyright (c) 2021-2025 Barbara Geller
* Copyright (c) 2021-2025 Ansel Sermersheim
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

#ifndef QDESIGNER_DOCKWIDGET_H
#define QDESIGNER_DOCKWIDGET_H

#include <QDockWidget>

class QDesignerFormWindowInterface;

class QDesignerDockWidget: public QDockWidget
{
   CS_OBJECT(QDesignerDockWidget)

   CS_PROPERTY_READ(dockWidgetArea, dockWidgetArea)
   CS_PROPERTY_WRITE(dockWidgetArea, setDockWidgetArea)
   CS_PROPERTY_DESIGNABLE_NONSTATIC(dockWidgetArea, docked())
   CS_PROPERTY_STORED(dockWidgetArea, false)

   CS_PROPERTY_READ(docked, docked)
   CS_PROPERTY_WRITE(docked, setDocked)
   CS_PROPERTY_DESIGNABLE_NONSTATIC(docked, inMainWindow())
   CS_PROPERTY_STORED(docked, false)

 public:
   QDesignerDockWidget(QWidget *parent = nullptr);
   virtual ~QDesignerDockWidget();

   bool docked() const;
   void setDocked(bool b);

   Qt::DockWidgetArea dockWidgetArea() const;
   void setDockWidgetArea(Qt::DockWidgetArea dockWidgetArea);

   bool inMainWindow() const;

 private:
   QDesignerFormWindowInterface *formWindow() const;
   QMainWindow *findMainWindow() const;
};

#endif
