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

#ifndef QDESIGNER_COMMAND_H
#define QDESIGNER_COMMAND_H

#include <container.h>
#include <designer_formeditor_command.h>
#include <designer_formwindow_command.h>
#include <designer_utils.h>
#include <layout_info.h>
#include <layoutdecoration.h>

#include <shared_enums_p.h>

#include <QHash>
#include <QIcon>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QPoint>
#include <QRect>
#include <QWidget>

class QDesignerMenu;
class QDesignerMetaDataBaseItemInterface;

class QComboBox;
class QDockWidget;
class QFormLayout;
class QListWidget;
class QListWidgetItem;
class QMainWindow;
class QMenuBar;
class QStackedWidget;
class QStatusBar;
class QTabWidget;
class QTableWidget;
class QTableWidgetItem;
class QToolBar;
class QToolBox;
class QTreeWidget;
class QTreeWidgetItem;

namespace qdesigner_internal {

class BreakLayoutCommand;
class DesignerIconCache;
class Layout;
class LayoutCommand;
class LayoutHelper;
class PropertySheetIconValue;

struct LayoutProperties;

class InsertWidgetCommand: public QDesignerFormWindowCommand
{
 public:
   explicit InsertWidgetCommand(QDesignerFormWindowInterface *formWindow);
   ~InsertWidgetCommand();

   void init(QWidget *widget, bool already_in_form = false, int layoutRow = -1, int layoutColumn = -1);

   void redo() override;
   void undo() override;

 private:
   void refreshBuddyLabels();

   QPointer<QWidget> m_widget;
   QDesignerLayoutDecorationExtension::InsertMode m_insertMode;
   QPair<int, int> m_cell;
   LayoutHelper *m_layoutHelper;
   bool m_widgetWasManaged;
};

class ChangeZOrderCommand: public QDesignerFormWindowCommand
{
 public:
   explicit ChangeZOrderCommand(QDesignerFormWindowInterface *formWindow);

   void init(QWidget *widget);

   void redo() override;
   void undo() override;

 protected:
   virtual QWidgetList reorderWidget(const QWidgetList &list, QWidget *widget) const = 0;
   virtual void reorder(QWidget *widget) const = 0;

 private:
   QPointer<QWidget> m_widget;
   QPointer<QWidget> m_oldPreceding;
   QList<QWidget *> m_oldParentZOrder;
};

class RaiseWidgetCommand: public ChangeZOrderCommand
{

 public:
   explicit RaiseWidgetCommand(QDesignerFormWindowInterface *formWindow);

   void init(QWidget *widget);

 protected:
   QWidgetList reorderWidget(const QWidgetList &list, QWidget *widget) const override;
   void reorder(QWidget *widget) const override;
};

class LowerWidgetCommand: public ChangeZOrderCommand
{

 public:
   explicit LowerWidgetCommand(QDesignerFormWindowInterface *formWindow);

   void init(QWidget *widget);

 protected:
   QWidgetList reorderWidget(const QWidgetList &list, QWidget *widget) const override;
   void reorder(QWidget *widget) const override;
};

class AdjustWidgetSizeCommand: public QDesignerFormWindowCommand
{
 public:
   explicit AdjustWidgetSizeCommand(QDesignerFormWindowInterface *formWindow);

   void init(QWidget *widget);

   void redo() override;
   void undo() override;

 private:
   QWidget *widgetForAdjust() const;
   bool adjustNonLaidOutMainContainer(QWidget *integrationContainer);
   void updatePropertyEditor() const;

   QPointer<QWidget> m_widget;
   QRect m_geometry;
};

// Helper to correctly unmanage a widget and its children for delete operations
class  ManageWidgetCommandHelper
{
 public:
   ManageWidgetCommandHelper();
   void init(const QDesignerFormWindowInterface *fw, QWidget *widget);
   void init(QWidget *widget, const QVector<QWidget *> &managedChildren);

