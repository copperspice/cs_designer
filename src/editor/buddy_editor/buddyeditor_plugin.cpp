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

#include <buddyeditor_plugin.h>
#include <buddyeditor_tool.h>
#include <abstract_formwindow.h>
#include <formwindowbase_p.h>
#include <abstract_formeditor.h>
#include <abstract_formwindowmanager.h>

#include <QAction>

using namespace qdesigner_internal;

BuddyEditorPlugin::BuddyEditorPlugin()
   : m_initialized(false)
{
}

BuddyEditorPlugin::~BuddyEditorPlugin()
{
}

bool BuddyEditorPlugin::isInitialized() const
{
   return m_initialized;
}

void BuddyEditorPlugin::initialize(QDesignerFormEditorInterface *core)
{
   Q_ASSERT(!isInitialized());

   m_action = new QAction(tr("Edit Buddies"), this);
   m_action->setObjectName("__qt_edit_buddies_action");

   QIcon buddyIcon(getResourceLocation() + "/buddytool.png");

   m_action->setIcon(buddyIcon);
   m_action->setEnabled(false);

   setParent(core);
   m_core = core;
   m_initialized = true;

   connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowAdded,
         this, &BuddyEditorPlugin::addFormWindow);

   connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowRemoved,
         this, &BuddyEditorPlugin::removeFormWindow);

   connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::activeFormWindowChanged,
         this, &BuddyEditorPlugin::activeFormWindowChanged);
}

QDesignerFormEditorInterface *BuddyEditorPlugin::core() const
{
   return m_core;
}

void BuddyEditorPlugin::addFormWindow(QDesignerFormWindowInterface *formWindow)
{
   Q_ASSERT(formWindow != 0);
   Q_ASSERT(m_tools.contains(formWindow) == false);

   BuddyEditorTool *tool = new BuddyEditorTool(formWindow, this);
   m_tools[formWindow] = tool;
   connect(m_action, &QAction::triggered, tool->action(), &QAction::trigger);
   formWindow->registerTool(tool);
}

void BuddyEditorPlugin::removeFormWindow(QDesignerFormWindowInterface *formWindow)
{
   Q_ASSERT(formWindow != 0);
   Q_ASSERT(m_tools.contains(formWindow) == true);

   BuddyEditorTool *tool = m_tools.value(formWindow);
   m_tools.remove(formWindow);

   disconnect(m_action, &QAction::triggered, tool->action(), &QAction::trigger);
   delete tool;
}

QAction *BuddyEditorPlugin::action() const
{
   return m_action;
}

void BuddyEditorPlugin::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
   m_action->setEnabled(formWindow != 0);
}

