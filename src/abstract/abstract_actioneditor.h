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

#ifndef ABSTRACTACTIONEDITOR_H
#define ABSTRACTACTIONEDITOR_H

#include <QWidget>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QDesignerActionEditorInterface : public QWidget
{
   CS_OBJECT(QDesignerActionEditorInterface)

 public:
   explicit QDesignerActionEditorInterface(QWidget *parent, Qt::WindowFlags flags = Qt::WindowFlags());
   virtual ~QDesignerActionEditorInterface();

   virtual QDesignerFormEditorInterface *core() const;

   virtual void manageAction(QAction *action) = 0;
   virtual void unmanageAction(QAction *action) = 0;

   CS_SLOT_1(Public, virtual void setFormWindow(QDesignerFormWindowInterface *formWindow) = 0)
   CS_SLOT_2(setFormWindow)
};

#endif // ABSTRACTACTIONEDITOR_H
