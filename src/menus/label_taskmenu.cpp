/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#include <abstract_formwindow.h>
#include <inplace_editor.h>
#include <label_taskmenu.h>

#include <QAction>
#include <QStyle>
#include <QStyleOption>
#include <QTextDocument>

static const QString textPropertyC = "text";

namespace qdesigner_internal {

class LabelTaskMenuInlineEditor : public  TaskMenuInlineEditor
{
 public:
   LabelTaskMenuInlineEditor(QLabel *button, QObject *parent);

 protected:
   QRect editRectangle() const override;
};

LabelTaskMenuInlineEditor::LabelTaskMenuInlineEditor(QLabel *w, QObject *parent)
   : TaskMenuInlineEditor(w, ValidationRichText, textPropertyC, parent)
{
}

QRect LabelTaskMenuInlineEditor::editRectangle() const
{
   QStyleOptionButton opt;
   opt.initFrom(widget());

   return opt.rect;
}

LabelTaskMenu::LabelTaskMenu(QLabel *label, QObject *parent)
   : QDesignerTaskMenu(label, parent), m_label(label),
     m_editRichTextAction(new QAction(tr("Change rich text..."), this)),
     m_editPlainTextAction(new QAction(tr("Change plain text..."), this))
{
   LabelTaskMenuInlineEditor *editor = new LabelTaskMenuInlineEditor(label, this);
   connect(m_editPlainTextAction, &QAction::triggered, editor, &LabelTaskMenuInlineEditor::editText);
   m_taskActions.append(m_editPlainTextAction);

   connect(m_editRichTextAction, &QAction::triggered, this, &LabelTaskMenu::editRichText);
   m_taskActions.append(m_editRichTextAction);

   QAction *sep = new QAction(this);
   sep->setSeparator(true);
   m_taskActions.append(sep);
}

QAction *LabelTaskMenu::preferredEditAction() const
{
   if (m_label->textFormat () == Qt::PlainText) {
      return m_editPlainTextAction;
   }

   return Qt::mightBeRichText(m_label->text()) ? m_editRichTextAction : m_editPlainTextAction;
}

QList<QAction *> LabelTaskMenu::taskActions() const
{
   return m_taskActions + QDesignerTaskMenu::taskActions();
}

void LabelTaskMenu::editRichText()
{
   changeTextProperty(textPropertyC, QString(), MultiSelectionMode, m_label->textFormat());
}

}   // end namespace

