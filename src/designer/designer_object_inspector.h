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

#ifndef DESIGNEROBJECTINSPECTOR_H
#define DESIGNEROBJECTINSPECTOR_H

#include <abstract_objectinspector.h>

#include <QList>

class QDesignerDnDItemInterface;

namespace qdesigner_internal {

struct Selection {
   bool empty() const;
   void clear();

   // Merge all lists
   QObjectList selection() const;

   // Selection in cursor (managed widgets)
   QWidgetList managed;

   // Unmanaged widgets
   QWidgetList unmanaged;

   // Remaining selected objects (non-widgets)
   QObjectList objects;
};

// Extends the QDesignerObjectInspectorInterface by functionality
// to access the selection

class QDesignerObjectInspector : public QDesignerObjectInspectorInterface
{
   CS_OBJECT(QDesignerObjectInspector)

 public:
   explicit QDesignerObjectInspector(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   // Select a qobject unmanaged by form window
   virtual bool selectObject(QObject *o) = 0;
   virtual void getSelection(Selection &s) const = 0;
   virtual void clearSelection() = 0;

   CS_SLOT_1(Public, virtual void mainContainerChanged())
   CS_SLOT_2(mainContainerChanged)
};

}  // namespace qdesigner_internal


#endif // DESIGNEROBJECTINSPECTOR_H
