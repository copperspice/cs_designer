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

#include <sheet_delegate_p.h>

#include <QAbstractItemModel>
#include <QPainter>
#include <QStyle>
#include <QTreeView>

namespace qdesigner_internal {

SheetDelegate::SheetDelegate(QTreeView *view, QWidget *parent)
   : QItemDelegate(parent), m_view(view)
{
}

void SheetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   const QAbstractItemModel *model = index.model();
   Q_ASSERT(model);

   if (!model->parent(index).isValid()) {
      // this is a top-level item.
      QStyleOptionButton buttonOption;

      buttonOption.state = option.state;
#ifdef Q_OS_DARWIN
      buttonOption.state |= QStyle::State_Raised;
#endif
      buttonOption.state &= ~QStyle::State_HasFocus;

      buttonOption.rect = option.rect;
      buttonOption.palette = option.palette;
      buttonOption.features = QStyleOptionButton::None;

      painter->save();
      QColor buttonColor(230, 230, 230);
      QBrush buttonBrush = option.palette.button();
      if (!buttonBrush.gradient() && buttonBrush.texture().isNull()) {
         buttonColor = buttonBrush.color();
      }
      QColor outlineColor = buttonColor.darker(150);
      QColor highlightColor = buttonColor.lighter(130);

      // Only draw topline if the previous item is expanded
      QModelIndex previousIndex = model->index(index.row() - 1, index.column());
      bool drawTopline = (index.row() > 0 && m_view->isExpanded(previousIndex));
      int highlightOffset = drawTopline ? 1 : 0;

      QLinearGradient gradient(option.rect.topLeft(), option.rect.bottomLeft());
      gradient.setColorAt(0, buttonColor.lighter(102));
      gradient.setColorAt(1, buttonColor.darker(106));

      painter->setPen(Qt::NoPen);
      painter->setBrush(gradient);
      painter->drawRect(option.rect);
      painter->setPen(highlightColor);
      painter->drawLine(option.rect.topLeft() + QPoint(0, highlightOffset),
         option.rect.topRight() + QPoint(0, highlightOffset));
      painter->setPen(outlineColor);
      if (drawTopline) {
         painter->drawLine(option.rect.topLeft(), option.rect.topRight());
      }
      painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
      painter->restore();

      QStyleOption branchOption;
      static const int i = 9; // ### hardcoded in qcommonstyle.cpp
      QRect r = option.rect;
      branchOption.rect = QRect(r.left() + i / 2, r.top() + (r.height() - i) / 2, i, i);
      branchOption.palette = option.palette;
      branchOption.state = QStyle::State_Children;

      if (m_view->isExpanded(index)) {
         branchOption.state |= QStyle::State_Open;
      }

      m_view->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, m_view);

      // draw text
      QRect textrect = QRect(r.left() + i * 2, r.top(), r.width() - ((5 * i) / 2), r.height());
      QString text = elidedText(option.fontMetrics, textrect.width(), Qt::ElideMiddle,
            model->data(index, Qt::DisplayRole).toString());
      m_view->style()->drawItemText(painter, textrect, Qt::AlignCenter,
         option.palette, m_view->isEnabled(), text);

   } else {
      QItemDelegate::paint(painter, option, index);
   }
}

QSize SheetDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
   QStyleOptionViewItem option = opt;
   QSize sz = QItemDelegate::sizeHint(opt, index) + QSize(2, 2);
   return sz;
}

}   // end namespace qdesigner_internal

