/***********************************************************************
*
* Copyright (c) 2021-2022 Barbara Geller
* Copyright (c) 2021-2022 Ansel Sermersheim
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

#ifndef ABSTRACTFORMWINDOWMANAGER_H
#define ABSTRACTFORMWINDOWMANAGER_H

#include <abstract_formwindow.h>

class QDesignerFormEditorInterface;
class QDesignerDnDItemInterface;

#include <QObject>
#include <QScopedPointer>

class QWidget;
class QPixmap;
class QAction;
class QActionGroup;

class QDesignerFormWindowManagerInterface: public QObject
{
   CS_OBJECT(QDesignerFormWindowManagerInterface)

 public:
   explicit QDesignerFormWindowManagerInterface(QObject *parent = nullptr);
   virtual ~QDesignerFormWindowManagerInterface();

   enum Action {
      CutAction = 100,
      CopyAction,
      PasteAction,
      DeleteAction,
      SelectAllAction,

      LowerAction = 200,
      RaiseAction,

      UndoAction =  300,
      RedoAction,

      HorizontalLayoutAction = 400,
      VerticalLayoutAction,
      SplitHorizontalAction,
      SplitVerticalAction,
      GridLayoutAction,
      FormLayoutAction,
      BreakLayoutAction,
      AdjustSizeAction,
      SimplifyLayoutAction,

      DefaultPreviewAction = 500,

      FormWindowSettingsDialogAction =  600
   };

   enum ActionGroup {
      StyledPreviewActionGroup = 100
   };

   virtual QAction *action(Action action) const = 0;
   virtual QActionGroup *actionGroup(ActionGroup actionGroup) const = 0;

   QAction *actionCut() const;
   QAction *actionCopy() const;
   QAction *actionPaste() const;
   QAction *actionDelete() const;
   QAction *actionSelectAll() const;
   QAction *actionLower() const;
   QAction *actionRaise() const;
   QAction *actionUndo() const;
   QAction *actionRedo() const;

   QAction *actionHorizontalLayout() const;
   QAction *actionVerticalLayout() const;
   QAction *actionSplitHorizontal() const;
   QAction *actionSplitVertical() const;
   QAction *actionGridLayout() const;
   QAction *actionFormLayout() const;
   QAction *actionBreakLayout() const;
   QAction *actionAdjustSize() const;
   QAction *actionSimplifyLayout() const;

   virtual QDesignerFormWindowInterface *activeFormWindow() const = 0;

   virtual int formWindowCount() const = 0;
   virtual QDesignerFormWindowInterface *formWindow(int index) const = 0;

   virtual QDesignerFormWindowInterface *createFormWindow(QWidget *parentWidget = nullptr, Qt::WindowFlags flags = Qt::WindowFlags()) = 0;
   virtual QDesignerFormEditorInterface *core() const = 0;

   virtual void dragItems(const QList<QDesignerDnDItemInterface *> &item_list) = 0;

   virtual QPixmap createPreviewPixmap() const = 0;

 public:
   CS_SIGNAL_1(Public, void formWindowAdded(QDesignerFormWindowInterface *formWindow))
   CS_SIGNAL_2(formWindowAdded, formWindow)

   CS_SIGNAL_1(Public, void formWindowRemoved(QDesignerFormWindowInterface *formWindow))
   CS_SIGNAL_2(formWindowRemoved, formWindow)

   CS_SIGNAL_1(Public, void activeFormWindowChanged(QDesignerFormWindowInterface *formWindow))
   CS_SIGNAL_2(activeFormWindowChanged, formWindow)

   CS_SIGNAL_1(Public, void formWindowSettingsChanged(QDesignerFormWindowInterface *fw))
   CS_SIGNAL_2(formWindowSettingsChanged, fw)

   CS_SLOT_1(Public, virtual void addFormWindow(QDesignerFormWindowInterface *formWindow) = 0)
   CS_SLOT_2(addFormWindow)

   CS_SLOT_1(Public, virtual void removeFormWindow(QDesignerFormWindowInterface *formWindow) = 0)
   CS_SLOT_2(removeFormWindow)

   CS_SLOT_1(Public, virtual void setActiveFormWindow(QDesignerFormWindowInterface *formWindow) = 0)
   CS_SLOT_2(setActiveFormWindow)

   CS_SLOT_1(Public, virtual void showPreview() = 0)
   CS_SLOT_2(showPreview)

   CS_SLOT_1(Public, virtual void closeAllPreviews() = 0)
   CS_SLOT_2(closeAllPreviews)

   CS_SLOT_1(Public, virtual void showPluginDialog() = 0)
   CS_SLOT_2(showPluginDialog)

 private:
   QScopedPointer<int> d;
};


#endif // ABSTRACTFORMWINDOWMANAGER_H
