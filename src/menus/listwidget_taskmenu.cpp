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

#include <listwidget_taskmenu.h>
#include <listwidget_editor.h>
#include <abstract_formwindow.h>
#include <designer_utils.h>
#include <designer_command.h>

#include <QAction>
#include <QStyle>
#include <QLineEdit>
#include <QStyleOption>
#include <QEvent>
#include <QVariant>
#include <QDebug>

using namespace qdesigner_internal;

ListWidgetTaskMenu::ListWidgetTaskMenu(QListWidget *button, QObject *parent)
   : QDesignerTaskMenu(button, parent), m_listWidget(button)
{
   m_editItemsAction = new QAction(this);
   m_editItemsAction->setText(tr("Edit Items..."));
   connect(m_editItemsAction, &QAction::triggered, this, &ListWidgetTaskMenu::editItems);
   m_taskActions.append(m_editItemsAction);

   QAction *sep = new QAction(this);
   sep->setSeparator(true);
   m_taskActions.append(sep);
}

ListWidgetTaskMenu::~ListWidgetTaskMenu()
{
}

QAction *ListWidgetTaskMenu::preferredEditAction() const
{
   return m_editItemsAction;
}

QList<QAction *> ListWidgetTaskMenu::taskActions() const
{
   return m_taskActions + QDesignerTaskMenu::taskActions();
}

void ListWidgetTaskMenu::editItems()
{
   m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_listWidget);
   if (m_formWindow.isNull()) {
      return;
   }

   Q_ASSERT(m_listWidget != nullptr);

   ListWidgetEditor dlg(m_formWindow, m_listWidget->window());
   ListData oldItems = dlg.fillContentsFromListWidget(m_listWidget);

   if (dlg.exec() == QDialog::Accepted) {
      ListData items = dlg.contents();

      if (items != oldItems) {
         ChangeListDataCommand *cmd = new ChangeListDataCommand(m_formWindow);
         cmd->init(m_listWidget, oldItems, items);
         cmd->setText(tr("Change List Contents"));
         m_formWindow->commandHistory()->push(cmd);
      }
   }
}

void ListWidgetTaskMenu::updateSelection()
{
   if (m_editor) {
      m_editor->deleteLater();
   }
}

