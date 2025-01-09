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

#include <textedit_findwidget.h>

#include <QCheckBox>
#include <QTextCursor>
#include <QTextEdit>

TextEditFindWidget::TextEditFindWidget(FindFlags flags, QWidget *parent)
   : AbstractFindWidget(flags, parent), m_textEdit(nullptr)
{
}

void TextEditFindWidget::setTextEdit(QTextEdit *textEdit)
{
   if (m_textEdit) {
      m_textEdit->removeEventFilter(this);
   }

   m_textEdit = textEdit;

   if (m_textEdit) {
      m_textEdit->installEventFilter(this);
   }
}

void TextEditFindWidget::deactivate()
{
   // Pass focus to the text edit
   if (m_textEdit) {
      m_textEdit->setFocus();
   }

   AbstractFindWidget::deactivate();
}

void TextEditFindWidget::find(const QString &ttf, bool skipCurrent, bool backward, bool *found, bool *wrapped)
{
   if (!m_textEdit) {
      return;
   }

   QTextCursor cursor = m_textEdit->textCursor();
   QTextDocument *doc = m_textEdit->document();

   if (!doc || cursor.isNull()) {
      return;
   }

   if (cursor.hasSelection()) {
      cursor.setPosition((skipCurrent && !backward) ? cursor.position() : cursor.anchor());
   }

   *found = true;
   QTextCursor newCursor = cursor;

   if (!ttf.isEmpty()) {
      QTextDocument::FindFlags options;

      if (backward) {
         options |= QTextDocument::FindBackward;
      }

      if (caseSensitive()) {
         options |= QTextDocument::FindCaseSensitively;
      }

      if (wholeWords()) {
         options |= QTextDocument::FindWholeWords;
      }

      newCursor = doc->find(ttf, cursor, options);
      if (newCursor.isNull()) {
         QTextCursor ac(doc);
         ac.movePosition(options & QTextDocument::FindBackward
            ? QTextCursor::End : QTextCursor::Start);
         newCursor = doc->find(ttf, ac, options);
         if (newCursor.isNull()) {
            *found = false;
            newCursor = cursor;
         } else {
            *wrapped = true;
         }
      }
   }

   if (!isVisible()) {
      show();
   }

   m_textEdit->setTextCursor(newCursor);
}

