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

#include <itemview_findwidget.h>

#include <QAbstractItemView>
#include <QCheckBox>
#include <QTreeView>
#include <QRegularExpression>

#include <algorithm>

ItemViewFindWidget::ItemViewFindWidget(FindFlags flags, QWidget *parent)
   : AbstractFindWidget(flags, parent), m_itemView(nullptr)
{
}

void ItemViewFindWidget::setItemView(QAbstractItemView *itemView)
{
   if (m_itemView) {
      m_itemView->removeEventFilter(this);
   }

   m_itemView = itemView;

   if (m_itemView) {
      m_itemView->installEventFilter(this);
   }
}

void ItemViewFindWidget::deactivate()
{
   if (m_itemView) {
      m_itemView->setFocus();
   }

   AbstractFindWidget::deactivate();
}

// Sorting is needed to find the start/end of the selection.
// This is utter black magic. And it is damn slow.
static bool indexLessThan(const QModelIndex &a, const QModelIndex &b)
{
   // First determine the nesting of each index in the tree.
   QModelIndex aa = a;
   int aDepth = 0;

   while (aa.parent() != QModelIndex()) {
      // As a side effect, check if one of the items is the parent of the other.
      // Children are always displayed below their parents, so sort them further down.
      if (aa.parent() == b) {
         return true;
      }
      aa = aa.parent();
      aDepth++;
   }
   QModelIndex ba = b;
   int bDepth = 0;
   while (ba.parent() != QModelIndex()) {
      if (ba.parent() == a) {
         return false;
      }
      ba = ba.parent();
      bDepth++;
   }
   // Now find indices at comparable depth.
   for (aa = a; aDepth > bDepth; aDepth--) {
      aa = aa.parent();
   }
   for (ba = b; aDepth < bDepth; bDepth--) {
      ba = ba.parent();
   }
   // If they have the same parent, sort them within a top-to-bottom, left-to-right rectangle.
   if (aa.parent() == ba.parent()) {
      if (aa.row() < ba.row()) {
         return true;
      }
      if (aa.row() > ba.row()) {
         return false;
      }
      return aa.column() < ba.column();
   }
   // Now try to find indices that have the same grandparent. This ends latest at the root node.
   while (aa.parent().parent() != ba.parent().parent()) {
      aa = aa.parent();
      ba = ba.parent();
   }
   // A bigger row is always displayed further down.
   if (aa.parent().row() < ba.parent().row()) {
      return true;
   }
   if (aa.parent().row() > ba.parent().row()) {
      return false;
   }
   // Here's the trick: a child spawned from a bigger column is displayed further *up*.
   // That's because the tree lines are on the left and are supposed not to cross each other.
   // This case is mostly academical, as "all" models spawn children from the first column.
   return aa.parent().column() > ba.parent().column();
}

void ItemViewFindWidget::find(const QString &ttf, bool skipCurrent, bool backward, bool *found, bool *wrapped)
{
   if (!m_itemView || !m_itemView->model()->hasChildren()) {
      return;
   }

   QModelIndex idx;
   if (skipCurrent && m_itemView->selectionModel()->hasSelection()) {
      QModelIndexList il = m_itemView->selectionModel()->selectedIndexes();
      std::sort(il.begin(), il.end(), indexLessThan);
      idx = backward ? il.first() : il.last();
   } else {
      idx = m_itemView->currentIndex();
   }

   *found = true;
   QModelIndex newIdx = idx;

   if (!ttf.isEmpty()) {
      if (newIdx.isValid()) {
         int column = newIdx.column();
         if (skipCurrent)
            if (QTreeView *tv = dynamic_cast<QTreeView *>(m_itemView))
               if (tv->allColumnsShowFocus()) {
                  column = backward ? 0 : m_itemView->model()->columnCount(newIdx.parent()) - 1;
               }
         newIdx = findHelper(ttf, skipCurrent, backward,
               newIdx.parent(), newIdx.row(), column);
      }
      if (!newIdx.isValid()) {
         int row = backward ? m_itemView->model()->rowCount() : 0;
         int column = backward ? 0 : -1;
         newIdx = findHelper(ttf, true, backward, m_itemView->rootIndex(), row, column);
         if (!newIdx.isValid()) {
            *found = false;
            newIdx = idx;
         } else {
            *wrapped = true;
         }
      }
   }

   if (!isVisible()) {
      show();
   }

   m_itemView->setCurrentIndex(newIdx);
}

// The traversal order is described in the indexLessThan() comments above.

static inline bool skipForward(const QAbstractItemModel *model, QModelIndex &parent, int &row, int &column)
{
   while (true) {
      ++column;

      if (column < model->columnCount(parent)) {
         return true;
      }

      while (true) {
         while (--column >= 0) {
            QModelIndex nIdx = model->index(row, column, parent);

            if (nIdx.isValid()) {
               if (model->hasChildren(nIdx)) {
                  row = 0;
                  column = 0;
                  parent = nIdx;
                  return true;
               }
            }
         }

         if (++row < model->rowCount(parent)) {
            break;
         }

         if (!parent.isValid()) {
            return false;
         }

         row = parent.row();
         column = parent.column();
         parent = parent.parent();
      }
   }
}

static inline bool skipBackward(const QAbstractItemModel *model, QModelIndex &parent, int &row, int &column)
{
   --column;

   if (column == -1) {
      if (--row < 0) {
         if (!parent.isValid()) {
            return false;
         }
         row = parent.row();
         column = parent.column();
         parent = parent.parent();
      }

      while (++column < model->columnCount(parent)) {
         QModelIndex nIdx = model->index(row, column, parent);
         if (nIdx.isValid()) {
            if (model->hasChildren(nIdx)) {
               row = model->rowCount(nIdx) - 1;
               column = -1;
               parent = nIdx;
            }
         }
      }
      column--;
   }

   return true;
}

// QAbstractItemModel::match() does not support backwards searching. Still using it would
// be just a bit inefficient (not much worse than when no match is found).
// The bigger problem is that QAbstractItemView does not provide a method to sort a
// set of indices in traversal order (to find the start and end of the selection).
// Consequently, we do everything by ourselves to be consistent. Of course, this puts
// constraints on the allowable visualizations.

QModelIndex ItemViewFindWidget::findHelper(const QString &textToFind, bool skipCurrent, bool backward,
   QModelIndex parent, int row, int column)
{
   const QAbstractItemModel *model = m_itemView->model();

   while (true) {
      if (skipCurrent) {
         if (backward) {
            if (! skipBackward(model, parent, row, column)) {
               return QModelIndex();
            }
         } else {
            if (! skipForward(model, parent, row, column)) {
               return QModelIndex();
            }
         }
      }

      QModelIndex idx = model->index(row, column, parent);

      if (idx.isValid()) {

         if (wholeWords()) {
            QString rx = "\\b" + QRegularExpression::escape(textToFind) + "\\b";

            QPatternOption cs = caseSensitive() ? QPatternOption::NoPatternOption : QPatternOption::CaseInsensitiveOption;

            if (idx.data().toString().indexOf(QRegularExpression(rx, cs)) >= 0) {
               return idx;
            }

         } else {
            auto cs = caseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive;

            if (idx.data().toString().indexOf(textToFind, 0, cs) >= 0) {
               return idx;
            }
         }
      }

      skipCurrent = true;
   }
}

