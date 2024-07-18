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

#ifndef QDESIGNER_TABWIDGET_H
#define QDESIGNER_TABWIDGET_H

#include <designer_propertysheet.h>
#include <designer_utils.h>

#include <QIcon>
#include <QPointer>

class QDesignerFormWindowInterface;

class QAction;
class QMenu;
class QTabBar;
class QTabWidget;

namespace qdesigner_internal {
class PromotionTaskMenu;
}

class QTabWidgetEventFilter : public QObject
{
   CS_OBJECT(QTabWidgetEventFilter)

 public:
   explicit QTabWidgetEventFilter(QTabWidget *parent);
   ~QTabWidgetEventFilter();

   // Install helper on QTabWidget
   static void install(QTabWidget *tabWidget);
   static QTabWidgetEventFilter *eventFilterOf(const QTabWidget *tabWidget);

   // add a menu on a tackedWidget
   static QMenu *addTabWidgetContextMenuActions(const QTabWidget *tabWidget, QMenu *popup);

   // Add context menu and return page submenu or 0.
   QMenu *addContextMenuActions(QMenu *popup);

   bool eventFilter(QObject *o, QEvent *e) override;

   QDesignerFormWindowInterface *formWindow() const;

 private:
   CS_SLOT_1(Private, void removeCurrentPage())
   CS_SLOT_2(removeCurrentPage)

   CS_SLOT_1(Private, void addPage())
   CS_SLOT_2(addPage)

   CS_SLOT_1(Private, void addPageAfter())
   CS_SLOT_2(addPageAfter)

   int pageFromPosition(const QPoint &pos, QRect &rect) const;
   QTabBar *tabBar() const;

   QTabWidget *m_tabWidget;
   mutable QPointer<QTabBar> m_cachedTabBar;
   QPoint m_pressPoint;
   QWidget *m_dropIndicator;
   int m_dragIndex;
   QWidget *m_dragPage;
   QString m_dragLabel;
   QIcon m_dragIcon;
   bool m_mousePressed;
   QAction *m_actionDeletePage;
   QAction *m_actionInsertPage;
   QAction *m_actionInsertPageAfter;
   qdesigner_internal::PromotionTaskMenu *m_pagePromotionTaskMenu;
};

// PropertySheet to handle the page properties
class QTabWidgetPropertySheet : public QDesignerPropertySheet
{
 public:
   explicit QTabWidgetPropertySheet(QTabWidget *object, QObject *parent = nullptr);

   void setProperty(int index, const QVariant &value) override;
   QVariant property(int index) const override;
   bool reset(int index) override;
   bool isEnabled(int index) const override;

   // Check whether the property is to be saved. Returns false for the page
   // properties (as the property sheet has no concept of 'stored')
   static bool checkProperty(const QString &propertyName);

 private:
   enum TabWidgetProperty { PropertyCurrentTabText, PropertyCurrentTabName, PropertyCurrentTabIcon,
      PropertyCurrentTabToolTip, PropertyCurrentTabWhatsThis, PropertyTabWidgetNone
   };

   static TabWidgetProperty tabWidgetPropertyFromName(const QString &name);
   QTabWidget *m_tabWidget;
   struct PageData {
      qdesigner_internal::PropertySheetStringValue text;
      qdesigner_internal::PropertySheetStringValue tooltip;
      qdesigner_internal::PropertySheetStringValue whatsthis;
      qdesigner_internal::PropertySheetIconValue icon;
   };
   QMap<QWidget *, PageData> m_pageToData;
};

typedef QDesignerPropertySheetFactory<QTabWidget, QTabWidgetPropertySheet> QTabWidgetPropertySheetFactory;

#endif