   void manage(QDesignerFormWindowInterface *fw);
   void unmanage(QDesignerFormWindowInterface *fw);

   const QVector<QWidget *> &managedChildren() const {
      return m_managedChildren;
   }

 private:
   QWidget *m_widget;
   QVector<QWidget *> m_managedChildren;
};

class DeleteWidgetCommand: public QDesignerFormWindowCommand
{
 public:
   explicit DeleteWidgetCommand(QDesignerFormWindowInterface *formWindow);
   ~DeleteWidgetCommand();

   enum DeleteFlags { DoNotUnmanage = 0x1, DoNotSimplifyLayout = 0x2 };

   void init(QWidget *widget, unsigned flags = 0);

   void redo() override;
   void undo() override;

 private:
   QPointer<QWidget> m_widget;
   QPointer<QWidget> m_parentWidget;
   QRect m_geometry;
   LayoutInfo::Type m_layoutType;
   LayoutHelper *m_layoutHelper;
   unsigned m_flags;
   QRect m_layoutPosition;
   int m_splitterIndex;
   bool m_layoutSimplified;
   QDesignerMetaDataBaseItemInterface *m_formItem;
   int m_tabOrderIndex;
   int m_widgetOrderIndex;
   int m_zOrderIndex;
   ManageWidgetCommandHelper m_manageHelper;
};

class ReparentWidgetCommand: public QDesignerFormWindowCommand
{
 public:
   explicit ReparentWidgetCommand(QDesignerFormWindowInterface *formWindow);

   void init(QWidget *widget, QWidget *parentWidget);

   void redo() override;
   void undo() override;

 private:
   QPointer<QWidget> m_widget;
   QPoint m_oldPos;
   QPoint m_newPos;
   QPointer<QWidget> m_oldParentWidget;
   QPointer<QWidget> m_newParentWidget;
   QList<QWidget *> m_oldParentList;
   QList<QWidget *> m_oldParentZOrder;
};

class ChangeFormLayoutItemRoleCommand : public QDesignerFormWindowCommand
{
 public:
   enum Operation { SpanningToLabel = 0x1, SpanningToField = 0x2, LabelToSpanning = 0x4, FieldToSpanning = 0x8 };

   explicit ChangeFormLayoutItemRoleCommand(QDesignerFormWindowInterface *formWindow);

   void init(QWidget *widget, Operation op);

   void redo() override;
   void undo() override;

   // Return a mask of possible operations of that item
   static unsigned possibleOperations(QDesignerFormEditorInterface *core, QWidget *w);

 private:
   static QFormLayout *managedFormLayoutOf(QDesignerFormEditorInterface *core, QWidget *w);
   static Operation reverseOperation(Operation op);
   void doOperation(Operation op);

   QPointer<QWidget> m_widget;
   Operation m_operation;
};

class ChangeLayoutItemGeometry: public QDesignerFormWindowCommand
{
 public:
   explicit ChangeLayoutItemGeometry(QDesignerFormWindowInterface *formWindow);

   void init(QWidget *widget, int row, int column, int rowspan, int colspan);

   void redo() override;
   void undo() override;

 protected:
   void changeItemPosition(const QRect &g);

 private:
   QPointer<QWidget> m_widget;
   QRect m_oldInfo;
   QRect m_newInfo;
};

class TabOrderCommand: public QDesignerFormWindowCommand
{
 public:
   explicit TabOrderCommand(QDesignerFormWindowInterface *formWindow);

   void init(const QList<QWidget *> &newTabOrder);

   inline QList<QWidget *> oldTabOrder() const {
      return m_oldTabOrder;
   }

   inline QList<QWidget *> newTabOrder() const {
      return m_newTabOrder;
   }

   void redo() override;
   void undo() override;

