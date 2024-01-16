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

#ifndef TABORDEREDITOR_TOOL_H
#define TABORDEREDITOR_TOOL_H

#include <QPointer>

#include <abstract_formwindowtool.h>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QAction;

namespace qdesigner_internal {

class TabOrderEditor;

class TabOrderEditorTool: public QDesignerFormWindowToolInterface
{
   CS_OBJECT(TabOrderEditorTool)

 public:
   explicit TabOrderEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent = nullptr);
   virtual ~TabOrderEditorTool();

   QDesignerFormEditorInterface *core() const override;
   QDesignerFormWindowInterface *formWindow() const override;

   QWidget *editor() const override;
   QAction *action() const override;

   void activated() override;
   void deactivated() override;

   bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) override;

 private:
   QDesignerFormWindowInterface *m_formWindow;
   mutable QPointer<TabOrderEditor> m_editor;
   QAction *m_action;
};

}  // namespace qdesigner_internal


#endif // TABORDEREDITOR_TOOL_H
