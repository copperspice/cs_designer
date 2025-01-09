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

#ifndef LAYOUT_PROPERTYSHEET_H
#define LAYOUT_PROPERTYSHEET_H

#include <designer_propertysheet.h>

#include <extensionfactory_p.h>

#include <QLayout>

class QDesignerFormEditorInterface;
class DomLayout;

namespace qdesigner_internal {

class LayoutPropertySheet: public QDesignerPropertySheet
{
   CS_OBJECT(LayoutPropertySheet)
   CS_INTERFACES(QDesignerPropertySheetExtension)

 public:
   explicit LayoutPropertySheet(QLayout *object, QObject *parent = nullptr);
   virtual ~LayoutPropertySheet();

   void setProperty(int index, const QVariant &value) override;
   QVariant property(int index) const override;
   bool reset(int index) override;
   void setChanged(int index, bool changed) override;

   static void stretchAttributesToDom(QDesignerFormEditorInterface *core, QLayout *lt, DomLayout *domLayout);
   static void markChangedStretchProperties(QDesignerFormEditorInterface *core, QLayout *lt, const DomLayout *domLayout);

 private:
   QLayout *m_layout;
};

typedef QDesignerPropertySheetFactory<QLayout, LayoutPropertySheet>  LayoutPropertySheetFactory;

}   // end namespace qdesigner_internal

#endif
