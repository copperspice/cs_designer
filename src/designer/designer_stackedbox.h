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

#ifndef QDESIGNER_STACKEDBOX_H
#define QDESIGNER_STACKEDBOX_H

#include <designer_propertysheet.h>

class QStackedWidget;
class QWidget;
class QAction;
class QMenu;
class QToolButton;

namespace qdesigner_internal {
class PromotionTaskMenu;
}

// Event filter to be installed on a QStackedWidget in preview mode.
// Create two buttons to switch pages.

class QStackedWidgetPreviewEventFilter : public QObject
{
   CS_OBJECT(QStackedWidgetPreviewEventFilter)
 public:
   explicit QStackedWidgetPreviewEventFilter(QStackedWidget *parent);

   // Install helper on QStackedWidget
   static void install(QStackedWidget *stackedWidget);
   bool eventFilter(QObject *watched, QEvent *event);

   void setButtonToolTipEnabled(bool v) {
      m_buttonToolTipEnabled = v;
   }
   bool buttonToolTipEnabled() const    {
      return m_buttonToolTipEnabled;
   }

 public :
   CS_SLOT_1(Public, void updateButtons())
   CS_SLOT_2(updateButtons)
   CS_SLOT_1(Public, void prevPage())
   CS_SLOT_2(prevPage)
   CS_SLOT_1(Public, void nextPage())
   CS_SLOT_2(nextPage)

 protected:
   QStackedWidget *stackedWidget() const {
      return m_stackedWidget;
   }
   virtual void gotoPage(int page);

 private:
   void updateButtonToolTip(QObject *o);

   bool m_buttonToolTipEnabled;
   QStackedWidget *m_stackedWidget;
   QToolButton *m_prev;
   QToolButton *m_next;
};

// Event filter to be installed on a QStackedWidget in editing mode.
//  In addition to the browse buttons, handles context menu and everything

class QStackedWidgetEventFilter : public QStackedWidgetPreviewEventFilter
{
   CS_OBJECT(QStackedWidgetEventFilter)

 public:
   explicit QStackedWidgetEventFilter(QStackedWidget *parent);

   // Install helper on QStackedWidget
   static void install(QStackedWidget *stackedWidget);
   static QStackedWidgetEventFilter *eventFilterOf(const QStackedWidget *stackedWidget);
   // Convenience to add a menu on a tackedWidget
   static QMenu *addStackedWidgetContextMenuActions(const QStackedWidget *stackedWidget, QMenu *popup);

   // Add context menu and return page submenu or 0.
   QMenu *addContextMenuActions(QMenu *popup);

 protected:
   void gotoPage(int page) override;

 private:
   CS_SLOT_1(Private, void removeCurrentPage())
   CS_SLOT_2(removeCurrentPage)
   CS_SLOT_1(Private, void addPage())
   CS_SLOT_2(addPage)
   CS_SLOT_1(Private, void addPageAfter())
   CS_SLOT_2(addPageAfter)
   CS_SLOT_1(Private, void changeOrder())
   CS_SLOT_2(changeOrder)

   QAction *m_actionPreviousPage;
   QAction *m_actionNextPage;
   QAction *m_actionDeletePage;
   QAction *m_actionInsertPage;
   QAction *m_actionInsertPageAfter;
   QAction *m_actionChangePageOrder;
   qdesigner_internal::PromotionTaskMenu *m_pagePromotionTaskMenu;
};

// PropertySheet to handle the "currentPageName" property
class QStackedWidgetPropertySheet : public QDesignerPropertySheet
{
 public:
   explicit QStackedWidgetPropertySheet(QStackedWidget *object, QObject *parent = nullptr);

   void setProperty(int index, const QVariant &value) override;
   QVariant property(int index) const override;
   bool reset(int index) override;
   bool isEnabled(int index) const override;

   // Check whether the property is to be saved. Returns false for the page
   // properties (as the property sheet has no concept of 'stored')
   static bool checkProperty(const QString &propertyName);

 private:
   QStackedWidget *m_stackedWidget;
};

typedef QDesignerPropertySheetFactory<QStackedWidget, QStackedWidgetPropertySheet> QStackedWidgetPropertySheetFactory;

#endif
