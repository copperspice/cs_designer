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

#ifndef QDESIGNER_FORMWINDOWCOMMAND_H
#define QDESIGNER_FORMWINDOWCOMMAND_H

#include <QPointer>
#include <QUndoCommand>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QDesignerPropertySheetExtension;

namespace qdesigner_internal {

class QDesignerFormWindowCommand: public QUndoCommand
{
 public:
   QDesignerFormWindowCommand(const QString &description,
      QDesignerFormWindowInterface *formWindow,
      QUndoCommand *parent = nullptr);

   void undo() override;
   void redo() override;

   static void updateBuddies(QDesignerFormWindowInterface *form,
      const QString &old_name, const QString &new_name);

 protected:
   QDesignerFormWindowInterface *formWindow() const;
   QDesignerFormEditorInterface *core() const;
   QDesignerPropertySheetExtension *propertySheet(QObject *object) const;

   void cheapUpdate();

   void selectUnmanagedObject(QObject *unmanagedObject);

 private:
   QPointer<QDesignerFormWindowInterface> m_formWindow;
};

} // namespace qdesigner_internal

#endif
