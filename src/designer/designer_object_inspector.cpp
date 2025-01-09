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

#include <designer_object_inspector.h>

namespace qdesigner_internal {

QDesignerObjectInspector::QDesignerObjectInspector(QWidget *parent, Qt::WindowFlags flags)
   : QDesignerObjectInspectorInterface(parent, flags)
{
}

void QDesignerObjectInspector::mainContainerChanged()
{
}

void Selection::clear()
{
   managed.clear();
   unmanaged.clear();
   objects.clear();
}

bool Selection::empty() const
{
   return managed.empty() && unmanaged.empty() && objects.empty();
}

QObjectList Selection::selection() const
{
   QObjectList rc(objects);

   for (QObject *o : managed) {
      rc.push_back(o);
   }

   for (QObject *o : unmanaged) {
      rc.push_back(o);
   }
   return rc;
}

}   // end namespace

