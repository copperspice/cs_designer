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

#ifndef LINEEDIT_TASKMENU_H
#define LINEEDIT_TASKMENU_H

#include <designer_taskmenu.h>
#include <extensionfactory_p.h>

#include <QLineEdit>
#include <QPointer>

namespace qdesigner_internal {

class LineEditTaskMenu: public QDesignerTaskMenu
{
   CS_OBJECT(LineEditTaskMenu)

 public:
   explicit LineEditTaskMenu(QLineEdit *button, QObject *parent = nullptr);

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 private:
   QList<QAction *> m_taskActions;
   QAction *m_editTextAction;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QLineEdit, LineEditTaskMenu> LineEditTaskMenuFactory;

}   // end namespace qdesigner_internal

#endif
