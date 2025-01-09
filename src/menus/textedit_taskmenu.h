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

#ifndef TEXTEDIT_TASKMENU_H
#define TEXTEDIT_TASKMENU_H

#include <designer_taskmenu.h>

#include <extensionfactory_p.h>

#include <QPlainTextEdit>
#include <QTextEdit>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class TextEditTaskMenu: public QDesignerTaskMenu
{
   CS_OBJECT(TextEditTaskMenu)

 public:
   explicit TextEditTaskMenu(QTextEdit *button, QObject *parent = nullptr);
   explicit TextEditTaskMenu(QPlainTextEdit *button, QObject *parent = nullptr);

   virtual ~TextEditTaskMenu();

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 private:
   CS_SLOT_1(Private, void editText())
   CS_SLOT_2(editText)

   void initialize();

   const Qt::TextFormat m_format;
   const QString m_property;
   const QString m_windowTitle;

   mutable QList<QAction *> m_taskActions;
   QAction *m_editTextAction;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QTextEdit, TextEditTaskMenu> TextEditTaskMenuFactory;
typedef ExtensionFactory<QDesignerTaskMenuExtension, QPlainTextEdit, TextEditTaskMenu> PlainTextEditTaskMenuFactory;

}   // end namespace qdesigner_internal

#endif
