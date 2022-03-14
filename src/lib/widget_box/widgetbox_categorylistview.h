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

#ifndef WIDGETBOX_CATEGORYLISTVIEW_H
#define WIDGETBOX_CATEGORYLISTVIEW_H

#include <abstract_widgetbox.h>

class QDesignerFormEditorInterface;
class QDesignerDnDItemInterface;
class QSortFilterProxyModel;

#include <QListView>
#include <QList>
#include <QRegularExpression>

namespace qdesigner_internal {

class WidgetBoxCategoryModel;

// List view of a category, switchable between icon and list mode.
// Provides a filtered view.
class WidgetBoxCategoryListView : public QListView
{
   CS_OBJECT(WidgetBoxCategoryListView)
 public:
   // Whether to access the filtered or unfiltered view
   enum AccessMode { FilteredAccess, UnfilteredAccess };

   explicit WidgetBoxCategoryListView(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
   void setViewMode(ViewMode vm);

   void dropWidgets(const QList<QDesignerDnDItemInterface *> &item_list);

   using QListView::contentsSize;

   // These methods operate on the filtered/unfiltered model according to accessmode
   int count(AccessMode am) const;
   QDesignerWidgetBoxInterface::Widget widgetAt(AccessMode am, const QModelIndex &index) const;
   QDesignerWidgetBoxInterface::Widget widgetAt(AccessMode am, int row) const;
   void removeRow(AccessMode am, int row);
   void setCurrentItem(AccessMode am, int row);

   // These methods operate on the unfiltered model and are used for serialization
   void addWidget(const QDesignerWidgetBoxInterface::Widget &widget, const QIcon &icon, bool editable);
   bool containsWidget(const QString &name);
   QDesignerWidgetBoxInterface::Category category() const;
   bool removeCustomWidgets();

   // Helper: Ensure a <ui> tag in the case of empty XML
   static QString widgetDomXml(const QDesignerWidgetBoxInterface::Widget &widget);

 public:
   CS_SIGNAL_1(Public, void scratchPadChanged())
   CS_SIGNAL_2(scratchPadChanged)

   CS_SIGNAL_1(Public, void pressed(const QString &name, const QString &xml, const QPoint &globalPos))
   CS_SIGNAL_2(pressed, name, xml, globalPos)

   CS_SIGNAL_1(Public, void itemRemoved())
   CS_SIGNAL_2(itemRemoved)

   CS_SIGNAL_1(Public, void lastItemRemoved())
   CS_SIGNAL_2(lastItemRemoved)

   CS_SLOT_1(Public, void filter(const QRegularExpression &regex))
   CS_SLOT_2(filter)

   CS_SLOT_1(Public, void removeCurrentItem())
   CS_SLOT_2(removeCurrentItem)

   CS_SLOT_1(Public, void editCurrentItem())
   CS_SLOT_2(editCurrentItem)

 private:
   CS_SLOT_1(Private, void slotPressed(const QModelIndex &index))
   CS_SLOT_2(slotPressed)

   int mapRowToSource(int filterRow) const;
   QSortFilterProxyModel *m_proxyModel;
   WidgetBoxCategoryModel *m_model;
};

}  // namespace qdesigner_internal



#endif // WIDGETBOXCATEGORYLISTVIEW_H
