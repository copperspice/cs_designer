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

#ifndef ABSTRACT_FORMWINDOWTOOL_H
#define ABSTRACT_FORMWINDOWTOOL_H

#include <QObject>

class DomUI;
class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QAction;
class QWidget;

class QDesignerFormWindowToolInterface: public QObject
{
   CS_OBJECT(QDesignerFormWindowToolInterface)

 public:
   explicit QDesignerFormWindowToolInterface(QObject *parent = nullptr);
   virtual ~QDesignerFormWindowToolInterface();

   virtual QDesignerFormEditorInterface *core() const = 0;
   virtual QDesignerFormWindowInterface *formWindow() const = 0;
   virtual QWidget *editor() const = 0;

   virtual QAction *action() const = 0;

   virtual void activated() = 0;
   virtual void deactivated() = 0;

   virtual void saveToDom(DomUI *, QWidget *) {}
   virtual void loadFromDom(DomUI *, QWidget *) {}

   virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) = 0;
};



#endif // ABSTRACTFORMWINDOWTOOL_H
