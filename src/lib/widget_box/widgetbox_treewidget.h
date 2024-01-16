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

#ifndef WIDGETBOX_TREEWIDGET_H
#define WIDGETBOX_TREEWIDGET_H

#include <designer_widgetbox.h>

class QDesignerFormEditorInterface;
class QDesignerDnDItemInterface;

#include <QTreeWidget>
#include <QIcon>
#include <QList>
#include <QHash>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class QTimer;

namespace qdesigner_internal {

class WidgetBoxCategoryListView;

class WidgetBoxTreeWidget : public QTreeWidget
{
   CS_OBJECT(WidgetBoxTreeWidget)

 public:
   typedef QDesignerWidgetBoxInterface::Widget Widget;
   typedef QDesignerWidgetBoxInterface::Category Category;
   typedef QDesignerWidgetBoxInterface::CategoryList CategoryList;

   explicit WidgetBoxTreeWidget(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
   ~WidgetBoxTreeWidget();

   int categoryCount() const;
   Category category(int cat_idx) const;
   void addCategory(const Category &cat);
   void removeCategory(int cat_idx);

   int widgetCount(int cat_idx) const;
   Widget widget(int cat_idx, int wgt_idx) const;
   void addWidget(int cat_idx, const Widget &wgt);
   void removeWidget(int cat_idx, int wgt_idx);

   void dropWidgets(const QList<QDesignerDnDItemInterface *> &item_list);

   void setFileName(const QString &file_name);
   QString fileName() const;
   bool load(QDesignerWidgetBox::LoadMode loadMode);
   bool loadContents(const QString &contents);
   bool save();
   QIcon iconForWidget(QString iconName) const;

   CS_SIGNAL_1(Public, void pressed(const QString &name, const QString &dom_xml, const QPoint &global_mouse_pos))
   CS_SIGNAL_2(pressed, name, dom_xml, global_mouse_pos)

   CS_SLOT_1(Public, void filter(const QString &un_named_arg1))
   CS_SLOT_2(filter)

 protected:
   void contextMenuEvent(QContextMenuEvent *e) override;
   void resizeEvent(QResizeEvent *e) override;

 private:
   CS_SLOT_1(Private, void slotSave())
   CS_SLOT_2(slotSave)
   CS_SLOT_1(Private, void slotScratchPadItemDeleted())
   CS_SLOT_2(slotScratchPadItemDeleted)
   CS_SLOT_1(Private, void slotLastScratchPadItemDeleted())
   CS_SLOT_2(slotLastScratchPadItemDeleted)

   CS_SLOT_1(Private, void handleMousePress(QTreeWidgetItem *item))
   CS_SLOT_2(handleMousePress)
   CS_SLOT_1(Private, void deleteScratchpad())
   CS_SLOT_2(deleteScratchpad)
   CS_SLOT_1(Private, void slotListMode())
   CS_SLOT_2(slotListMode)
   CS_SLOT_1(Private, void slotIconMode())
   CS_SLOT_2(slotIconMode)

   WidgetBoxCategoryListView *addCategoryView(QTreeWidgetItem *parent, bool iconMode);
   WidgetBoxCategoryListView *categoryViewAt(int idx) const;
   void adjustSubListSize(QTreeWidgetItem *cat_item);

   static bool readCategories(const QString &fileName, const QString &xml, CategoryList *cats, QString *errorMessage);
   static bool readWidget(Widget *w, const QString &xml, QXmlStreamReader &r);

   CategoryList loadCustomCategoryList() const;
   void writeCategories(QXmlStreamWriter &writer, const CategoryList &cat_list) const;

   int indexOfCategory(const QString &name) const;
   int indexOfScratchpad() const;
   int ensureScratchpad();
   void addCustomCategories(bool replace);

   void saveExpandedState() const;
   void restoreExpandedState();
   void updateViewMode();

   QDesignerFormEditorInterface *m_core;
   QString m_file_name;
   typedef QHash<QString, QIcon> IconCache;
   mutable IconCache m_pluginIcons;
   bool m_iconMode;
   QTimer *m_scratchPadDeleteTimer;
};

}  // namespace qdesigner_internal

#endif
