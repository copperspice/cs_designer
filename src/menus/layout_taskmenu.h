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

#ifndef LAYOUTTASKMENU_H
#define LAYOUTTASKMENU_H

#include <designer_taskmenu.h>
#include <layout_widget.h>

#include <extensionfactory_p.h>
#include <spacer_widget_p.h>

namespace qdesigner_internal {

class FormLayoutMenu;
class MorphMenu;

}

// Morph menu for QLayoutWidget.
class LayoutWidgetTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
   CS_OBJECT(LayoutWidgetTaskMenu)
   CS_INTERFACES(QDesignerTaskMenuExtension)

 public:
   explicit LayoutWidgetTaskMenu(QLayoutWidget *w, QObject *parent = nullptr);

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 private:
   QLayoutWidget *m_widget;
   qdesigner_internal::MorphMenu *m_morphMenu;
   qdesigner_internal::FormLayoutMenu *m_formLayoutMenu;
};

// Empty task menu for spacers.
class SpacerTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
   CS_OBJECT(SpacerTaskMenu)
   CS_INTERFACES(QDesignerTaskMenuExtension)

 public:
   explicit SpacerTaskMenu(Spacer *bar, QObject *parent = nullptr);

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;
};

using LayoutWidgetTaskMenuFactory = qdesigner_internal::ExtensionFactory<QDesignerTaskMenuExtension, QLayoutWidget, LayoutWidgetTaskMenu>;
using SpacerTaskMenuFactory       = qdesigner_internal::ExtensionFactory<QDesignerTaskMenuExtension, Spacer, SpacerTaskMenu>;

#endif
