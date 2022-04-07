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

#include <tablewidget_taskmenu.h>
#include <tablewidget_editor.h>
#include <abstract_formwindow.h>

#include <QTableWidget>
#include <QAction>
#include <QLineEdit>
#include <QStyle>
#include <QStyleOption>
#include <QEvent>
#include <QVariant>
#include <QDebug>

using namespace qdesigner_internal;

TableWidgetTaskMenu::TableWidgetTaskMenu(QTableWidget *button, QObject *parent)
   : QDesignerTaskMenu(button, parent), m_tableWidget(button),
     m_editItemsAction(new QAction(tr("Edit Items..."), this))
{
   connect(m_editItemsAction, &QAction::triggered, this, &TableWidgetTaskMenu::editItems);
   m_taskActions.append(m_editItemsAction);

   QAction *sep = new QAction(this);
   sep->setSeparator(true);
   m_taskActions.append(sep);
}

TableWidgetTaskMenu::~TableWidgetTaskMenu()
{
}

QAction *TableWidgetTaskMenu::preferredEditAction() const
{
   return m_editItemsAction;
}

QList<QAction *> TableWidgetTaskMenu::taskActions() const
{
   return m_taskActions + QDesignerTaskMenu::taskActions();
}

void TableWidgetTaskMenu::editItems()
{
   m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_tableWidget);
   if (m_formWindow.isNull()) {
      return;
   }

   Q_ASSERT(m_tableWidget != nullptr);

   TableWidgetEditorDialog dlg(m_formWindow, m_tableWidget->window());
   TableWidgetData oldCont = dlg.fillContentsFromTableWidget(m_tableWidget);

   if (dlg.exec() == QDialog::Accepted) {
      TableWidgetData newCont = dlg.contents();

      if (newCont != oldCont) {
         ChangeTableDataCommand *cmd = new ChangeTableDataCommand(m_formWindow);
         cmd->init(m_tableWidget, oldCont, newCont);
         m_formWindow->commandHistory()->push(cmd);
      }
   }
}

void TableWidgetTaskMenu::updateSelection()
{
   if (m_editor) {
      m_editor->deleteLater();
   }
}
