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

#ifndef WIZARD_CONTAINER_H
#define WIZARD_CONTAINER_H

#include <container.h>

#include <designer_propertysheet.h>
#include <extensionfactory_p.h>

#include <QWizard>
#include <QWizardPage>

class QWizardPage;

namespace qdesigner_internal {

// Container for QWizard. Care must be taken to position
// the  QWizard at some valid page after removal/insertion
// as it is not used to having its pages ripped out.
class QWizardContainer: public QObject, public QDesignerContainerExtension
{
   CS_OBJECT(QWizardContainer)
   CS_INTERFACES(QDesignerContainerExtension)

 public:
   explicit QWizardContainer(QWizard *widget, QObject *parent = nullptr);

   bool canAddWidget() const override {
      return true;
   }

   bool canRemove(int) const override {
      return true;
   }

   int count() const override;
   QWidget *widget(int index) const override;
   int currentIndex() const override;
   void setCurrentIndex(int index) override;
   void addWidget(QWidget *widget) override;
   void insertWidget(int index, QWidget *widget) override;
   void remove(int index) override;

 private:
   QWizard *m_wizard;
};

// QWizardPagePropertySheet: Introduces a attribute string fake property
// "pageId" that allows for specifying enumeration values (uic only).
// This breaks the pattern of having a "currentSth" property for the
// container, but was deemed to make sense here since the Page has
// its own "title" properties.
class QWizardPagePropertySheet: public QDesignerPropertySheet
{
   CS_OBJECT(QWizardPagePropertySheet)
 public:
   explicit QWizardPagePropertySheet(QWizardPage *object, QObject *parent = nullptr);

   bool reset(int index) override;

   static const QString pageIdProperty;

 private:
   const int m_pageIdIndex;
};

// QWizardPropertySheet: Hides the "startId" property. It cannot be used
// as QWizard cannot handle setting it as a property before the actual
// page is added.

class QWizardPropertySheet: public QDesignerPropertySheet
{
   CS_OBJECT(QWizardPropertySheet)

 public:
   explicit QWizardPropertySheet(QWizard *object, QObject *parent = nullptr);
   bool isVisible(int index) const override;

 private:
   const QString m_startId;
};

// Factories
typedef QDesignerPropertySheetFactory<QWizard, QWizardPropertySheet>  QWizardPropertySheetFactory;
typedef QDesignerPropertySheetFactory<QWizardPage, QWizardPagePropertySheet>  QWizardPagePropertySheetFactory;
typedef ExtensionFactory<QDesignerContainerExtension,  QWizard,  QWizardContainer> QWizardContainerFactory;

}  // namespace qdesigner_internal

#endif