 private:
   QDesignerMetaDataBaseItemInterface *m_widgetItem;
   QList<QWidget *> m_oldTabOrder;
   QList<QWidget *> m_newTabOrder;
};

class PromoteToCustomWidgetCommand : public QDesignerFormWindowCommand
{
 public:
   typedef QList<QPointer<QWidget>> WidgetList;

   explicit PromoteToCustomWidgetCommand(QDesignerFormWindowInterface *formWindow);

   void init(const WidgetList &widgets, const QString &customClassName);
   void redo() override;
   void undo() override;

 private:
   void updateSelection();
   WidgetList m_widgets;
   QString m_customClassName;
};

class DemoteFromCustomWidgetCommand : public QDesignerFormWindowCommand
{
 public:
   using WidgetList = PromoteToCustomWidgetCommand::WidgetList;

   explicit DemoteFromCustomWidgetCommand(QDesignerFormWindowInterface *formWindow);

   void init(const WidgetList &promoted);
   void redo() override;
   void undo() override;

 private:
   PromoteToCustomWidgetCommand m_promote_cmd;
};

// Mixin class for storing the selection state
class CursorSelectionState
{
   Q_DISABLE_COPY(CursorSelectionState)
 public:
   CursorSelectionState();

   void save(const QDesignerFormWindowInterface *formWindow);
   void restore(QDesignerFormWindowInterface *formWindow) const;

 private:
   typedef QList<QPointer<QWidget>> WidgetPointerList;
   WidgetPointerList m_selection;
   QPointer<QWidget> m_current;
};

class LayoutCommand: public QDesignerFormWindowCommand
{

 public:
   explicit LayoutCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~LayoutCommand();

   QWidgetList widgets() const {
      return m_widgets;
   }

   void init(QWidget *parentWidget, const QWidgetList &widgets, LayoutInfo::Type layoutType,
      QWidget *layoutBase = nullptr,
      // Reparent/Hide instances of QLayoutWidget.
      bool reparentLayoutWidget = true);

   void redo() override;
   void undo() override;

 private:
   QPointer<QWidget> m_parentWidget;
   QWidgetList m_widgets;
   QPointer<QWidget> m_layoutBase;
   QPointer<Layout> m_layout;
   CursorSelectionState m_cursorSelectionState;
   bool m_setup;
};

class BreakLayoutCommand: public QDesignerFormWindowCommand
{
 public:
   explicit BreakLayoutCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~BreakLayoutCommand();

   inline QWidgetList widgets() const {
      return m_widgets;
   }

   void init(const QWidgetList &widgets, QWidget *layoutBase,
      // Reparent/Hide instances of QLayoutWidget.
      bool reparentLayoutWidget = true);

   void redo() override;
   void undo() override;

   // Access the properties of the layout, 0 in case of splitters.
   const LayoutProperties *layoutProperties() const;
   int propertyMask() const;

 private:
   QWidgetList m_widgets;
   QPointer<QWidget> m_layoutBase;
   QPointer<Layout> m_layout;
   LayoutHelper *m_layoutHelper;
   LayoutProperties *m_properties;
   int m_propertyMask;
   CursorSelectionState m_cursorSelectionState;
};

class SimplifyLayoutCommand: public QDesignerFormWindowCommand
{
 public:
   explicit SimplifyLayoutCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~SimplifyLayoutCommand();

   bool init(QWidget *layoutBase);

   // Quick check
   static bool canSimplify(QDesignerFormEditorInterface *core, const QWidget *w, int *layoutType = nullptr);

   void redo() override;
   void undo() override;

 private:
   const QRect m_area;
   QWidget *m_layoutBase;
   LayoutHelper *m_layoutHelper;
   bool m_layoutSimplified;
};

class ToolBoxCommand: public QDesignerFormWindowCommand
{
 public:
   explicit ToolBoxCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~ToolBoxCommand();

   void init(QToolBox *toolBox);

   virtual void removePage();
   virtual void addPage();

