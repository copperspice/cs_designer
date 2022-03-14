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

#include <toolbar_taskmenu.h>
#include <abstract_formwindow.h>

#include <qdesigner_toolbar_p.h>
#include <promotiontaskmenu_p.h>
#include <qdesigner_command_p.h>

#include <QAction>
#include <QUndoStack>
#include <QDebug>

namespace qdesigner_internal {

ToolBarTaskMenu::ToolBarTaskMenu(QToolBar *tb, QObject *parent) :
   QObject(parent),
   m_toolBar(tb)
{
}

QAction *ToolBarTaskMenu::preferredEditAction() const
{
   return 0;
}

QList<QAction *> ToolBarTaskMenu::taskActions() const
{
   if (ToolBarEventFilter *ef = ToolBarEventFilter::eventFilterOf(m_toolBar)) {
      return ef->contextMenuActions();
   }
   return QList<QAction *>();
}

StatusBarTaskMenu::StatusBarTaskMenu(QStatusBar *sb, QObject *parent) :
   QObject(parent),
   m_statusBar(sb),
   m_removeAction(new QAction(tr("Remove"), this)),
   m_promotionTaskMenu(new PromotionTaskMenu(sb, PromotionTaskMenu::ModeSingleWidget, this))
{
   connect(m_removeAction, &QAction::triggered, this, &StatusBarTaskMenu::removeStatusBar);
}

QAction *StatusBarTaskMenu::preferredEditAction() const
{
   return 0;
}

QList<QAction *> StatusBarTaskMenu::taskActions() const
{
   QList<QAction *> rc;
   rc.push_back(m_removeAction);
   m_promotionTaskMenu->addActions(PromotionTaskMenu::LeadingSeparator, rc);
   return rc;
}

void StatusBarTaskMenu::removeStatusBar()
{
   if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(m_statusBar)) {
      DeleteStatusBarCommand *cmd = new DeleteStatusBarCommand(fw);
      cmd->init(m_statusBar);
      fw->commandHistory()->push(cmd);
   }
}

}   // end namespace

