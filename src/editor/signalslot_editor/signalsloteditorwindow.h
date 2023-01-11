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

#ifndef SIGNALSLOTEDITORWINDOW_H
#define SIGNALSLOTEDITORWINDOW_H

#include <QPointer>
#include <QWidget>

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;
class QModelIndex;
class QSortFilterProxyModel;
class QTreeView;
class QToolButton;

namespace qdesigner_internal {

class SignalSlotEditor;
class ConnectionModel;
class Connection;

class SignalSlotEditorWindow : public QWidget
{
   CS_OBJECT(SignalSlotEditorWindow)

 public:
   explicit SignalSlotEditorWindow(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);

   CS_SLOT_1(Public, void setActiveFormWindow(QDesignerFormWindowInterface *form))
   CS_SLOT_2(setActiveFormWindow)

 private:
   CS_SLOT_1(Private, void updateDialogSelection(Connection *con))
   CS_SLOT_2(updateDialogSelection)
   CS_SLOT_1(Private, void updateEditorSelection(const QModelIndex &index))
   CS_SLOT_2(updateEditorSelection)

   CS_SLOT_1(Private, void objectNameChanged(QDesignerFormWindowInterface *formWindow, QObject *object,
         const QString &newName, const QString &oldName))
   CS_SLOT_2(objectNameChanged)

   CS_SLOT_1(Private, void addConnectionX())
   CS_SLOT_2(addConnectionX)

   CS_SLOT_1(Private, void removeConnection())
   CS_SLOT_2(removeConnection)

   CS_SLOT_1(Private, void updateUi())
   CS_SLOT_2(updateUi)

   QTreeView *m_view;
   QPointer<SignalSlotEditor> m_editor;
   QToolButton *m_add_button, *m_remove_button;
   QDesignerFormEditorInterface *m_core;
   ConnectionModel *m_model;
   QSortFilterProxyModel *m_proxy_model;
   bool m_handling_selection_change;
};

}   // end namespace qdesigner_internal

#endif
