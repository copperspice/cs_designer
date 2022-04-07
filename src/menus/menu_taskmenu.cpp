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

#include <menu_taskmenu.h>
#include <promotion_taskmenu.h>

#include <QAction>
#include <QDebug>

namespace qdesigner_internal {

MenuTaskMenu::MenuTaskMenu(QDesignerMenu *menu, QObject *parent)
   : QObject(parent), m_menu(menu),
     m_removeAction(new QAction(tr("Remove"), this)),
     m_promotionTaskMenu(new PromotionTaskMenu(menu, PromotionTaskMenu::ModeSingleWidget, this))
{
   connect(m_removeAction, &QAction::triggered, this, &MenuTaskMenu::removeMenu);
}

QAction *MenuTaskMenu::preferredEditAction() const
{
   return nullptr;
}

QList<QAction *> MenuTaskMenu::taskActions() const
{
   QList<QAction *> rc;
   rc.push_back(m_removeAction);
   m_promotionTaskMenu->addActions(PromotionTaskMenu::LeadingSeparator, rc);

   return rc;
}

void MenuTaskMenu::removeMenu()
{
   // Are we on a menu bar or on a menu?
   QWidget *pw = m_menu->parentWidget();

   if (QDesignerMenuBar *mb = dynamic_cast<QDesignerMenuBar *>(pw)) {
      mb->deleteMenuAction(m_menu->menuAction());
      return;
   }

   if (QDesignerMenu *m = dynamic_cast<QDesignerMenu *>(pw)) {
      m->deleteAction(m_menu->menuAction());
   }
}

MenuBarTaskMenu::MenuBarTaskMenu(QDesignerMenuBar *bar, QObject *parent)
   : QObject(parent), m_bar(bar)
{
}

QAction *MenuBarTaskMenu::preferredEditAction() const
{
   return nullptr;
}

QList<QAction *> MenuBarTaskMenu::taskActions() const
{
   return m_bar->contextMenuActions();
}

}   // end namespace

