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
#include <abstract_formwindow.h>
#include <abstract_widgetdatabase.h>
#include <container.h>
#include <designer_property_command.h>
#include <designer_utils.h>
#include <extension.h>
#include <extension_manager.h>
#include <layout.h>
#include <layout_widget.h>
#include <object_inspector_model.h>
#include <utils.h>

#include <iconloader_p.h>
#include <metadatabase_p.h>

#include <QAction>
#include <QButtonGroup>
#include <QCoreApplication>
#include <QLayout>
#include <QLayoutItem>
#include <QMenu>
#include <QSet>

#include <algorithm>

constexpr const int DATA_ROLE = 1000;

static inline QObject *objectOfItem(const QStandardItem *item)
{
   QVariant tmp = item->data(DATA_ROLE);
   return tmp.value<QObject *>();
}

static bool sortEntry(const QObject *a, const QObject *b)
{
   return a->objectName() < b->objectName();
}

static bool sameIcon(const QIcon &i1, const QIcon &i2)
{
   if (i1.isNull() &&  i2.isNull()) {
      return true;
   }

   if (i1.isNull() !=  i2.isNull()) {
      return false;
   }

   return i1.cacheKey() == i2.cacheKey();
}

static inline bool isNameColumnEditable(const QObject *)
{
   return true;
}

static qdesigner_internal::ObjectData::StandardItemList createModelRow(const QObject *obj)
{
   qdesigner_internal::ObjectData::StandardItemList rc;
   const Qt::ItemFlags baseFlags = Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;

   for (int i = 0; i < qdesigner_internal::ObjectInspectorModel::NumColumns; ++i) {
      QStandardItem *item = new QStandardItem;
      Qt::ItemFlags flags = baseFlags;

      if (i == qdesigner_internal::ObjectInspectorModel::ObjectNameColumn && isNameColumnEditable(obj)) {
         flags |= Qt::ItemIsEditable;
      }

      item->setFlags(flags);
      rc += item;
   }

   return rc;
}

static inline bool isQLayoutWidget(const QObject *obj)
{
   return compareMetaObject(*(obj->metaObject()), QLayoutWidget::staticMetaObject());
}

namespace qdesigner_internal {

// context kept while building a model, just there to reduce string allocations
struct ModelRecursionContext {
   explicit ModelRecursionContext(QDesignerFormEditorInterface *core, const QString &sepName);

   const QString designerPrefix;
   const QString separator;

