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

#ifndef ITEMLISTEDITOR_H
#define ITEMLISTEDITOR_H

#include <abstract_formwindow.h>
#include <ui_edit_itemlist.h>

class QtProperty;
class QtVariantProperty;
class QtTreePropertyBrowser;

#include <QDialog>

class QSplitter;
class QVBoxLayout;

namespace qdesigner_internal {

class DesignerIconCache;
class DesignerPropertyManager;
class DesignerEditorFactory;

// Utility class that ensures a bool is true while in scope.
// Courtesy of QBoolBlocker in qobject_p.h
class BoolBlocker
{
 public:
   inline BoolBlocker(bool &b): block(b), reset(b) {
      block = true;
   }

   inline ~BoolBlocker() {
      block = reset;
   }

 private:
   bool &block;
   bool reset;
};

class AbstractItemEditor: public QWidget
{
   CS_OBJECT(AbstractItemEditor)

 public:
   explicit AbstractItemEditor(QDesignerFormWindowInterface *form, QWidget *parent);
   ~AbstractItemEditor();

   DesignerIconCache *iconCache() const {
      return m_iconCache;
   }

   struct PropertyDefinition {
      int role;
      uint type;
      uint (*typeFunc)();
      const QString name;
   };

   CS_SLOT_1(Public, void cacheReloaded())
   CS_SLOT_2(cacheReloaded)

 protected:
   void setupProperties(PropertyDefinition *propDefs);
   void setupObject(QWidget *object);
   void setupEditor(QWidget *object, PropertyDefinition *propDefs);
   void injectPropertyBrowser(QWidget *parent, QWidget *widget);
   void updateBrowser();

   virtual void setItemData(int role, const QVariant &v) = 0;
   virtual QVariant getItemData(int role) const = 0;

   DesignerIconCache *m_iconCache;
   DesignerPropertyManager *m_propertyManager;
   DesignerEditorFactory *m_editorFactory;
   QSplitter *m_propertySplitter;
   QtTreePropertyBrowser *m_propertyBrowser;
   QList<QtVariantProperty *> m_properties;
   QList<QtVariantProperty *> m_rootProperties;
   QHash<QtVariantProperty *, int> m_propertyToRole;
   bool m_updatingBrowser;

 private:
   CS_SLOT_1(Private, void propertyChanged(QtProperty *property))
   CS_SLOT_2(propertyChanged)

   CS_SLOT_1(Private, void resetProperty(QtProperty *property))
   CS_SLOT_2(resetProperty)
};

class ItemListEditor: public AbstractItemEditor
{
   CS_OBJECT(ItemListEditor)

 public:
   explicit ItemListEditor(QDesignerFormWindowInterface *form, QWidget *parent);

   void setupEditor(QWidget *object, PropertyDefinition *propDefs);
   QListWidget *listWidget() const {
      return ui.listWidget;
   }

   void setNewItemText(const QString &str) {
      m_newItemText = str;
   }

   QString newItemText() const {
      return m_newItemText;
   }

   void setCurrentIndex(int idx);

   CS_SIGNAL_1(Public, void indexChanged(int idx))
   CS_SIGNAL_2(indexChanged, idx)

   CS_SIGNAL_1(Public, void itemChanged(int idx, int role, const QVariant &v))
   CS_SIGNAL_2(itemChanged, idx, role, v)

   CS_SIGNAL_1(Public, void itemInserted(int idx))
   CS_SIGNAL_2(itemInserted, idx)

   CS_SIGNAL_1(Public, void itemDeleted(int idx))
   CS_SIGNAL_2(itemDeleted, idx)

   CS_SIGNAL_1(Public, void itemMovedUp(int idx))
   CS_SIGNAL_2(itemMovedUp, idx)

   CS_SIGNAL_1(Public, void itemMovedDown(int idx))
   CS_SIGNAL_2(itemMovedDown, idx)

 protected:
   void setItemData(int role, const QVariant &v) override;
   QVariant getItemData(int role) const override;

 private:
   // slots
   void newListItemButton();
   void deleteListItemButton();
   void moveListItemUpButton();
   void moveListItemDownButton();

   CS_SLOT_1(Private, void on_listWidget_currentRowChanged(int))   // broom - keep as slot macro for now
   CS_SLOT_2(on_listWidget_currentRowChanged)

   // slots
   void listWidget_itemChanged(QListWidgetItem *item);
   void togglePropertyBrowser();
   void cacheReloaded();

   void setPropertyBrowserVisible(bool v);
   void updateEditor();

   Ui::ItemListEditor ui;
   bool m_updating;
   QString m_newItemText;
};

}   // end namespace qdesigner_internal

#endif
