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

#include <stylesheet_editor.h>
#include <textproperty_editor.h>

#include <propertylineedit_p.h>

#include <QAbstractItemView>
#include <QCompleter>
#include <QFile>
#include <QLineEdit>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QUrl>
#include <QValidator>

namespace {
const QChar NewLineChar('\n');
const QString EscapedNewLine("\\n");

// A validator that replaces offending strings
class ReplacementValidator : public QValidator
{
 public:
   ReplacementValidator (QObject *parent, const QString &offending, const QString &replacement);
   void fixup ( QString &input ) const override;
   State validate ( QString &input, int &pos) const override;

 private:
   const QString m_offending;
   const QString m_replacement;
};

ReplacementValidator::ReplacementValidator (QObject *parent, const QString &offending, const QString &replacement)
   : QValidator(parent), m_offending(offending), m_replacement(replacement)
{
}

void ReplacementValidator::fixup ( QString &input ) const
{
   input.replace(m_offending, m_replacement);
}

QValidator::State ReplacementValidator::validate ( QString &input, int &/* pos */) const
{
   fixup (input);
   return Acceptable;
}

// A validator for style sheets. Does newline handling and validates sheets.
class StyleSheetValidator : public ReplacementValidator
{
 public:
   StyleSheetValidator (QObject *parent);
   State validate(QString &input, int &pos) const override;
};

StyleSheetValidator::StyleSheetValidator (QObject *parent) :
   ReplacementValidator(parent, NewLineChar, EscapedNewLine)
{
}

QValidator::State StyleSheetValidator::validate(QString &input, int &pos) const
{
   // base class
   const QValidator::State state = ReplacementValidator:: validate(input, pos);

   if (state != QValidator::State::Acceptable) {
      return state;
   }

   // now check style sheet, create string with newlines
   const QString styleSheet = qdesigner_internal::TextPropertyEditor::editorStringToString(input,
         qdesigner_internal::ValidationStyleSheet);

   return QValidator::State::Acceptable;
}

// A validator for URLs based on QUrl. Enforces complete protocol
// specification with a completer (adds a trailing slash)
class UrlValidator : public QValidator
{
 public:
   UrlValidator(QCompleter *completer, QObject *parent);

   State validate(QString &input, int &pos) const override;
   void fixup(QString &input) const override;
 private:
   QUrl guessUrlFromString(const QString &string) const;
   QCompleter *m_completer;
};

UrlValidator::UrlValidator(QCompleter *completer, QObject *parent) :
   QValidator(parent),
   m_completer(completer)
{
}

QValidator::State UrlValidator::validate(QString &input, int &pos) const
{
   (void) pos;

   if (input.isEmpty()) {
      return Acceptable;
   }

   const QUrl url(input, QUrl::StrictMode);

   if (!url.isValid() || url.isEmpty()) {
      return Intermediate;
   }

   if (url.scheme().isEmpty()) {
      return Intermediate;
   }

   if (url.host().isEmpty() && url.path().isEmpty()) {
      return Intermediate;
   }

   return Acceptable;
}

void UrlValidator::fixup(QString &input) const
{
   // Don't try to fixup if the user is busy selecting a completion proposal
   if (const QAbstractItemView *iv = m_completer->popup()) {
      if (iv->isVisible()) {
         return;
      }
   }

   input = guessUrlFromString(input).toString();
}

QUrl UrlValidator::guessUrlFromString(const QString &string) const
{
   const QString urlStr = string.trimmed();

   QRegularExpression qualifiedUrl("^[a-zA-Z]+\\:.*", QPatternOption::ExactMatchOption);

   auto match = qualifiedUrl.match(urlStr);

   // Check if it looks like a qualified URL
   const bool hasSchema = match.hasMatch();

   if (hasSchema) {
      const QUrl url(urlStr, QUrl::TolerantMode);
      if (url.isValid()) {
         return url;
      }
   }

   // Might be a resource
   if (string.startsWith(":/")) {
      return QUrl("qrc" + string);
   }

   // Might be a file
   if (QFile::exists(urlStr)) {
      return QUrl::fromLocalFile(urlStr);
   }

   // Might be a short url, try to detect the schema
   if (! hasSchema) {
      const int dotIndex = urlStr.indexOf('.');

      if (dotIndex != -1) {
         const QString prefix = urlStr.left(dotIndex).toLower();
         QString urlString;

         if (prefix == "ftp") {
            urlString += prefix;
         } else {
            urlString += "http";
         }

         urlString += "://";
         urlString += urlStr;
         const QUrl url(urlString, QUrl::TolerantMode);

         if (url.isValid()) {
            return url;
         }
      }
   }

   // Fall back to QUrl's own tolerant parser.
   return QUrl(string, QUrl::TolerantMode);
}

}

