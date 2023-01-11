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

#include <default_layoutdecoration.h>
#include <abstract_metadatabase.h>
#include <abstract_formwindow.h>
#include <abstract_formeditor.h>
#include <layout_widget.h>
#include <layout_info.h>

namespace qdesigner_internal {

QDesignerLayoutDecorationFactory::QDesignerLayoutDecorationFactory(QExtensionManager *parent)
   : QExtensionFactory(parent)
{
}

QObject *QDesignerLayoutDecorationFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
   if (!object->isWidgetType() || iid != CS_TYPEID(QDesignerLayoutDecorationExtension)) {
      return nullptr;
   }

   QWidget *widget = dynamic_cast<QWidget *>(object);

   if (const QLayoutWidget *layoutWidget = dynamic_cast<const QLayoutWidget *>(widget)) {
      return QLayoutSupport::createLayoutSupport(layoutWidget->formWindow(), widget, parent);
   }

   if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(widget))
      if (LayoutInfo::managedLayout(fw->core(), widget)) {
         return QLayoutSupport::createLayoutSupport(fw, widget, parent);
      }

   return nullptr;
}

}   // end namespace


