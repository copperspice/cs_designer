/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#ifndef ABSTRACTWIDGETFACTORY_H
#define ABSTRACTWIDGETFACTORY_H

#include <QObject>

class QDesignerFormEditorInterface;

class QWidget;
class QLayout;

class QDesignerWidgetFactoryInterface : public QObject
{
   CS_OBJECT(QDesignerWidgetFactoryInterface)

 public:
   explicit QDesignerWidgetFactoryInterface(QObject *parent = nullptr);
   virtual ~QDesignerWidgetFactoryInterface();

   virtual QDesignerFormEditorInterface *core() const = 0;

   virtual QWidget *containerOfWidget(QWidget *w) const = 0;
   virtual QWidget *widgetOfContainer(QWidget *w) const = 0;

   virtual QWidget *createWidget(const QString &name, QWidget *parentWidget = nullptr) const = 0;
   virtual QLayout *createLayout(QWidget *widget, QLayout *layout, int type) const = 0;

   virtual bool isPassiveInteractor(QWidget *widget) = 0;
   virtual void initialize(QObject *object) const = 0;
};

#endif // ABSTRACTWIDGETFACTORY_H
