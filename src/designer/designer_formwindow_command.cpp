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

#include <abstract_actioneditor.h>
#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <abstract_propertyeditor.h>
#include <designer_formwindow_command.h>
#include <designer_object_inspector.h>
#include <designer_object_inspector.h>
#include <designer_propertysheet.h>
#include <extension.h>
#include <layout.h>

#include <metadatabase_p.h>

#include <QLabel>
#include <QVariant>
#include <QWidget>

namespace qdesigner_internal {

QDesignerFormWindowCommand::QDesignerFormWindowCommand(const QString &description,
   QDesignerFormWindowInterface *formWindow,
   QUndoCommand *parent)
   : QUndoCommand(description, parent),
     m_formWindow(formWindow)
{
}

QDesignerFormWindowInterface *QDesignerFormWindowCommand::formWindow() const
{
   return m_formWindow;
}

QDesignerFormEditorInterface *QDesignerFormWindowCommand::core() const
{
   if (QDesignerFormWindowInterface *fw = formWindow()) {
      return fw->core();
   }

   return nullptr;
}

void QDesignerFormWindowCommand::undo()
{
   cheapUpdate();
}

void QDesignerFormWindowCommand::redo()
{
   cheapUpdate();
}

void QDesignerFormWindowCommand::cheapUpdate()
{
   if (core()->objectInspector()) {
      core()->objectInspector()->setFormWindow(formWindow());
   }

   if (core()->actionEditor()) {
      core()->actionEditor()->setFormWindow(formWindow());
   }
}

QDesignerPropertySheetExtension *QDesignerFormWindowCommand::propertySheet(QObject *object) const
{
   return  qt_extension<QDesignerPropertySheetExtension *>(formWindow()->core()->extensionManager(), object);
}

void QDesignerFormWindowCommand::updateBuddies(QDesignerFormWindowInterface *form,
   const QString &old_name,
   const QString &new_name)
{
   QExtensionManager *extensionManager = form->core()->extensionManager();

   typedef QList<QLabel *> LabelList;

   const LabelList label_list = form->findChildren<QLabel *>();
   if (label_list.empty()) {
      return;
   }

   const QString buddyProperty = QString("buddy");
   const QByteArray oldNameU8 = old_name.toUtf8();
   const QByteArray newNameU8 = new_name.toUtf8();

   const LabelList::const_iterator cend = label_list.constEnd();
   for (LabelList::const_iterator it = label_list.constBegin(); it != cend; ++it ) {
      if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(extensionManager, *it)) {
         const int idx = sheet->indexOf(buddyProperty);
         if (idx != -1) {
            const QByteArray oldBuddy = sheet->property(idx).toByteArray();
            if (oldBuddy == oldNameU8) {
               sheet->setProperty(idx, newNameU8);
            }
         }
      }
   }
}

void QDesignerFormWindowCommand::selectUnmanagedObject(QObject *unmanagedObject)
{
   // Keep selection in sync
   if (QDesignerObjectInspector *oi = dynamic_cast<QDesignerObjectInspector *>(core()->objectInspector())) {
      oi->clearSelection();
      oi->selectObject(unmanagedObject);
   }
   core()->propertyEditor()->setObject(unmanagedObject);
}

}   // end namespace qdesigner_internal
