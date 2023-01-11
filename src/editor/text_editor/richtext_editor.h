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

#ifndef RICHTEXT_EDITOR_H
#define RICHTEXT_EDITOR_H

#include <QTextEdit>
#include <QDialog>

class QTabWidget;
class QToolBar;

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class RichTextEditor;

class RichTextEditorDialog : public QDialog
{
   CS_OBJECT(RichTextEditorDialog)
 public:
   explicit RichTextEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
   ~RichTextEditorDialog();

   int showDialog();
   void setDefaultFont(const QFont &font);
   void setText(const QString &text);
   QString text(Qt::TextFormat format = Qt::AutoText) const;

 private:
   CS_SLOT_1(Private, void tabIndexChanged(int newIndex))
   CS_SLOT_2(tabIndexChanged)
   CS_SLOT_1(Private, void richTextChanged())
   CS_SLOT_2(richTextChanged)
   CS_SLOT_1(Private, void sourceChanged())
   CS_SLOT_2(sourceChanged)

 private:
   enum TabIndex { RichTextIndex, SourceIndex };
   enum State { Clean, RichTextChanged, SourceChanged };
   RichTextEditor *m_editor;
   QTextEdit      *m_text_edit;
   QTabWidget     *m_tab_widget;
   State m_state;
   QDesignerFormEditorInterface *m_core;
   int m_initialTab;
};

} // namespace qdesigner_internal



#endif // RITCHTEXTEDITOR_H
