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
#include <treewidget_editor.h>
#include <treewidget_taskmenu.h>

#include <QAction>
#include <QEvent>
#include <QLineEdit>
#include <QStyle>
#include <QStyleOption>
#include <QVariant>

using namespace qdesigner_internal;

TreeWidgetTaskMenu::TreeWidgetTaskMenu(QTreeWidget *button, QObject *parent)
   : QDesignerTaskMenu(button, parent), m_treeWidget(button),
     m_editItemsAction(new QAction(tr("Edit Items..."), this))
{
   connect(m_editItemsAction, &QAction::triggered, this, &TreeWidgetTaskMenu::editItems);
   m_taskActions.append(m_editItemsAction);

   QAction *sep = new QAction(this);
   sep->setSeparator(true);
   m_taskActions.append(sep);
}

TreeWidgetTaskMenu::~TreeWidgetTaskMenu()
{
}

QAction *TreeWidgetTaskMenu::preferredEditAction() const
{
   return m_editItemsAction;
}

QList<QAction *> TreeWidgetTaskMenu::taskActions() const
{
   return m_taskActions + QDesignerTaskMenu::taskActions();
}

void TreeWidgetTaskMenu::editItems()
{
   m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_treeWidget);
   if (m_formWindow.isNull()) {
      return;
   }

   Q_ASSERT(m_treeWidget != nullptr);

   TreeWidgetEditorDialog dlg(m_formWindow, m_treeWidget->window());
   TreeWidgetData oldCont = dlg.fillContentsFromTreeWidget(m_treeWidget);

   if (dlg.exec() == QDialog::Accepted) {
      TreeWidgetData newCont = dlg.contents();

      if (newCont != oldCont) {
         ChangeTreeDataCommand *cmd = new ChangeTreeDataCommand(m_formWindow);
         cmd->init(m_treeWidget, oldCont, newCont);
         m_formWindow->commandHistory()->push(cmd);
      }
   }
}

void TreeWidgetTaskMenu::updateSelection()
{
   if (m_editor) {
      m_editor->deleteLater();
   }
}

