/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#include <extension.h>
#include <formeditor.h>
#include <formwindow.h>
#include <layout_widget.h>
#include <layoutwidget_propertysheet.h>

#include <QLayout>

using namespace qdesigner_internal;

QLayoutWidgetPropertySheet::QLayoutWidgetPropertySheet(QLayoutWidget *object, QObject *parent)
   : QDesignerPropertySheet(object, parent)
{
   clearFakeProperties();
}

QLayoutWidgetPropertySheet::~QLayoutWidgetPropertySheet()
{
}

bool QLayoutWidgetPropertySheet::isVisible(int index) const
{
   static const QString layoutPropertyGroup = "Layout";

   if (propertyGroup(index) == layoutPropertyGroup) {
      return QDesignerPropertySheet::isVisible(index);
   }

   return false;
}

void QLayoutWidgetPropertySheet::setProperty(int index, const QVariant &value)
{
   QDesignerPropertySheet::setProperty(index, value);
}

bool QLayoutWidgetPropertySheet::dynamicPropertiesAllowed() const
{
   return false;
}
