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

#include <abstract_formwindowmanager.h>

#include <QMap>

QDesignerFormWindowManagerInterface::QDesignerFormWindowManagerInterface(QObject *parent)
   : QObject(parent)
{
}

QDesignerFormWindowManagerInterface::~QDesignerFormWindowManagerInterface()
{
}

QAction *QDesignerFormWindowManagerInterface::actionCut() const
{
   return action(CutAction);
}

QAction *QDesignerFormWindowManagerInterface::actionCopy() const
{
   return action(CopyAction);
}

QAction *QDesignerFormWindowManagerInterface::actionPaste() const
{
   return action(PasteAction);
}

QAction *QDesignerFormWindowManagerInterface::actionDelete() const
{
   return action(DeleteAction);
}

QAction *QDesignerFormWindowManagerInterface::actionSelectAll() const
{
   return action(SelectAllAction);
}

QAction *QDesignerFormWindowManagerInterface::actionLower() const
{
   return action(LowerAction);
}

QAction *QDesignerFormWindowManagerInterface::actionRaise() const
{
   return action(RaiseAction);
}

QAction *QDesignerFormWindowManagerInterface::actionHorizontalLayout() const
{
   return action(HorizontalLayoutAction);
}

QAction *QDesignerFormWindowManagerInterface::actionVerticalLayout() const
{
   return action(VerticalLayoutAction);
}

QAction *QDesignerFormWindowManagerInterface::actionSplitHorizontal() const
{
   return action(SplitHorizontalAction);
}

QAction *QDesignerFormWindowManagerInterface::actionSplitVertical() const
{
   return action(SplitVerticalAction);
}

QAction *QDesignerFormWindowManagerInterface::actionGridLayout() const
{
   return action(GridLayoutAction);
}

QAction *QDesignerFormWindowManagerInterface::actionFormLayout() const
{
   return action(FormLayoutAction);
}

QAction *QDesignerFormWindowManagerInterface::actionBreakLayout() const
{
   return action(BreakLayoutAction);
}

QAction *QDesignerFormWindowManagerInterface::actionAdjustSize() const
{
   return action(AdjustSizeAction);
}

QAction *QDesignerFormWindowManagerInterface::actionSimplifyLayout() const
{
   return action(SimplifyLayoutAction);
}

QAction *QDesignerFormWindowManagerInterface::actionUndo() const
{
   return action(UndoAction);
}

QAction *QDesignerFormWindowManagerInterface::actionRedo() const
{
   return action(RedoAction);
}



