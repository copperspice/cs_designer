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

#include <tabordereditor_tool.h>
#include <tabordereditor.h>
#include <abstract_formwindow.h>

#include <QEvent>
#include <QAction>

using namespace qdesigner_internal;

TabOrderEditorTool::TabOrderEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent)
   : QDesignerFormWindowToolInterface(parent), m_formWindow(formWindow),
     m_action(new QAction(tr("Edit Tab Order"), this))
{
}

TabOrderEditorTool::~TabOrderEditorTool()
{
}

QDesignerFormEditorInterface *TabOrderEditorTool::core() const
{
   return m_formWindow->core();
}

QDesignerFormWindowInterface *TabOrderEditorTool::formWindow() const
{
   return m_formWindow;
}

bool TabOrderEditorTool::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
   Q_UNUSED(widget);
   Q_UNUSED(managedWidget);

   if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
      return true;
   }

   return false;
}

QWidget *TabOrderEditorTool::editor() const
{
   if (!m_editor) {
      Q_ASSERT(formWindow() != nullptr);
      m_editor = new TabOrderEditor(formWindow(), nullptr);

      connect(formWindow(), &QDesignerFormWindowInterface::mainContainerChanged,
            m_editor.data(), &TabOrderEditor::setBackground);
   }

   return m_editor;
}

void TabOrderEditorTool::activated()
{
   connect(formWindow(), &QDesignerFormWindowInterface::changed,
         m_editor.data(), &TabOrderEditor::updateBackground);
}

void TabOrderEditorTool::deactivated()
{
   disconnect(formWindow(), &QDesignerFormWindowInterface::changed,
         m_editor.data(), &TabOrderEditor::updateBackground);
}

QAction *TabOrderEditorTool::action() const
{
   return m_action;
}

