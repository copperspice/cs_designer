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

#ifndef TASKMENU_H
#define TASKMENU_H

#include <extension.h>

class QAction;

class QDesignerTaskMenuExtension
{
 public:
   virtual ~QDesignerTaskMenuExtension()
   {
   }

   virtual QAction *preferredEditAction() const;

   virtual QList<QAction *> taskActions() const = 0;
};

CS_DECLARE_EXTENSION_INTERFACE(QDesignerTaskMenuExtension, "copperspice.com.CS.Designer.TaskMenu")

inline QAction *QDesignerTaskMenuExtension::preferredEditAction() const
{
   return nullptr;
}

#endif