 protected:
   QPointer<QToolBox> m_toolBox;
   QPointer<QWidget> m_widget;
   int m_index;
   QString m_itemText;
   QIcon m_itemIcon;
};

class MoveToolBoxPageCommand: public ToolBoxCommand
{

 public:
   explicit MoveToolBoxPageCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~MoveToolBoxPageCommand();

   void init(QToolBox *toolBox, QWidget *page, int newIndex);

   void redo() override;
   void undo() override;

 private:
   int m_newIndex;
   int m_oldIndex;
};

class DeleteToolBoxPageCommand: public ToolBoxCommand
{

 public:
   explicit DeleteToolBoxPageCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~DeleteToolBoxPageCommand();

   void init(QToolBox *toolBox);

   void redo() override;
   void undo() override;
};

class AddToolBoxPageCommand: public ToolBoxCommand
{

 public:
   enum InsertionMode {
      InsertBefore,
      InsertAfter
   };
   explicit AddToolBoxPageCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~AddToolBoxPageCommand();

   void init(QToolBox *toolBox);
   void init(QToolBox *toolBox, InsertionMode mode);

   void redo() override;
   void undo() override;
};

class TabWidgetCommand: public QDesignerFormWindowCommand
{
 public:
   explicit TabWidgetCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~TabWidgetCommand();

   void init(QTabWidget *tabWidget);

   virtual void removePage();
   virtual void addPage();

 protected:
   QPointer<QTabWidget> m_tabWidget;
   QPointer<QWidget> m_widget;
   int m_index;
   QString m_itemText;
   QIcon m_itemIcon;
};

class DeleteTabPageCommand: public TabWidgetCommand
{

 public:
   explicit DeleteTabPageCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~DeleteTabPageCommand();

   void init(QTabWidget *tabWidget);

   void redo() override;
   void undo() override;
};

class AddTabPageCommand: public TabWidgetCommand
{

 public:
   enum InsertionMode {
      InsertBefore,
      InsertAfter
   };
   explicit AddTabPageCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~AddTabPageCommand();

   void init(QTabWidget *tabWidget);
   void init(QTabWidget *tabWidget, InsertionMode mode);

   void redo() override;
   void undo() override;
};

class MoveTabPageCommand: public TabWidgetCommand
{

 public:
   explicit MoveTabPageCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~MoveTabPageCommand();

   void init(QTabWidget *tabWidget, QWidget *page,
      const QIcon &icon, const QString &label,
      int index, int newIndex);

   void redo() override;
   void undo() override;

 private:
   int m_newIndex;
   int m_oldIndex;
   QPointer<QWidget> m_page;
   QString m_label;
   QIcon m_icon;
};

class StackedWidgetCommand: public QDesignerFormWindowCommand
{

 public:
   explicit StackedWidgetCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~StackedWidgetCommand();

   void init(QStackedWidget *stackedWidget);

   virtual void removePage();
   virtual void addPage();

 protected:
   QPointer<QStackedWidget> m_stackedWidget;
   QPointer<QWidget> m_widget;
   int m_index;
};

class MoveStackedWidgetCommand: public StackedWidgetCommand
{

 public:
   explicit MoveStackedWidgetCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~MoveStackedWidgetCommand();

   void init(QStackedWidget *stackedWidget, QWidget *page, int newIndex);

   void redo() override;
   void undo() override;

 private:
   int m_newIndex;
   int m_oldIndex;
};

class DeleteStackedWidgetPageCommand: public StackedWidgetCommand
{

 public:
   explicit DeleteStackedWidgetPageCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~DeleteStackedWidgetPageCommand();

   void init(QStackedWidget *stackedWidget);

   void redo() override;
   void undo() override;
};

class AddStackedWidgetPageCommand: public StackedWidgetCommand
{

 public:
   enum InsertionMode {
      InsertBefore,
      InsertAfter
   };
   explicit AddStackedWidgetPageCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~AddStackedWidgetPageCommand();

   void init(QStackedWidget *stackedWidget);
   void init(QStackedWidget *stackedWidget, InsertionMode mode);

