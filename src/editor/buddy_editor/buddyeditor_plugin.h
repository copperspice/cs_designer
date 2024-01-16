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

#ifndef BUDDYEDITOR_PLUGIN_H
#define BUDDYEDITOR_PLUGIN_H

#include <abstract_formeditorplugin.h>

#include <QPointer>
#include <QHash>

class QDesignerFormWindowInterface;
class QAction;

namespace qdesigner_internal {

class BuddyEditorTool;

class BuddyEditorPlugin: public QObject, public QDesignerFormEditorPluginInterface
{
   CS_OBJECT(BuddyEditorPlugin)

   // Q_PLUGIN_METADATA(IID "copperspice.CS.QDesignerFormEditorPluginInterface" FILE "buddyeditor.json")
   CS_INTERFACES(QDesignerFormEditorPluginInterface)

 public:
   BuddyEditorPlugin();
   virtual ~BuddyEditorPlugin();

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
   QHash<QDesignerFormWindowInterface *, BuddyEditorTool *> m_tools;
   bool m_initialized;
   QAction *m_action;
};

}   // end namespace - qdesigner_internal

#endif
