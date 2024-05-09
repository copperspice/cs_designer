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

#include <abstract_formwindow.h>
#include <inplace_editor.h>
#include <lineedit_taskmenu.h>

#include <QAction>
#include <QStyle>
#include <QStyleOption>

namespace qdesigner_internal {

class LineEditTaskMenuInlineEditor : public  TaskMenuInlineEditor
{
 public:
   LineEditTaskMenuInlineEditor(QLineEdit *button, QObject *parent);

 protected:
   QRect editRectangle() const override;
};

LineEditTaskMenuInlineEditor::LineEditTaskMenuInlineEditor(QLineEdit *w, QObject *parent) :
   TaskMenuInlineEditor(w, ValidationSingleLine, QString("text"), parent)
{
}

QRect LineEditTaskMenuInlineEditor::editRectangle() const
{
   QStyleOption opt;
   opt.initFrom(widget());
   return opt.rect;
}

LineEditTaskMenu::LineEditTaskMenu(QLineEdit *lineEdit, QObject *parent)
   : QDesignerTaskMenu(lineEdit, parent), m_editTextAction(new QAction(tr("Change text..."), this))
{
   TaskMenuInlineEditor *editor = new LineEditTaskMenuInlineEditor(lineEdit, this);
   connect(m_editTextAction, &QAction::triggered, editor, &LineEditTaskMenuInlineEditor::editText);
   m_taskActions.append(m_editTextAction);

   QAction *sep = new QAction(this);
   sep->setSeparator(true);
   m_taskActions.append(sep);
}

QAction *LineEditTaskMenu::preferredEditAction() const
{
   return m_editTextAction;
}

QList<QAction *> LineEditTaskMenu::taskActions() const
{
   return m_taskActions + QDesignerTaskMenu::taskActions();
}

}   // end namespace

