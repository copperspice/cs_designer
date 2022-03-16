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

#ifndef MENUTASKMENU_H
#define MENUTASKMENU_H

#include <taskmenu.h>

#include <qdesigner_taskmenu_p.h>
#include <qdesigner_menu_p.h>
#include <qdesigner_menubar_p.h>
#include <extensionfactory_p.h>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class PromotionTaskMenu;

// The QMenu task menu provides promotion and a remove option. The actual
// menu context options are not forwarded since they make only sense
// when a menu is being edited/visible.

class MenuTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
   CS_OBJECT(MenuTaskMenu)

   CS_INTERFACES(QDesignerTaskMenuExtension)

 public:
   explicit MenuTaskMenu(QDesignerMenu *menu, QObject *parent = nullptr);

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 private:
   CS_SLOT_1(Private, void removeMenu())
   CS_SLOT_2(removeMenu)

   QDesignerMenu *m_menu;
   QAction *m_removeAction;
   PromotionTaskMenu *m_promotionTaskMenu;
};

// The QMenuBar task menu forwards the actions of QDesignerMenuBar,
// making them available in the object inspector.

class MenuBarTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
   CS_OBJECT(MenuBarTaskMenu)
   CS_INTERFACES(QDesignerTaskMenuExtension)

 public:
   explicit MenuBarTaskMenu(QDesignerMenuBar *bar, QObject *parent = nullptr);

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 private:
   QDesignerMenuBar *m_bar;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QDesignerMenu, MenuTaskMenu> MenuTaskMenuFactory;
typedef ExtensionFactory<QDesignerTaskMenuExtension, QDesignerMenuBar, MenuBarTaskMenu> MenuBarTaskMenuFactory;

}  // namespace qdesigner_internal

#endif
