/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#ifndef TREEWIDGET_TASKMENU_H
#define TREEWIDGET_TASKMENU_H

#include <designer_taskmenu.h>

#include <extensionfactory_p.h>

#include <QPointer>
#include <QTreeWidget>

class QDesignerFormWindowInterface;

class QLineEdit;

namespace qdesigner_internal {

class TreeWidgetTaskMenu: public QDesignerTaskMenu
{
   CS_OBJECT(TreeWidgetTaskMenu)

 public:
   explicit TreeWidgetTaskMenu(QTreeWidget *button, QObject *parent = nullptr);
   virtual ~TreeWidgetTaskMenu();

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 private:
   CS_SLOT_1(Private, void editItems())
   CS_SLOT_2(editItems)

   CS_SLOT_1(Private, void updateSelection())
   CS_SLOT_2(updateSelection)

   QTreeWidget *m_treeWidget;
   QPointer<QDesignerFormWindowInterface> m_formWindow;
   QPointer<QLineEdit> m_editor;
   mutable QList<QAction *> m_taskActions;
   QAction *m_editItemsAction;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QTreeWidget, TreeWidgetTaskMenu> TreeWidgetTaskMenuFactory;

}   // end namespace qdesigner_internal

#endif
