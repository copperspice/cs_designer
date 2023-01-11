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

#include <htmlhighlighter_p.h>

#include <QTextEdit>
#include <QTextStream>

namespace qdesigner_internal {

HtmlHighlighter::HtmlHighlighter(QTextEdit *textEdit)
   : QSyntaxHighlighter(textEdit->document())
{
   QTextCharFormat entityFormat;
   entityFormat.setForeground(Qt::red);
   setFormatFor(Entity, entityFormat);

   QTextCharFormat tagFormat;
   tagFormat.setForeground(Qt::darkMagenta);
   tagFormat.setFontWeight(QFont::Bold);
   setFormatFor(Tag, tagFormat);

   QTextCharFormat commentFormat;
   commentFormat.setForeground(Qt::gray);
   commentFormat.setFontItalic(true);
   setFormatFor(Comment, commentFormat);

   QTextCharFormat attributeFormat;
   attributeFormat.setForeground(Qt::black);
   attributeFormat.setFontWeight(QFont::Bold);
   setFormatFor(Attribute, attributeFormat);

   QTextCharFormat valueFormat;
   valueFormat.setForeground(Qt::blue);
   setFormatFor(Value, valueFormat);
}

void HtmlHighlighter::setFormatFor(Construct construct, const QTextCharFormat &format)
{
   m_formats[construct] = format;
   rehighlight();
}

void HtmlHighlighter::highlightBlock(const QString &text)
{
   static const QChar tab       = QChar('\t');
   static const QChar space     = QChar(' ');
   static const QChar amp       = QChar('&');
   static const QChar startTag  = QChar('<');
   static const QChar endTag    = QChar('>');
   static const QChar quot      = QChar('"');
   static const QChar apos      = QChar('\'');
   static const QChar semicolon = QChar(';');
   static const QChar equals    = QChar('=');

   static const QString startComment("<!--");
   static const QString endComment("-->");
   static const QString endElement("/>");

   int state = previousBlockState();
   int len   = text.length();
   int start = 0;
   int pos   = 0;

   while (pos < len) {
      switch (state) {
         case NormalState:
         default:
            while (pos < len) {
               QChar ch = text.at(pos);
               if (ch == startTag) {
                  if (text.mid(pos, 4) == startComment) {
                     state = InComment;

                  } else {
                     state = InTag;
                     start = pos;

                     while (pos < len && text.at(pos) != space
                        && text.at(pos) != endTag
                        && text.at(pos) != tab
                        && text.mid(pos, 2) != endElement) {
                        ++pos;
                     }

                     if (text.mid(pos, 2) == endElement) {
                        ++pos;
                     }

                     setFormat(start, pos - start, m_formats[Tag]);
                     break;
                  }
                  break;

               } else if (ch == amp) {
                  start = pos;
                  while (pos < len && text.at(pos++) != semicolon)
                     ;
                  setFormat(start, pos - start,
                     m_formats[Entity]);
               } else {
                  // No tag, comment or entity started, continue...
                  ++pos;
               }
            }
            break;
         case InComment:
            start = pos;
            while (pos < len) {
               if (text.mid(pos, 3) == endComment) {
                  pos += 3;
                  state = NormalState;
                  break;
               } else {
                  ++pos;
               }
            }
            setFormat(start, pos - start, m_formats[Comment]);
            break;
         case InTag:
            QChar quote = QChar::Null;
            while (pos < len) {
               QChar ch = text.at(pos);
               if (quote.isNull()) {
                  start = pos;
                  if (ch == apos || ch == quot) {
                     quote = ch;
                  } else if (ch == endTag) {
                     ++pos;
                     setFormat(start, pos - start, m_formats[Tag]);
                     state = NormalState;
                     break;

                  } else if (text.mid(pos, 2) == endElement) {
                     pos += 2;
                     setFormat(start, pos - start, m_formats[Tag]);
                     state = NormalState;
                     break;

                  } else if (ch != space && text.at(pos) != tab) {
                     // Tag not ending, not a quote and no whitespace, so
                     // we must be dealing with an attribute.
                     ++pos;

                     while (pos < len && text.at(pos) != space
                        && text.at(pos) != tab
                        && text.at(pos) != equals) {
                        ++pos;
                     }
                     setFormat(start, pos - start, m_formats[Attribute]);
                     start = pos;
                  }

               } else if (ch == quote) {
                  quote = QChar::Null;

                  // Anything quoted is a value
                  setFormat(start, pos - start, m_formats[Value]);
               }
               ++pos;
            }
            break;
      }
   }
   setCurrentBlockState(state);
}

}   // end namespace qdesigner_internal
