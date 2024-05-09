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

#ifndef PLAINTEXT_EDITOR_H
#define PLAINTEXT_EDITOR_H

#include <QDialog>
#include <QPlainTextEdit>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class PlainTextEditorDialog : public QDialog
{
   CS_OBJECT(PlainTextEditorDialog)

 public:
   explicit PlainTextEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
   ~PlainTextEditorDialog();

   int showDialog();

   void setDefaultFont(const QFont &font);

   void setText(const QString &text);
   QString text() const;

 private:
   QPlainTextEdit *m_editor;
   QDesignerFormEditorInterface *m_core;
};

}   // end namespace qdesigner_internal

#endif // RITCHTEXTEDITOR_H
