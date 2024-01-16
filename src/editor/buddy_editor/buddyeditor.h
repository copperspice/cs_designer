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

#ifndef BUDDYEDITOR_H
#define BUDDYEDITOR_H

#include <connection_edit.h>

class QDesignerFormWindowInterface;

#include <QPointer>
#include <QSet>

class QLabel;

namespace qdesigner_internal {

class BuddyEditor : public ConnectionEdit
{
   CS_OBJECT(BuddyEditor)

 public:
   BuddyEditor(QDesignerFormWindowInterface *form, QWidget *parent);

   QDesignerFormWindowInterface *formWindow() const;
   void setBackground(QWidget *background) override;
   void deleteSelected() override;

   CS_SLOT_1(Public, void updateBackground())
   CS_SLOT_2(updateBackground)

   CS_SLOT_1(Public, void widgetRemoved(QWidget *w)override)
   CS_SLOT_2(widgetRemoved)

   CS_SLOT_1(Public, void autoBuddy())
   CS_SLOT_2(autoBuddy)

 protected:
   QWidget *widgetAt(const QPoint &pos) const override;
   Connection *createConnection(QWidget *source, QWidget *destination) override;
   void endConnection(QWidget *target, const QPoint &pos) override;
   void createContextMenu(QMenu &menu) override;

 private:
   QWidget *findBuddy(QLabel *l, const QWidgetList &existingBuddies) const;

   QPointer<QDesignerFormWindowInterface> m_formWindow;
   bool m_updating;
};

}   // end namespace - qdesigner_internal

#endif
