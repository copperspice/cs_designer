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

#ifndef SIGNALSLOTEDITOR_TOOL_H
#define SIGNALSLOTEDITOR_TOOL_H

#include <abstract_formwindowtool.h>
#include <signalsloteditor.h>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

#include <QPointer>

class QAction;

namespace qdesigner_internal {

class SignalSlotEditor;

class SignalSlotEditorTool: public QDesignerFormWindowToolInterface
{
   CS_OBJECT(SignalSlotEditorTool)
 public:
   explicit SignalSlotEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent = nullptr);
   virtual ~SignalSlotEditorTool();

   QDesignerFormEditorInterface *core() const override;
   QDesignerFormWindowInterface *formWindow() const override;

   QWidget *editor() const override;

   QAction *action() const override;

   void activated() override;
   void deactivated() override;

   bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) override;

   void saveToDom(DomUI *ui, QWidget *mainContainer) override;
   void loadFromDom(DomUI *ui, QWidget *mainContainer) override;

 private:
   QDesignerFormWindowInterface *m_formWindow;
   mutable QPointer<qdesigner_internal::SignalSlotEditor> m_editor;
   QAction *m_action;
};

}   // end namespace qdesigner_internal

#endif
