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

#ifndef TABLEWIDGET_TASKMENU_H
#define TABLEWIDGET_TASKMENU_H

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

#include <QTableWidget>
#include <QPointer>

class QLineEdit;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class TableWidgetTaskMenu: public QDesignerTaskMenu
{
   CS_OBJECT(TableWidgetTaskMenu)

 public:
   explicit TableWidgetTaskMenu(QTableWidget *button, QObject *parent = nullptr);
   virtual ~TableWidgetTaskMenu();

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 private:
   CS_SLOT_1(Private, void editItems())
   CS_SLOT_2(editItems)
   CS_SLOT_1(Private, void updateSelection())
   CS_SLOT_2(updateSelection)

   QTableWidget *m_tableWidget;
   QPointer<QDesignerFormWindowInterface> m_formWindow;
   QPointer<QLineEdit> m_editor;
   mutable QList<QAction *> m_taskActions;
   QAction *m_editItemsAction;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QTableWidget, TableWidgetTaskMenu> TableWidgetTaskMenuFactory;

}   // end namespace qdesigner_internal


#endif