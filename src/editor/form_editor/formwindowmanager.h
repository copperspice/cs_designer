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

#ifndef FORMWINDOWMANAGER_H
#define FORMWINDOWMANAGER_H

#include <designer_formwindow_manager.h>

#include <QObject>
#include <QList>
#include <QPointer>
#include <QMap>

class QAction;
class QActionGroup;
class QUndoGroup;
class QDesignerFormEditorInterface;
class QDesignerWidgetBoxInterface;

namespace qdesigner_internal {

class FormWindow;
class PreviewManager;
class PreviewActionGroup;

class FormWindowManager : public QDesignerFormWindowManager
{
   CS_OBJECT(FormWindowManager)

 public:
   explicit FormWindowManager(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
   virtual ~FormWindowManager();

   QDesignerFormEditorInterface *core() const override;

   QAction *action(Action action) const override;
   QActionGroup *actionGroup(ActionGroup actionGroup) const override;

   QDesignerFormWindowInterface *activeFormWindow() const override;

   int formWindowCount() const override;
   QDesignerFormWindowInterface *formWindow(int index) const override;

   QDesignerFormWindowInterface *createFormWindow(QWidget *parentWidget = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag) override;

   QPixmap createPreviewPixmap() const override;

   bool eventFilter(QObject *o, QEvent *e) override;

   void dragItems(const QList<QDesignerDnDItemInterface *> &item_list) override;

   QUndoGroup *undoGroup() const;

   PreviewManager *previewManager() const override {
      return m_previewManager;
   }

   CS_SLOT_1(Public, void addFormWindow(QDesignerFormWindowInterface *formWindow)override)
   CS_SLOT_2(addFormWindow)

   CS_SLOT_1(Public, void removeFormWindow(QDesignerFormWindowInterface *formWindow)override)
   CS_SLOT_2(removeFormWindow)

   CS_SLOT_1(Public, void setActiveFormWindow(QDesignerFormWindowInterface *formWindow)override)
   CS_SLOT_2(setActiveFormWindow)

   CS_SLOT_1(Public, void closeAllPreviews())
   CS_SLOT_2(closeAllPreviews)

   CS_SLOT_1(Public, void deviceProfilesChanged())
   CS_SLOT_2(deviceProfilesChanged)

 private:
   enum CreateLayoutContext {
      LayoutContainer,
      LayoutSelection,
      MorphLayout
   };

   CS_SLOT_1(Private, void slotActionCutActivated())
   CS_SLOT_2(slotActionCutActivated)

   CS_SLOT_1(Private, void slotActionCopyActivated())
   CS_SLOT_2(slotActionCopyActivated)

   CS_SLOT_1(Private, void slotActionPasteActivated())
   CS_SLOT_2(slotActionPasteActivated)

   CS_SLOT_1(Private, void slotActionDeleteActivated())
   CS_SLOT_2(slotActionDeleteActivated)

   CS_SLOT_1(Private, void slotActionSelectAllActivated())
   CS_SLOT_2(slotActionSelectAllActivated)

   CS_SLOT_1(Private, void slotActionLowerActivated())
   CS_SLOT_2(slotActionLowerActivated)

   CS_SLOT_1(Private, void slotActionRaiseActivated())
   CS_SLOT_2(slotActionRaiseActivated)

   CS_SLOT_1(Private, void createLayout())
   CS_SLOT_2(createLayout)

   CS_SLOT_1(Private, void slotActionBreakLayoutActivated())
   CS_SLOT_2(slotActionBreakLayoutActivated)

   CS_SLOT_1(Private, void slotActionAdjustSizeActivated())
   CS_SLOT_2(slotActionAdjustSizeActivated)

   CS_SLOT_1(Private, void slotActionSimplifyLayoutActivated())
   CS_SLOT_2(slotActionSimplifyLayoutActivated)

   CS_SLOT_1(Private, void showPreview())
   CS_SLOT_2(showPreview)

   CS_SLOT_1(Private, void slotActionGroupPreviewInStyle(const QString &style, int deviceProfileIndex))
   CS_SLOT_2(slotActionGroupPreviewInStyle)

   CS_SLOT_1(Private, void slotActionShowFormWindowSettingsDialog())
   CS_SLOT_2(slotActionShowFormWindowSettingsDialog)

   CS_SLOT_1(Private, void slotUpdateActions())
   CS_SLOT_2(slotUpdateActions)

   void setupActions();
   FormWindow *findFormWindow(QWidget *w);
   QWidget *findManagedWidget(FormWindow *fw, QWidget *w);

   void setCurrentUndoStack(QUndoStack *stack);

   QDesignerFormEditorInterface *m_core;
   FormWindow *m_activeFormWindow;
   QList<FormWindow *> m_formWindows;

   PreviewManager *m_previewManager;

   /* Context of the layout actions and base for morphing layouts. Determined
    * in slotUpdateActions() and used later on in the action slots. */
   CreateLayoutContext m_createLayoutContext;
   QWidget *m_morphLayoutContainer;

   // edit actions
   QAction *m_actionCut;
   QAction *m_actionCopy;
   QAction *m_actionPaste;

   QAction *m_actionSelectAll;
   QAction *m_actionDelete;
   QAction *m_actionLower;
   QAction *m_actionRaise;

   // layout actions
   QAction *m_actionHorizontalLayout;
   QAction *m_actionVerticalLayout;
   QAction *m_actionFormLayout;
   QAction *m_actionSplitHorizontal;
   QAction *m_actionSplitVertical;
   QAction *m_actionGridLayout;
   QAction *m_actionBreakLayout;
   QAction *m_actionSimplifyLayout;
   QAction *m_actionAdjustSize;

   // preview actions
   QAction *m_actionDefaultPreview;
   mutable PreviewActionGroup *m_actionGroupPreviewInStyle;
   QAction *m_actionShowFormWindowSettingsDialog;

   QAction *m_actionUndo;
   QAction *m_actionRedo;

   QMap<QWidget *, bool> getUnsortedLayoutsToBeBroken(bool firstOnly) const;
   bool hasLayoutsToBeBroken() const;
   QWidgetList layoutsToBeBroken(QWidget *w) const;
   QWidgetList layoutsToBeBroken() const;

   QUndoGroup *m_undoGroup;
};

}  // end namespace - qdesigner_internal

#endif
