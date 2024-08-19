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

#ifndef MORPH_COMMAND_H
#define MORPH_COMMAND_H

#include <designer_formwindow_command.h>

class QAction;
class QSignalMapper;
class QMenu;

namespace qdesigner_internal {

//  morph menu that acts on a single widget.
class MorphMenu : public QObject
{
   CS_OBJECT(MorphMenu)

 public:
   typedef QList<QAction *> ActionList;

   explicit MorphMenu(QObject *parent = nullptr);

   void populate(QWidget *w, QDesignerFormWindowInterface *fw, ActionList &al);
   void populate(QWidget *w, QDesignerFormWindowInterface *fw, QMenu &m);

 private:
   Q_DISABLE_COPY(MorphMenu)

   CS_SLOT_1(Private, void slotMorph(const QString &newClassName))
   CS_SLOT_2(slotMorph)

   bool populateMenu(QWidget *w, QDesignerFormWindowInterface *fw);

   QAction *m_subMenuAction;
   QMenu *m_menu;
   QSignalMapper *m_mapper;

   QWidget *m_widget;
   QDesignerFormWindowInterface *m_formWindow;
};

} // namespace qdesigner_internal

#endif

