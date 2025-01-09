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

#ifndef HTMLHIGHLIGHTER_H
#define HTMLHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class QTextEdit;

namespace qdesigner_internal {

class HtmlHighlighter : public QSyntaxHighlighter
{
   CS_OBJECT(HtmlHighlighter)

 public:
   enum Construct {
      Entity,
      Tag,
      Comment,
      Attribute,
      Value,
      LastConstruct = Value
   };

   HtmlHighlighter(QTextEdit *textEdit);

   void setFormatFor(Construct construct, const QTextCharFormat &format);

   QTextCharFormat formatFor(Construct construct) const {
      return m_formats[construct];
   }

 protected:
   enum State {
      NormalState = -1,
      InComment,
      InTag
   };

   void highlightBlock(const QString &text) override;

 private:
   QTextCharFormat m_formats[LastConstruct + 1];
};

} // namespace qdesigner_internal

#endif
