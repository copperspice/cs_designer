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

#include <abstract_formbuilder.h>
#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <designer_command.h>
#include <designer_property.h>
#include <designer_utils.h>
#include <property_view_tree.h>
#include <tablewidget_editor.h>

#include <formwindowbase_p.h>
#include <iconloader_p.h>

#include <QDir>
#include <QQueue>
#include <QTextStream>

namespace qdesigner_internal {

TableWidgetEditor::TableWidgetEditor(QDesignerFormWindowInterface *form, QDialog *dialog)
   : AbstractItemEditor(form, nullptr), m_updatingBrowser(false)
{
   m_columnEditor = new ItemListEditor(form, this);
   m_columnEditor->setObjectName("columnEditor");
   m_columnEditor->setNewItemText(tr("New Column"));

   m_rowEditor = new ItemListEditor(form, this);
   m_rowEditor->setObjectName("rowEditor");
   m_rowEditor->setNewItemText(tr("New Row"));
   ui.setupUi(dialog);

   injectPropertyBrowser(ui.itemsTab, ui.widget);

   connect(ui.showPropertiesButton, &QAbstractButton::clicked,
         this, &TableWidgetEditor::togglePropertyBrowser);

   setPropertyBrowserVisible(false);

   ui.tabWidget->insertTab(0, m_columnEditor, tr("&Columns"));
   ui.tabWidget->insertTab(1, m_rowEditor, tr("&Rows"));
   ui.tabWidget->setCurrentIndex(0);
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

   connect(iconCache(), &DesignerIconCache::reloaded, this, &TableWidgetEditor::cacheReloaded);

   connect(ui.tableWidget, &QTableWidget::currentCellChanged,
         this, &TableWidgetEditor::table_currentCellChanged);

   connect(ui.tableWidget, &QTableWidget::itemChanged,
         this, &TableWidgetEditor::table_itemChanged);

   connect(m_columnEditor, &ItemListEditor::indexChanged,
         this, &TableWidgetEditor::column_indexChanged);

   connect(m_columnEditor, &ItemListEditor::itemChanged,
         this, &TableWidgetEditor::column_itemChanged);

   connect(m_columnEditor, &ItemListEditor::itemInserted,
         this, &TableWidgetEditor::column_itemInserted);

   connect(m_columnEditor, &ItemListEditor::itemDeleted,
         this, &TableWidgetEditor::column_itemDeleted);

   connect(m_columnEditor, &ItemListEditor::itemMovedUp,
         this, &TableWidgetEditor::column_itemMovedUp);

   connect(m_columnEditor, &ItemListEditor::itemMovedDown,
         this, &TableWidgetEditor::column_itemMovedDown);

   connect(m_rowEditor, &ItemListEditor::indexChanged,
         this, &TableWidgetEditor::row_indexChanged);

   connect(m_rowEditor, &ItemListEditor::itemChanged,
         this, &TableWidgetEditor::row_itemChanged);

   connect(m_rowEditor, &ItemListEditor::itemInserted,
         this, &TableWidgetEditor::row_itemInserted);

   connect(m_rowEditor, &ItemListEditor::itemDeleted,
         this, &TableWidgetEditor::row_itemDeleted);

   connect(m_rowEditor, &ItemListEditor::itemMovedUp,
         this, &TableWidgetEditor::row_itemMovedUp);

   connect(m_rowEditor, &ItemListEditor::itemMovedDown,
         this, &TableWidgetEditor::row_itemMovedDown);
}

static AbstractItemEditor::PropertyDefinition tableHeaderPropList[] = {
   { Qt::DisplayPropertyRole,       0, DesignerPropertyManager::designerStringTypeId, "text" },
   { Qt::DecorationPropertyRole,    0, DesignerPropertyManager::designerIconTypeId,   "icon" },
   { Qt::ToolTipPropertyRole,       0, DesignerPropertyManager::designerStringTypeId, "toolTip"   },
   //  { Qt::StatusTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "statusTip" },
   { Qt::WhatsThisPropertyRole,     0, DesignerPropertyManager::designerStringTypeId, "whatsThis" },
   { Qt::FontRole,                  QVariant::Font,  nullptr, "font" },
   { Qt::TextAlignmentRole,         0, DesignerPropertyManager::designerAlignmentTypeId, "textAlignment" },
   { Qt::BackgroundRole,            QVariant::Color, nullptr, "background" },
   { Qt::ForegroundRole,            QVariant::Brush, nullptr, "foreground" },
   { 0, 0, nullptr, QString() }
};

static AbstractItemEditor::PropertyDefinition tableItemPropList[] = {
   { Qt::DisplayPropertyRole,       0, DesignerPropertyManager::designerStringTypeId, "text" },
   { Qt::DecorationPropertyRole,    0, DesignerPropertyManager::designerIconTypeId,   "icon" },
   { Qt::ToolTipPropertyRole,       0, DesignerPropertyManager::designerStringTypeId, "toolTip"   },
   //  { Qt::StatusTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "statusTip" },
   { Qt::WhatsThisPropertyRole,     0, DesignerPropertyManager::designerStringTypeId, "whatsThis" },
   { Qt::FontRole,                  QVariant::Font,  nullptr, "font" },
   { Qt::TextAlignmentRole,         0, DesignerPropertyManager::designerAlignmentTypeId, "textAlignment" },
   { Qt::BackgroundRole,            QVariant::Brush, nullptr, "background" },
   { Qt::ForegroundRole,            QVariant::Brush, nullptr, "foreground" },
   { ItemFlagsShadowRole,           0, QtVariantPropertyManager::flagTypeId, "flags" },
   { Qt::CheckStateRole,            0, QtVariantPropertyManager::enumTypeId, "checkState" },
   { 0, 0, nullptr, QString() }
};

TableWidgetData TableWidgetEditor::fillContentsFromTableWidget(QTableWidget *tableWidget)
{
   TableWidgetData tblCont;
   tblCont.fromTableWidget(tableWidget, false);
   tblCont.applyToTableWidget(ui.tableWidget, iconCache(), true);

   tblCont.m_verticalHeader.applyToListWidget(m_rowEditor->listWidget(), iconCache(), true);
   m_rowEditor->setupEditor(tableWidget, tableHeaderPropList);

   tblCont.m_horizontalHeader.applyToListWidget(m_columnEditor->listWidget(), iconCache(), true);
   m_columnEditor->setupEditor(tableWidget, tableHeaderPropList);

   setupEditor(tableWidget, tableItemPropList);
   if (ui.tableWidget->columnCount() > 0 && ui.tableWidget->rowCount() > 0) {
      ui.tableWidget->setCurrentCell(0, 0);
   }

   updateEditor();

   return tblCont;
}

TableWidgetData TableWidgetEditor::contents() const
{
   TableWidgetData retVal;
   retVal.fromTableWidget(ui.tableWidget, true);

   return retVal;
}

void TableWidgetEditor::setItemData(int role, const QVariant &v)
{
   QTableWidgetItem *item = ui.tableWidget->currentItem();
   BoolBlocker block(m_updatingBrowser);

   if (!item) {
      item = new QTableWidgetItem;
      ui.tableWidget->setItem(ui.tableWidget->currentRow(), ui.tableWidget->currentColumn(), item);
   }

   QVariant newValue = v;

   if (role == Qt::FontRole && newValue.type() == QVariant::Font) {
      QFont oldFont = ui.tableWidget->font();
      QFont newFont = newValue.value<QFont>().resolve(oldFont);

      newValue = QVariant::fromValue(newFont);

      item->setData(role, QVariant()); // force the right font with the current resolve mask is set (item view bug)
   }

   item->setData(role, newValue);
}

QVariant TableWidgetEditor::getItemData(int role) const
{
   QTableWidgetItem *item = ui.tableWidget->currentItem();
   if (! item) {
      return QVariant();
   }

   return item->data(role);
}

void TableWidgetEditor::table_currentCellChanged(int currentRow, int currentCol)
{
   m_rowEditor->setCurrentIndex(currentRow);
   m_columnEditor->setCurrentIndex(currentCol);
   updateBrowser();
}

void TableWidgetEditor::table_itemChanged(QTableWidgetItem *item)
{
   if (m_updatingBrowser) {
      return;
   }

   QVariant data = item->data(Qt::DisplayPropertyRole);
   PropertySheetStringValue val = data.value<PropertySheetStringValue>();

   val.setValue(item->text());
   BoolBlocker block(m_updatingBrowser);
   item->setData(Qt::DisplayPropertyRole, QVariant::fromValue(val));

   updateBrowser();
}

void TableWidgetEditor::column_indexChanged(int col)
{
   ui.tableWidget->setCurrentCell(ui.tableWidget->currentRow(), col);
}

void TableWidgetEditor::column_itemChanged(int idx, int role, const QVariant &v)
{
   ui.tableWidget->horizontalHeaderItem(idx)->setData(role, v);
   ui.tableWidget->horizontalHeaderItem(idx)->setText(v.value<PropertySheetStringValue>().value());
}

void TableWidgetEditor::row_indexChanged(int col)
{
   ui.tableWidget->setCurrentCell(col, ui.tableWidget->currentColumn());
}

void TableWidgetEditor::row_itemChanged(int idx, int role, const QVariant &v)
{
   ui.tableWidget->verticalHeaderItem(idx)->setData(role, v);
   ui.tableWidget->verticalHeaderItem(idx)->setText(v.value<PropertySheetStringValue>().value());
}

void TableWidgetEditor::setPropertyBrowserVisible(bool v)
{
   ui.showPropertiesButton->setText(v ? tr("Properties &>>") : tr("Properties &<<"));
   m_propertyBrowser->setVisible(v);
}

void TableWidgetEditor::togglePropertyBrowser()
{
   setPropertyBrowserVisible(!m_propertyBrowser->isVisible());
}

void TableWidgetEditor::updateEditor()
{
   const bool wasEnabled = ui.tabWidget->isTabEnabled(2);
   const bool isEnabled  = ui.tableWidget->columnCount() && ui.tableWidget->rowCount();
   ui.tabWidget->setTabEnabled(2, isEnabled);

   if (! wasEnabled && isEnabled) {
      ui.tableWidget->setCurrentCell(0, 0);
   }

   QMetaObject::invokeMethod(ui.tableWidget, "updateGeometries");
   ui.tableWidget->viewport()->update();
}

void TableWidgetEditor::moveColumnsLeft(int fromColumn, int toColumn)
{
   if (fromColumn >= toColumn) {
      return;
   }

   QTableWidgetItem *lastItem = ui.tableWidget->takeHorizontalHeaderItem(toColumn);
   for (int i = toColumn; i > fromColumn; i--) {
      ui.tableWidget->setHorizontalHeaderItem(i,
         ui.tableWidget->takeHorizontalHeaderItem(i - 1));
   }
   ui.tableWidget->setHorizontalHeaderItem(fromColumn, lastItem);

   for (int i = 0; i < ui.tableWidget->rowCount(); i++) {
      QTableWidgetItem *lastItem = ui.tableWidget->takeItem(i, toColumn);
      for (int j = toColumn; j > fromColumn; j--) {
         ui.tableWidget->setItem(i, j, ui.tableWidget->takeItem(i, j - 1));
      }
      ui.tableWidget->setItem(i, fromColumn, lastItem);
   }
}

void TableWidgetEditor::moveColumnsRight(int fromColumn, int toColumn)
{
   if (fromColumn >= toColumn) {
      return;
   }

   QTableWidgetItem *lastItem = ui.tableWidget->takeHorizontalHeaderItem(fromColumn);
   for (int i = fromColumn; i < toColumn; i++) {
      ui.tableWidget->setHorizontalHeaderItem(i,
         ui.tableWidget->takeHorizontalHeaderItem(i + 1));
   }
   ui.tableWidget->setHorizontalHeaderItem(toColumn, lastItem);

   for (int i = 0; i < ui.tableWidget->rowCount(); i++) {
      QTableWidgetItem *lastItem = ui.tableWidget->takeItem(i, fromColumn);
      for (int j = fromColumn; j < toColumn; j++) {
         ui.tableWidget->setItem(i, j, ui.tableWidget->takeItem(i, j + 1));
      }
      ui.tableWidget->setItem(i, toColumn, lastItem);
   }
}

void TableWidgetEditor::moveRowsDown(int fromRow, int toRow)
{
   if (fromRow >= toRow) {
      return;
   }

   QTableWidgetItem *lastItem = ui.tableWidget->takeVerticalHeaderItem(toRow);
   for (int i = toRow; i > fromRow; i--) {
      ui.tableWidget->setVerticalHeaderItem(i,
         ui.tableWidget->takeVerticalHeaderItem(i - 1));
   }
   ui.tableWidget->setVerticalHeaderItem(fromRow, lastItem);

   for (int i = 0; i < ui.tableWidget->columnCount(); i++) {
      QTableWidgetItem *lastItem = ui.tableWidget->takeItem(toRow, i);
      for (int j = toRow; j > fromRow; j--) {
         ui.tableWidget->setItem(j, i, ui.tableWidget->takeItem(j - 1, i));
      }
      ui.tableWidget->setItem(fromRow, i, lastItem);
   }
}

void TableWidgetEditor::moveRowsUp(int fromRow, int toRow)
{
   if (fromRow >= toRow) {
      return;
   }

   QTableWidgetItem *lastItem = ui.tableWidget->takeVerticalHeaderItem(fromRow);
   for (int i = fromRow; i < toRow; i++) {
      ui.tableWidget->setVerticalHeaderItem(i,
         ui.tableWidget->takeVerticalHeaderItem(i + 1));
   }
   ui.tableWidget->setVerticalHeaderItem(toRow, lastItem);

   for (int i = 0; i < ui.tableWidget->columnCount(); i++) {
      QTableWidgetItem *lastItem = ui.tableWidget->takeItem(fromRow, i);
      for (int j = fromRow; j < toRow; j++) {
         ui.tableWidget->setItem(j, i, ui.tableWidget->takeItem(j + 1, i));
      }
      ui.tableWidget->setItem(toRow, i, lastItem);
   }
}

void TableWidgetEditor::column_itemInserted(int idx)
{
   const int columnCount = ui.tableWidget->columnCount();
   ui.tableWidget->setColumnCount(columnCount + 1);

   QTableWidgetItem *newItem = new QTableWidgetItem(m_columnEditor->newItemText());
   newItem->setData(Qt::DisplayPropertyRole, QVariant::fromValue(PropertySheetStringValue(m_columnEditor->newItemText())));
   ui.tableWidget->setHorizontalHeaderItem(columnCount, newItem);

   moveColumnsLeft(idx, columnCount);

   int row = ui.tableWidget->currentRow();
   if (row >= 0) {
      ui.tableWidget->setCurrentCell(row, idx);
   }

   updateEditor();
}

void TableWidgetEditor::column_itemDeleted(int idx)
{
   const int columnCount = ui.tableWidget->columnCount();

   moveColumnsRight(idx, columnCount - 1);
   ui.tableWidget->setColumnCount(columnCount - 1);

   updateEditor();
}

void TableWidgetEditor::column_itemMovedUp(int idx)
{
   moveColumnsRight(idx - 1, idx);

   ui.tableWidget->setCurrentCell(ui.tableWidget->currentRow(), idx - 1);
}

void TableWidgetEditor::column_itemMovedDown(int idx)
{
   moveColumnsLeft(idx, idx + 1);

   ui.tableWidget->setCurrentCell(ui.tableWidget->currentRow(), idx + 1);
}

void TableWidgetEditor::row_itemInserted(int idx)
{
   const int rowCount = ui.tableWidget->rowCount();
   ui.tableWidget->setRowCount(rowCount + 1);

   QTableWidgetItem *newItem = new QTableWidgetItem(m_rowEditor->newItemText());
   newItem->setData(Qt::DisplayPropertyRole, QVariant::fromValue(PropertySheetStringValue(m_rowEditor->newItemText())));
   ui.tableWidget->setVerticalHeaderItem(rowCount, newItem);

   moveRowsDown(idx, rowCount);

   int col = ui.tableWidget->currentColumn();
   if (col >= 0) {
      ui.tableWidget->setCurrentCell(idx, col);
   }

   updateEditor();
}

void TableWidgetEditor::row_itemDeleted(int idx)
{
   const int rowCount = ui.tableWidget->rowCount();

   moveRowsUp(idx, rowCount - 1);
   ui.tableWidget->setRowCount(rowCount - 1);

   updateEditor();
}

void TableWidgetEditor::row_itemMovedUp(int idx)
{
   moveRowsUp(idx - 1, idx);

   ui.tableWidget->setCurrentCell(idx - 1, ui.tableWidget->currentColumn());
}

void TableWidgetEditor::row_itemMovedDown(int idx)
{
   moveRowsDown(idx, idx + 1);

   ui.tableWidget->setCurrentCell(idx + 1, ui.tableWidget->currentColumn());
}

void TableWidgetEditor::cacheReloaded()
{
   reloadIconResources(iconCache(), ui.tableWidget);
}

TableWidgetEditorDialog::TableWidgetEditorDialog(QDesignerFormWindowInterface *form, QWidget *parent) :
   QDialog(parent), m_editor(form, this)
{
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

TableWidgetData TableWidgetEditorDialog::fillContentsFromTableWidget(QTableWidget *tableWidget)
{
   return m_editor.fillContentsFromTableWidget(tableWidget);
}

TableWidgetData TableWidgetEditorDialog::contents() const
{
   return m_editor.contents();
}

}   // end namespace qdesigner_internal


