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

#ifndef SIGNALSLOTEDITOR_PLUGIN_H
#define SIGNALSLOTEDITOR_PLUGIN_H

#include <abstract_formeditorplugin.h>

#include <QHash>
#include <QPointer>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class SignalSlotEditorTool;

class SignalSlotEditorPlugin : public QObject, public QDesignerFormEditorPluginInterface
{
   CS_OBJECT(SignalSlotEditorPlugin)

   CS_INTERFACES(QDesignerFormEditorPluginInterface)

 public:
   SignalSlotEditorPlugin();
   virtual ~SignalSlotEditorPlugin();

   bool isInitialized() const override;
   void initialize(QDesignerFormEditorInterface *core) override;
   QAction *action() const override;

   QDesignerFormEditorInterface *core() const override;

   CS_SLOT_1(Public, void activeFormWindowChanged(QDesignerFormWindowInterface *formWindow))
   CS_SLOT_2(activeFormWindowChanged)

 private:
   CS_SLOT_1(Private, void addFormWindow(QDesignerFormWindowInterface *formWindow))
   CS_SLOT_2(addFormWindow)

   CS_SLOT_1(Private, void removeFormWindow(QDesignerFormWindowInterface *formWindow))
   CS_SLOT_2(removeFormWindow)

   QPointer<QDesignerFormEditorInterface> m_core;
   QHash<QDesignerFormWindowInterface *, SignalSlotEditorTool *> m_tools;
   bool m_initialized;
   QAction *m_action;
};

}  // namespace qdesigner_internal

#endif
