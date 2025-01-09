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

#ifndef ABSTRACTOBJECTINSPECTOR_H
#define ABSTRACTOBJECTINSPECTOR_H

#include <QWidget>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QDesignerObjectInspectorInterface: public QWidget
{
   CS_OBJECT(QDesignerObjectInspectorInterface)

 public:
   explicit QDesignerObjectInspectorInterface(QWidget *parent, Qt::WindowFlags flags = Qt::WindowFlags());
   virtual ~QDesignerObjectInspectorInterface();

   virtual QDesignerFormEditorInterface *core() const;

   CS_SLOT_1(Public, virtual void setFormWindow(QDesignerFormWindowInterface *formWindow) = 0)
   CS_SLOT_2(setFormWindow)
};

#endif // ABSTRACTOBJECTINSPECTOR_H
