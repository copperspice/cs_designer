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

#include <abstract_formeditor.h>
#include <action_repository.h>
#include <designer_propertysheet.h>
#include <designer_utils.h>
#include <extension.h>
#include <resource_view.h>

#include <iconloader_p.h>

#include <QAction>
#include <QContextMenuEvent>
#include <QDrag>
#include <QEvent>
#include <QHeaderView>
#include <QMenu>
#include <QPixmap>
#include <QSet>
#include <QStandardItemModel>
#include <QToolBar>
#include <QToolButton>

#include <qalgorithms.h>

CS_DECLARE_METATYPE(QAction *)

constexpr const int LIST_MODE_SIZE = 16;
constexpr const int ICON_MODE_SIZE = 24;

static const QString actionMimeType    = "action-repository/actions";
static const QString plainTextMimeType = "text/plain";

static inline QAction *actionOfItem(const QStandardItem *item)
{
   QVariant data = item->data(qdesigner_internal::ActionModel::ActionRole);

   return data.value<QAction *>();
}

namespace qdesigner_internal {

ActionModel::ActionModel(QWidget *parent )
   : QStandardItemModel(parent), m_emptyIcon(emptyIcon()), m_core(nullptr)
{
   QStringList headers;
   headers += tr("Name");
   headers += tr("Used");
   headers += tr("Text");
   headers += tr("Shortcut");
   headers += tr("Checkable");
   headers += tr("ToolTip");
   Q_ASSERT(NumColumns == headers.size());
   setHorizontalHeaderLabels(headers);
}

void ActionModel::clearActions()
{
   removeRows(0, rowCount());
}

int ActionModel::findAction(QAction *action) const
{
   const int rows = rowCount();

   for (int i = 0; i < rows; i++) {
      if (action == actionOfItem(item(i))) {
         return i;
      }
   }

   return -1;
}

void ActionModel::update(int row)
{
   Q_ASSERT(m_core);

   // need to create the row list
   if (row >= rowCount()) {
      return;
   }

   QStandardItemList list;
   for (int i = 0; i < NumColumns; i++) {
      list += item(row, i);
   }

   setItems(m_core, actionOfItem(list.front()), m_emptyIcon, list);
}

void ActionModel::remove(int row)
{
   qDeleteAll(takeRow(row));
}

QModelIndex ActionModel::addAction(QAction *action)
{
   Q_ASSERT(m_core);
   QStandardItemList items;
   const  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;

   QVariant itemData;
   itemData.setValue(action);

   for (int i = 0; i < NumColumns; i++) {
      QStandardItem *item = new QStandardItem;
      item->setData(itemData, ActionRole);
      item->setFlags(flags);
      items.push_back(item);
   }

   setItems(m_core, action, m_emptyIcon, items);
   appendRow(items);

   return indexFromItem(items.front());
}

// Find the associated menus and toolbars, ignore toolbuttons
QWidgetList ActionModel::associatedWidgets(const QAction *action)
{
   QWidgetList rc = action->associatedWidgets();

   for (QWidgetList::iterator it = rc.begin(); it != rc.end(); ) {
      if (dynamic_cast<const QMenu *>(*it) || dynamic_cast<const QToolBar *>(*it)) {
         ++it;
      } else {
         it = rc.erase(it);
      }
   }

   return rc;
}

// shortcut is a fake property, need to retrieve it via property sheet.
PropertySheetKeySequenceValue ActionModel::actionShortCut(QDesignerFormEditorInterface *core, QAction *action)
{
   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), action);

   if (!sheet) {
      return PropertySheetKeySequenceValue();
   }

   return actionShortCut(sheet);
}

PropertySheetKeySequenceValue ActionModel::actionShortCut(const QDesignerPropertySheetExtension *sheet)
{
   const int index = sheet->indexOf(QString("shortcut"));
   if (index == -1) {
      return PropertySheetKeySequenceValue();
   }

   QVariant data = sheet->property(index);

   return data.value<PropertySheetKeySequenceValue>();
}