   void redo() override;
   void undo() override;
};

class CreateMenuBarCommand: public QDesignerFormWindowCommand
{

 public:
   explicit CreateMenuBarCommand(QDesignerFormWindowInterface *formWindow);

   void init(QMainWindow *mainWindow);

   void redo() override;
   void undo() override;

 private:
   QPointer<QMainWindow> m_mainWindow;
   QPointer<QMenuBar> m_menuBar;
};

class DeleteMenuBarCommand: public QDesignerFormWindowCommand
{

 public:
   explicit DeleteMenuBarCommand(QDesignerFormWindowInterface *formWindow);

   void init(QMenuBar *menuBar);

   void redo() override;
   void undo() override;

 private:
   QPointer<QMainWindow> m_mainWindow;
   QPointer<QMenuBar> m_menuBar;
};

class CreateStatusBarCommand: public QDesignerFormWindowCommand
{

 public:
   explicit CreateStatusBarCommand(QDesignerFormWindowInterface *formWindow);

   void init(QMainWindow *mainWindow);

   void redo() override;
   void undo() override;

 private:
   QPointer<QMainWindow> m_mainWindow;
   QPointer<QStatusBar> m_statusBar;
};

class DeleteStatusBarCommand: public QDesignerFormWindowCommand
{
 public:
   explicit DeleteStatusBarCommand(QDesignerFormWindowInterface *formWindow);

   void init(QStatusBar *statusBar);

   void redo() override;
   void undo() override;

 private:
   QPointer<QMainWindow> m_mainWindow;
   QPointer<QStatusBar> m_statusBar;
};

class AddToolBarCommand: public QDesignerFormWindowCommand
{
 public:
   explicit AddToolBarCommand(QDesignerFormWindowInterface *formWindow);

   void init(QMainWindow *mainWindow);

   void redo() override;
   void undo() override;

 private:
   QPointer<QMainWindow> m_mainWindow;
   QPointer<QToolBar> m_toolBar;
};

class DeleteToolBarCommand: public QDesignerFormWindowCommand
{
 public:
   explicit DeleteToolBarCommand(QDesignerFormWindowInterface *formWindow);

   void init(QToolBar *toolBar);

   void redo() override;
   void undo() override;

 private:
   QPointer<QMainWindow> m_mainWindow;
   QPointer<QToolBar> m_toolBar;
};

class DockWidgetCommand: public QDesignerFormWindowCommand
{
 public:
   explicit DockWidgetCommand(const QString &description, QDesignerFormWindowInterface *formWindow);
   virtual ~DockWidgetCommand();

   void init(QDockWidget *dockWidget);

 protected:
   QPointer<QDockWidget> m_dockWidget;
};

class AddDockWidgetCommand: public QDesignerFormWindowCommand
{

 public:
   explicit AddDockWidgetCommand(QDesignerFormWindowInterface *formWindow);

   void init(QMainWindow *mainWindow, QDockWidget *dockWidget);
   void init(QMainWindow *mainWindow);

   void redo() override;
   void undo() override;

 private:
   QPointer<QMainWindow> m_mainWindow;
   QPointer<QDockWidget> m_dockWidget;
};

class ContainerWidgetCommand: public QDesignerFormWindowCommand
{

 public:
   explicit ContainerWidgetCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~ContainerWidgetCommand();

   QDesignerContainerExtension *containerExtension() const;

   void init(QWidget *containerWidget);

   virtual void removePage();
   virtual void addPage();

 protected:
   QPointer<QWidget> m_containerWidget;
   QPointer<QWidget> m_widget;
   int m_index;
};

class DeleteContainerWidgetPageCommand: public ContainerWidgetCommand
{

 public:
   explicit DeleteContainerWidgetPageCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~DeleteContainerWidgetPageCommand();

   void init(QWidget *containerWidget, ContainerType ct);

