/***********************************************************************
*
* Copyright (c) 2021-2023 Barbara Geller
* Copyright (c) 2021-2023 Ansel Sermersheim
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

#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <container.h>
#include <containerwidget_taskmenu.h>
#include <designer_command.h>
#include <designer_dockwidget.h>
#include <extension.h>
#include <promotion_taskmenu.h>

#include <widgetdatabase_p.h>

#include <QAction>
#include <QMainWindow>
#include <QToolBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QScrollArea>
#include <QMdiArea>
#include <QWizard>
#include <QMenu>
#include <QDebug>

namespace qdesigner_internal {

ContainerWidgetTaskMenu::ContainerWidgetTaskMenu(QWidget *widget, ContainerType type, QObject *parent)
   : QDesignerTaskMenu(widget, parent), m_type(type), m_containerWidget(widget),
     m_core(formWindow()->core()),
     m_pagePromotionTaskMenu(new PromotionTaskMenu(nullptr, PromotionTaskMenu::ModeSingleWidget, this)),
     m_pageMenuAction(new QAction(this)), m_pageMenu(new QMenu),
     m_actionInsertPageAfter(new QAction(this)), m_actionInsertPage(nullptr),
     m_actionDeletePage(new QAction(tr("Delete"), this))
{
   Q_ASSERT(m_core);
   m_taskActions.append(createSeparator());

   connect(m_actionDeletePage, &QAction::triggered, this, &ContainerWidgetTaskMenu::removeCurrentPage);

   connect(m_actionInsertPageAfter, &QAction::triggered, this, &ContainerWidgetTaskMenu::addPageAfter);

   // Empty Per-Page submenu, deletion and promotion. Updated on demand due to promotion state
   switch (m_type) {
      case WizardContainer:
      case PageContainer:
         m_taskActions.append(createSeparator()); // for the browse actions
         break;

      case MdiContainer:
         break;
   }

   // submenu
   m_pageMenuAction->setMenu(m_pageMenu);
   m_taskActions.append(m_pageMenuAction);

   // Insertion
   switch (m_type) {
      case WizardContainer:
      case PageContainer: {
         // Before and after in a submenu
         QAction *insertMenuAction = new QAction(tr("Insert"), this);
         QMenu *insertMenu = new QMenu;

         // before
         m_actionInsertPage = new QAction(tr("Insert Page Before Current Page"), this);
         connect(m_actionInsertPage, &QAction::triggered, this, &ContainerWidgetTaskMenu::addPage);
         insertMenu->addAction(m_actionInsertPage);

         // after
         m_actionInsertPageAfter->setText(tr("Insert Page After Current Page"));
         insertMenu->addAction(m_actionInsertPageAfter);

         insertMenuAction->setMenu(insertMenu);
         m_taskActions.append(insertMenuAction);
      }
      break;

      case MdiContainer: // No concept of order
         m_actionInsertPageAfter->setText(tr("Add Subwindow"));
         m_taskActions.append(m_actionInsertPageAfter);
         break;
   }
}

ContainerWidgetTaskMenu::~ContainerWidgetTaskMenu()
{
}

QAction *ContainerWidgetTaskMenu::preferredEditAction() const
{
   return nullptr;
}

bool ContainerWidgetTaskMenu::canDeletePage() const
{
   switch (pageCount()) {
      case 0:
         return false;

      case 1:
         return m_type != PageContainer; // Do not delete last page of page-type container

      default:
         break;
   }

   return true;
}

int ContainerWidgetTaskMenu::pageCount() const
{
   if (const QDesignerContainerExtension *ce = containerExtension()) {
      return ce->count();
   }

   return 0;
}

QString ContainerWidgetTaskMenu::pageMenuText(ContainerType ct, int index, int count)
{
   if (ct == MdiContainer) {
      return tr("Subwindow");   // No concept of order, same text everywhere
   }

   if (index < 0) {
      return tr("Page");
   }

   return tr("Page %1 of %2").formatArg(index + 1).formatArg(count);
}

QList<QAction *> ContainerWidgetTaskMenu::taskActions() const
{
   const QDesignerContainerExtension *ce = containerExtension();
   const int index = ce->currentIndex();

   QList<QAction *> actions = QDesignerTaskMenu::taskActions();
   actions += m_taskActions;

   // Update the page submenu, deletion and promotion. Updated on demand due to promotion state.
   m_pageMenu->clear();

   const bool canAddWidget = ce->canAddWidget();
   if (m_actionInsertPage) {
      m_actionInsertPage->setEnabled(canAddWidget);
   }

   m_actionInsertPageAfter->setEnabled(canAddWidget);
   m_pageMenu->addAction(m_actionDeletePage);
   m_actionDeletePage->setEnabled(index >= 0 && ce->canRemove(index) && canDeletePage());
   m_pageMenuAction->setText(pageMenuText(m_type, index, ce->count()));

   if (index != -1) { // Has a page
      m_pageMenuAction->setEnabled(true);
      m_pagePromotionTaskMenu->setWidget(ce->widget(index));
      m_pagePromotionTaskMenu->addActions(PromotionTaskMenu::LeadingSeparator | PromotionTaskMenu::SuppressGlobalEdit, m_pageMenu);
   } else { // No page
      m_pageMenuAction->setEnabled(false);
   }

   return actions;
}

QDesignerFormWindowInterface *ContainerWidgetTaskMenu::formWindow() const
{
   return QDesignerFormWindowInterface::findFormWindow(m_containerWidget);
}

QDesignerContainerExtension *ContainerWidgetTaskMenu::containerExtension() const
{
   QExtensionManager *mgr = m_core->extensionManager();
   return qt_extension<QDesignerContainerExtension *>(mgr, m_containerWidget);
}

void ContainerWidgetTaskMenu::removeCurrentPage()
{
   if (QDesignerContainerExtension *c = containerExtension()) {
      if (c->currentIndex() == -1) {
         return;
      }

      QDesignerFormWindowInterface *fw = formWindow();
      DeleteContainerWidgetPageCommand *cmd = new DeleteContainerWidgetPageCommand(fw);
      cmd->init(m_containerWidget, m_type);
      fw->commandHistory()->push(cmd);
   }
}

void ContainerWidgetTaskMenu::addPage()
{
   if (containerExtension()) {
      QDesignerFormWindowInterface *fw = formWindow();
      AddContainerWidgetPageCommand *cmd = new AddContainerWidgetPageCommand(fw);
      cmd->init(m_containerWidget, m_type, AddContainerWidgetPageCommand::InsertBefore);
      fw->commandHistory()->push(cmd);
   }
}

void ContainerWidgetTaskMenu::addPageAfter()
{
   if (containerExtension()) {
      QDesignerFormWindowInterface *fw = formWindow();
      AddContainerWidgetPageCommand *cmd = new AddContainerWidgetPageCommand(fw);
      cmd->init(m_containerWidget, m_type, AddContainerWidgetPageCommand::InsertAfter);
      fw->commandHistory()->push(cmd);
   }
}

WizardContainerWidgetTaskMenu::WizardContainerWidgetTaskMenu(QWizard *w, QObject *parent)
   : ContainerWidgetTaskMenu(w, WizardContainer, parent),
     m_nextAction(new QAction(tr("Next"), this)),
     m_previousAction(new QAction(tr("Back"), this))
{
   connect(m_nextAction,     &QAction::triggered, w, &QWizard::next);
   connect(m_previousAction, &QAction::triggered, w, &QWizard::back);

   QList<QAction *> &l = containerActions();
   l.push_front(createSeparator());
   l.push_front(m_nextAction);
   l.push_front(m_previousAction);
   l.push_front(createSeparator());
}

QList<QAction *> WizardContainerWidgetTaskMenu::taskActions() const
{
   // Enable
   const QDesignerContainerExtension *ce = containerExtension();
   const int index = ce->currentIndex();
   m_previousAction->setEnabled(index > 0);
   m_nextAction->setEnabled(index >= 0 && index < (ce->count() - 1));
   return ContainerWidgetTaskMenu::taskActions();
}


MdiContainerWidgetTaskMenu::MdiContainerWidgetTaskMenu(QMdiArea *m, QObject *parent)
   : ContainerWidgetTaskMenu(m, MdiContainer, parent)
{
   initializeActions();
   connect(m_nextAction, &QAction::triggered, m, &QMdiArea::activateNextSubWindow);
   connect(m_previousAction, &QAction::triggered, m, &QMdiArea::activatePreviousSubWindow);
   connect(m_tileAction, &QAction::triggered, m, &QMdiArea::tileSubWindows);
   connect(m_cascadeAction, &QAction::triggered, m, &QMdiArea::cascadeSubWindows);
}

void MdiContainerWidgetTaskMenu::initializeActions()
{
   m_nextAction = new QAction(tr("Next Subwindow"), this);
   m_previousAction = new QAction(tr("Previous Subwindow"), this);
   m_tileAction = new QAction(tr("Tile"), this);
   m_cascadeAction = new QAction(tr("Cascade"), this);

   QList<QAction *> &l = containerActions();
   l.push_front(createSeparator());
   l.push_front(m_tileAction);
   l.push_front(m_cascadeAction);
   l.push_front(m_previousAction);
   l.push_front(m_nextAction);
   l.push_front(createSeparator());
}

QList<QAction *> MdiContainerWidgetTaskMenu::taskActions() const
{
   const QList<QAction *> rc = ContainerWidgetTaskMenu::taskActions();
   // Enable
   const int count = pageCount();
   m_nextAction->setEnabled(count > 1);
   m_previousAction->setEnabled(count > 1);
   m_tileAction->setEnabled(count);
   m_cascadeAction->setEnabled(count);
   return rc;
}

ContainerWidgetTaskMenuFactory::ContainerWidgetTaskMenuFactory(QDesignerFormEditorInterface *core,
      QExtensionManager *extensionManager)
   :  QExtensionFactory(extensionManager), m_core(core)
{
}

QObject *ContainerWidgetTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
   if (iid != "QDesignerInternalTaskMenuExtension" || ! object->isWidgetType()) {
      return nullptr;
   }

   QWidget *widget = dynamic_cast<QWidget *>(object);

   if (dynamic_cast<QStackedWidget *>(widget) || dynamic_cast<QToolBox *>(widget)
      || dynamic_cast<QTabWidget *>(widget)   || dynamic_cast<QMainWindow *>(widget)) {

      // Are we using Designer's own container extensions and task menus or did
      // someone provide an extra one with an addpage method, for example for a QScrollArea?

      if (const WidgetDataBase *wb = dynamic_cast<const WidgetDataBase *>(m_core->widgetDataBase())) {
         const int idx = wb->indexOfObject(widget);
         const WidgetDataBaseItem *item = static_cast<const WidgetDataBaseItem *>(wb->item(idx));

         if (item->addPageMethod().isEmpty()) {
            return nullptr;
         }
      }
   }

   if (qt_extension<QDesignerContainerExtension *>(extensionManager(), object) == nullptr) {
      return nullptr;
   }

   if (QMdiArea *ma = dynamic_cast<QMdiArea *>(widget)) {
      return new MdiContainerWidgetTaskMenu(ma, parent);
   }

   if (QWizard *wz = dynamic_cast<QWizard *>(widget)) {
      return new WizardContainerWidgetTaskMenu(wz, parent);
   }

   return new ContainerWidgetTaskMenu(widget, PageContainer, parent);
}

}   // end namespace