void  ActionModel::setItems(QDesignerFormEditorInterface *core, QAction *action, const QIcon &defaultIcon, QStandardItemList &sl)
{
   // Tooltip, mostly for icon view mode
   QString firstTooltip = action->objectName();
   const QString text   = action->text();

   if (! text.isEmpty()) {
      firstTooltip += '\n';
      firstTooltip += text;
   }

   Q_ASSERT(sl.size() == NumColumns);

   QStandardItem *item =  sl[NameColumn];
   item->setText(action->objectName());
   QIcon icon = action->icon();
   if (icon.isNull()) {
      icon = defaultIcon;
   }

   item->setIcon(icon);
   item->setToolTip(firstTooltip);
   item->setWhatsThis(firstTooltip);

   // Used
   const QWidgetList associatedDesignerWidgets = associatedWidgets(action);
   const bool used = !associatedDesignerWidgets.empty();
   item = sl[UsedColumn];
   item->setCheckState(used ? Qt::Checked : Qt::Unchecked);

   if (used) {
      QString usedToolTip;
      const QString separator = QString(", ");
      const int count = associatedDesignerWidgets.size();

      for (int i = 0; i < count; i++) {
         if (i) {
            usedToolTip += separator;
         }
         usedToolTip += associatedDesignerWidgets.at(i)->objectName();
      }
      item->setToolTip(usedToolTip);
   } else {
      item->setToolTip(QString());
   }

   // text
   item = sl[TextColumn];
   item->setText(action->text());
   item->setToolTip(action->text());

   // shortcut
   const QString shortcut = actionShortCut(core, action).value().toString(QKeySequence::NativeText);
   item = sl[ShortCutColumn];
   item->setText(shortcut);
   item->setToolTip(shortcut);

   // checkable
   sl[CheckedColumn]->setCheckState(action->isCheckable() ?  Qt::Checked : Qt::Unchecked);

   // ToolTip. This might be multi-line, rich text
   QString toolTip = action->toolTip();
   item = sl[ToolTipColumn];
   item->setToolTip(toolTip);
   item->setText(toolTip.replace(QLatin1Char('\n'), QLatin1Char(' ')));
}

QMimeData *ActionModel::mimeData(const QModelIndexList &indexes ) const
{
   ActionRepositoryMimeData::ActionList actionList;

   QSet<QAction *> actions;
   for (const QModelIndex &index : indexes) {
      if (QStandardItem *item = itemFromIndex(index)) {
         if (QAction *action = actionOfItem(item)) {
            actions.insert(action);
         }
      }
   }

   return new ActionRepositoryMimeData(actions.toList(), Qt::CopyAction);
}

// Resource images are plain text. The drag needs to be restricted, however.
QStringList ActionModel::mimeTypes() const
{
   return QStringList(plainTextMimeType);
}

QString ActionModel::actionName(int row) const
{
   return item(row, NameColumn)->text();
}

bool ActionModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &)
{
   if (action != Qt::CopyAction) {
      return false;
   }

   QStandardItem *droppedItem = item(row, column);
   if (!droppedItem) {
      return false;
   }

   QtResourceView::ResourceType type;
   QString path;
   if (!QtResourceView::decodeMimeData(data, &type, &path) || type !=  QtResourceView::ResourceImage) {
      return false;
   }

   emit resourceImageDropped(path, actionOfItem(droppedItem));
   return true;
}

QAction *ActionModel::actionAt(const  QModelIndex &index) const
{
   if (!index.isValid()) {
      return nullptr;
   }

   QStandardItem *i = itemFromIndex(index);
   if (!i) {
      return nullptr;
   }

   return actionOfItem(i);
}

// helpers

static bool handleImageDragEnterMoveEvent(QDropEvent *event)
{
   QtResourceView::ResourceType type;
   const bool rc = QtResourceView::decodeMimeData(event->mimeData(), &type) && type ==  QtResourceView::ResourceImage;

   if (rc) {
      event->acceptProposedAction();
   } else {
      event->ignore();
   }

   return rc;
}

static void handleImageDropEvent(const QAbstractItemView *iv, QDropEvent *event, ActionModel *am)
{
   const QModelIndex index = iv->indexAt(event->pos());
   if (!index.isValid()) {
      event->ignore();
      return;
   }

   if (!handleImageDragEnterMoveEvent(event)) {
      return;
   }

   am->dropMimeData(event->mimeData(),  event->proposedAction(), index.row(), 0, iv->rootIndex());
}

// Basically mimic  QAbstractItemView's startDrag routine, except that
// another pixmap is used, we don't want the whole row.

void startActionDrag(QWidget *dragParent, ActionModel *model, const QModelIndexList &indexes, Qt::DropActions supportedActions)
{
   if (indexes.empty()) {
      return;
   }

   QDrag *drag = new QDrag(dragParent);
   QMimeData *data = model->mimeData(indexes);
   drag->setMimeData(data);
   if (ActionRepositoryMimeData *actionMimeData = dynamic_cast<ActionRepositoryMimeData *>(data)) {
      drag->setPixmap(ActionRepositoryMimeData::actionDragPixmap(actionMimeData->actionList().front()));
   }

   drag->start(supportedActions);
}

ActionTreeView::ActionTreeView(ActionModel *model, QWidget *parent)
   : QTreeView(parent), m_model(model)
{
   setDragEnabled(true);
   setAcceptDrops(true);
   setDropIndicatorShown(true);
   setDragDropMode(DragDrop);
   setModel(model);
   setRootIsDecorated(false);
   setTextElideMode(Qt::ElideMiddle);

   setModel(model);

   connect(this, &QTreeView::activated,
         this, &ActionTreeView::slotActivated);

   connect(header(), &QHeaderView::sectionDoubleClicked,
         this, &QTreeView::resizeColumnToContents);

   setIconSize(QSize(LIST_MODE_SIZE, LIST_MODE_SIZE));

}

