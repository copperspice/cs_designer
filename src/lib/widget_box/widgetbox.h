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

#ifndef WIDGETBOX_H
#define WIDGETBOX_H

#include <qdesigner_widgetbox_p.h>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class WidgetBoxTreeWidget;

class WidgetBox : public QDesignerWidgetBox
{
   CS_OBJECT(WidgetBox)
 public:
   explicit WidgetBox(QDesignerFormEditorInterface *core, QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
   virtual ~WidgetBox();

   QDesignerFormEditorInterface *core() const;

   int categoryCount() const override;
   Category category(int cat_idx) const override;
   void addCategory(const Category &cat) override;
   void removeCategory(int cat_idx) override;

   int widgetCount(int cat_idx) const override;
   Widget widget(int cat_idx, int wgt_idx) const override;
   void addWidget(int cat_idx, const Widget &wgt) override;
   void removeWidget(int cat_idx, int wgt_idx) override;

   void dropWidgets(const QList<QDesignerDnDItemInterface *> &item_list, const QPoint &global_mouse_pos) override;

   void setFileName(const QString &file_name) override;
   QString fileName() const override;
   bool load() override;
   bool save() override;

   bool loadContents(const QString &contents) override;
   QIcon iconForWidget(const QString &className, const QString &category = QString()) const override;

 protected:
   void dragEnterEvent (QDragEnterEvent *event) override;
   void dragMoveEvent(QDragMoveEvent *event) override;
   void dropEvent (QDropEvent *event) override;

 private:
   CS_SLOT_1(Private, void handleMousePress(const QString &name, const QString &xml, const QPoint &global_mouse_pos))
   CS_SLOT_2(handleMousePress)

 private:
   QDesignerFormEditorInterface *m_core;
   WidgetBoxTreeWidget *m_view;
};

}  // namespace qdesigner_internal



#endif // WIDGETBOX_H
