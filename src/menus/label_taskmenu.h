/***********************************************************************
*
* Copyright (c) 2021-2025 Barbara Geller
* Copyright (c) 2021-2025 Ansel Sermersheim
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

#ifndef LABEL_TASKMENU_H
#define LABEL_TASKMENU_H

#include <designer_taskmenu.h>

#include <extensionfactory_p.h>

#include <QLabel>
#include <QPointer>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class LabelTaskMenu: public QDesignerTaskMenu
{
   CS_OBJECT(LabelTaskMenu)

 public:
   explicit LabelTaskMenu(QLabel *button, QObject *parent = nullptr);

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 private:
   CS_SLOT_1(Private, void editRichText())
   CS_SLOT_2(editRichText)

   QLabel *m_label;
   QList<QAction *> m_taskActions;
   QAction *m_editRichTextAction;
   QAction *m_editPlainTextAction;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QLabel, LabelTaskMenu>  LabelTaskMenuFactory;

}   // end namespace qdesigner_internal

#endif