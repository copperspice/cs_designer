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

#ifndef TABORDEREDITOR_PLUGIN_H
#define TABORDEREDITOR_PLUGIN_H

#include <abstract_formeditorplugin.h>

class QDesignerFormWindowInterface;

#include <QPointer>
#include <QHash>

class QAction;

namespace qdesigner_internal {

class TabOrderEditorTool;

class TabOrderEditorPlugin: public QObject, public QDesignerFormEditorPluginInterface
{
   CS_OBJECT(TabOrderEditorPlugin)

   CS_INTERFACES(QDesignerFormEditorPluginInterface)

 public:
   TabOrderEditorPlugin();
   virtual ~TabOrderEditorPlugin();

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
   QHash<QDesignerFormWindowInterface *, TabOrderEditorTool *> m_tools;
   bool m_initialized;
   QAction *m_action;
};

}  // namespace qdesigner_internal

#endif