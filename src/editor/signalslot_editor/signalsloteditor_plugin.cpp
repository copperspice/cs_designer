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

#include <signalsloteditor_plugin.h>
#include <signalsloteditor_tool.h>
#include <abstract_formeditor.h>
#include <formwindowbase_p.h>

#include <QAction>

using namespace qdesigner_internal;

SignalSlotEditorPlugin::SignalSlotEditorPlugin()
   : m_initialized(false), m_action(nullptr)
{
}

SignalSlotEditorPlugin::~SignalSlotEditorPlugin()
{
}

bool SignalSlotEditorPlugin::isInitialized() const
{
   return m_initialized;
}

void SignalSlotEditorPlugin::initialize(QDesignerFormEditorInterface *core)
{
   Q_ASSERT(! isInitialized());

   m_action = new QAction(tr("Edit Signals/Slots"), this);
   m_action->setObjectName("__qt_edit_signals_slots_action");
   m_action->setShortcut(tr("F4"));

   QIcon icon(getResourceLocation() + "/signalslottool.png");

   m_action->setIcon(icon);
   m_action->setEnabled(false);

   setParent(core);
   m_core = core;
   m_initialized = true;

   connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowAdded,
         this, &SignalSlotEditorPlugin::addFormWindow);

   connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowRemoved,
         this, &SignalSlotEditorPlugin::removeFormWindow);

   connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::activeFormWindowChanged,
         this, &SignalSlotEditorPlugin::activeFormWindowChanged);
}

QDesignerFormEditorInterface *SignalSlotEditorPlugin::core() const
{
   return m_core;
}

void SignalSlotEditorPlugin::addFormWindow(QDesignerFormWindowInterface *formWindow)
{
   Q_ASSERT(formWindow != 0);
   Q_ASSERT(m_tools.contains(formWindow) == false);

   SignalSlotEditorTool *tool = new SignalSlotEditorTool(formWindow, this);
   m_tools[formWindow] = tool;

   connect(m_action, &QAction::triggered, tool->action(), &QAction::trigger);

   formWindow->registerTool(tool);
}

void SignalSlotEditorPlugin::removeFormWindow(QDesignerFormWindowInterface *formWindow)
{
   Q_ASSERT(formWindow != 0);
   Q_ASSERT(m_tools.contains(formWindow) == true);

   SignalSlotEditorTool *tool = m_tools.value(formWindow);
   m_tools.remove(formWindow);

   disconnect(m_action, &QAction::triggered, tool->action(), &QAction::trigger);
   delete tool;
}

QAction *SignalSlotEditorPlugin::action() const
{
   return m_action;
}

void SignalSlotEditorPlugin::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
   m_action->setEnabled(formWindow != 0);
}

