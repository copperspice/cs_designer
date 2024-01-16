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

#ifndef ITEMVIEW_FINDWIDGET_H
#define ITEMVIEW_FINDWIDGET_H

#include <abstract_findwidget.h>

#include <QModelIndex>

class QAbstractItemView;

class ItemViewFindWidget : public AbstractFindWidget
{
   CS_OBJECT(ItemViewFindWidget)

 public:
   explicit ItemViewFindWidget(FindFlags flags = FindFlags(), QWidget *parent = nullptr);

   QAbstractItemView *itemView() const {
      return m_itemView;
   }

   void setItemView(QAbstractItemView *itemView);

 protected:
   void deactivate() override;
   void find(const QString &textToFind, bool skipCurrent, bool backward,
         bool *found, bool *wrapped) override;

 private:
   QModelIndex findHelper(const QString &textToFind, bool skipCurrent, bool backward,
         QModelIndex parent, int row, int column);

   QAbstractItemView *m_itemView;
};

#endif
