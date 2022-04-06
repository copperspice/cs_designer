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

#ifndef ITEMVIEW_PROPERTYSHEET_H
#define ITEMVIEW_PROPERTYSHEET_H

#include <designer_propertysheet.h>
#include <extensionfactory_p.h>

#include <QTreeView>
#include <QTableView>

namespace qdesigner_internal {

struct ItemViewPropertySheetPrivate;

class ItemViewPropertySheet: public QDesignerPropertySheet
{
   CS_OBJECT(ItemViewPropertySheet)

   CS_INTERFACES(QDesignerPropertySheetExtension)

 public:
   explicit ItemViewPropertySheet(QTreeView *treeViewObject, QObject *parent = nullptr);
   explicit ItemViewPropertySheet(QTableView *tableViewObject, QObject *parent = nullptr);

   ~ItemViewPropertySheet();

   QHash<QString, QString> propertyNameMap() const;

   // QDesignerPropertySheet
   QVariant property(int index) const override;
   void setProperty(int index, const QVariant &value) override;

   void setChanged(int index, bool changed) override;
   bool isChanged(int index) const override;

   bool hasReset(int index) const override;
   bool reset(int index) override;

 private:
   void initHeaderProperties(QHeaderView *hv, const QString &prefix);

   ItemViewPropertySheetPrivate *d;
};

typedef QDesignerPropertySheetFactory<QTreeView, ItemViewPropertySheet>  QTreeViewPropertySheetFactory;
typedef QDesignerPropertySheetFactory<QTableView, ItemViewPropertySheet> QTableViewPropertySheetFactory;

}  // namespace qdesigner_internal

#endif
