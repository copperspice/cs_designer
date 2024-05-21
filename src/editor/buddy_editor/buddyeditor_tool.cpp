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

#include <abstract_formwindow.h>
#include <buddyeditor.h>
#include <buddyeditor_tool.h>

#include <QAction>

using namespace qdesigner_internal;

BuddyEditorTool::BuddyEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent)
   : QDesignerFormWindowToolInterface(parent),
     m_formWindow(formWindow),
     m_action(new QAction(tr("Edit Buddies"), this))
{
}

BuddyEditorTool::~BuddyEditorTool()
{
}

QDesignerFormEditorInterface *BuddyEditorTool::core() const
{
   return m_formWindow->core();
}

QDesignerFormWindowInterface *BuddyEditorTool::formWindow() const
{
   return m_formWindow;
}

bool BuddyEditorTool::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
   (void) widget;
   (void) managedWidget;
   (void) event;

   return false;
}

QWidget *BuddyEditorTool::editor() const
{
   if (!m_editor) {
      Q_ASSERT(formWindow() != nullptr);
      m_editor = new BuddyEditor(formWindow(), nullptr);

      connect(formWindow(), &QDesignerFormWindowInterface::mainContainerChanged,
            m_editor.data(), &BuddyEditor::setBackground);

      connect(formWindow(), &QDesignerFormWindowInterface::changed,
            m_editor.data(), &BuddyEditor::updateBackground);
   }

   return m_editor;
}

void BuddyEditorTool::activated()
{
   m_editor->enableUpdateBackground(true);
}

void BuddyEditorTool::deactivated()
{
   m_editor->enableUpdateBackground(false);
}

QAction *BuddyEditorTool::action() const
{
   return m_action;
}

