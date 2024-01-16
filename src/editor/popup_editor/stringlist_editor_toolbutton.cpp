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

#include <stringlist_editor.h>
#include <stringlist_editor_toolbutton.h>

#include <QDebug>

using namespace qdesigner_internal;

StringListEditorButton::StringListEditorButton(const QStringList &stringList, QWidget *parent)
   : QToolButton(parent), m_stringList(stringList)
{
   setFocusPolicy(Qt::NoFocus);
   setText(tr("Change String List"));
   setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

   connect(this, &QAbstractButton::clicked, this, &StringListEditorButton::showStringListEditor);
}

StringListEditorButton::~StringListEditorButton()
{
}

void StringListEditorButton::setStringList(const QStringList &stringList)
{
   m_stringList = stringList;
}

void StringListEditorButton::showStringListEditor()
{
   int result;
   QStringList list = StringListEditor::getStringList(nullptr, m_stringList, &result);

   if (result == QDialog::Accepted) {
      m_stringList = list;
      emit stringListChanged(m_stringList);
   }
}
