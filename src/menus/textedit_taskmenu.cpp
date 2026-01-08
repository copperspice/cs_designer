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
#include <textedit_taskmenu.h>

#include <QAction>
#include <QEvent>

namespace qdesigner_internal {

TextEditTaskMenu::TextEditTaskMenu(QTextEdit *textEdit, QObject *parent)
   : QDesignerTaskMenu(textEdit, parent), m_format(Qt::RichText),
     m_property(QString("html")), m_windowTitle(tr("Edit HTML")),
     m_editTextAction(new QAction(tr("Change HTML..."), this))
{
   initialize();
}

TextEditTaskMenu::TextEditTaskMenu(QPlainTextEdit *textEdit, QObject *parent)
   : QDesignerTaskMenu(textEdit, parent), m_format(Qt::PlainText),
     m_property(QString("plainText")), m_windowTitle(tr("Edit Text")),
     m_editTextAction(new QAction(tr("Change Plain Text..."), this))
{
   initialize();
}

void TextEditTaskMenu::initialize()
{
   connect(m_editTextAction, &QAction::triggered, this, &TextEditTaskMenu::editText);
   m_taskActions.append(m_editTextAction);

   QAction *sep = new QAction(this);
   sep->setSeparator(true);
   m_taskActions.append(sep);
}

TextEditTaskMenu::~TextEditTaskMenu()
{
}

QAction *TextEditTaskMenu::preferredEditAction() const
{
   return m_editTextAction;
}

QList<QAction *> TextEditTaskMenu::taskActions() const
{
   return m_taskActions + QDesignerTaskMenu::taskActions();
}

void TextEditTaskMenu::editText()
{
   changeTextProperty(m_property, m_windowTitle, MultiSelectionMode, m_format);
}

}   // end namespace

