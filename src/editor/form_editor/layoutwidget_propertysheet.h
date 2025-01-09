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

#ifndef LAYOUTWIDGET_PROPERTYSHEET_H
#define LAYOUTWIDGET_PROPERTYSHEET_H

#include <designer_propertysheet.h>
#include <layout_widget.h>

#include <extensionfactory_p.h>

namespace qdesigner_internal {

class QLayoutWidgetPropertySheet: public QDesignerPropertySheet
{
   CS_OBJECT(QLayoutWidgetPropertySheet)
   CS_INTERFACES(QDesignerPropertySheetExtension)

 public:
   explicit QLayoutWidgetPropertySheet(QLayoutWidget *object, QObject *parent = nullptr);
   virtual ~QLayoutWidgetPropertySheet();

   void setProperty(int index, const QVariant &value) override;
   bool isVisible(int index) const override;

   bool dynamicPropertiesAllowed() const override;
};

typedef QDesignerPropertySheetFactory<QLayoutWidget, QLayoutWidgetPropertySheet> QLayoutWidgetPropertySheetFactory;

}   // end namespace qdesigner_internal

#endif
