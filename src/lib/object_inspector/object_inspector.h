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

#ifndef OBJECT_INSPECTOR_H
#define OBJECT_INSPECTOR_H

#include <designer_object_inspector.h>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QItemSelection;

namespace qdesigner_internal {

class ObjectInspector: public QDesignerObjectInspector
{
   CS_OBJECT(ObjectInspector)

 public:
   explicit ObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
   virtual ~ObjectInspector();

   QDesignerFormEditorInterface *core() const override;

   void getSelection(Selection &s) const override;
   bool selectObject(QObject *o) override;
   void clearSelection() override;

   void setFormWindow(QDesignerFormWindowInterface *formWindow) override;

   CS_SLOT_1(Public, void mainContainerChanged())
   CS_SLOT_2(mainContainerChanged)

 protected:
   void dragEnterEvent (QDragEnterEvent *event) override;
   void dragMoveEvent(QDragMoveEvent *event) override;
   void dragLeaveEvent(QDragLeaveEvent *event) override;
   void dropEvent (QDropEvent *event) override;

 private:
   CS_SLOT_1(Private, void slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected))
   CS_SLOT_2(slotSelectionChanged)
   CS_SLOT_1(Private, void slotPopupContextMenu(const QPoint &pos))
   CS_SLOT_2(slotPopupContextMenu)
   CS_SLOT_1(Private, void slotHeaderDoubleClicked(int column))
   CS_SLOT_2(slotHeaderDoubleClicked)

   class ObjectInspectorPrivate;
   ObjectInspectorPrivate *m_impl;
};

}   // end namespace - qdesigner_internal

#endif


