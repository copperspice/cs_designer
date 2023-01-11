/***********************************************************************
*
* Copyright (c) 2021-2023 Barbara Geller
* Copyright (c) 2021-2023 Ansel Sermersheim
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

#ifndef QDESIGNER_COMPONENTS_H
#define QDESIGNER_COMPONENTS_H

class QDesignerFormEditorInterface;
class QDesignerWidgetBoxInterface;
class QDesignerPropertyEditorInterface;
class QDesignerObjectInspectorInterface;
class QDesignerActionEditorInterface;

#include <qglobal.h>

class QObject;
class QWidget;

class QDesignerComponents
{
 public:
   // emerald - temporary hold, plugins
   // static void initializePlugins(QDesignerFormEditorInterface *core);

   static QDesignerFormEditorInterface *createFormEditor(QObject *parent);
   static QDesignerWidgetBoxInterface *createWidgetBox(QDesignerFormEditorInterface *core, QWidget *parent);
   static QDesignerPropertyEditorInterface *createPropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent);
   static QDesignerObjectInspectorInterface *createObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent);
   static QDesignerActionEditorInterface *createActionEditor(QDesignerFormEditorInterface *core, QWidget *parent);

   static QObject *createTaskMenu(QDesignerFormEditorInterface *core, QObject *parent);
   static QWidget *createResourceEditor(QDesignerFormEditorInterface *core, QWidget *parent);
   static QWidget *createSignalSlotEditor(QDesignerFormEditorInterface *core, QWidget *parent);
};

#endif