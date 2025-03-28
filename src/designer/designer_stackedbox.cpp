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

#include <abstract_formwindow.h>
#include <designer_command.h>
#include <designer_property_command.h>
#include <designer_stackedbox.h>
#include <order_dialog.h>
#include <promotion_taskmenu.h>
#include <widgetfactory.h>

#include <QAction>
#include <QEvent>
#include <QMenu>
#include <QStackedWidget>
#include <QToolButton>

static QToolButton *createToolButton(QWidget *parent, Qt::ArrowType at, const QString &name)
{
   QToolButton *rc =  new QToolButton();
   rc->setAttribute(Qt::WA_NoChildEventsForParent, true);
   rc->setParent(parent);
   rc->setObjectName(name);
   rc->setArrowType(at);
   rc->setAutoRaise(true);
   rc->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
   rc->setFixedSize(QSize(15, 15));
   return rc;
}

// ---------------  QStackedWidgetPreviewEventFilter
QStackedWidgetPreviewEventFilter::QStackedWidgetPreviewEventFilter(QStackedWidget *parent)
   : QObject(parent), m_buttonToolTipEnabled(false), // Not on preview
     m_stackedWidget(parent),
     m_prev(createToolButton(m_stackedWidget, Qt::LeftArrow,  "__qt__passive_prev")),
     m_next(createToolButton(m_stackedWidget, Qt::RightArrow, "__qt__passive_next"))
{
   connect(m_prev, &QAbstractButton::clicked, this, &QStackedWidgetPreviewEventFilter::prevPage);
   connect(m_next, &QAbstractButton::clicked, this, &QStackedWidgetPreviewEventFilter::nextPage);

   updateButtons();
   m_stackedWidget->installEventFilter(this);
   m_prev->installEventFilter(this);
   m_next->installEventFilter(this);
}

void QStackedWidgetPreviewEventFilter::install(QStackedWidget *stackedWidget)
{
   new QStackedWidgetPreviewEventFilter(stackedWidget);
}

void QStackedWidgetPreviewEventFilter::updateButtons()
{
   m_prev->move(m_stackedWidget->width() - 31, 1);
   m_prev->show();
   m_prev->raise();

   m_next->move(m_stackedWidget->width() - 16, 1);
   m_next->show();
   m_next->raise();
}

void QStackedWidgetPreviewEventFilter::prevPage()
{
   if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget())) {
      fw->clearSelection();
      fw->selectWidget(stackedWidget(), true);
   }
   const int count = m_stackedWidget->count();
   if (count > 1) {
      int newIndex = m_stackedWidget->currentIndex() - 1;
      if (newIndex < 0) {
         newIndex = count - 1;
      }
      gotoPage(newIndex);
   }
}

void QStackedWidgetPreviewEventFilter::nextPage()
{
   if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget())) {
      fw->clearSelection();
      fw->selectWidget(stackedWidget(), true);
   }
   const int count = m_stackedWidget->count();
   if (count > 1) {
      gotoPage((m_stackedWidget->currentIndex() + 1) % count);
   }
}

bool QStackedWidgetPreviewEventFilter::eventFilter(QObject *watched, QEvent *event)
{
   if (watched->isWidgetType()) {
      if (watched == m_stackedWidget) {
         switch (event->type()) {
            case QEvent::LayoutRequest:
               updateButtons();
               break;
            case QEvent::ChildAdded:
            case QEvent::ChildRemoved:
            case QEvent::Resize:
            case QEvent::Show:
               updateButtons();
               break;
            default:
               break;
         }
      }
      if (m_buttonToolTipEnabled && (watched == m_next || watched == m_prev)) {
         switch (event->type()) {
            case QEvent::ToolTip:
               updateButtonToolTip(watched); // Tooltip includes page number, so, refresh on demand
               break;
            default:
               break;
         }
      }
   }
   return QObject::eventFilter(watched, event);
}

void QStackedWidgetPreviewEventFilter::gotoPage(int page)
{
   m_stackedWidget->setCurrentIndex(page);
   updateButtons();
}

static inline QString stackedClassName(QStackedWidget *w)
{
   if (const QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(w)) {
      return qdesigner_internal::WidgetFactory::classNameOf(fw->core(), w);
   }

   return QString("Stacked widget");
}

void QStackedWidgetPreviewEventFilter::updateButtonToolTip(QObject *o)
{
   if (o == m_prev) {
      const QString msg = tr("Go to previous page of %1 '%2' (%3/%4).").formatArg(stackedClassName(m_stackedWidget))
            .formatArg(m_stackedWidget->objectName()).formatArg(m_stackedWidget->currentIndex() + 1)
            .formatArg(m_stackedWidget->count());

      m_prev->setToolTip(msg);

   } else {
      if (o == m_next) {
         const QString msg = tr("Go to next page of %1 '%2' (%3/%4).").formatArg(stackedClassName(m_stackedWidget))
               .formatArg(m_stackedWidget->objectName()).formatArg(m_stackedWidget->currentIndex() + 1)
               .formatArg(m_stackedWidget->count());

         m_next->setToolTip(msg);
      }
   }
}

