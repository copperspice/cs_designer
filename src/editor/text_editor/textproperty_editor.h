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

#ifndef TEXTPROPERTY_EDITOR_H
#define TEXTPROPERTY_EDITOR_H

#include <shared_enums_p.h>

#include <QWidget>

namespace qdesigner_internal {

class PropertyLineEdit;

// Inline-Editor for text properties. Does escaping of newline characters
// to '\n' and back and provides validation modes. The interface
// corresponds to that of QLineEdit.

class TextPropertyEditor : public QWidget
{
   TextPropertyEditor(const TextPropertyEditor &);
   TextPropertyEditor &operator=(const TextPropertyEditor &);

   CS_OBJECT(TextPropertyEditor)

   CS_PROPERTY_READ(text, text)
   CS_PROPERTY_WRITE(text, setText)
   CS_PROPERTY_USER(text, true)

 public:
   enum EmbeddingMode {
      // Stand-alone widget
      EmbeddingNone,
      // Disable frame
      EmbeddingTreeView,
      // For editing in forms
      EmbeddingInPlace
   };

   enum UpdateMode {
      // Emit textChanged() as the user types
      UpdateAsYouType,
      // Emit textChanged() only when the user finishes (for QUrl, etc.)
      UpdateOnFinished
   };

   explicit TextPropertyEditor(QWidget *parent = nullptr, EmbeddingMode embeddingMode = EmbeddingNone,
      TextPropertyValidationMode validationMode = ValidationMultiLine);

   TextPropertyValidationMode textPropertyValidationMode() const {
      return m_validationMode;
   }
   void setTextPropertyValidationMode(TextPropertyValidationMode vm);

   UpdateMode updateMode() const                {
      return m_updateMode;
   }
   void setUpdateMode(UpdateMode um) {
      m_updateMode = um;
   }

   QString text() const;

   QSize sizeHint () const override;
   QSize minimumSizeHint () const override;

   void setAlignment(Qt::Alignment alignment);

   bool hasAcceptableInput() const;

   // installs an event filter object on the private QLineEdit
   void installEventFilter(QObject *filterObject);

   // Replace newline characters by literal "\n" for inline editing
   // in mode ValidationMultiLine
   static QString stringToEditorString(const QString &s, TextPropertyValidationMode validationMode = ValidationMultiLine);

   // Replace literal "\n"  by actual new lines in mode ValidationMultiLine
   static QString editorStringToString(const QString &s, TextPropertyValidationMode validationMode = ValidationMultiLine);

   // Returns whether newline characters are valid in validationMode.
   static bool multiLine(TextPropertyValidationMode validationMode);

   CS_SIGNAL_1(Public, void textChanged(const QString &text))
   CS_SIGNAL_2(textChanged, text)
   CS_SIGNAL_1(Public, void editingFinished())
   CS_SIGNAL_2(editingFinished)

   CS_SLOT_1(Public, void setText(const QString &text))
   CS_SLOT_2(setText)
   CS_SLOT_1(Public, void selectAll())
   CS_SLOT_2(selectAll)
   CS_SLOT_1(Public, void clear())
   CS_SLOT_2(clear)

 protected:
   void resizeEvent(QResizeEvent *event ) override;

 private:
   CS_SLOT_1(Private, void slotTextChanged(const QString &text))
   CS_SLOT_2(slotTextChanged)
   CS_SLOT_1(Private, void slotTextEdited())
   CS_SLOT_2(slotTextEdited)
   CS_SLOT_1(Private, void slotEditingFinished())
   CS_SLOT_2(slotEditingFinished)

   void setRegExpValidator(const QString &pattern);
   void markIntermediateState();

   TextPropertyValidationMode m_validationMode;
   UpdateMode m_updateMode;
   PropertyLineEdit *m_lineEdit;

   // Cached text containing real newline characters.
   QString m_cachedText;
   bool m_textEdited;
};

}   // end namespace

#endif
