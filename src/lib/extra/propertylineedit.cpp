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

#include <propertylineedit_p.h>

#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QMenu>

namespace qdesigner_internal {

PropertyLineEdit::PropertyLineEdit(QWidget *parent)
   : QLineEdit(parent), m_wantNewLine(false)
{
}

bool PropertyLineEdit::event(QEvent *e)
{
   // handle 'Select all' here as it is not done in the QLineEdit
   if (e->type() == QEvent::ShortcutOverride && !isReadOnly()) {
      QKeyEvent *ke = static_cast<QKeyEvent *> (e);
      if (ke->modifiers() & Qt::ControlModifier) {
         if (ke->key() == Qt::Key_A) {
            ke->accept();
            return true;
         }
      }
   }
   return QLineEdit::event(e);
}

void PropertyLineEdit::insertNewLine()
{
   insertText(QString("\\n"));
}

void PropertyLineEdit::insertText(const QString &text)
{
   // position cursor after new text and grab focus
   const int oldCursorPosition = cursorPosition ();
   insert(text);
   setCursorPosition (oldCursorPosition + text.length());
   setFocus(Qt::OtherFocusReason);
}

void PropertyLineEdit::contextMenuEvent(QContextMenuEvent *event)
{
   QMenu  *menu = createStandardContextMenu ();

   if (m_wantNewLine) {
      menu->addSeparator();
      menu->addAction(tr("Insert line break"), this, &PropertyLineEdit::insertNewLine);
   }

   menu->exec(event->globalPos());
}

}   // end namespace