QAction *ActionTreeView::currentAction() const
{
   return m_model->actionAt(currentIndex());
}

void ActionTreeView::filter(const QString &text)
{
   const int rowCount = m_model->rowCount();
   const bool empty = text.isEmpty();
   const QModelIndex parent = rootIndex();

   for (int i = 0; i < rowCount; i++) {
      setRowHidden(i, parent, !empty && !m_model->actionName(i).contains(text, Qt::CaseInsensitive));
   }
}

void ActionTreeView::dragEnterEvent(QDragEnterEvent *event)
{
   handleImageDragEnterMoveEvent(event);
}

void ActionTreeView::dragMoveEvent(QDragMoveEvent *event)
{
   handleImageDragEnterMoveEvent(event);
}

void ActionTreeView::dropEvent(QDropEvent *event)
{
   handleImageDropEvent(this, event, m_model);
}

void ActionTreeView::focusInEvent(QFocusEvent *event)
{
   QTreeView::focusInEvent(event);
   // Make property editor display current action
   if (QAction *a = currentAction()) {
      emit currentActionChanged(a);
   }
}

void ActionTreeView::contextMenuEvent(QContextMenuEvent *event)
{
   emit actionContextMenuRequested(event, m_model->actionAt(indexAt(event->pos())));
}

void ActionTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
   emit currentActionChanged(m_model->actionAt(current));
   QTreeView::currentChanged(current, previous);
}

void ActionTreeView::slotActivated(const QModelIndex &index)
{
   emit actionActivated(m_model->actionAt(index));
}

void ActionTreeView::startDrag(Qt::DropActions supportedActions)
{
   startActionDrag(this, m_model, selectedIndexes(), supportedActions);
}

// ---------------- ActionListView:
ActionListView::ActionListView(ActionModel *model, QWidget *parent) :
   QListView(parent),
   m_model(model)
{
   setDragEnabled(true);
   setAcceptDrops(true);
   setDropIndicatorShown(true);
   setDragDropMode(DragDrop);
   setModel(model);
   setTextElideMode(Qt::ElideMiddle);
   connect(this, &QListView::activated, this, &ActionListView::slotActivated);

   // actually want 'Static' as the user should be able to drag away actions only (not to rearrange icons).
   // We emulate that by not accepting our own drag data. 'Static' causes the list view to disable
   // drag and drop  on the viewport

   setMovement(Snap);
   setViewMode(IconMode);
   setIconSize(QSize(ICON_MODE_SIZE, ICON_MODE_SIZE));
   setGridSize(QSize(4 * ICON_MODE_SIZE, 2 *  ICON_MODE_SIZE));
   setSpacing(ICON_MODE_SIZE / 3);
}

QAction *ActionListView::currentAction() const
{
   return m_model->actionAt(currentIndex());
}

void ActionListView::filter(const QString &text)
{
   const int rowCount = m_model->rowCount();
   const bool empty = text.isEmpty();
   for (int i = 0; i < rowCount; i++) {

      setRowHidden(i, !empty && !m_model->actionName(i).contains(text, Qt::CaseInsensitive));
   }
}

void ActionListView::dragEnterEvent(QDragEnterEvent *event)
{
   handleImageDragEnterMoveEvent(event);
}

void ActionListView::dragMoveEvent(QDragMoveEvent *event)
{
   handleImageDragEnterMoveEvent(event);
}

void ActionListView::dropEvent(QDropEvent *event)
{
   handleImageDropEvent(this, event, m_model);
}

void ActionListView::focusInEvent(QFocusEvent *event)
{
   QListView::focusInEvent(event);

   // Make property editor display current action
   if (QAction *a = currentAction()) {
      emit currentActionChanged(a);
   }
}

void ActionListView::contextMenuEvent(QContextMenuEvent *event)
{
   emit actionContextMenuRequested(event, m_model->actionAt(indexAt(event->pos())));
}

void ActionListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
   emit currentActionChanged(m_model->actionAt(current));
   QListView::currentChanged(current, previous);
}

void ActionListView::slotActivated(const QModelIndex &index)
{
   emit actionActivated(m_model->actionAt(index));
}

void ActionListView::startDrag(Qt::DropActions supportedActions)
{
   startActionDrag(this, m_model, selectedIndexes(), supportedActions);
}