// ---------------  QStackedWidgetEventFilter
QStackedWidgetEventFilter::QStackedWidgetEventFilter(QStackedWidget *parent) :
   QStackedWidgetPreviewEventFilter(parent),
   m_actionPreviousPage(new QAction(tr("Previous Page"), this)),
   m_actionNextPage(new QAction(tr("Next Page"), this)),
   m_actionDeletePage(new QAction(tr("Delete"), this)),
   m_actionInsertPage(new QAction(tr("Before Current Page"), this)),
   m_actionInsertPageAfter(new QAction(tr("After Current Page"), this)),
   m_actionChangePageOrder(new QAction(tr("Change Page Order..."), this)),
   m_pagePromotionTaskMenu(new qdesigner_internal::PromotionTaskMenu(nullptr, qdesigner_internal::PromotionTaskMenu::ModeSingleWidget, this))
{
   setButtonToolTipEnabled(true);
   connect(m_actionPreviousPage,    &QAction::triggered, this, &QStackedWidgetEventFilter::prevPage);
   connect(m_actionNextPage,        &QAction::triggered, this, &QStackedWidgetEventFilter::nextPage);
   connect(m_actionDeletePage,      &QAction::triggered, this, &QStackedWidgetEventFilter::removeCurrentPage);
   connect(m_actionInsertPage,      &QAction::triggered, this, &QStackedWidgetEventFilter::addPage);
   connect(m_actionInsertPageAfter, &QAction::triggered, this, &QStackedWidgetEventFilter::addPageAfter);
   connect(m_actionChangePageOrder, &QAction::triggered, this, &QStackedWidgetEventFilter::changeOrder);
}

void QStackedWidgetEventFilter::install(QStackedWidget *stackedWidget)
{
   new QStackedWidgetEventFilter(stackedWidget);
}

QStackedWidgetEventFilter *QStackedWidgetEventFilter::eventFilterOf(const QStackedWidget *stackedWidget)
{
   // Look for 1st order children only..otherwise, we might get filters of nested widgets
   const QObjectList children = stackedWidget->children();
   const QObjectList::const_iterator cend = children.constEnd();
   for (QObjectList::const_iterator it = children.constBegin(); it != cend; ++it) {
      QObject *o = *it;
      if (!o->isWidgetType())
         if (QStackedWidgetEventFilter *ef = dynamic_cast<QStackedWidgetEventFilter *>(o)) {
            return ef;
         }
   }
   return nullptr;
}

QMenu *QStackedWidgetEventFilter::addStackedWidgetContextMenuActions(const QStackedWidget *stackedWidget, QMenu *popup)
{
   QStackedWidgetEventFilter *filter = eventFilterOf(stackedWidget);
   if (!filter) {
      return nullptr;
   }
   return filter->addContextMenuActions(popup);
}

void QStackedWidgetEventFilter::removeCurrentPage()
{
   if (stackedWidget()->currentIndex() == -1) {
      return;
   }

   if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget())) {
      qdesigner_internal::DeleteStackedWidgetPageCommand *cmd = new qdesigner_internal::DeleteStackedWidgetPageCommand(fw);
      cmd->init(stackedWidget());
      fw->commandHistory()->push(cmd);
   }
}

void QStackedWidgetEventFilter::changeOrder()
{
   QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget());

   if (!fw) {
      return;
   }

   const QWidgetList oldPages = qdesigner_internal::OrderDialog::pagesOfContainer(fw->core(), stackedWidget());
   const int pageCount = oldPages.size();
   if (pageCount < 2) {
      return;
   }

   qdesigner_internal::OrderDialog dlg(fw);
   dlg.setPageList(oldPages);
   if (dlg.exec() == QDialog::Rejected) {
      return;
   }

   const QWidgetList newPages = dlg.pageList();
   if (newPages == oldPages) {
      return;
   }

   fw->beginCommand(tr("Change Page Order"));

   for (int i = 0; i < pageCount; ++i) {
      if (newPages.at(i) == stackedWidget()->widget(i)) {
         continue;
      }
      qdesigner_internal::MoveStackedWidgetCommand *cmd = new qdesigner_internal::MoveStackedWidgetCommand(fw);
      cmd->init(stackedWidget(), newPages.at(i), i);
      fw->commandHistory()->push(cmd);
   }

   fw->endCommand();
}

