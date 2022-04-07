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

#include <abstract_formwindow.h>
#include <designer_command.h>
#include <designer_toolbox.h>
#include <order_dialog.h>
#include <promotion_taskmenu.h>

#include <formwindowbase_p.h>

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QEvent>
#include <QHash>
#include <QLayout>
#include <QMenu>
#include <QToolBox>

QToolBoxHelper::QToolBoxHelper(QToolBox *toolbox)
   : QObject(toolbox), m_toolbox(toolbox),
     m_actionDeletePage(new QAction(tr("Delete Page"), this)),
     m_actionInsertPage(new QAction(tr("Before Current Page"), this)),
     m_actionInsertPageAfter(new QAction(tr("After Current Page"), this)),
     m_actionChangePageOrder(new QAction(tr("Change Page Order..."), this)),
     m_pagePromotionTaskMenu(new qdesigner_internal::PromotionTaskMenu(nullptr, qdesigner_internal::PromotionTaskMenu::ModeSingleWidget, this))
{
   connect(m_actionDeletePage,      &QAction::triggered, this, &QToolBoxHelper::removeCurrentPage);
   connect(m_actionInsertPage,      &QAction::triggered, this, &QToolBoxHelper::addPage);
   connect(m_actionInsertPageAfter, &QAction::triggered, this, &QToolBoxHelper::addPageAfter);
   connect(m_actionChangePageOrder, &QAction::triggered, this, &QToolBoxHelper::changeOrder);

   m_toolbox->installEventFilter(this);
}

void QToolBoxHelper::install(QToolBox *toolbox)
{
   new QToolBoxHelper(toolbox);
}

bool QToolBoxHelper::eventFilter(QObject *watched, QEvent *event)
{
   switch (event->type()) {
      case QEvent::ChildPolished:
         // Install on the buttons
         if (watched == m_toolbox) {
            QChildEvent *ce = static_cast<QChildEvent *>(event);

            if (ce->child()->metaObject()->className() == "QToolBoxButton") {
               ce->child()->installEventFilter(this);
            }
         }
         break;

      case QEvent::ContextMenu:
         if (watched != m_toolbox) {
            // An action invoked from the passive interactor (ToolBox button) might
            // cause its deletion within its event handler, triggering a warning. Re-post
            // the event to the toolbox.
            QContextMenuEvent *current = static_cast<QContextMenuEvent *>(event);
            QContextMenuEvent *copy = new QContextMenuEvent(current->reason(), current->pos(), current-> globalPos(), current->modifiers());
            QApplication::postEvent(m_toolbox, copy);
            current->accept();
            return true;
         }
         break;
      case QEvent::MouseButtonRelease:
         if (watched != m_toolbox)
            if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(m_toolbox)) {
               fw->clearSelection();
               fw->selectWidget(m_toolbox, true);
            }
         break;
      default:
         break;
   }
   return QObject::eventFilter(watched, event);
}

QToolBoxHelper *QToolBoxHelper::helperOf(const QToolBox *toolbox)
{
   // Look for 1st order children only..otherwise, we might get filters of nested widgets
   const QObjectList children = toolbox->children();
   const QObjectList::const_iterator cend = children.constEnd();
   for (QObjectList::const_iterator it = children.constBegin(); it != cend; ++it) {
      QObject *o = *it;
      if (!o->isWidgetType())
         if (QToolBoxHelper *h = dynamic_cast<QToolBoxHelper *>(o)) {
            return h;
         }
   }
   return nullptr;
}

QMenu *QToolBoxHelper::addToolBoxContextMenuActions(const QToolBox *toolbox, QMenu *popup)
{
   QToolBoxHelper *helper = helperOf(toolbox);
   if (!helper) {
      return nullptr;
   }
   return helper->addContextMenuActions(popup);
}

void QToolBoxHelper::removeCurrentPage()
{
   if (m_toolbox->currentIndex() == -1 || !m_toolbox->widget(m_toolbox->currentIndex())) {
      return;
   }

   if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(m_toolbox)) {
      qdesigner_internal::DeleteToolBoxPageCommand *cmd = new qdesigner_internal::DeleteToolBoxPageCommand(fw);
      cmd->init(m_toolbox);
      fw->commandHistory()->push(cmd);
   }
}

void QToolBoxHelper::addPage()
{
   if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(m_toolbox)) {
      qdesigner_internal::AddToolBoxPageCommand *cmd = new qdesigner_internal::AddToolBoxPageCommand(fw);
      cmd->init(m_toolbox, qdesigner_internal::AddToolBoxPageCommand::InsertBefore);
      fw->commandHistory()->push(cmd);
   }
}

void QToolBoxHelper::changeOrder()
{
   QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(m_toolbox);

   if (!fw) {
      return;
   }

   const QWidgetList oldPages = qdesigner_internal::OrderDialog::pagesOfContainer(fw->core(), m_toolbox);
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
      if (newPages.at(i) == m_toolbox->widget(i)) {
         continue;
      }
      qdesigner_internal::MoveToolBoxPageCommand *cmd = new qdesigner_internal::MoveToolBoxPageCommand(fw);
      cmd->init(m_toolbox, newPages.at(i), i);
      fw->commandHistory()->push(cmd);
   }
   fw->endCommand();
}