   void redo() override;
   void undo() override;
};

class AddContainerWidgetPageCommand: public ContainerWidgetCommand
{

 public:
   enum InsertionMode {
      InsertBefore,
      InsertAfter
   };
   explicit AddContainerWidgetPageCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~AddContainerWidgetPageCommand();

   void init(QWidget *containerWidget, ContainerType ct, InsertionMode mode);

   void redo() override;
   void undo() override;
};

class ChangeCurrentPageCommand: public QDesignerFormWindowCommand
{

 public:
   explicit ChangeCurrentPageCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~ChangeCurrentPageCommand();

   QDesignerContainerExtension *containerExtension() const;

   void init(QWidget *containerWidget, int newIndex);

   void redo() override;
   void undo() override;

 protected:
   QPointer<QWidget> m_containerWidget;
   QPointer<QWidget> m_widget;
   int m_oldIndex;
   int m_newIndex;
};

struct ItemData {
   ItemData()
   {
   }

   ItemData(const QListWidgetItem *item, bool editor);
   ItemData(const QTableWidgetItem *item, bool editor);
   ItemData(const QTreeWidgetItem *item, int column);

   QListWidgetItem *createListItem(DesignerIconCache *iconCache, bool editor) const;
   QTableWidgetItem *createTableItem(DesignerIconCache *iconCache, bool editor) const;
   void fillTreeItemColumn(QTreeWidgetItem *item, int column, DesignerIconCache *iconCache) const;

   bool isValid() const {
      return !m_properties.isEmpty();
   }

   bool operator==(const ItemData &rhs) const {
      return m_properties == rhs.m_properties;
   }

   bool operator!=(const ItemData &rhs) const {
      return m_properties != rhs.m_properties;
   }

   QHash<int, QVariant> m_properties;
};

struct ListData {
   ListData()
   {
   }

   ListData(const QTreeWidgetItem *item);

   QTreeWidgetItem *createTreeItem(DesignerIconCache *iconCache) const;

   void createFromListWidget(const QListWidget *listWidget, bool editor);
   void applyToListWidget(QListWidget *listWidget, DesignerIconCache *iconCache, bool editor) const;
   void createFromComboBox(const QComboBox *listWidget);
   void applyToComboBox(QComboBox *listWidget, DesignerIconCache *iconCache) const;

   bool operator==(const ListData &other) const {
      return m_items == other.m_items;
   }

   bool operator!=(const ListData &other) const {
      return m_items != other.m_items;
   }

   QList<ItemData> m_items;
};

struct TableWidgetData {
   TableWidgetData();
   void clear();

   void fromTableWidget(const QTableWidget *tableWidget, bool editor);
   void applyToTableWidget(QTableWidget *tableWidget, DesignerIconCache *iconCache, bool editor) const;

   bool operator==(const TableWidgetData &rhs) const;
   bool operator!=(const TableWidgetData &rhs) const {
      return !(*this == rhs);
   }

   static bool nonEmpty(const QTableWidgetItem *item, int headerColumn);
   static QString defaultHeaderText(int i);
   static void insertHeaderItem(const QTableWidgetItem *item, int i, ListData *header, bool editor);

   int m_columnCount;
   int m_rowCount;

   ListData m_horizontalHeader;
   ListData m_verticalHeader;

   QMap<QPair<int, int>, ItemData> m_items;
};

class ChangeTableDataCommand : public QDesignerFormWindowCommand
{
 public:
   explicit ChangeTableDataCommand(QDesignerFormWindowInterface *formWindow);

   void init(QTableWidget *tableWidget, const TableWidgetData &oldCont, const TableWidgetData &newCont);
   void redo() override;
   void undo() override;

 private:
   QPointer<QTableWidget> m_tableWidget;
   TableWidgetData m_oldContents;
   TableWidgetData m_newContents;
   DesignerIconCache *m_iconCache;
};

struct TreeWidgetData {

