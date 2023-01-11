/***********************************************************************
*
* Copyright (c) 2021-2023 Barbara Geller
* Copyright (c) 2021-2023 Ansel Sermersheim
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

#ifndef SIGNALSLOTEDITOR_H
#define SIGNALSLOTEDITOR_H

#include <connection_edit.h>

class DomConnections;

namespace qdesigner_internal {

class SignalSlotConnection;

class SignalSlotEditor : public ConnectionEdit
{
   CS_OBJECT(SignalSlotEditor)

 public:
   SignalSlotEditor(QDesignerFormWindowInterface *form_window, QWidget *parent);

   virtual void setSignal(SignalSlotConnection *con, const QString &member);
   virtual void setSlot(SignalSlotConnection *con, const QString &member);
   void setSource(Connection *con, const QString &obj_name) override;
   void setTarget(Connection *con, const QString &obj_name) override;

   DomConnections *toUi() const;
   void fromUi(const DomConnections *connections, QWidget *parent);

   QDesignerFormWindowInterface *formWindow() const {
      return m_form_window;
   }

   QObject *objectByName(QWidget *topLevel, const QString &name) const;

   void addEmptyConnection();

 protected:
   QWidget *widgetAt(const QPoint &pos) const override;

 private:
   Connection *createConnection(QWidget *source, QWidget *destination) override;
   void modifyConnection(Connection *con) override;

   QDesignerFormWindowInterface *m_form_window;
   bool m_showAllSignalsSlots;

   friend class SetMemberCommand;
};

} // namespace qdesigner_internal

#endif