void QToolBoxHelper::addPageAfter()
{
   if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(m_toolbox)) {
      qdesigner_internal::AddToolBoxPageCommand *cmd = new qdesigner_internal::AddToolBoxPageCommand(fw);
      cmd->init(m_toolbox, qdesigner_internal::AddToolBoxPageCommand::InsertAfter);
      fw->commandHistory()->push(cmd);
   }
}

QPalette::ColorRole QToolBoxHelper::currentItemBackgroundRole() const
{
   const QWidget *w = m_toolbox->widget(0);
   if (!w) {
      return  QPalette::Window;
   }
   return w->backgroundRole();
}

void QToolBoxHelper::setCurrentItemBackgroundRole(QPalette::ColorRole role)
{
   const int count = m_toolbox->count();
   for (int i = 0; i < count; ++i) {
      QWidget *w = m_toolbox->widget(i);
      w->setBackgroundRole(role);
      w->update();
   }
}

QMenu *QToolBoxHelper::addContextMenuActions(QMenu *popup) const
{
   QMenu *pageMenu = nullptr;
   const int count = m_toolbox->count();
   m_actionDeletePage->setEnabled(count > 1);
   if (count) {
      const QString pageSubMenuLabel = tr("Page %1 of %2").formatArg(m_toolbox->currentIndex() + 1).formatArg(count);
      pageMenu = popup->addMenu(pageSubMenuLabel);

      pageMenu->addAction(m_actionDeletePage);
      // Set up promotion menu for current widget.
      if (QWidget *page =  m_toolbox->currentWidget ()) {
         m_pagePromotionTaskMenu->setWidget(page);
         m_pagePromotionTaskMenu->addActions(QDesignerFormWindowInterface::findFormWindow(m_toolbox),
            qdesigner_internal::PromotionTaskMenu::SuppressGlobalEdit,
            pageMenu);
      }
   }

   QMenu *insertPageMenu = popup->addMenu(tr("Insert Page"));
   insertPageMenu->addAction(m_actionInsertPageAfter);
   insertPageMenu->addAction(m_actionInsertPage);

   if (count > 1) {
      popup->addAction(m_actionChangePageOrder);
   }

   popup->addSeparator();

   return pageMenu;
}

// -------- QToolBoxWidgetPropertySheet

static const QString currentItemTextKey    = "currentItemText";
static const QString currentItemNameKey    = "currentItemName";
static const QString currentItemIconKey    = "currentItemIcon";
static const QString currentItemToolTipKey = "currentItemToolTip";
static const QString tabSpacingKey         = "tabSpacing";

constexpr const int tabSpacingDefault = -1;

QToolBoxWidgetPropertySheet::QToolBoxWidgetPropertySheet(QToolBox *object, QObject *parent)
   : QDesignerPropertySheet(object, parent), m_toolBox(object)
{
   createFakeProperty(currentItemTextKey, QVariant::fromValue(qdesigner_internal::PropertySheetStringValue()));
   createFakeProperty(currentItemNameKey, QString());
   createFakeProperty(currentItemIconKey, QVariant::fromValue(qdesigner_internal::PropertySheetIconValue()));

   if (formWindowBase()) {
      formWindowBase()->addReloadableProperty(this, indexOf(currentItemIconKey));
   }

   createFakeProperty(currentItemToolTipKey, QVariant::fromValue(qdesigner_internal::PropertySheetStringValue()));
   createFakeProperty(tabSpacingKey, QVariant(tabSpacingDefault));
}

QToolBoxWidgetPropertySheet::ToolBoxProperty QToolBoxWidgetPropertySheet::toolBoxPropertyFromName(const QString &name)
{
   typedef QHash<QString, ToolBoxProperty> ToolBoxPropertyHash;
   static ToolBoxPropertyHash toolBoxPropertyHash;

   if (toolBoxPropertyHash.empty()) {
      toolBoxPropertyHash.insert(currentItemTextKey,    PropertyCurrentItemText);
      toolBoxPropertyHash.insert(currentItemNameKey,    PropertyCurrentItemName);
      toolBoxPropertyHash.insert(currentItemIconKey,    PropertyCurrentItemIcon);
      toolBoxPropertyHash.insert(currentItemToolTipKey, PropertyCurrentItemToolTip);
      toolBoxPropertyHash.insert(tabSpacingKey,         PropertyTabSpacing);
   }

   return toolBoxPropertyHash.value(name, PropertyToolBoxNone);
}

