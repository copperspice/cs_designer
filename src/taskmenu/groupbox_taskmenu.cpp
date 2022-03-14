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

#include <groupbox_taskmenu.h>
#include <inplace_editor.h>
#include <abstract_formwindow.h>

#include <QAction>
#include <QStyle>
#include <QStyleOption>

namespace qdesigner_internal {

// -------- GroupBoxTaskMenuInlineEditor
class GroupBoxTaskMenuInlineEditor : public  TaskMenuInlineEditor
{
 public:
   GroupBoxTaskMenuInlineEditor(QGroupBox *button, QObject *parent);

 protected:
   QRect editRectangle() const override;
};

GroupBoxTaskMenuInlineEditor::GroupBoxTaskMenuInlineEditor(QGroupBox *w, QObject *parent) :
   TaskMenuInlineEditor(w, ValidationSingleLine, "title", parent)
{
}

QRect GroupBoxTaskMenuInlineEditor::editRectangle() const
{
   QWidget *w = widget();

   QStyleOption opt;          // ## QStyleOptionGroupBox
   opt.initFrom(w);

   return QRect(QPoint(), QSize(w->width(), 20));
}

GroupBoxTaskMenu::GroupBoxTaskMenu(QGroupBox *groupbox, QObject *parent)
   : QDesignerTaskMenu(groupbox, parent), m_editTitleAction(new QAction(tr("Change title..."), this))

{
   TaskMenuInlineEditor *editor = new GroupBoxTaskMenuInlineEditor(groupbox, this);
   connect(m_editTitleAction, &QAction::triggered, editor, &TaskMenuInlineEditor::editText);
   m_taskActions.append(m_editTitleAction);

   QAction *sep = new QAction(this);
   sep->setSeparator(true);
   m_taskActions.append(sep);
}

QList<QAction *> GroupBoxTaskMenu::taskActions() const
{
   return m_taskActions + QDesignerTaskMenu::taskActions();
}

QAction *GroupBoxTaskMenu::preferredEditAction() const
{
   return m_editTitleAction;
}

}   // end namespace
