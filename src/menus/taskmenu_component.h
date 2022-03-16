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

#ifndef TASKMENU_COMPONENT_H
#define TASKMENU_COMPONENT_H

#include <taskmenu.h>

#include <QObject>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class TaskMenuComponent: public QObject
{
   CS_OBJECT(TaskMenuComponent)

 public:
   explicit TaskMenuComponent(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
   virtual ~TaskMenuComponent();

   QDesignerFormEditorInterface *core() const;

 private:
   QDesignerFormEditorInterface *m_core;
};

}  // namespace qdesigner_internal



#endif // TASKMENU_COMPONENT_H