void QToolBoxWidgetPropertySheet::setProperty(int index, const QVariant &value)
{
   const ToolBoxProperty toolBoxProperty = toolBoxPropertyFromName(propertyName(index));

   // independent of index
   switch (toolBoxProperty) {
      case PropertyTabSpacing:
         m_toolBox->layout()->setSpacing(value.toInt());
         return;

      case PropertyToolBoxNone:
         QDesignerPropertySheet::setProperty(index, value);
         return;

      default:
         break;
   }

   // index-dependent
   const int currentIndex = m_toolBox->currentIndex();
   QWidget *currentWidget = m_toolBox->currentWidget();

   if (! currentWidget) {
      return;
   }

   switch (toolBoxProperty) {
      case PropertyCurrentItemText: {
         QVariant data = resolvePropertyValue(index, value);
         m_toolBox->setItemText(currentIndex, data.value<QString>());

         m_pageToData[currentWidget].text = value.value<qdesigner_internal::PropertySheetStringValue>();
         break;
      }

      case PropertyCurrentItemName:
         currentWidget->setObjectName(value.toString());
         break;

      case PropertyCurrentItemIcon: {
         QVariant data = resolvePropertyValue(index, value);
         m_toolBox->setItemIcon(currentIndex, data.value<QIcon>());

         m_pageToData[currentWidget].icon = value.value<qdesigner_internal::PropertySheetIconValue>();
         break;
      }

      case PropertyCurrentItemToolTip: {
         QVariant data = resolvePropertyValue(index, value);
         m_toolBox->setItemToolTip(currentIndex, data.value<QString>());

         m_pageToData[currentWidget].tooltip = value.value<qdesigner_internal::PropertySheetStringValue>();
         break;
      }

      case PropertyTabSpacing:
      case PropertyToolBoxNone:
         break;
   }
}

bool QToolBoxWidgetPropertySheet::isEnabled(int index) const
{
   switch (toolBoxPropertyFromName(propertyName(index))) {
      case PropertyToolBoxNone:  // independent of index
      case PropertyTabSpacing:
         return QDesignerPropertySheet::isEnabled(index);

      default:
         break;
   }

   return m_toolBox->currentIndex() != -1;
}

QVariant QToolBoxWidgetPropertySheet::property(int index) const
{
   const ToolBoxProperty toolBoxProperty = toolBoxPropertyFromName(propertyName(index));
   // independent of index
   switch (toolBoxProperty) {
      case PropertyTabSpacing:
         return m_toolBox->layout()->spacing();

      case PropertyToolBoxNone:
         return QDesignerPropertySheet::property(index);

      default:
         break;
   }
   // index-dependent
   QWidget *currentWidget = m_toolBox->currentWidget();
   if (! currentWidget) {
      if (toolBoxProperty == PropertyCurrentItemIcon) {
         return  QVariant::fromValue(qdesigner_internal::PropertySheetIconValue());
      }

      if (toolBoxProperty == PropertyCurrentItemText) {
         return  QVariant::fromValue(qdesigner_internal::PropertySheetStringValue());
      }

      if (toolBoxProperty == PropertyCurrentItemToolTip) {
         return  QVariant::fromValue(qdesigner_internal::PropertySheetStringValue());
      }

      return QVariant(QString());
   }

   // index-dependent
   switch (toolBoxProperty) {
      case PropertyCurrentItemText:
         return QVariant::fromValue(m_pageToData.value(currentWidget).text);

      case PropertyCurrentItemName:
         return currentWidget->objectName();

      case PropertyCurrentItemIcon:
         return QVariant::fromValue(m_pageToData.value(currentWidget).icon);

      case PropertyCurrentItemToolTip:
         return QVariant::fromValue(m_pageToData.value(currentWidget).tooltip);

      case PropertyTabSpacing:
      case PropertyToolBoxNone:
         break;
   }
   return QVariant();
}

bool QToolBoxWidgetPropertySheet::reset(int index)
{
   const ToolBoxProperty toolBoxProperty = toolBoxPropertyFromName(propertyName(index));
   // independent of index
   switch (toolBoxProperty) {
      case PropertyTabSpacing:
         setProperty(index, QVariant(tabSpacingDefault));
         return true;

      case PropertyToolBoxNone:
         return QDesignerPropertySheet::reset(index);

      default:
         break;
   }

   // index-dependent
   QWidget *currentWidget = m_toolBox->currentWidget();
   if (!currentWidget) {
      return false;
   }

   // index-dependent
   switch (toolBoxProperty) {
      case PropertyCurrentItemName:
         setProperty(index, QString());
         break;

     case PropertyCurrentItemToolTip:
         m_pageToData[currentWidget].tooltip = qdesigner_internal::PropertySheetStringValue();
         setProperty(index, QString());
         break;

      case PropertyCurrentItemText:
         m_pageToData[currentWidget].text = qdesigner_internal::PropertySheetStringValue();
         setProperty(index, QString());
         break;

      case PropertyCurrentItemIcon:
         m_pageToData[currentWidget].icon = qdesigner_internal::PropertySheetIconValue();
         setProperty(index, QIcon());
         break;

      case PropertyTabSpacing:
      case PropertyToolBoxNone:
         break;
   }
   return true;
}

bool QToolBoxWidgetPropertySheet::checkProperty(const QString &propertyName)
{
   switch (toolBoxPropertyFromName(propertyName)) {
      case PropertyCurrentItemText:
      case PropertyCurrentItemName:
      case PropertyCurrentItemToolTip:
      case PropertyCurrentItemIcon:
         return false;

      default:
         break;
   }

   return true;
}