   QDesignerFormEditorInterface *core;
   const QDesignerWidgetDataBaseInterface *db;
   const QDesignerMetaDataBaseInterface *mdb;
};

ModelRecursionContext::ModelRecursionContext(QDesignerFormEditorInterface *c, const QString &sepName)
   : designerPrefix("QDesigner"), separator(sepName), core(c),
     db(c->widgetDataBase()), mdb(c->metaDataBase())
{
}

// ------------  ObjectData/ ObjectModel:
// Whenever the selection changes, ObjectInspector::setFormWindow is
// called. To avoid rebuilding the tree every time (loosing expanded state)
// a model is first built from the object tree by recursion.
// As a tree is difficult to represent, a flat list of entries (ObjectData)
// containing object and parent object is used.
// ObjectData has an overloaded operator== that compares the object pointers.
// Structural changes which cause a rebuild can be detected by
// comparing the lists of ObjectData. If it is the same, only the item data (class name [changed by promotion],
// object name and icon) are checked and the existing items are updated.

ObjectData::ObjectData()
   : m_parent(nullptr), m_object(nullptr), m_type(Object), m_managedLayoutType(LayoutInfo::NoLayout)
{
}

ObjectData::ObjectData(QObject *parent, QObject *object, const ModelRecursionContext &ctx)
   : m_parent(parent), m_object(object), m_type(Object),
     m_className(object->metaObject()->className()),
     m_objectName(object->objectName()), m_managedLayoutType(LayoutInfo::NoLayout)
{

   // 1) set entry
   if (object->isWidgetType()) {
      initWidget(static_cast<QWidget *>(object), ctx);
   } else {
      initObject(ctx);
   }

   if (m_className.startsWith(ctx.designerPrefix)) {
      m_className.remove(1, ctx.designerPrefix.size() - 1);
   }
}

void ObjectData::initObject(const ModelRecursionContext &ctx)
{
   // Check objects: Action?
   if (const QAction *act = dynamic_cast<const QAction *>(m_object)) {
      if (act->isSeparator()) {
         // separator is reserved
         m_objectName = ctx.separator;
         m_type = SeparatorAction;
      } else {
         m_type = Action;
      }

      m_classIcon = act->icon();

   } else {
      m_type = Object;
   }
}

void ObjectData::initWidget(QWidget *w, const ModelRecursionContext &ctx)
{
   // Check for extension container, QLayoutwidget, or normal container
   bool isContainer = false;
   if (const QDesignerWidgetDataBaseItemInterface *widgetItem = ctx.db->item(ctx.db->indexOfObject(w, true))) {
      m_classIcon = widgetItem->icon();
      m_className = widgetItem->name();
      isContainer = widgetItem->isContainer();
   }

   // We might encounter temporary states with no layouts when re-layouting.
   // Just default to Widget handling for the moment.
   if (isQLayoutWidget(w)) {
      if (const QLayout *layout = w->layout()) {
         m_type = LayoutWidget;
         m_managedLayoutType = LayoutInfo::layoutType(ctx.core, layout);
         m_className  = layout->metaObject()->className();
         m_objectName = layout->objectName();
      }
      return;
   }

   if (qt_extension<QDesignerContainerExtension *>(ctx.core->extensionManager(), w)) {
      m_type = ExtensionContainer;
      return;
   }

   if (isContainer) {
      m_type = LayoutableContainer;
      m_managedLayoutType = LayoutInfo::managedLayoutType(ctx.core, w);
      return;
   }

   m_type = ChildWidget;
}

bool ObjectData::equals(const ObjectData &me) const
{
   return m_parent == me.m_parent && m_object == me.m_object;
}

unsigned ObjectData::compare(const ObjectData &rhs) const
{
   unsigned rc = 0;
   if (m_className != rhs.m_className) {
      rc |= ClassNameChanged;
   }

   if (m_objectName != rhs.m_objectName) {
      rc |= ObjectNameChanged;
   }

   if (!sameIcon(m_classIcon, rhs.m_classIcon)) {
      rc |= ClassIconChanged;
   }

   if (m_type != rhs.m_type) {
      rc |= TypeChanged;
   }

   if (m_managedLayoutType != rhs.m_managedLayoutType) {
      rc |= LayoutTypeChanged;
   }

   return rc;
}

void ObjectData::setItemsDisplayData(const StandardItemList &row, const ObjectInspectorIcons &icons, unsigned mask) const
{
   if (mask & ObjectNameChanged) {
      row[ObjectInspectorModel::ObjectNameColumn]->setText(m_objectName);
   }
   if (mask & ClassNameChanged) {
      row[ObjectInspectorModel::ClassNameColumn]->setText(m_className);
      row[ObjectInspectorModel::ClassNameColumn]->setToolTip(m_className);
   }

   // Set a layout icon only for containers. Note that QLayoutWidget don't have real class icons
   if (mask & (ClassIconChanged | TypeChanged | LayoutTypeChanged)) {
      switch (m_type) {
         case LayoutWidget:
            row[ObjectInspectorModel::ObjectNameColumn]->setIcon(icons.layoutIcons[m_managedLayoutType]);
            row[ObjectInspectorModel::ClassNameColumn]->setIcon(icons.layoutIcons[m_managedLayoutType]);
            break;

         case LayoutableContainer:
            row[ObjectInspectorModel::ObjectNameColumn]->setIcon(icons.layoutIcons[m_managedLayoutType]);
            row[ObjectInspectorModel::ClassNameColumn]->setIcon(m_classIcon);
            break;

         default:
            row[ObjectInspectorModel::ObjectNameColumn]->setIcon(QIcon());
            row[ObjectInspectorModel::ClassNameColumn]->setIcon(m_classIcon);
            break;
      }
   }
}

void ObjectData::setItems(const StandardItemList &row, const ObjectInspectorIcons &icons) const
{
   const QVariant object = QVariant::fromValue(m_object);
   row[ObjectInspectorModel::ObjectNameColumn]->setData(object, DATA_ROLE);
   row[ObjectInspectorModel::ClassNameColumn]->setData(object, DATA_ROLE);
   setItemsDisplayData(row, icons, ClassNameChanged | ObjectNameChanged | ClassIconChanged | TypeChanged | LayoutTypeChanged);
}

// Recursive routine that creates the model by traversing the form window object tree.
void createModelRecursion(const QDesignerFormWindowInterface *fwi, QObject *parent,
      QObject *object, QList<ObjectData> &model, const ModelRecursionContext &ctx)
{
   typedef QList<QButtonGroup *> ButtonGroupList;
   typedef QList<QAction *> ActionList;

   // 1) Create entry
   const ObjectData entry(parent, object, ctx);
   model.push_back(entry);

   // 2) recurse over widget children via container extension or children list
   const QDesignerContainerExtension *containerExtension = nullptr;

   if (entry.type() == ObjectData::ExtensionContainer) {
      containerExtension = qt_extension<QDesignerContainerExtension *>(fwi->core()->extensionManager(), object);
      Q_ASSERT(containerExtension);

      const int count = containerExtension->count();

      for (int i = 0; i < count; ++i) {
         QObject *page = containerExtension->widget(i);
         Q_ASSERT(page != nullptr);
         createModelRecursion(fwi, object, page, model, ctx);
      }
   }

   QObjectList children = object->children();
   if (! children.empty()) {
      ButtonGroupList buttonGroups;
      std::sort(children.begin(), children.end(), sortEntry);
      auto cend = children.constEnd();

      for (auto it = children.constBegin(); it != cend; ++it) {
         // Managed child widgets unless we had a container extension
         if ((*it)->isWidgetType()) {
            if (!containerExtension) {
               QWidget *widget = dynamic_cast<QWidget *>(*it);
               if (fwi->isManaged(widget)) {
                  createModelRecursion(fwi, object, widget, model, ctx);
               }
            }

         } else {
            if (ctx.mdb->item(*it)) {
               if (QButtonGroup *bg = dynamic_cast<QButtonGroup *>(*it)) {
                  buttonGroups.push_back(bg);
               }
            } // Has MetaDataBase entry
         }
      }

      // Add button groups
      if (!buttonGroups.empty()) {
         const ButtonGroupList::const_iterator bgcend = buttonGroups.constEnd();
         for (ButtonGroupList::const_iterator bgit = buttonGroups.constBegin(); bgit != bgcend; ++bgit) {
            createModelRecursion(fwi, object, *bgit, model, ctx);
         }
      }
   } // has children

   if (object->isWidgetType()) {
      // Add actions
      const ActionList actions = static_cast<QWidget *>(object)->actions();
      if (! actions.empty()) {
         auto cend = actions.constEnd();

         for (auto it = actions.constBegin(); it != cend; ++it)
            if (ctx.mdb->item(*it)) {
               QAction *action = *it;
               QObject *obj = action;

               if (action->menu()) {
                  obj = action->menu();
               }

               createModelRecursion(fwi, object, obj, model, ctx);
            }
      }
   }
}

ObjectInspectorModel::ObjectInspectorModel(QObject *parent)
   : QStandardItemModel(0, NumColumns, parent)
{
   QStringList headers;
   headers += QCoreApplication::translate("ObjectInspectorModel", "Object");
   headers += QCoreApplication::translate("ObjectInspectorModel", "Class");
   Q_ASSERT(headers.size() == NumColumns);

   setColumnCount(NumColumns);
   setHorizontalHeaderLabels(headers);

   // Icons
   m_icons.layoutIcons[LayoutInfo::NoLayout]  = createIconSet(QString("editbreaklayout.png"));
   m_icons.layoutIcons[LayoutInfo::HSplitter] = createIconSet(QString("edithlayoutsplit.png"));
   m_icons.layoutIcons[LayoutInfo::VSplitter] = createIconSet(QString("editvlayoutsplit.png"));
   m_icons.layoutIcons[LayoutInfo::HBox]      = createIconSet(QString("edithlayout.png"));
   m_icons.layoutIcons[LayoutInfo::VBox]      = createIconSet(QString("editvlayout.png"));
   m_icons.layoutIcons[LayoutInfo::Grid]      = createIconSet(QString("editgrid.png"));
   m_icons.layoutIcons[LayoutInfo::Form]      = createIconSet(QString("editform.png"));
}

void ObjectInspectorModel::clearItems()
{
   beginResetModel();
   m_objectIndexMultiMap.clear();
   m_model.clear();
   endResetModel();                // force editors to be closed in views
   removeRow(0);
}

ObjectInspectorModel::UpdateResult ObjectInspectorModel::update(QDesignerFormWindowInterface *fw)
{
   QWidget *mainContainer = fw ? fw->mainContainer() : static_cast<QWidget *>(nullptr);

   if (! mainContainer) {
      clearItems();
      m_formWindow = nullptr;
      return NoForm;
   }

   m_formWindow = fw;

   // Build new model and compare to previous one. If the structure is
   // identical, just update, else rebuild
   QList<ObjectData> newModel;

   static const QString separator = QCoreApplication::translate("ObjectInspectorModel", "separator");
   const ModelRecursionContext ctx(fw->core(),  separator);
   createModelRecursion(fw, nullptr, mainContainer, newModel, ctx);

   if (newModel == m_model) {
      updateItemContents(m_model, newModel);
      return Updated;
   }

   rebuild(newModel);
   m_model = newModel;

   return Rebuilt;
}

QObject *ObjectInspectorModel::objectAt(const QModelIndex &index) const
{
   if (index.isValid()) {
      if (const QStandardItem *item = itemFromIndex(index)) {
         return objectOfItem(item);
      }
   }

   return nullptr;
}

// Missing API: get a row
ObjectInspectorModel::StandardItemList ObjectInspectorModel::rowAt(QModelIndex index) const
{
   StandardItemList rc;

   while (true) {
      rc += itemFromIndex(index);
      const int nextColumn = index.column() + 1;

      if (nextColumn >=  NumColumns) {
         break;
      }

      index = index.sibling(index.row(), nextColumn);
   }

   return rc;
}

// Rebuild the tree in case the model has completely changed.
void ObjectInspectorModel::rebuild(const QList<ObjectData> &newModel)
{
   clearItems();
   if (newModel.empty()) {
      return;
   }

   auto iter_end = newModel.constEnd();
   auto iter     = newModel.constBegin();

   // Set up root element
   StandardItemList rootRow = createModelRow(iter->object());
   iter->setItems(rootRow, m_icons);

   appendRow(rootRow);
   m_objectIndexMultiMap.insert(iter->object(), indexFromItem(rootRow.front()));

   ++iter;

   while (iter != iter_end) {
      // Add to parent item, found via map
      const QModelIndex parentIndex = m_objectIndexMultiMap.value(iter->parent(), QModelIndex());
      Q_ASSERT(parentIndex.isValid());

      QStandardItem *parentItem = itemFromIndex(parentIndex);
      StandardItemList row = createModelRow(iter->object());

      iter->setItems(row, m_icons);
      parentItem->appendRow(row);
      m_objectIndexMultiMap.insert(iter->object(), indexFromItem(row.front()));

      ++iter;
   }
}

// Update item data in case the model has the same structure
void ObjectInspectorModel::updateItemContents(QList<ObjectData> &oldModel, const QList<ObjectData> &newModel)
{
   // Change text and icon. Keep a set of changed object
   // as for example actions might occur several times in the tree.
   typedef QSet<QObject *> QObjectSet;

   QObjectSet changedObjects;

   const int size = newModel.size();
   Q_ASSERT(oldModel.size() ==  size);

   for (int i = 0; i < size; ++i) {
      const ObjectData &newEntry = newModel[i];
      ObjectData &entry =  oldModel[i];

      // Has some data changed?
      if (const unsigned changedMask = entry.compare(newEntry)) {
         entry = newEntry;
         QObject *o = entry.object();

         if (!changedObjects.contains(o)) {
            changedObjects.insert(o);
            const QModelIndexList indexes =  m_objectIndexMultiMap.values(o);

            for (const QModelIndex &index : indexes) {
               entry.setItemsDisplayData(rowAt(index), m_icons, changedMask);
            }
         }
      }
   }
}

QVariant ObjectInspectorModel::data(const QModelIndex &index, int role) const
{
   const QVariant rc = QStandardItemModel::data(index, role);

   // Return <noname> if the string is empty for the display role
   // only (else, editing starts with <noname>).
   if (role == Qt::DisplayRole && rc.type() == QVariant::String) {
      const QString s = rc.toString();
      if (s.isEmpty()) {
         static const QString noName = QCoreApplication::translate("ObjectInspectorModel", "<noname>");
         return  QVariant(noName);
      }
   }
   return rc;
}

bool ObjectInspectorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   if (role != Qt::EditRole || !m_formWindow) {
      return false;
   }

   QObject *object = objectAt(index);
   if (!object) {
      return false;
   }

   // Is this a layout widget?
   const QString nameProperty = isQLayoutWidget(object) ? QString("layoutName") : QString("objectName");
   m_formWindow->commandHistory()->push(createTextPropertyCommand(nameProperty, value.toString(), object, m_formWindow));
   return true;
}
}

