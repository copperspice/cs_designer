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

#ifndef ACTION_REPOSITORY_H
#define ACTION_REPOSITORY_H

#include <QIcon>
#include <QListView>
#include <QMimeData>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QTreeView>

class QDesignerFormEditorInterface;
class QDesignerPropertySheetExtension;

class QPixmap;
namespace qdesigner_internal {

class PropertySheetKeySequenceValue;

// Shared model of actions, to be used for several views (detailed/icon view).
class ActionModel: public QStandardItemModel
{
   CS_OBJECT(ActionModel)

 public:
   enum Columns {
      NameColumn,
      UsedColumn,
      TextColumn,
      ShortCutColumn,
      CheckedColumn,
      ToolTipColumn,
      NumColumns
   };

   static constexpr const int ActionRole = Qt::UserRole + 1000;

   explicit ActionModel(QWidget *parent = nullptr);
   void initialize(QDesignerFormEditorInterface *core) {
      m_core = core;
   }

   void clearActions();
   QModelIndex addAction(QAction *a);

   // remove row
   void remove(int row);

   // update the row from the underlying action
   void update(int row);

   // return row of action or -1.
   int findAction(QAction *) const;

   QString actionName(int row) const;
   QAction *actionAt(const QModelIndex &index) const;

   QMimeData *mimeData(const QModelIndexList &indexes) const override;
   QStringList mimeTypes() const override;
   bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

   // Find the associated menus and toolbars, ignore toolbuttons
   static QWidgetList associatedWidgets(const QAction *action);

   // Retrieve shortcut via property sheet as it is a fake property
   static PropertySheetKeySequenceValue actionShortCut(QDesignerFormEditorInterface *core, QAction *action);
   static PropertySheetKeySequenceValue actionShortCut(const QDesignerPropertySheetExtension *ps);

 public:
   CS_SIGNAL_1(Public, void resourceImageDropped(const QString &path, QAction *action))
   CS_SIGNAL_2(resourceImageDropped, path, action)

 private:
   typedef QList<QStandardItem *> QStandardItemList;

   void initializeHeaders();
   static void setItems(QDesignerFormEditorInterface *core, QAction *a,
      const QIcon &defaultIcon,
      QStandardItemList &sl);

   const QIcon m_emptyIcon;

   QDesignerFormEditorInterface *m_core;
};

// Internal class that provides the detailed view of actions.
class  ActionTreeView: public QTreeView
{
   CS_OBJECT(ActionTreeView)

 public:
   explicit ActionTreeView(ActionModel *model, QWidget *parent = nullptr);
   QAction *currentAction() const;

   CS_SLOT_1(Public, void filter(const QString &text))
   CS_SLOT_2(filter)

   CS_SIGNAL_1(Public, void actionContextMenuRequested(QContextMenuEvent *event, QAction *un_named_arg2))
   CS_SIGNAL_2(actionContextMenuRequested, event, un_named_arg2)
   CS_SIGNAL_1(Public, void currentActionChanged(QAction *action))
   CS_SIGNAL_2(currentActionChanged, action)
   CS_SIGNAL_1(Public, void actionActivated(QAction *action))
   CS_SIGNAL_2(actionActivated, action)

 protected:
   CS_SLOT_1(Protected, void currentChanged(const QModelIndex &current, const QModelIndex &previous)override)
   CS_SLOT_2(currentChanged)

   void dragEnterEvent(QDragEnterEvent *event) override;
   void dragMoveEvent(QDragMoveEvent *event) override;
   void dropEvent(QDropEvent *event) override;
   void focusInEvent(QFocusEvent *event) override;
   void contextMenuEvent(QContextMenuEvent *event) override;
   void startDrag(Qt::DropActions supportedActions) override;

 private:
   CS_SLOT_1(Private, void slotActivated(const QModelIndex &un_named_arg1))
   CS_SLOT_2(slotActivated)

   ActionModel *m_model;
};

// Internal class that provides the icon view of actions.
class ActionListView: public QListView
{
   CS_OBJECT(ActionListView)

 public:
   explicit ActionListView(ActionModel *model, QWidget *parent = nullptr);
   QAction *currentAction() const;

   CS_SLOT_1(Public, void filter(const QString &text))
   CS_SLOT_2(filter)

