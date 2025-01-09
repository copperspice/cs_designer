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

#ifndef QDESIGNER_TOOLBOX_H
#define QDESIGNER_TOOLBOX_H

#include <designer_propertysheet.h>
#include <designer_utils.h>

#include <QPalette>

namespace qdesigner_internal {
class PromotionTaskMenu;
}

class QToolBox;
class QAction;
class QMenu;

class QToolBoxHelper : public QObject
{
   CS_OBJECT(QToolBoxHelper)

   explicit QToolBoxHelper(QToolBox *toolbox);

 public:
   // Install helper on QToolBox
   static void install(QToolBox *toolbox);
   static QToolBoxHelper *helperOf(const QToolBox *toolbox);

   // Convenience to add a menu on a toolbox
   static QMenu *addToolBoxContextMenuActions(const QToolBox *toolbox, QMenu *popup);

   QPalette::ColorRole currentItemBackgroundRole() const;
   void setCurrentItemBackgroundRole(QPalette::ColorRole role);

   bool eventFilter(QObject *watched, QEvent *event) override;
   // Add context menu and return page submenu or 0.

   QMenu *addContextMenuActions(QMenu *popup) const;

 private:
   CS_SLOT_1(Private, void removeCurrentPage())
   CS_SLOT_2(removeCurrentPage)

   CS_SLOT_1(Private, void addPage())
   CS_SLOT_2(addPage)

   CS_SLOT_1(Private, void addPageAfter())
   CS_SLOT_2(addPageAfter)

   CS_SLOT_1(Private, void changeOrder())
   CS_SLOT_2(changeOrder)

   QToolBox *m_toolbox;
   QAction *m_actionDeletePage;
   QAction *m_actionInsertPage;
   QAction *m_actionInsertPageAfter;
   QAction *m_actionChangePageOrder;

   qdesigner_internal::PromotionTaskMenu *m_pagePromotionTaskMenu;
};

// PropertySheet to handle the page properties
class QToolBoxWidgetPropertySheet : public QDesignerPropertySheet
{
 public:
   explicit QToolBoxWidgetPropertySheet(QToolBox *object, QObject *parent = nullptr);

   void setProperty(int index, const QVariant &value) override;
   QVariant property(int index) const override;
   bool reset(int index) override;
   bool isEnabled(int index) const override;

   // Check whether the property is to be saved. Returns false for the page
   // properties (as the property sheet has no concept of 'stored')
   static bool checkProperty(const QString &propertyName);

 private:
   enum ToolBoxProperty { PropertyCurrentItemText, PropertyCurrentItemName, PropertyCurrentItemIcon,
      PropertyCurrentItemToolTip,  PropertyTabSpacing, PropertyToolBoxNone
   };

   static ToolBoxProperty toolBoxPropertyFromName(const QString &name);
   QToolBox *m_toolBox;
   struct PageData {
      qdesigner_internal::PropertySheetStringValue text;
      qdesigner_internal::PropertySheetStringValue tooltip;
      qdesigner_internal::PropertySheetIconValue icon;
   };
   QMap<QWidget *, PageData> m_pageToData;
};

typedef QDesignerPropertySheetFactory<QToolBox, QToolBoxWidgetPropertySheet> QToolBoxWidgetPropertySheetFactory;

#endif
