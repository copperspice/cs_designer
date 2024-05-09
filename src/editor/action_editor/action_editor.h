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

#ifndef ACTION_EDITOR_H
#define ACTION_EDITOR_H

#include <abstract_actioneditor.h>
#include <abstract_propertyeditor.h>
#include <abstract_settings.h>

#include <QPointer>

class QActionGroup;
class QItemSelection;
class QLineEdit;
class QListWidget;
class QMenu;
class QPushButton;
class QSignalMapper;
class QToolButton;

namespace qdesigner_internal {

class ActionView;
class ResourceMimeData;

class ActionEditor: public QDesignerActionEditorInterface
{
   CS_OBJECT(ActionEditor)

 public:
   explicit ActionEditor(QDesignerFormEditorInterface *core, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);
   virtual ~ActionEditor();

   QDesignerFormWindowInterface *formWindow() const;
   void setFormWindow(QDesignerFormWindowInterface *formWindow) override;

   QDesignerFormEditorInterface *core() const override;

   QAction *actionNew() const;
   QAction *actionDelete() const;

   QString filter() const;

   void manageAction(QAction *action) override;
   void unmanageAction(QAction *action) override;

   static QString actionTextToName(const QString &text, const QString &prefix = "action");

   // Utility to create a configure button with menu for usage on toolbars
   static QToolButton *createConfigureMenuButton(const QString &t, QMenu **ptrToMenu);

   CS_SLOT_1(Public, void setFilter(const QString &filter))
   CS_SLOT_2(setFilter)

   CS_SLOT_1(Public, void mainContainerChanged())
   CS_SLOT_2(mainContainerChanged)

   CS_SIGNAL_1(Public, void itemActivated(QAction *item))
   CS_SIGNAL_2(itemActivated, item)

   // Context menu for item or global menu if item == 0.
   CS_SIGNAL_1(Public, void contextMenuRequested(QMenu *menu, QAction *item))
   CS_SIGNAL_2(contextMenuRequested, menu, item)

 private:
   CS_SLOT_1(Private, void slotCurrentItemChanged(QAction *item))
   CS_SLOT_2(slotCurrentItemChanged)
   CS_SLOT_1(Private, void slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected))
   CS_SLOT_2(slotSelectionChanged)
   CS_SLOT_1(Private, void editAction(QAction *item))
   CS_SLOT_2(editAction)
   CS_SLOT_1(Private, void editCurrentAction())
   CS_SLOT_2(editCurrentAction)
   CS_SLOT_1(Private, void navigateToSlotCurrentAction())
   CS_SLOT_2(navigateToSlotCurrentAction)
   CS_SLOT_1(Private, void slotActionChanged())
   CS_SLOT_2(slotActionChanged)
   CS_SLOT_1(Private, void slotNewAction())
   CS_SLOT_2(slotNewAction)
   CS_SLOT_1(Private, void slotDelete())
   CS_SLOT_2(slotDelete)
   CS_SLOT_1(Private, void resourceImageDropped(const QString &path, QAction *action))
   CS_SLOT_2(resourceImageDropped)
   CS_SLOT_1(Private, void slotContextMenuRequested(QContextMenuEvent *un_named_arg1, QAction *un_named_arg2))
   CS_SLOT_2(slotContextMenuRequested)
   CS_SLOT_1(Private, void slotViewMode(QAction *a))
   CS_SLOT_2(slotViewMode)
   CS_SLOT_1(Private, void slotSelectAssociatedWidget(QWidget *w))
   CS_SLOT_2(slotSelectAssociatedWidget)

   CS_SLOT_1(Private, void slotCopy())
   CS_SLOT_2(slotCopy)
   CS_SLOT_1(Private, void slotCut())
   CS_SLOT_2(slotCut)
   CS_SLOT_1(Private, void slotPaste())
   CS_SLOT_2(slotPaste)

   typedef QList<QAction *> ActionList;

   void deleteActions(QDesignerFormWindowInterface *formWindow, const ActionList &);
   void copyActions(QDesignerFormWindowInterface *formWindow, const ActionList &);

   void restoreSettings();
   void saveSettings();

   void updateViewModeActions();

   QDesignerFormEditorInterface *m_core;
   QPointer<QDesignerFormWindowInterface> m_formWindow;
   QListWidget *m_actionGroups;

   ActionView *m_actionView;

   QAction *m_actionNew;
   QAction *m_actionEdit;
   QAction *m_actionNavigateToSlot;

   QAction *m_actionCopy;
   QAction *m_actionCut;
   QAction *m_actionPaste;

   QAction *m_actionSelectAll;
   QAction *m_actionDelete;

   QActionGroup *m_viewModeGroup;
   QAction *m_iconViewAction;
   QAction *m_listViewAction;

   QString m_filter;
   QWidget *m_filterWidget;
   QSignalMapper *m_selectAssociatedWidgetsMapper;
};

} // namespace qdesigner_internal

#endif // ACTIONEDITOR_H
