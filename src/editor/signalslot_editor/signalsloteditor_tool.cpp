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

#include <abstract_formwindow.h>
#include <signalsloteditor.h>
#include <signalsloteditor_tool.h>
#include <ui4.h>

#include <QAction>

using namespace qdesigner_internal;

SignalSlotEditorTool::SignalSlotEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent)
   : QDesignerFormWindowToolInterface(parent), m_formWindow(formWindow),
     m_action(new QAction(tr("Edit Signals/Slots"), this))
{
}

SignalSlotEditorTool::~SignalSlotEditorTool()
{
}

QDesignerFormEditorInterface *SignalSlotEditorTool::core() const
{
   return m_formWindow->core();
}

QDesignerFormWindowInterface *SignalSlotEditorTool::formWindow() const
{
   return m_formWindow;
}

bool SignalSlotEditorTool::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
   (void) widget;
   (void) managedWidget;
   (void) event;

   return false;
}

QWidget *SignalSlotEditorTool::editor() const
{
   if (!m_editor) {
      Q_ASSERT(formWindow() != nullptr);

      m_editor = new qdesigner_internal::SignalSlotEditor(formWindow(), nullptr);

      connect(formWindow(), &QDesignerFormWindowInterface::mainContainerChanged,
         m_editor.data(), &SignalSlotEditor::setBackground);

      connect(formWindow(), &QDesignerFormWindowInterface::changed,
         m_editor.data(), &SignalSlotEditor::updateBackground);
   }

   return m_editor;
}

QAction *SignalSlotEditorTool::action() const
{
   return m_action;
}

void SignalSlotEditorTool::activated()
{
   m_editor->enableUpdateBackground(true);
}

void SignalSlotEditorTool::deactivated()
{
   m_editor->enableUpdateBackground(false);
}

void SignalSlotEditorTool::saveToDom(DomUI *ui, QWidget *)
{
   ui->setElementConnections(m_editor->toUi());
}

void SignalSlotEditorTool::loadFromDom(DomUI *ui, QWidget *mainContainer)
{
   m_editor->fromUi(ui->elementConnections(), mainContainer);
}
