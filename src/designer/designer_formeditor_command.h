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

#ifndef QDESIGNER_FORMEDITORCOMMAND_H
#define QDESIGNER_FORMEDITORCOMMAND_H

#include <QPointer>
#include <QUndoCommand>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class QDesignerFormEditorCommand: public QUndoCommand
{

 public:
   QDesignerFormEditorCommand(const QString &description, QDesignerFormEditorInterface *core);

 protected:
   QDesignerFormEditorInterface *core() const;

 private:
   QPointer<QDesignerFormEditorInterface> m_core;
};

} // namespace qdesigner_internal

#endif // QDESIGNER_FORMEDITORCOMMAND_H