   CS_SIGNAL_1(Public, void actionContextMenuRequested(QContextMenuEvent *event, QAction *un_named_arg2))
   CS_SIGNAL_2(actionContextMenuRequested, event, un_named_arg2)

   CS_SIGNAL_1(Public, void currentActionChanged(QAction *action))
   CS_SIGNAL_2(currentActionChanged, action)

   CS_SIGNAL_1(Public, void actionActivated(QAction *action))
   CS_SIGNAL_2(actionActivated, action)

 protected:
   CS_SLOT_1(Protected, void currentChanged(const QModelIndex &current, const QModelIndex &previous)override)
   CS_SLOT_2(currentChanged)

   void dragEnterEvent(QDragEnterEvent *event) override;
   void dragMoveEvent(QDragMoveEvent *event) override;
   void dropEvent(QDropEvent *event) override;
   void focusInEvent(QFocusEvent *event) override;
   void contextMenuEvent(QContextMenuEvent *event) override;
   void startDrag(Qt::DropActions supportedActions) override;

 private:
   CS_SLOT_1(Private, void slotActivated(const QModelIndex &un_named_arg1))
   CS_SLOT_2(slotActivated)

   ActionModel *m_model;
};

// Action View that can be switched between detailed and icon view
// using a  QStackedWidget of  ActionListView / ActionTreeView
// that share the item model and the selection model.

class ActionView : public QStackedWidget
{
   CS_OBJECT(ActionView)

 public:
   enum ViewMode {
      DetailedView,
      IconView
   };

   // Separate initialize() function takes core argument to make this
   // thing usable as promoted widget.
   explicit ActionView(QWidget *parent = nullptr);

   void initialize(QDesignerFormEditorInterface *core) {
      m_model->initialize(core);
   }

   int viewMode() const;
   void setViewMode(int lm);

   void setSelectionMode(QAbstractItemView::SelectionMode sm);
   QAbstractItemView::SelectionMode selectionMode() const;

   ActionModel *model() const {
      return m_model;
   }

   QAction *currentAction() const;
   void setCurrentIndex(const QModelIndex &index);

   typedef QList<QAction *> ActionList;
   ActionList selectedActions() const;
   QItemSelection selection() const;

   CS_SLOT_1(Public, void filter(const QString &text))
   CS_SLOT_2(filter)

   CS_SLOT_1(Public, void selectAll())
   CS_SLOT_2(selectAll)

   CS_SLOT_1(Public, void clearSelection())
   CS_SLOT_2(clearSelection)

   CS_SIGNAL_1(Public, void contextMenuRequested(QContextMenuEvent *event, QAction *un_named_arg2))
   CS_SIGNAL_2(contextMenuRequested, event, un_named_arg2)

   CS_SIGNAL_1(Public, void currentChanged(QAction *action))
   CS_SIGNAL_2(currentChanged, action)

   CS_SIGNAL_1(Public, void activated(QAction *action))
   CS_SIGNAL_2(activated, action)

   CS_SIGNAL_1(Public, void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected))
   CS_SIGNAL_2(selectionChanged, selected, deselected)

   CS_SIGNAL_1(Public, void resourceImageDropped(const QString &imageData, QAction *action))
   CS_SIGNAL_2(resourceImageDropped, imageData, action)

 private:
   CS_SLOT_1(Private, void slotCurrentChanged(QAction *action))
   CS_SLOT_2(slotCurrentChanged)

   ActionModel *m_model;
   ActionTreeView *m_actionTreeView;
   ActionListView *m_actionListView;
};

class ActionRepositoryMimeData: public QMimeData
{
   CS_OBJECT(ActionRepositoryMimeData)

 public:
   typedef QList<QAction *> ActionList;

   ActionRepositoryMimeData(const ActionList &, Qt::DropAction dropAction);
   ActionRepositoryMimeData(QAction *, Qt::DropAction dropAction);

   const ActionList &actionList() const {
      return m_actionList;
   }
   QStringList formats() const override;

   static QPixmap actionDragPixmap(const QAction *action);

   // Utility to accept with right action
   void accept(QDragMoveEvent *event) const;

 private:
   const Qt::DropAction m_dropAction;
   ActionList m_actionList;
};

} // namespace qdesigner_internal

#endif // ACTIONREPOSITORY_H
