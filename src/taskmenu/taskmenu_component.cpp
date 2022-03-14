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

#include <taskmenu_component.h>
#include <button_taskmenu.h>
#include <groupbox_taskmenu.h>
#include <label_taskmenu.h>
#include <lineedit_taskmenu.h>
#include <listwidget_taskmenu.h>
#include <treewidget_taskmenu.h>
#include <tablewidget_taskmenu.h>
#include <containerwidget_taskmenu.h>
#include <combobox_taskmenu.h>
#include <textedit_taskmenu.h>
#include <menu_taskmenu.h>
#include <toolbar_taskmenu.h>
#include <layout_taskmenu.h>
#include <abstract_formeditor.h>
#include <extension.h>

using namespace qdesigner_internal;

TaskMenuComponent::TaskMenuComponent(QDesignerFormEditorInterface *core, QObject *parent)
   : QObject(parent), m_core(core)
{
   Q_ASSERT(m_core != 0);

   QExtensionManager *mgr = core->extensionManager();
   const QString taskMenuId =  QString("QDesignerInternalTaskMenuExtension");

   ButtonTaskMenuFactory::registerExtension(mgr, taskMenuId);
   CommandLinkButtonTaskMenuFactory::registerExtension(mgr, taskMenuId); // Order!
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