//  ActionView
ActionView::ActionView(QWidget *parent)
   : QStackedWidget(parent), m_model(new ActionModel(this)),
     m_actionTreeView(new ActionTreeView(m_model)), m_actionListView(new ActionListView(m_model))
{
   addWidget(m_actionListView);
   addWidget(m_actionTreeView);

   // Wire signals
   connect(m_actionTreeView, &ActionTreeView::actionContextMenuRequested,
         this, &ActionView::contextMenuRequested);

   connect(m_actionListView, &ActionListView::actionContextMenuRequested,
         this, &ActionView::contextMenuRequested);

   // make it possible for vs integration to reimplement edit action dialog
   // [which it shouldn't do actually]
   connect(m_actionListView, &ActionListView::actionActivated, this, &ActionView::activated);
   connect(m_actionTreeView, &ActionTreeView::actionActivated, this, &ActionView::activated);

   connect(m_actionListView, &ActionListView::currentActionChanged,
         this, &ActionView::slotCurrentChanged);

   connect(m_actionTreeView, &ActionTreeView::currentActionChanged,
         this, &ActionView::slotCurrentChanged);

   connect(m_model, &ActionModel::resourceImageDropped,
         this, &ActionView::resourceImageDropped);

   // sync selection models
   QItemSelectionModel *selectionModel = m_actionTreeView->selectionModel();
   m_actionListView->setSelectionModel(selectionModel);

   connect(selectionModel, &QItemSelectionModel::selectionChanged,
         this, &ActionView::selectionChanged);
}

int ActionView::viewMode() const
{
   if (currentWidget() == m_actionListView) {
      return ViewMode::IconView;

   } else {
      return ViewMode::DetailedView;
   }
}

void ActionView::setViewMode(int lm)
{
   if (viewMode() == lm) {
      return;
   }

   switch (lm) {
      case ViewMode::IconView:
         setCurrentWidget(m_actionListView);
         break;

      case ViewMode::DetailedView:
         setCurrentWidget(m_actionTreeView);
         break;

      default:
         break;
   }
}

void ActionView::slotCurrentChanged(QAction *action)
{
   // emit only for currently visible
   if (sender() == currentWidget()) {
      emit currentChanged(action);
   }
}

void ActionView::filter(const QString &text)
{
   m_actionTreeView->filter(text);
   m_actionListView->filter(text);
}

void ActionView::selectAll()
{
   m_actionTreeView->selectAll();
}

void ActionView::clearSelection()
{
   m_actionTreeView->selectionModel()->clearSelection();
}

void ActionView::setCurrentIndex(const QModelIndex &index)
{
   m_actionTreeView->setCurrentIndex(index);
}

QAction *ActionView::currentAction() const
{
   return m_actionListView->currentAction();
}

void ActionView::setSelectionMode(QAbstractItemView::SelectionMode sm)
{
   m_actionTreeView->setSelectionMode(sm);
   m_actionListView->setSelectionMode(sm);
}

QAbstractItemView::SelectionMode ActionView::selectionMode() const
{
   return m_actionListView->selectionMode();
}

QItemSelection ActionView::selection() const
{
   return  m_actionListView->selectionModel()->selection();
}

ActionView::ActionList ActionView::selectedActions() const
{
   ActionList rc;

   for (const QModelIndex &index : selection().indexes()) {
      if (index.column() == 0) {
         rc += actionOfItem(m_model->itemFromIndex(index));
      }
   }

   return rc;
}

ActionRepositoryMimeData::ActionRepositoryMimeData(QAction *a, Qt::DropAction dropAction)
   : m_dropAction(dropAction)
{
   m_actionList += a;
}

ActionRepositoryMimeData::ActionRepositoryMimeData(const ActionList &al, Qt::DropAction dropAction)
   : m_dropAction(dropAction), m_actionList(al)
{
}

QStringList ActionRepositoryMimeData::formats() const
{
   return QStringList(actionMimeType);
}

QPixmap  ActionRepositoryMimeData::actionDragPixmap(const QAction *action)
{
   // Try to find a suitable pixmap. Grab either widget or icon.
   const QIcon icon = action->icon();
   if (!icon.isNull()) {
      return icon.pixmap(QSize(22, 22));
   }

   for (QWidget *w : action->associatedWidgets())
      if (QToolButton *tb = dynamic_cast<QToolButton *>(w)) {
         return tb->grab(QRect(0, 0, -1, -1));
      }

   // Create a QToolButton
   QToolButton *tb = new QToolButton;
   tb->setText(action->text());
   tb->setToolButtonStyle(Qt::ToolButtonTextOnly);
   tb->adjustSize();
   const QPixmap rc = tb->grab(QRect(0, 0, -1, -1));
   tb->deleteLater();

   return rc;
}

void ActionRepositoryMimeData::accept(QDragMoveEvent *event) const
{
   if (event->proposedAction() == m_dropAction) {
      event->acceptProposedAction();
   } else {
      event->setDropAction(m_dropAction);
      event->accept();
   }
}

} // namespace qdesigner_internal

