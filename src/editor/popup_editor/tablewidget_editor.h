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

#ifndef TABLEWIDGET_EDITOR_H
#define TABLEWIDGET_EDITOR_H

#include <listwidget_editor.h>
#include <ui_edit_tablewidget.h>

#include <QDialog>

class QDesignerFormWindowInterface;

class QTableWidget;

namespace qdesigner_internal {

class FormWindowBase;
class PropertySheetIconValue;

class TableWidgetEditor: public AbstractItemEditor
{
   CS_OBJECT(TableWidgetEditor)

 public:
   explicit TableWidgetEditor(QDesignerFormWindowInterface *form, QDialog *dialog);

   TableWidgetData fillContentsFromTableWidget(QTableWidget *tableWidget);
   TableWidgetData contents() const;

 protected:
   void setItemData(int role, const QVariant &v) override;
   QVariant getItemData(int role) const override;

 private:
   // slots
   void table_currentCellChanged(int currentRow, int currentCol);
   void table_itemChanged(QTableWidgetItem *item);

   CS_SLOT_1(Private, void column_indexChanged(int idx))
   CS_SLOT_2(column_indexChanged)

   CS_SLOT_1(Private, void column_itemChanged(int idx, int role, const QVariant &v))
   CS_SLOT_2(column_itemChanged)

   CS_SLOT_1(Private, void column_itemInserted(int idx))
   CS_SLOT_2(column_itemInserted)

   CS_SLOT_1(Private, void column_itemDeleted(int idx))
   CS_SLOT_2(column_itemDeleted)

   CS_SLOT_1(Private, void column_itemMovedUp(int idx))
   CS_SLOT_2(column_itemMovedUp)

   CS_SLOT_1(Private, void column_itemMovedDown(int idx))
   CS_SLOT_2(column_itemMovedDown)

   CS_SLOT_1(Private, void row_indexChanged(int idx))
   CS_SLOT_2(row_indexChanged)

   CS_SLOT_1(Private, void row_itemChanged(int idx, int role, const QVariant &v))
   CS_SLOT_2(row_itemChanged)

   CS_SLOT_1(Private, void row_itemInserted(int idx))
   CS_SLOT_2(row_itemInserted)

   CS_SLOT_1(Private, void row_itemDeleted(int idx))
   CS_SLOT_2(row_itemDeleted)

   CS_SLOT_1(Private, void row_itemMovedUp(int idx))
   CS_SLOT_2(row_itemMovedUp)

   CS_SLOT_1(Private, void row_itemMovedDown(int idx))
   CS_SLOT_2(row_itemMovedDown)

   CS_SLOT_1(Private, void togglePropertyBrowser())
   CS_SLOT_2(togglePropertyBrowser)

   CS_SLOT_1(Private, void cacheReloaded())
   CS_SLOT_2(cacheReloaded)

   void setPropertyBrowserVisible(bool v);
   void updateEditor();
   void moveColumnsLeft(int fromColumn, int toColumn);
   void moveColumnsRight(int fromColumn, int toColumn);
   void moveRowsUp(int fromRow, int toRow);
   void moveRowsDown(int fromRow, int toRow);

   Ui::TableWidgetEditor ui;
   ItemListEditor *m_rowEditor;
   ItemListEditor *m_columnEditor;
   bool m_updatingBrowser;
};

class TableWidgetEditorDialog : public QDialog
{
   CS_OBJECT(TableWidgetEditorDialog)

 public:
   explicit TableWidgetEditorDialog(QDesignerFormWindowInterface *form, QWidget *parent);

   TableWidgetData fillContentsFromTableWidget(QTableWidget *tableWidget);
   TableWidgetData contents() const;

 private:
   TableWidgetEditor m_editor;
};

}   // end namespace qdesigner_internal

#endif
