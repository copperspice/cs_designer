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

#ifndef TABORDEREDITOR_H
#define TABORDEREDITOR_H

#include <QFont>
#include <QFontMetrics>
#include <QPointer>
#include <QRegion>
#include <QWidget>

class QDesignerFormWindowInterface;

class QUndoStack;

namespace qdesigner_internal {

class TabOrderEditor : public QWidget
{
   CS_OBJECT(TabOrderEditor)

 public:
   TabOrderEditor(QDesignerFormWindowInterface *form, QWidget *parent);

   QDesignerFormWindowInterface *formWindow() const;

   CS_SLOT_1(Public, void setBackground(QWidget *background))
   CS_SLOT_2(setBackground)

   CS_SLOT_1(Public, void updateBackground())
   CS_SLOT_2(updateBackground)

   CS_SLOT_1(Public, void widgetRemoved(QWidget *un_named_arg1))
   CS_SLOT_2(widgetRemoved)

   CS_SLOT_1(Public, void initTabOrder())
   CS_SLOT_2(initTabOrder)

 protected:
   void paintEvent(QPaintEvent *e) override;
   void mouseMoveEvent(QMouseEvent *e) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseDoubleClickEvent(QMouseEvent *e) override;
   void contextMenuEvent(QContextMenuEvent *e) override;
   void resizeEvent(QResizeEvent *e) override;
   void showEvent(QShowEvent *e) override;

 private:
   CS_SLOT_1(Private, void showTabOrderDialog())
   CS_SLOT_2(showTabOrderDialog)

   QRect indicatorRect(int index) const;
   int widgetIndexAt(const QPoint &pos) const;
   bool skipWidget(QWidget *w) const;

   QPointer<QDesignerFormWindowInterface> m_form_window;

   QWidgetList m_tab_order_list;

   QWidget *m_bg_widget;
   QUndoStack *m_undo_stack;
   QRegion m_indicator_region;

   QFontMetrics m_font_metrics;
   int m_current_index;
   bool m_beginning;
};

}  // namespace qdesigner_internal

#endif
