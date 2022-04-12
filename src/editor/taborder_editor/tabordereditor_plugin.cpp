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

#include <tabordereditor_plugin.h>
#include <tabordereditor_tool.h>
#include <abstract_formwindowmanager.h>
#include <abstract_formeditor.h>
#include <abstract_formwindow.h>

#include <formwindowbase_p.h>

#include <QAction>

using namespace qdesigner_internal;

TabOrderEditorPlugin::TabOrderEditorPlugin()
   : m_initialized(false)
{
}

TabOrderEditorPlugin::~TabOrderEditorPlugin()
{
}

bool TabOrderEditorPlugin::isInitialized() const
{
   return m_initialized;
}

void TabOrderEditorPlugin::initialize(QDesignerFormEditorInterface *core)
{
   Q_ASSERT(!isInitialized());

   m_action = new QAction(tr("Edit Tab Order"), this);
   m_action->setObjectName(QString("_qt_edit_tab_order_action"));

   QIcon icon(getResourceLocation() + "/tabordertool.png");

   m_action->setIcon(icon);
   m_action->setEnabled(false);

   setParent(core);
   m_core = core;
   m_initialized = true;

   connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowAdded,
         this, &TabOrderEditorPlugin::addFormWindow);

   connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowRemoved,
         this, &TabOrderEditorPlugin::removeFormWindow);

   connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::activeFormWindowChanged,
         this, &TabOrderEditorPlugin::activeFormWindowChanged);
}

void TabOrderEditorPlugin::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
   m_action->setEnabled(formWindow != nullptr);
}

QDesignerFormEditorInterface *TabOrderEditorPlugin::core() const
{
   return m_core;
}

void TabOrderEditorPlugin::addFormWindow(QDesignerFormWindowInterface *formWindow)
{
   Q_ASSERT(formWindow != nullptr);
   Q_ASSERT(m_tools.contains(formWindow) == false);

   TabOrderEditorTool *tool = new TabOrderEditorTool(formWindow, this);
   m_tools[formWindow] = tool;

   connect(m_action, &QAction::triggered, tool->action(), &QAction::trigger);

   formWindow->registerTool(tool);
}

void TabOrderEditorPlugin::removeFormWindow(QDesignerFormWindowInterface *formWindow)
{
   Q_ASSERT(formWindow != nullptr);
   Q_ASSERT(m_tools.contains(formWindow) == true);

   TabOrderEditorTool *tool = m_tools.value(formWindow);
   m_tools.remove(formWindow);

   disconnect(m_action, &QAction::triggered, tool->action(), &QAction::trigger);
   delete tool;
}

QAction *TabOrderEditorPlugin::action() const
{
   return m_action;
}
