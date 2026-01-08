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

#ifndef FORMLAYOUTMENU
#define FORMLAYOUTMENU

#include <QList>
#include <QObject>
#include <QPointer>

class QDesignerFormWindowInterface;

class QAction;
class QWidget;

namespace qdesigner_internal {

// Task menu to be used for form layouts. Offers an options "Add row" which
// pops up a dialog in which the user can specify label name, text and buddy.
class FormLayoutMenu : public QObject
{
   CS_OBJECT(FormLayoutMenu)

 public:
   typedef QList<QAction *> ActionList;

   explicit FormLayoutMenu(QObject *parent);

   // Populate a list of actions with the form layout actions.
   void populate(QWidget *w, QDesignerFormWindowInterface *fw, ActionList &actions);

   // For implementing QDesignerTaskMenuExtension::preferredEditAction():
   // Return appropriate action for double clicking.
   QAction *preferredEditAction(QWidget *w, QDesignerFormWindowInterface *fw);

 private:
   Q_DISABLE_COPY(FormLayoutMenu)

   CS_SLOT_1(Private, void slotAddRow())
   CS_SLOT_2(slotAddRow)

   QAction *m_separator1;
   QAction *m_populateFormAction;
   QAction *m_separator2;
   QPointer<QWidget> m_widget;
};
}  // namespace qdesigner_internal

#endif // FORMLAYOUTMENU