void QStackedWidgetEventFilter::addPage()
{
   if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget())) {
      qdesigner_internal::AddStackedWidgetPageCommand *cmd = new qdesigner_internal::AddStackedWidgetPageCommand(fw);
      cmd->init(stackedWidget(), qdesigner_internal::AddStackedWidgetPageCommand::InsertBefore);
      fw->commandHistory()->push(cmd);
   }
}

void QStackedWidgetEventFilter::addPageAfter()
{
   if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget())) {
      qdesigner_internal::AddStackedWidgetPageCommand *cmd = new qdesigner_internal::AddStackedWidgetPageCommand(fw);
      cmd->init(stackedWidget(), qdesigner_internal::AddStackedWidgetPageCommand::InsertAfter);
      fw->commandHistory()->push(cmd);
   }
}

void QStackedWidgetEventFilter::gotoPage(int page)
{
   // Are we on a form or in a preview?
   if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(stackedWidget())) {
      qdesigner_internal::SetPropertyCommand *cmd = new  qdesigner_internal::SetPropertyCommand(fw);
      cmd->init(stackedWidget(), QString("currentIndex"), page);
      fw->commandHistory()->push(cmd);
      fw->emitSelectionChanged(); // Magically prevent an endless loop triggered by auto-repeat.
      updateButtons();
   } else {
      QStackedWidgetPreviewEventFilter::gotoPage(page);
   }
}

QMenu *QStackedWidgetEventFilter::addContextMenuActions(QMenu *popup)
{
   QMenu *pageMenu = nullptr;
   const int count = stackedWidget()->count();
   const bool hasSeveralPages = count > 1;
   m_actionDeletePage->setEnabled(count);

   if (count) {
      const QString pageSubMenuLabel = tr("Page %1 of %2").formatArg(stackedWidget()->currentIndex() + 1).formatArg(count);
      pageMenu = popup->addMenu(pageSubMenuLabel);
      pageMenu->addAction(m_actionDeletePage);
      // Set up promotion menu for current widget.
      if (QWidget *page =  stackedWidget()->currentWidget ()) {
         m_pagePromotionTaskMenu->setWidget(page);
         m_pagePromotionTaskMenu->addActions(QDesignerFormWindowInterface::findFormWindow(stackedWidget()),
            qdesigner_internal::PromotionTaskMenu::SuppressGlobalEdit,
            pageMenu);
      }
      QMenu *insertPageMenu = popup->addMenu(tr("Insert Page"));
      insertPageMenu->addAction(m_actionInsertPageAfter);
      insertPageMenu->addAction(m_actionInsertPage);
   } else {
      QAction *insertPageAction = popup->addAction(tr("Insert Page"));
      connect(insertPageAction, &QAction::triggered, this, &QStackedWidgetEventFilter::addPage);
   }
   popup->addAction(m_actionNextPage);
   m_actionNextPage->setEnabled(hasSeveralPages);
   popup->addAction(m_actionPreviousPage);
   m_actionPreviousPage->setEnabled(hasSeveralPages);
   popup->addAction(m_actionChangePageOrder);
   m_actionChangePageOrder->setEnabled(hasSeveralPages);
   popup->addSeparator();
   return pageMenu;
}

// --------  QStackedWidgetPropertySheet

static QString pagePropertyName = "currentPageName";

QStackedWidgetPropertySheet::QStackedWidgetPropertySheet(QStackedWidget *object, QObject *parent) :
   QDesignerPropertySheet(object, parent),
   m_stackedWidget(object)
{
   createFakeProperty(pagePropertyName, QString());
}

bool QStackedWidgetPropertySheet::isEnabled(int index) const
{
   if (propertyName(index) != pagePropertyName) {
      return QDesignerPropertySheet::isEnabled(index);
   }

   return  m_stackedWidget->currentWidget() != nullptr;
}

void QStackedWidgetPropertySheet::setProperty(int index, const QVariant &value)
{
   if (propertyName(index) == pagePropertyName) {
      if (QWidget *w = m_stackedWidget->currentWidget()) {
         w->setObjectName(value.toString());
      }
   } else {
      QDesignerPropertySheet::setProperty(index, value);
   }
}

QVariant QStackedWidgetPropertySheet::property(int index) const
{
   if (propertyName(index) == pagePropertyName) {
      if (const QWidget *w = m_stackedWidget->currentWidget()) {
         return w->objectName();
      }
      return QString();
   }
   return QDesignerPropertySheet::property(index);
}

bool QStackedWidgetPropertySheet::reset(int index)
{
   if (propertyName(index) == pagePropertyName) {
      setProperty(index, QString());
      return true;
   }
   return QDesignerPropertySheet::reset(index);
}

bool QStackedWidgetPropertySheet::checkProperty(const QString &propertyName)
{
   return propertyName != pagePropertyName;
}
