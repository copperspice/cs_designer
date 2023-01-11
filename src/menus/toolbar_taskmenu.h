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

#ifndef TOOLBAR_TASKMENU_H
#define TOOLBAR_TASKMENU_H

#include <designer_taskmenu.h>

#include <extensionfactory_p.h>

#include <QToolBar>
#include <QStatusBar>

namespace qdesigner_internal {
class PromotionTaskMenu;

// ToolBarTaskMenu forwards the actions of ToolBarEventFilter
class ToolBarTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
   CS_OBJECT(ToolBarTaskMenu)
   CS_INTERFACES(QDesignerTaskMenuExtension)

 public:
   explicit ToolBarTaskMenu(QToolBar *tb, QObject *parent = nullptr);

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 private:
   QToolBar *m_toolBar;
};

// StatusBarTaskMenu provides promotion and deletion
class StatusBarTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
   CS_OBJECT(StatusBarTaskMenu)
   CS_INTERFACES(QDesignerTaskMenuExtension)

 public:
   explicit StatusBarTaskMenu(QStatusBar *tb, QObject *parent = nullptr);

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 private:
   CS_SLOT_1(Private, void removeStatusBar())
   CS_SLOT_2(removeStatusBar)

   QStatusBar *m_statusBar;
   QAction *m_removeAction;
   PromotionTaskMenu *m_promotionTaskMenu;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QToolBar, ToolBarTaskMenu> ToolBarTaskMenuFactory;
typedef ExtensionFactory<QDesignerTaskMenuExtension, QStatusBar, StatusBarTaskMenu> StatusBarTaskMenuFactory;

}   // end namespace qdesigner_internal

#endif
