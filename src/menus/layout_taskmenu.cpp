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
#include <layout_taskmenu.h>

#include <formlayoutmenu_p.h>
#include <morphmenu_p.h>

#include <QAction>
#include <QDebug>

LayoutWidgetTaskMenu::LayoutWidgetTaskMenu(QLayoutWidget *lw, QObject *parent)
   : QObject(parent), m_widget(lw), m_morphMenu(new qdesigner_internal::MorphMenu(this)),
     m_formLayoutMenu(new qdesigner_internal::FormLayoutMenu(this))
{
}

QAction *LayoutWidgetTaskMenu::preferredEditAction() const
{
   return m_formLayoutMenu->preferredEditAction(m_widget, m_widget->formWindow());
}

QList<QAction *> LayoutWidgetTaskMenu::taskActions() const
{
   QList<QAction *> rc;
   QDesignerFormWindowInterface *fw = m_widget->formWindow();
   m_morphMenu->populate(m_widget, fw, rc);
   m_formLayoutMenu->populate(m_widget, fw, rc);
   return rc;
}

SpacerTaskMenu::SpacerTaskMenu(Spacer *, QObject *parent) :
   QObject(parent)
{
}

QAction *SpacerTaskMenu::preferredEditAction() const
{
   return nullptr;
}

QList<QAction *> SpacerTaskMenu::taskActions() const
{
   return QList<QAction *>();
}
