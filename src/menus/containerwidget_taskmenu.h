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

#ifndef CONTAINERWIDGER_TASKMENU_H
#define CONTAINERWIDGER_TASKMENU_H

#include <designer_taskmenu.h>

#include <extensionfactory_p.h>
#include <shared_enums_p.h>

#include <QPointer>

class QDesignerContainerExtension;
class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QAction;
class QMdiArea;
class QMenu;
class QWizard;

namespace qdesigner_internal {

class PromotionTaskMenu;

// ContainerWidgetTaskMenu: Task menu for containers with extension

class ContainerWidgetTaskMenu: public QDesignerTaskMenu
{
   CS_OBJECT(ContainerWidgetTaskMenu)

 public:
   explicit ContainerWidgetTaskMenu(QWidget *widget, ContainerType type, QObject *parent = nullptr);
   virtual ~ContainerWidgetTaskMenu();

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 protected:
   QDesignerContainerExtension *containerExtension() const;
   QList<QAction *> &containerActions() {
      return m_taskActions;
   }
   int pageCount() const;

 private:
   CS_SLOT_1(Private, void removeCurrentPage())
   CS_SLOT_2(removeCurrentPage)
   CS_SLOT_1(Private, void addPage())
   CS_SLOT_2(addPage)
   CS_SLOT_1(Private, void addPageAfter())
   CS_SLOT_2(addPageAfter)

   QDesignerFormWindowInterface *formWindow() const;

   static QString pageMenuText(ContainerType ct, int index, int count);
   bool canDeletePage() const;

   const ContainerType m_type;
   QWidget *m_containerWidget;
   QDesignerFormEditorInterface *m_core;
   PromotionTaskMenu *m_pagePromotionTaskMenu;
   QAction *m_pageMenuAction;
   QMenu *m_pageMenu;
   QList<QAction *> m_taskActions;
   QAction *m_actionInsertPageAfter;
   QAction *m_actionInsertPage;
   QAction *m_actionDeletePage;
};

// WizardContainerWidgetTaskMenu: Provide next/back since QWizard
// has modes in which the "Back" button is not visible.

class WizardContainerWidgetTaskMenu : public ContainerWidgetTaskMenu
{
   CS_OBJECT(WizardContainerWidgetTaskMenu)

 public:
   explicit WizardContainerWidgetTaskMenu(QWizard *w, QObject *parent = nullptr);

   QList<QAction *> taskActions() const override;

 private:
   QAction *m_nextAction;
   QAction *m_previousAction;
};


// MdiContainerWidgetTaskMenu: Provide tile/cascade for MDI containers in addition

class MdiContainerWidgetTaskMenu : public ContainerWidgetTaskMenu
{
   CS_OBJECT(MdiContainerWidgetTaskMenu)

 public:
   explicit MdiContainerWidgetTaskMenu(QMdiArea *m, QObject *parent = nullptr);

   QList<QAction *> taskActions() const override;

 private:
   void initializeActions();

   QAction *m_nextAction;
   QAction *m_previousAction;
   QAction *m_tileAction;
   QAction *m_cascadeAction;
};

class ContainerWidgetTaskMenuFactory: public QExtensionFactory
{
   CS_OBJECT(ContainerWidgetTaskMenuFactory)

 public:
   explicit ContainerWidgetTaskMenuFactory(QDesignerFormEditorInterface *core, QExtensionManager *extensionManager = nullptr);

 protected:
   QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const override;

 private:
   QDesignerFormEditorInterface *m_core;
};

}   // end namespace qdesigner_internal

#endif
