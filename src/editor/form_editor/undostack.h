/***********************************************************************
*
* Copyright (c) 2021-2024 Barbara Geller
* Copyright (c) 2021-2024 Ansel Sermersheim
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

#ifndef QDESIGNERUNDOSTACK_H
#define QDESIGNERUNDOSTACK_H

#include <QObject>

class QUndoCommand;
class QUndoStack;

namespace qdesigner_internal {

/* QDesignerUndoStack: A QUndoStack extended by a way of setting it to
 * "dirty" indepently of commands (by modifications without commands
 * such as resizing). Accomplished via bool m_fakeDirty flag. The
 * lifecycle of the QUndoStack is managed by the QUndoGroup. */
class QDesignerUndoStack : public QObject
{
   CS_OBJECT(QDesignerUndoStack)

 public:
   explicit QDesignerUndoStack(QObject *parent = nullptr);
   virtual ~QDesignerUndoStack();

   void clear();
   void push(QUndoCommand *cmd);
   void beginMacro(const QString &text);
   void endMacro();
   int  index() const;

   const QUndoStack *qundoStack() const;
   QUndoStack *qundoStack();

   bool isDirty() const;

   CS_SIGNAL_1(Public, void changed())
   CS_SIGNAL_2(changed)

   CS_SLOT_1(Public, void setDirty(bool un_named_arg1))
   CS_SLOT_2(setDirty)

 private:
   QDesignerUndoStack(const QDesignerUndoStack &) = delete;
   QDesignerUndoStack &operator=(const QDesignerUndoStack &) = delete;

   QUndoStack *m_undoStack;
   bool m_fakeDirty;
};

}   // end namespace qdesigner_internal

#endif
