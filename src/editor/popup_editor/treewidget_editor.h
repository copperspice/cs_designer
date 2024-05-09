/***********************************************************************
*
* Copyright (c) 2021-2024 Barbara Geller
* Copyright (c) 2021-2024 Ansel Sermersheim
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

#ifndef TREEWIDGET_EDITOR_H
#define TREEWIDGET_EDITOR_H

#include <listwidget_editor.h>
#include <ui_edit_treewidget.h>

#include <QDialog>

class QDesignerFormWindowInterface;

class QTreeWidget;

namespace qdesigner_internal {

class FormWindowBase;
class PropertySheetIconValue;

class TreeWidgetEditor: public AbstractItemEditor
{
   CS_OBJECT(TreeWidgetEditor)

 public:
   explicit TreeWidgetEditor(QDesignerFormWindowInterface *form, QDialog *dialog);

   TreeWidgetData fillContentsFromTreeWidget(QTreeWidget *treeWidget);
   TreeWidgetData contents() const;

 protected:
   void setItemData(int role, const QVariant &v) override;
   QVariant getItemData(int role) const override;

 private:
   // slots
   void newItemButton();
   void newSubItemButton();
   void deleteItemButton();
   void moveItemUpButton();
   void moveItemDownButton();
   void moveItemRightButton();
   void moveItemLeftButton();
   void treeWidget_currentItemChanged();
   void treeWidget_itemChanged(QTreeWidgetItem *item, int column);
   void columnEditor_indexChanged(int idx);
   void columnEditor_itemChanged(int idx, int role, const QVariant &v);
   void columnEditor_itemInserted(int idx);
   void columnEditor_itemDeleted(int idx);
   void columnEditor_itemMovedUp(int idx);
   void columnEditor_itemMovedDown(int idx);
   void togglePropertyBrowser();
   void cacheReloaded();


   void setPropertyBrowserVisible(bool v);
   QtVariantProperty *setupPropertyGroup(const QString &title, PropertyDefinition *propDefs);
   void updateEditor();
   void moveColumnItems(const PropertyDefinition *propList, QTreeWidgetItem *item, int fromColumn, int toColumn, int step);
   void moveColumns(int fromColumn, int toColumn, int step);
   void moveColumnsLeft(int fromColumn, int toColumn);
   void moveColumnsRight(int fromColumn, int toColumn);
   void closeEditors();

   Ui::TreeWidgetEditor ui;
   ItemListEditor *m_columnEditor;
   bool m_updatingBrowser;
};

class TreeWidgetEditorDialog : public QDialog
{
   CS_OBJECT(TreeWidgetEditorDialog)

 public:
   explicit TreeWidgetEditorDialog(QDesignerFormWindowInterface *form, QWidget *parent);

   TreeWidgetData fillContentsFromTreeWidget(QTreeWidget *treeWidget);
   TreeWidgetData contents() const;

 private:
   TreeWidgetEditor m_editor;
};

}   // end namespace qdesigner_internal

#endif
