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

#ifndef MDI_AREA_CONTAINER_H
#define MDI_AREA_CONTAINER_H

#include <container.h>
#include <designer_propertysheet.h>

#include <extensionfactory_p.h>

#include <QMdiArea>

namespace qdesigner_internal {

// Container for QMdiArea
class QMdiAreaContainer: public QObject, public QDesignerContainerExtension
{
   CS_OBJECT(QMdiAreaContainer)
   CS_INTERFACES(QDesignerContainerExtension)

 public:
   explicit QMdiAreaContainer(QMdiArea *widget, QObject *parent = nullptr);

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

   // Semismart positioning of a new MDI child after cascading
   static void positionNewMdiChild(const QWidget *area, QWidget *mdiChild);

 private:
   QMdiArea *m_mdiArea;
};

// PropertySheet for QMdiArea: Fakes window title and name.

class QMdiAreaPropertySheet: public QDesignerPropertySheet
{
   CS_OBJECT(QMdiAreaPropertySheet)
   CS_INTERFACES(QDesignerPropertySheetExtension)

 public:
   explicit QMdiAreaPropertySheet(QWidget *mdiArea, QObject *parent = nullptr);

   void setProperty(int index, const QVariant &value) override;
   bool reset(int index) override;
   bool isEnabled(int index) const override;
   bool isChanged(int index) const override;
   QVariant property(int index) const override;

   // Check whether the property is to be saved. Returns false for the page
   // properties (as the property sheet has no concept of 'stored')
   static bool checkProperty(const QString &propertyName);

 private:
   const QString m_windowTitleProperty;
   QWidget *currentWindow() const;
   QDesignerPropertySheetExtension *currentWindowSheet() const;

   enum MdiAreaProperty { MdiAreaSubWindowName, MdiAreaSubWindowTitle, MdiAreaNone };
   static MdiAreaProperty mdiAreaProperty(const QString &name);
};

// Factories

typedef ExtensionFactory<QDesignerContainerExtension,  QMdiArea,  QMdiAreaContainer> QMdiAreaContainerFactory;
typedef QDesignerPropertySheetFactory<QMdiArea, QMdiAreaPropertySheet> QMdiAreaPropertySheetFactory;

}  // namespace qdesigner_internal

#endif // QMDIAREA_CONTAINER_H
