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

#ifndef ABSTRACTFORMWINDOWCURSOR_H
#define ABSTRACTFORMWINDOWCURSOR_H

#include <QString>

class QDesignerFormWindowInterface;
class QWidget;
class QVariant;

class QDesignerFormWindowCursorInterface
{
 public:
   enum MoveOperation {
      NoMove,

      Start,
      End,
      Next,
      Prev,
      Left,
      Right,
      Up,
      Down
   };

   enum MoveMode {
      MoveAnchor,
      KeepAnchor
   };

 public:
   virtual ~QDesignerFormWindowCursorInterface() {}

   virtual QDesignerFormWindowInterface *formWindow() const = 0;

   virtual bool movePosition(MoveOperation op, MoveMode mode = MoveAnchor) = 0;

   virtual int position() const = 0;
   virtual void setPosition(int pos, MoveMode mode = MoveAnchor) = 0;

   virtual QWidget *current() const = 0;

   virtual int widgetCount() const = 0;
   virtual QWidget *widget(int index) const = 0;

   virtual bool hasSelection() const = 0;
   virtual int selectedWidgetCount() const = 0;
   virtual QWidget *selectedWidget(int index) const = 0;

   virtual void setProperty(const QString &name, const QVariant &value) = 0;
   virtual void setWidgetProperty(QWidget *widget, const QString &name, const QVariant &value) = 0;
   virtual void resetWidgetProperty(QWidget *widget, const QString &name) = 0;

   bool isWidgetSelected(QWidget *widget) const;
};



#endif // ABSTRACTFORMWINDOWCURSOR_H
