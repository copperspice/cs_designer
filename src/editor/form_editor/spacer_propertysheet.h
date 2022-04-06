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

#ifndef SPACER_PROPERTYSHEET_H
#define SPACER_PROPERTYSHEET_H

#include <designer_propertysheet.h>
#include <extensionfactory_p.h>
#include <spacer_widget_p.h>

namespace qdesigner_internal {

class SpacerPropertySheet: public QDesignerPropertySheet
{
   CS_OBJECT(SpacerPropertySheet)
   CS_INTERFACES(QDesignerPropertySheetExtension)

 public:
   explicit SpacerPropertySheet(Spacer *object, QObject *parent = nullptr);
   virtual ~SpacerPropertySheet();

   void setProperty(int index, const QVariant &value) override;
   bool isVisible(int index) const override;

   bool dynamicPropertiesAllowed() const override;
};

using SpacerPropertySheetFactory = QDesignerPropertySheetFactory<Spacer, SpacerPropertySheet>;

}  // end namespace qdesigner_internal

#endif
