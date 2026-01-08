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

#ifndef SIGNALSLOTEDITOR_P_H
#define SIGNALSLOTEDITOR_P_H

#include <connection_edit.h>

#include <QAbstractItemModel>
#include <QList>
#include <QPointer>
#include <QString>
#include <QStringList>

class DomConnection;
class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class SignalSlotEditor;

class SignalSlotConnection : public Connection
{
 public:
   explicit SignalSlotConnection(ConnectionEdit *edit, QWidget *source = nullptr, QWidget *target = nullptr);

   void setSignal(const QString &signal);
   void setSlot(const QString &slot);

   QString sender() const;
   QString receiver() const;

   QString signal() const {
      return m_signal;
   }

   QString slot() const {
      return m_slot;
   }

   DomConnection *toUi() const;

   virtual void updateVisibility() override;

   enum State { Valid, ObjectDeleted, InvalidMethod, NotAncestor };
   State isValid(const QWidget *background) const;

   // format for messages, etc.
   QString toString() const;

 private:
   QString m_signal;
   QString m_slot;
};

class ConnectionModel : public QAbstractItemModel
{
   CS_OBJECT(ConnectionModel)

 public:
   explicit ConnectionModel(QObject *parent = nullptr);
   void setEditor(SignalSlotEditor *editor = nullptr);

   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
   QModelIndex parent(const QModelIndex &child) const override;
   int rowCount(const QModelIndex &parent = QModelIndex()) const override;
   int columnCount(const QModelIndex &parent = QModelIndex()) const override;
   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
   bool setData(const QModelIndex &index, const QVariant &data, int role = Qt::DisplayRole) override;
   Qt::ItemFlags flags(const QModelIndex &index) const override;
   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

   QModelIndex connectionToIndex(Connection *con) const;
   Connection *indexToConnection(const QModelIndex &index) const;
   void updateAll();

 private:
   CS_SLOT_1(Private, void connectionAdded(Connection *con))
   CS_SLOT_2(connectionAdded)
   CS_SLOT_1(Private, void connectionRemoved(int idx))
   CS_SLOT_2(connectionRemoved)
   CS_SLOT_1(Private, void aboutToRemoveConnection(Connection *con))
   CS_SLOT_2(aboutToRemoveConnection)
   CS_SLOT_1(Private, void aboutToAddConnection(int idx))
   CS_SLOT_2(aboutToAddConnection)
   CS_SLOT_1(Private, void connectionChanged(Connection *con))
   CS_SLOT_2(connectionChanged)

   QPointer<SignalSlotEditor> m_editor;
};

} //   end namespace - qdesigner_internal

#endif
