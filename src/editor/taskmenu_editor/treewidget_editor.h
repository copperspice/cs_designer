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

#ifndef TREEWIDGET_EDITOR_H
#define TREEWIDGET_EDITOR_H

#include <listwidget_editor.h>
#include <ui_treewidget_editor.h>

class QDesignerFormWindowInterface;

#include <QDialog>

class QTreeWidget;

namespace qdesigner_internal {

class FormWindowBase;
class PropertySheetIconValue;

class TreeWidgetEditor: public AbstractItemEditor
{
   CS_OBJECT(TreeWidgetEditor)
 public:
   explicit TreeWidgetEditor(QDesignerFormWindowInterface *form, QDialog *dialog);

   TreeWidgetContents fillContentsFromTreeWidget(QTreeWidget *treeWidget);
   TreeWidgetContents contents() const;

 private:
   CS_SLOT_1(Private, void on_newItemButton_clicked())
   CS_SLOT_2(on_newItemButton_clicked)
   CS_SLOT_1(Private, void on_newSubItemButton_clicked())
   CS_SLOT_2(on_newSubItemButton_clicked)
   CS_SLOT_1(Private, void on_deleteItemButton_clicked())
   CS_SLOT_2(on_deleteItemButton_clicked)
   CS_SLOT_1(Private, void on_moveItemUpButton_clicked())
   CS_SLOT_2(on_moveItemUpButton_clicked)
   CS_SLOT_1(Private, void on_moveItemDownButton_clicked())
   CS_SLOT_2(on_moveItemDownButton_clicked)
   CS_SLOT_1(Private, void on_moveItemRightButton_clicked())
   CS_SLOT_2(on_moveItemRightButton_clicked)
   CS_SLOT_1(Private, void on_moveItemLeftButton_clicked())
   CS_SLOT_2(on_moveItemLeftButton_clicked)

   CS_SLOT_1(Private, void on_treeWidget_currentItemChanged())
   CS_SLOT_2(on_treeWidget_currentItemChanged)
   CS_SLOT_1(Private, void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column))
   CS_SLOT_2(on_treeWidget_itemChanged)

   CS_SLOT_1(Private, void on_columnEditor_indexChanged(int idx))
   CS_SLOT_2(on_columnEditor_indexChanged)
   CS_SLOT_1(Private, void on_columnEditor_itemChanged(int idx, int role, const QVariant &v))
   CS_SLOT_2(on_columnEditor_itemChanged)

   CS_SLOT_1(Private, void on_columnEditor_itemInserted(int idx))
   CS_SLOT_2(on_columnEditor_itemInserted)
   CS_SLOT_1(Private, void on_columnEditor_itemDeleted(int idx))
   CS_SLOT_2(on_columnEditor_itemDeleted)
   CS_SLOT_1(Private, void on_columnEditor_itemMovedUp(int idx))
   CS_SLOT_2(on_columnEditor_itemMovedUp)
   CS_SLOT_1(Private, void on_columnEditor_itemMovedDown(int idx))
   CS_SLOT_2(on_columnEditor_itemMovedDown)

   CS_SLOT_1(Private, void togglePropertyBrowser())
   CS_SLOT_2(togglePropertyBrowser)
   CS_SLOT_1(Private, void cacheReloaded())
   CS_SLOT_2(cacheReloaded)

 protected:
   void setItemData(int role, const QVariant &v) override;
   QVariant getItemData(int role) const override;

 private:
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

   TreeWidgetContents fillContentsFromTreeWidget(QTreeWidget *treeWidget);
   TreeWidgetContents contents() const;

 private:
   TreeWidgetEditor m_editor;
};

}  // namespace qdesigner_internal



#endif // TREEWIDGETEDITOR_H
