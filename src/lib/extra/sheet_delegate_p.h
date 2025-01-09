/***********************************************************************
*
* Copyright (c) 2021-2025 Barbara Geller
* Copyright (c) 2021-2025 Ansel Sermersheim
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

#ifndef SHEET_DELEGATE_H
#define SHEET_DELEGATE_H

#include <QItemDelegate>
#include <QTreeView>

namespace qdesigner_internal {

class SheetDelegate: public QItemDelegate
{
   CS_OBJECT(SheetDelegate)

 public:
   SheetDelegate(QTreeView *view, QWidget *parent);

   void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
   QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const override;

 private:
   QTreeView *m_view;
};

}   // end namespace - qdesigner_internal

#endif