namespace qdesigner_internal {

TextPropertyEditor::TextPropertyEditor(QWidget *parent, EmbeddingMode embeddingMode, TextPropertyValidationMode validationMode)
   : QWidget(parent), m_validationMode(ValidationSingleLine), m_updateMode(UpdateAsYouType),
     m_lineEdit(new PropertyLineEdit(this)), m_textEdited(false)
{
   switch (embeddingMode) {
      case EmbeddingNone:
         break;

      case EmbeddingTreeView:
         m_lineEdit->setFrame(false);
         break;

      case EmbeddingInPlace:
         m_lineEdit->setFrame(false);
         Q_ASSERT(parent);

         m_lineEdit->setBackgroundRole(parent->backgroundRole());
         break;
   }

   setFocusProxy(m_lineEdit);

   connect(m_lineEdit, &QLineEdit::editingFinished, this, &TextPropertyEditor::editingFinished);
   connect(m_lineEdit, &QLineEdit::returnPressed,   this, &TextPropertyEditor::slotEditingFinished);
   connect(m_lineEdit, &QLineEdit::textChanged,     this, &TextPropertyEditor::slotTextChanged);
   connect(m_lineEdit, &QLineEdit::textEdited,      this, &TextPropertyEditor::slotTextEdited);

   setTextPropertyValidationMode(validationMode);
}

void TextPropertyEditor::setTextPropertyValidationMode(TextPropertyValidationMode vm)
{
   m_validationMode = vm;
   m_lineEdit->setWantNewLine(multiLine(m_validationMode));

   switch (m_validationMode) {
      case ValidationStyleSheet:
         m_lineEdit->setValidator(new  StyleSheetValidator(m_lineEdit));
         m_lineEdit->setCompleter(nullptr);
         break;

      case ValidationMultiLine:
      case ValidationRichText:
         // Set a validator which replaces newline characters by literal "\\n".
         // While it is not possible to actually type a newline  characters,
         // it can be pasted into the line edit.
         m_lineEdit->setValidator(new ReplacementValidator(m_lineEdit, NewLineChar, EscapedNewLine));
         m_lineEdit->setCompleter(nullptr);
         break;

      case ValidationSingleLine:
         // Set a validator which replaces newline characters by a blank
         m_lineEdit->setValidator(new ReplacementValidator(m_lineEdit, NewLineChar, " "));
         m_lineEdit->setCompleter(nullptr);
         break;

      case ValidationObjectName:
         setRegExpValidator("[_a-zA-Z][_a-zA-Z0-9]{0,1023}");
         m_lineEdit->setCompleter(nullptr);
         break;

      case ValidationObjectNameScope:
         setRegExpValidator("[_a-zA-Z:][_a-zA-Z0-9:]{0,1023}");
         m_lineEdit->setCompleter(nullptr);
         break;

      case ValidationURL: {
         static QStringList urlCompletions;

         if (urlCompletions.empty()) {
            urlCompletions.push_back("about:blank");
            urlCompletions.push_back("https://");
            urlCompletions.push_back("http://");
            urlCompletions.push_back("http://www.");
            urlCompletions.push_back("https://www.");
            urlCompletions.push_back("file://");
            urlCompletions.push_back("ftp://");
            urlCompletions.push_back("data:");
            urlCompletions.push_back("data:text/html,");
            urlCompletions.push_back("qrc:/");
         }

         QCompleter *completer = new QCompleter(urlCompletions, m_lineEdit);
         m_lineEdit->setCompleter(completer);
         m_lineEdit->setValidator(new UrlValidator(completer, m_lineEdit));
      }

      break;
   }

   setFocusProxy(m_lineEdit);
   setText(m_cachedText);
   markIntermediateState();
}

void TextPropertyEditor::setRegExpValidator(const QString &pattern)
{
   QRegularExpression regex(pattern);
   Q_ASSERT(regex.isValid());

   m_lineEdit->setValidator(new QRegularExpressionValidator(regex, m_lineEdit));
}

QString TextPropertyEditor::text() const
{
   return m_cachedText;
}

void TextPropertyEditor::markIntermediateState()
{
   if (m_lineEdit->hasAcceptableInput()) {
      m_lineEdit->setPalette(QPalette());

   } else {
      QPalette palette = m_lineEdit->palette();
      palette.setColor(QPalette::Active, QPalette::Text, Qt::red);
      m_lineEdit->setPalette(palette);
   }

}

void TextPropertyEditor::setText(const QString &text)
{
   m_cachedText = text;
   m_lineEdit->setText(stringToEditorString(text, m_validationMode));
   markIntermediateState();
   m_textEdited = false;
}

void TextPropertyEditor::slotTextEdited()
{
   m_textEdited = true;
}

void  TextPropertyEditor::slotTextChanged(const QString &text)
{
   m_cachedText = editorStringToString(text, m_validationMode);
   markIntermediateState();

   if (m_updateMode == UpdateAsYouType) {
      emit textChanged(m_cachedText);
   }
}

void TextPropertyEditor::slotEditingFinished()
{
   if (m_updateMode == UpdateOnFinished && m_textEdited) {
      emit textChanged(m_cachedText);
      m_textEdited = false;
   }
}

void TextPropertyEditor::selectAll()
{
   m_lineEdit->selectAll();
}

void TextPropertyEditor::clear()
{
   m_lineEdit->clear();
}

void TextPropertyEditor::setAlignment(Qt::Alignment alignment)
{
   m_lineEdit->setAlignment(alignment);
}

void TextPropertyEditor::installEventFilter(QObject *filterObject)
{
   if (m_lineEdit) {
      m_lineEdit->installEventFilter(filterObject);
   }
}

void TextPropertyEditor::resizeEvent ( QResizeEvent *event )
{
   m_lineEdit->resize( event->size());
}

QSize TextPropertyEditor::sizeHint () const
{
   return  m_lineEdit->sizeHint ();
}

QSize TextPropertyEditor::minimumSizeHint () const
{
   return  m_lineEdit->minimumSizeHint ();
}

// Returns whether newline characters are valid in validationMode.
bool TextPropertyEditor::multiLine(TextPropertyValidationMode validationMode)
{
   return validationMode == ValidationMultiLine || validationMode == ValidationStyleSheet || validationMode == ValidationRichText;
}

// Replace newline characters literal "\n"  for inline editing in mode ValidationMultiLine
QString TextPropertyEditor::stringToEditorString(const QString &s, TextPropertyValidationMode  validationMode)
{
   if (s.isEmpty() || !multiLine(validationMode)) {
      return s;
   }

   QString rc(s);

   // protect backslashes
   rc.replace('\\', "\\\\");

   // escape newlines
   rc.replace(NewLineChar, EscapedNewLine);
   return rc;

}

// Replace literal "\n"  by actual new lines for inline editing in mode ValidationMultiLine
// Note: As the properties are updated while the user types, it is important
// that trailing slashes ('bla\') are not deleted nor ignored, else this will
// cause jumping of the  cursor
QString  TextPropertyEditor::editorStringToString(const QString &s, TextPropertyValidationMode  validationMode)
{
   if (s.isEmpty() || !multiLine(validationMode)) {
      return s;
   }

   QString rc(s);

   for (int pos = 0; (pos = rc.indexOf('\\', pos)) >= 0 ; ) {
      // found an escaped character. If not a newline or at end of string, leave as is, else insert '\n'
      const int nextpos = pos + 1;

      if (nextpos  >= rc.length()) { // trailing '\\'
         break;
      }

      // Escaped NewLine
      if (rc.at(nextpos) == 'n') {
         rc.replace(nextpos, 1, NewLineChar);
      }

      // Remove escape, go past escaped
      rc.remove(pos, 1);
      ++pos;
   }

   return rc;
}

bool TextPropertyEditor::hasAcceptableInput() const
{
   return m_lineEdit->hasAcceptableInput();
}

}   // end namespace
