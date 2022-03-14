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

#include <qdesignerundostack.h>

#include <QUndoStack>
#include <QUndoCommand>

namespace qdesigner_internal {

QDesignerUndoStack::QDesignerUndoStack(QObject *parent) :
   QObject(parent),
   m_undoStack(new QUndoStack),
   m_fakeDirty(false)
{
   connect(m_undoStack, &QUndoStack::indexChanged, this, &QDesignerUndoStack::changed);
}

QDesignerUndoStack::~QDesignerUndoStack()
{
   // QUndoStack is managed by the QUndoGroup
}

void QDesignerUndoStack::clear()
{
   m_fakeDirty  = false;
   m_undoStack->clear();
}

void QDesignerUndoStack::push(QUndoCommand *cmd)
{
   m_undoStack->push(cmd);
}

void QDesignerUndoStack::beginMacro(const QString &text)
{
   m_undoStack->beginMacro(text);
}

void QDesignerUndoStack::endMacro()
{
   m_undoStack->endMacro();
}

int  QDesignerUndoStack::index() const
{
   return m_undoStack->index();
}

const QUndoStack *QDesignerUndoStack::qundoStack() const
{
   return m_undoStack;
}
QUndoStack *QDesignerUndoStack::qundoStack()
{
   return m_undoStack;
}

bool QDesignerUndoStack::isDirty() const
{
   return m_fakeDirty || !m_undoStack->isClean();
}

void QDesignerUndoStack::setDirty(bool v)
{
   if (isDirty() == v) {
      return;
   }

   if (v) {
      m_fakeDirty = true;
      emit changed();
   } else {
      m_fakeDirty = false;
      m_undoStack->setClean();
   }
}

}   // end namespace qdesigner_internal

