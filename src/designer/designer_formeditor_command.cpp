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

#include <abstract_formeditor.h>
#include <designer_formeditor_command.h>

namespace qdesigner_internal {

QDesignerFormEditorCommand::QDesignerFormEditorCommand(const QString &description, QDesignerFormEditorInterface *core)
   : QUndoCommand(description), m_core(core)
{
}

QDesignerFormEditorInterface *QDesignerFormEditorCommand::core() const
{
   return m_core;
}

}   // end namespace qdesigner_internal


