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

#ifndef ABSTRACTFORMEDITORPLUGIN_H
#define ABSTRACTFORMEDITORPLUGIN_H

#include <QObject>

class QAction;

class QDesignerFormEditorInterface;

class QDesignerFormEditorPluginInterface
{
 public:
   virtual ~QDesignerFormEditorPluginInterface() {}

   virtual bool isInitialized() const = 0;
   virtual void initialize(QDesignerFormEditorInterface *core) = 0;
   virtual QAction *action() const = 0;

   virtual QDesignerFormEditorInterface *core() const = 0;
};

CS_DECLARE_INTERFACE(QDesignerFormEditorPluginInterface, "copperspice.com.CS.Designer.QDesignerFormEditorPluginInterface")

#endif // ABSTRACTFORMEDITORPLUGIN_H