   struct TreeNode : public ListData {
      TreeNode()
         : m_owner(nullptr), m_itemFlags(-1)
      {
      }

      TreeNode(TreeWidgetData *owner, const QTreeWidgetItem *item, bool editor);

      QTreeWidgetItem *createTreeItem(DesignerIconCache *iconCache, bool editor) const;

      bool operator==(const TreeNode &other) const;

      bool operator!=(const TreeNode &other) const {
         return ! (*this == other);
      }

      TreeWidgetData *m_owner;

      int m_itemFlags;
      int m_id;
   };

   void clear();

   void fromTreeWidget(const QTreeWidget *treeWidget, bool editor);
   void applyToTreeWidget(QTreeWidget *treeWidget, DesignerIconCache *iconCache, bool editor) const;

   bool operator==(const TreeWidgetData &other) const;
   bool operator!=(const TreeWidgetData &other) const {
      return !(*this == other);
   }

   ListData m_headerItem;
   QList<TreeNode> m_rootItems;

   QMultiMap<int, TreeNode> m_childNodes;
};

class ChangeTreeDataCommand: public QDesignerFormWindowCommand
{
 public:
   explicit ChangeTreeDataCommand(QDesignerFormWindowInterface *formWindow);

   void init(QTreeWidget *treeWidget, const TreeWidgetData &oldState, const TreeWidgetData &newState);
   void redo() override;
   void undo() override;

   enum ApplyIconStrategy {
      SetIconStrategy,
      ResetIconStrategy
   };

 private:
   QPointer<QTreeWidget> m_treeWidget;
   TreeWidgetData m_oldState;
   TreeWidgetData m_newState;
   DesignerIconCache *m_iconCache;
};

class ChangeListDataCommand : public QDesignerFormWindowCommand
{
 public:
   explicit ChangeListDataCommand(QDesignerFormWindowInterface *formWindow);

   void init(QListWidget *listWidget, const ListData &oldItems, const ListData &items);
   void init(QComboBox *comboBox, const ListData &oldItems, const ListData &items);
   void redo() override;
   void undo() override;

 private:
   QPointer<QListWidget> m_listWidget;
   QPointer<QComboBox> m_comboBox;
   ListData m_oldItemsState;
   ListData m_newItemsState;
   DesignerIconCache *m_iconCache;
};

class AddActionCommand : public QDesignerFormWindowCommand
{
 public:
   explicit AddActionCommand(QDesignerFormWindowInterface *formWindow);
   void init(QAction *action);
   void redo() override;
   void undo() override;

 private:
   QAction *m_action;
};

// Note: This command must be executed within a macro since it
// makes the form emit objectRemoved() which might cause other components
// to add commands (for example, removal of signals and slots
class RemoveActionCommand : public QDesignerFormWindowCommand
{
 public:
   explicit RemoveActionCommand(QDesignerFormWindowInterface *formWindow);
   void init(QAction *action);
   void redo() override;
   void undo() override;

   struct ActionDataItem {
      ActionDataItem(QAction *_before = nullptr, QWidget *_widget = nullptr)
         : before(_before), widget(_widget)
      {
      }

      QAction *before;
      QWidget *widget;
   };

   typedef QList<ActionDataItem> ActionData;

 private:
   QAction *m_action;

   ActionData m_actionData;
};

class ActionInsertionCommand : public QDesignerFormWindowCommand
{
 public:
   void init(QWidget *parentWidget, QAction *action, QAction *beforeAction = nullptr, bool update = true);

 protected:
   ActionInsertionCommand(const QString &text, QDesignerFormWindowInterface *formWindow);

   void insertAction();
   void removeAction();

 private:
   QWidget *m_parentWidget;
   QAction *m_action;
   QAction *m_beforeAction;
   bool m_update;
};

class InsertActionIntoCommand : public ActionInsertionCommand
{
 public:
   explicit InsertActionIntoCommand(QDesignerFormWindowInterface *formWindow);

