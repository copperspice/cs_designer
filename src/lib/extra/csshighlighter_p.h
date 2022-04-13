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

#ifndef CSSHIGHLIGHTER_H
#define CSSHIGHLIGHTER_H

#include <QSyntaxHighlighter>

namespace qdesigner_internal {

class CssHighlighter : public QSyntaxHighlighter
{
   CS_OBJECT(CssHighlighter)

 public:
   explicit CssHighlighter(QTextDocument *document);

 protected:
   void highlightBlock(const QString &) override;
   void highlight(const QString &, int, int, int/*State*/);

 private:
   enum State { Selector, Property, Value, Pseudo, Pseudo1, Pseudo2, Quote,
      MaybeComment, Comment, MaybeCommentEnd
   };
};

} // namespace qdesigner_internal



#endif // CSSHIGHLIGHTER_H
