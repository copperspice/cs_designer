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

#ifndef TREE_PROPERTYBROWSER_H
#define TREE_PROPERTYBROWSER_H

#include <propertybrowser.h>

class QTreeWidgetItem;
class QtTreePropertyBrowserPrivate;

class QtTreePropertyBrowser : public QtAbstractPropertyBrowser
{
   CS_OBJECT(QtTreePropertyBrowser)

   CS_ENUM(ResizeMode)

   CS_PROPERTY_READ(indentation, indentation)
   CS_PROPERTY_WRITE(indentation, setIndentation)
   CS_PROPERTY_READ(rootIsDecorated, rootIsDecorated)
   CS_PROPERTY_WRITE(rootIsDecorated, setRootIsDecorated)
   CS_PROPERTY_READ(alternatingRowColors, alternatingRowColors)
   CS_PROPERTY_WRITE(alternatingRowColors, setAlternatingRowColors)
   CS_PROPERTY_READ(headerVisible, isHeaderVisible)
   CS_PROPERTY_WRITE(headerVisible, setHeaderVisible)
   CS_PROPERTY_READ(resizeMode, resizeMode)
   CS_PROPERTY_WRITE(resizeMode, setResizeMode)
   CS_PROPERTY_READ(splitterPosition, splitterPosition)
   CS_PROPERTY_WRITE(splitterPosition, setSplitterPosition)
   CS_PROPERTY_READ(propertiesWithoutValueMarked, propertiesWithoutValueMarked)
   CS_PROPERTY_WRITE(propertiesWithoutValueMarked, setPropertiesWithoutValueMarked)

 public:

   enum ResizeMode {
      Interactive,
      Stretch,
      Fixed,
      ResizeToContents
   };

   QtTreePropertyBrowser(QWidget *parent = nullptr);
   ~QtTreePropertyBrowser();

   int indentation() const;
   void setIndentation(int i);

   bool rootIsDecorated() const;
   void setRootIsDecorated(bool show);

   bool alternatingRowColors() const;
   void setAlternatingRowColors(bool enable);

   bool isHeaderVisible() const;
   void setHeaderVisible(bool visible);

   ResizeMode resizeMode() const;
   void setResizeMode(ResizeMode mode);

   int splitterPosition() const;
   void setSplitterPosition(int position);

   void setExpanded(QtBrowserItem *item, bool expanded);
   bool isExpanded(QtBrowserItem *item) const;

   bool isItemVisible(QtBrowserItem *item) const;
   void setItemVisible(QtBrowserItem *item, bool visible);

   void setBackgroundColor(QtBrowserItem *item, const QColor &color);
   QColor backgroundColor(QtBrowserItem *item) const;
   QColor calculatedBackgroundColor(QtBrowserItem *item) const;

   void setPropertiesWithoutValueMarked(bool mark);
   bool propertiesWithoutValueMarked() const;

   void editItem(QtBrowserItem *item);

   CS_SIGNAL_1(Public, void collapsed(QtBrowserItem *item))
   CS_SIGNAL_2(collapsed, item)

   CS_SIGNAL_1(Public, void expanded(QtBrowserItem *item))
   CS_SIGNAL_2(expanded, item)

 protected:
   virtual void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem);
   virtual void itemRemoved(QtBrowserItem *item);
   virtual void itemChanged(QtBrowserItem *item);

 private:
   QScopedPointer<QtTreePropertyBrowserPrivate> d_ptr;

   Q_DECLARE_PRIVATE(QtTreePropertyBrowser)
   Q_DISABLE_COPY(QtTreePropertyBrowser)

   CS_SLOT_1(Private, void slotCollapsed(const QModelIndex &un_named_arg1))
   CS_SLOT_2(slotCollapsed)

   CS_SLOT_1(Private, void slotExpanded(const QModelIndex &un_named_arg1))
   CS_SLOT_2(slotExpanded)

   CS_SLOT_1(Private, void slotCurrentBrowserItemChanged(QtBrowserItem *un_named_arg1))
   CS_SLOT_2(slotCurrentBrowserItemChanged)

   CS_SLOT_1(Private, void slotCurrentTreeItemChanged(QTreeWidgetItem *un_named_arg1, QTreeWidgetItem *un_named_arg2))
   CS_SLOT_2(slotCurrentTreeItemChanged)
};

#endif