   void redo() override {
      insertAction();
   }

   void undo() override {
      removeAction();
   }
};

class RemoveActionFromCommand : public ActionInsertionCommand
{
 public:
   explicit RemoveActionFromCommand(QDesignerFormWindowInterface *formWindow);

   void redo() override {
      removeAction();
   }

   void undo() override {
      insertAction();
   }
};

class MenuActionCommand : public QDesignerFormWindowCommand
{
 public:
   void init(QAction *action, QAction *actionBefore, QWidget *associatedWidget, QWidget *objectToSelect);

 protected:
   MenuActionCommand(const QString &text, QDesignerFormWindowInterface *formWindow);
   void insertMenu();
   void removeMenu();

 private:
   QAction *m_action;
   QAction *m_actionBefore;
   QWidget *m_menuParent;
   QWidget *m_associatedWidget;
   QWidget *m_objectToSelect;
};

class AddMenuActionCommand : public MenuActionCommand
{
 public:
   explicit AddMenuActionCommand(QDesignerFormWindowInterface *formWindow);

   void redo()  override {
      insertMenu();
   }

   void undo()  override {
      removeMenu();
   }
};

class RemoveMenuActionCommand : public MenuActionCommand
{
 public:
   explicit RemoveMenuActionCommand(QDesignerFormWindowInterface *formWindow);

   void redo() override {
      removeMenu();
   }

   void undo() override {
      insertMenu();
   }
};

class CreateSubmenuCommand : public QDesignerFormWindowCommand
{
 public:
   explicit CreateSubmenuCommand(QDesignerFormWindowInterface *formWindow);
   void init(QDesignerMenu *menu, QAction *action, QObject *m_objectToSelect = nullptr);
   void redo() override;
   void undo() override;

 private:
   QAction *m_action;
   QDesignerMenu *m_menu;
   QObject *m_objectToSelect;
};


/* This command changes the type of a managed layout on a widget (including
 * red layouts of type 'QLayoutWidget') into another type, maintaining the
 * applicable properties. It does this by chaining BreakLayoutCommand and
 * LayoutCommand, parametrizing them not to actually delete/reparent QLayoutWidget's.
*/

class MorphLayoutCommand : public QDesignerFormWindowCommand
{
   Q_DISABLE_COPY(MorphLayoutCommand)

 public:
   explicit MorphLayoutCommand(QDesignerFormWindowInterface *formWindow);
   virtual ~MorphLayoutCommand();

   bool init(QWidget *w, int newType);

   static bool canMorph(const QDesignerFormWindowInterface *formWindow, QWidget *w, int *ptrToCurrentType = nullptr);

   void redo() override;
   void undo() override;

 private:
   static QString formatDescription(QDesignerFormEditorInterface *core, const QWidget *w, int oldType, int newType);

   BreakLayoutCommand *m_breakLayoutCommand;
   LayoutCommand *m_layoutCommand;
   int m_newType;
   QWidgetList m_widgets;
   QWidget *m_layoutBase;
};

// Change the alignment of a widget in a managed grid/box layout cell.
class LayoutAlignmentCommand : public QDesignerFormWindowCommand
{
   Q_DISABLE_COPY(LayoutAlignmentCommand)

 public:
   explicit LayoutAlignmentCommand(QDesignerFormWindowInterface *formWindow);

   bool init(QWidget *w, Qt::Alignment alignment);

   void redo() override;
   void undo() override;

   // Find out alignment and return whether command is enabled.
   static Qt::Alignment alignmentOf(const QDesignerFormEditorInterface *core, QWidget *w, bool *enabled = nullptr);

 private:
   static void applyAlignment(const QDesignerFormEditorInterface *core, QWidget *w, Qt::Alignment a);

   Qt::Alignment m_newAlignment;
   Qt::Alignment m_oldAlignment;
   QWidget *m_widget;
};

}   // end namespace qdesigner_internal

#endif
