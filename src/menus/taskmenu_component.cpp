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

#include <abstract_formeditor.h>
#include <button_taskmenu.h>
#include <combobox_taskmenu.h>
#include <containerwidget_taskmenu.h>
#include <extension.h>
#include <groupbox_taskmenu.h>
#include <label_taskmenu.h>
#include <layout_taskmenu.h>
#include <lineedit_taskmenu.h>
#include <listwidget_taskmenu.h>
#include <menu_taskmenu.h>
#include <tablewidget_taskmenu.h>
#include <taskmenu_component.h>
#include <textedit_taskmenu.h>
#include <toolbar_taskmenu.h>
#include <treewidget_taskmenu.h>

using namespace qdesigner_internal;

TaskMenuComponent::TaskMenuComponent(QDesignerFormEditorInterface *core, QObject *parent)
   : QObject(parent), m_core(core)
{
   Q_ASSERT(m_core != nullptr);

   QExtensionManager *mgr = core->extensionManager();
   const QString taskMenuId = "QDesignerInternalTaskMenuExtension";

   ButtonTaskMenuFactory::registerExtension(mgr, taskMenuId);
   CommandLinkButtonTaskMenuFactory::registerExtension(mgr, taskMenuId);

   ButtonGroupTaskMenuFactory::registerExtension(mgr, taskMenuId);

   GroupBoxTaskMenuFactory::registerExtension(mgr, taskMenuId);
   LabelTaskMenuFactory::registerExtension(mgr, taskMenuId);
   LineEditTaskMenuFactory::registerExtension(mgr, taskMenuId);
   ListWidgetTaskMenuFactory::registerExtension(mgr, taskMenuId);
   TreeWidgetTaskMenuFactory::registerExtension(mgr, taskMenuId);
   TableWidgetTaskMenuFactory::registerExtension(mgr, taskMenuId);
   TextEditTaskMenuFactory::registerExtension(mgr, taskMenuId);
   PlainTextEditTaskMenuFactory::registerExtension(mgr, taskMenuId);
   MenuTaskMenuFactory::registerExtension(mgr, taskMenuId);
   MenuBarTaskMenuFactory::registerExtension(mgr, taskMenuId);
   ToolBarTaskMenuFactory::registerExtension(mgr, taskMenuId);
   StatusBarTaskMenuFactory::registerExtension(mgr, taskMenuId);
   LayoutWidgetTaskMenuFactory::registerExtension(mgr, taskMenuId);
   SpacerTaskMenuFactory::registerExtension(mgr, taskMenuId);

   mgr->registerExtensions(new ContainerWidgetTaskMenuFactory(core, mgr), taskMenuId);
   mgr->registerExtensions(new ComboBoxTaskMenuFactory(taskMenuId, mgr), taskMenuId);
}

TaskMenuComponent::~TaskMenuComponent()
{
}

QDesignerFormEditorInterface *TaskMenuComponent::core() const
{
   return m_core;
}


