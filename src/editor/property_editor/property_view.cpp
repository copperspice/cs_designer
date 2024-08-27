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

#include <property_view.h>

#include <QIcon>
#include <QMap>
#include <QSet>

class QtPropertyPrivate
{
 public:
   QtPropertyPrivate(QtAbstractPropertyManager *manager)
      : m_enabled(true), m_modified(false), m_manager(manager)
   {
   }

   QtProperty *q_ptr;

   QSet<QtProperty *>  m_parentItems;
   QList<QtProperty *> m_subItems;

   QString m_valueToolTip;
   QString m_descriptionToolTip;
   QString m_statusTip;
   QString m_whatsThis;
   QString m_name;

   bool m_enabled;
   bool m_modified;

   QtAbstractPropertyManager *const m_manager;
};

class QtAbstractPropertyManagerPrivate
{
   QtAbstractPropertyManager *q_ptr;
   Q_DECLARE_PUBLIC(QtAbstractPropertyManager)

 public:
   void propertyDestroyed(QtProperty *property);
   void propertyChanged(QtProperty *property) const;
   void propertyRemoved(QtProperty *property, QtProperty *parentProperty) const;
   void propertyInserted(QtProperty *property, QtProperty *parentProperty, QtProperty *afterProperty) const;

   QSet<QtProperty *> m_properties;
};

QtProperty::QtProperty(QtAbstractPropertyManager *manager)
   : d_ptr(new QtPropertyPrivate(manager))
{
   d_ptr->q_ptr = this;
}

QtProperty::~QtProperty()
{
   QSetIterator<QtProperty *> itParent(d_ptr->m_parentItems);

   while (itParent.hasNext()) {
      QtProperty *property = itParent.next();
      property->d_ptr->m_manager->d_ptr->propertyRemoved(this, property);
   }

   d_ptr->m_manager->d_ptr->propertyDestroyed(this);

   QListIterator<QtProperty *> itChild(d_ptr->m_subItems);
   while (itChild.hasNext()) {
      QtProperty *property = itChild.next();
      property->d_ptr->m_parentItems.remove(this);
   }

   itParent.toFront();
   while (itParent.hasNext()) {
      QtProperty *property = itParent.next();
      property->d_ptr->m_subItems.removeAll(this);
   }
}

QList<QtProperty *> QtProperty::subProperties() const
{
   return d_ptr->m_subItems;
}

QtAbstractPropertyManager *QtProperty::propertyManager() const
{
   return d_ptr->m_manager;
}

QString QtProperty::valueToolTip() const
{
   return d_ptr->m_valueToolTip;
}

QString QtProperty::descriptionToolTip() const
{
   return d_ptr->m_descriptionToolTip;
}

QString QtProperty::statusTip() const
{
   return d_ptr->m_statusTip;
}

QString QtProperty::whatsThis() const
{
   return d_ptr->m_whatsThis;
}

QString QtProperty::propertyName() const
{
   return d_ptr->m_name;
}

bool QtProperty::isEnabled() const
{
   return d_ptr->m_enabled;
}

bool QtProperty::isModified() const
{
   return d_ptr->m_modified;
}

bool QtProperty::hasValue() const
{
   return d_ptr->m_manager->hasValue(this);
}

QIcon QtProperty::valueIcon() const
{
   return d_ptr->m_manager->valueIcon(this);
}

QString QtProperty::valueText() const
{
   return d_ptr->m_manager->valueText(this);
}

void QtProperty::setValueToolTip(const QString &text)
{
   if (d_ptr->m_valueToolTip == text) {
      return;
   }

   d_ptr->m_valueToolTip = text;
   propertyChanged();
}

void QtProperty::setDescriptionToolTip(const QString &text)
{
   if (d_ptr->m_descriptionToolTip == text) {
      return;
   }

   d_ptr->m_descriptionToolTip = text;
   propertyChanged();
}

void QtProperty::setStatusTip(const QString &text)
{
   if (d_ptr->m_statusTip == text) {
      return;
   }

   d_ptr->m_statusTip = text;
   propertyChanged();
}

void QtProperty::setWhatsThis(const QString &text)
{
   if (d_ptr->m_whatsThis == text) {
      return;
   }

   d_ptr->m_whatsThis = text;
   propertyChanged();
}

void QtProperty::setPropertyName(const QString &text)
{
   if (d_ptr->m_name == text) {
      return;
   }

   d_ptr->m_name = text;
   propertyChanged();
}

void QtProperty::setEnabled(bool enable)
{
   if (d_ptr->m_enabled == enable) {
      return;
   }

   d_ptr->m_enabled = enable;
   propertyChanged();
}

void QtProperty::setModified(bool modified)
{
   if (d_ptr->m_modified == modified) {
      return;
   }

   d_ptr->m_modified = modified;
   propertyChanged();
}

void QtProperty::addSubProperty(QtProperty *property)
{
   insertSubProperty(property, nullptr);
}

void QtProperty::insertSubProperty(QtProperty *property, QtProperty *afterProperty)
{
   if (property == nullptr || property == this) {
      return;
   }

   // traverse all children of item, if this item is a child of item then do not add
   QList<QtProperty *> pendingList = property->subProperties();
   QMap<QtProperty *, bool> visited;

   while (! pendingList.isEmpty()) {
      QtProperty *item = pendingList.first();

      if (item == this) {
         return;
      }

      pendingList.removeFirst();
      if (visited.contains(item)) {
         continue;
      }

      visited[item] = true;
      pendingList += item->subProperties();
   }

   pendingList = subProperties();
   int newPos  = 0;

   QtProperty *properAfterProperty = nullptr;

   const static QSet<QString> unSorted = { "Layout", "font", "geometry", "sizePolicy",
         "baseSize", "gridSize", "iconSize", "mimimumSize", "maximumSize", "sizeIncrement",
         "alignment",
   };

   if (unSorted.contains(this->propertyName())) {
      // set order based on lambda

      const static QHash<QString, int> groupOrder = {
         {"X", 0}, {"Y", 1}, {"Width", 2}, {"Height", 3},

         {"Family", 0}, {"Point Size", 1}, {"Bold", 2}, {"Italic", 3}, {"Underline", 4},
         {"Strikeout", 5}, {"Kerning", 6}, {"Antialiasing", 7},

         {"Horizontal Policy", 0}, {"Vertical Policy", 1}, {"Horizontal Stretch", 2}, {"Vertical Stretch", 3},
         {"Horizontal", 0}, {"Vertical", 1},

         {"layoutName", 0}, {"layoutLeftMargin", 1}, {"layoutTopMargin", 2}, {"layoutRightMargin", 3},
         {"layoutBottomMargin", 4}, {"layoutHorizontalSpacing", 5}, {"layoutVerticalSpacing", 6},
         {"layoutRowStretch", 7}, {"layoutColumnStretch", 8},
         {"layoutRowMinimumHeight", 9}, {"layoutColumnMinimumWidth", 10}, {"layoutSizeConstraint", 11},
      };

      auto compare = [] (auto a, auto b)
            {
               auto index_a = groupOrder.value(a->propertyName(), -1);
               auto index_b = groupOrder.value(b->propertyName(), -1);

               if (index_a == -1 && index_b == -1) {
                  return a->propertyName() < b->propertyName();

               } else if (index_a == -1) {
                  return false;

               } else if (index_b == -1) {
                  return true;

               } else {
                  return index_a < index_b;
               }
            };

      for (int pos = 0; pos < pendingList.count(); ++pos) {
         QtProperty *item = pendingList.at(pos);

         if (item == property) {
            // item is already inserted do not add again
            return;
         }

         if (compare(item, property)) {
            newPos = pos + 1;
            properAfterProperty = afterProperty;

         } else {
            break;

         }
      }

   } else if (afterProperty == nullptr) {
      // alpha order

      for (int pos = 0; pos < pendingList.count(); ++pos) {

         QtProperty *item = pendingList.at(pos);

         if (item == property) {
            // item is already inserted do not add again
            return;
         }

         if (item->propertyName() < property->propertyName()) {
            newPos = pos + 1;
            properAfterProperty = afterProperty;

         } else {
            break;
         }
      }

   } else {
      // call will indicate sort order

      for (int pos = 0; pos < pendingList.count(); ++pos) {
         QtProperty *item = pendingList.at(pos);

         if (item == property) {
            // item is already inserted do not add again
            return;
         }

         if (item == afterProperty) {
            newPos = pos + 1;
            properAfterProperty = afterProperty;
         }
      }
   }

   d_ptr->m_subItems.insert(newPos, property);
   property->d_ptr->m_parentItems.insert(this);

   d_ptr->m_manager->d_ptr->propertyInserted(property, this, properAfterProperty);
}

void QtProperty::removeSubProperty(QtProperty *property)
{
   if (! property) {
      return;
   }

   d_ptr->m_manager->d_ptr->propertyRemoved(property, this);

   QList<QtProperty *> pendingList = subProperties();
   int pos = 0;

   while (pos < pendingList.count()) {
      if (pendingList.at(pos) == property) {
         d_ptr->m_subItems.removeAt(pos);
         property->d_ptr->m_parentItems.remove(this);

         return;
      }

      ++pos;
   }
}

void QtProperty::propertyChanged()
{
   d_ptr->m_manager->d_ptr->propertyChanged(this);
}

void QtAbstractPropertyManagerPrivate::propertyDestroyed(QtProperty *property)
{
   if (m_properties.contains(property)) {
      emit q_ptr->propertyDestroyed(property);
      q_ptr->uninitializeProperty(property);
      m_properties.remove(property);
   }
}

void QtAbstractPropertyManagerPrivate::propertyChanged(QtProperty *property) const
{
   emit q_ptr->propertyChanged(property);
}

void QtAbstractPropertyManagerPrivate::propertyRemoved(QtProperty *property,
   QtProperty *parentProperty) const
{
   emit q_ptr->propertyRemoved(property, parentProperty);
}

void QtAbstractPropertyManagerPrivate::propertyInserted(QtProperty *property,
   QtProperty *parentProperty, QtProperty *afterProperty) const
{
   emit q_ptr->propertyInserted(property, parentProperty, afterProperty);
}

QtAbstractPropertyManager::QtAbstractPropertyManager(QObject *parent)
   : QObject(parent), d_ptr(new QtAbstractPropertyManagerPrivate)
{
   d_ptr->q_ptr = this;
}

QtAbstractPropertyManager::~QtAbstractPropertyManager()
{
   clear();
}

void QtAbstractPropertyManager::clear() const
{
   while (!properties().isEmpty()) {
      QSetIterator<QtProperty *> itProperty(properties());
      QtProperty *prop = itProperty.next();
      delete prop;
   }
}

QSet<QtProperty *> QtAbstractPropertyManager::properties() const
{
   return d_ptr->m_properties;
}

bool QtAbstractPropertyManager::hasValue(const QtProperty *property) const
{
   (void) property;
   return true;
}

QIcon QtAbstractPropertyManager::valueIcon(const QtProperty *property) const
{
   (void) property;
   return QIcon();
}

QString QtAbstractPropertyManager::valueText(const QtProperty *property) const
{
   (void) property;
   return QString();
}

QtProperty *QtAbstractPropertyManager::addProperty(const QString &name)
{
   QtProperty *property = createProperty();

   if (property != nullptr) {
      property->setPropertyName(name);
      d_ptr->m_properties.insert(property);
      initializeProperty(property);
   }

   return property;
}

QtProperty *QtAbstractPropertyManager::createProperty()
{
   return new QtProperty(this);
}

void QtAbstractPropertyManager::uninitializeProperty(QtProperty *property)
{
   (void) property;
}

class QtBrowserItemPrivate
{
 public:
   QtBrowserItemPrivate(QtAbstractPropertyBrowser *browser, QtProperty *property, QtBrowserItem *parent)
      : m_browser(browser), m_property(property), m_parent(parent), q_ptr(nullptr)
   {
   }

   void addChild(QtBrowserItem *index, QtBrowserItem *after);
   void removeChild(QtBrowserItem *index);

   QtAbstractPropertyBrowser *const m_browser;
   QtProperty *m_property;
   QtBrowserItem *m_parent;

   QtBrowserItem *q_ptr;

   QList<QtBrowserItem *> m_children;
};

void QtBrowserItemPrivate::addChild(QtBrowserItem *index, QtBrowserItem *after)
{
   if (m_children.contains(index)) {
      return;
   }

   // we insert after returned idx, if it was -1 then we set idx to 0;
   int idx = m_children.indexOf(after) + 1;

   m_children.insert(idx, index);
}

void QtBrowserItemPrivate::removeChild(QtBrowserItem *index)
{
   m_children.removeAll(index);
}

QtProperty *QtBrowserItem::property() const
{
   return d_ptr->m_property;
}

QtBrowserItem *QtBrowserItem::parent() const
{
   return d_ptr->m_parent;
}

QList<QtBrowserItem *> QtBrowserItem::children() const
{
   return d_ptr->m_children;
}


QtAbstractPropertyBrowser *QtBrowserItem::browser() const
{
   return d_ptr->m_browser;
}

QtBrowserItem::QtBrowserItem(QtAbstractPropertyBrowser *browser, QtProperty *property, QtBrowserItem *parent)
   : d_ptr(new QtBrowserItemPrivate(browser, property, parent))
{
   d_ptr->q_ptr = this;
}

QtBrowserItem::~QtBrowserItem()
{
}


typedef QMap<QtAbstractPropertyBrowser *, QMap<QtAbstractPropertyManager *,
         QtAbstractEditorFactoryBase *>> Map1;

typedef QMap<QtAbstractPropertyManager *, QMap<QtAbstractEditorFactoryBase *,
         QList<QtAbstractPropertyBrowser *>>> Map2;

Q_GLOBAL_STATIC(Map1, m_viewToManagerToFactory)
Q_GLOBAL_STATIC(Map2, m_managerToFactoryToViews)

class QtAbstractPropertyBrowserPrivate
{
   QtAbstractPropertyBrowser *q_ptr;
   Q_DECLARE_PUBLIC(QtAbstractPropertyBrowser)

 public:
   QtAbstractPropertyBrowserPrivate();

   void insertSubTree(QtProperty *property, QtProperty *parentProperty);
   void removeSubTree(QtProperty *property, QtProperty *parentProperty);
   void createBrowserIndexes(QtProperty *property, QtProperty *parentProperty, QtProperty *afterProperty);

   void removeBrowserIndexes(QtProperty *property, QtProperty *parentProperty);
   QtBrowserItem *createBrowserIndex(QtProperty *property, QtBrowserItem *parentIndex, QtBrowserItem *afterIndex);
   void removeBrowserIndex(QtBrowserItem *index);
   void clearIndex(QtBrowserItem *index);

   void slotPropertyInserted(QtProperty *property, QtProperty *parentProperty, QtProperty *afterProperty);

   void slotPropertyRemoved(QtProperty *property, QtProperty *parentProperty);
   void slotPropertyDestroyed(QtProperty *property);
   void slotPropertyDataChanged(QtProperty *property);

   QList<QtProperty *> m_subItems;
   QMap<QtAbstractPropertyManager *, QList<QtProperty *>> m_managerToProperties;
   QMap<QtProperty *, QList<QtProperty *>> m_propertyToParents;

   QMap<QtProperty *, QtBrowserItem *> m_topLevelPropertyToIndex;
   QList<QtBrowserItem *> m_topLevelIndexes;
   QMap<QtProperty *, QList<QtBrowserItem *>> m_propertyToIndexes;

   QtBrowserItem *m_currentItem;
};

QtAbstractPropertyBrowserPrivate::QtAbstractPropertyBrowserPrivate()
   : m_currentItem(nullptr)
{
}

void QtAbstractPropertyBrowserPrivate::insertSubTree(QtProperty *property, QtProperty *parentProperty)
{
   if (m_propertyToParents.contains(property)) {
      // property was already inserted, so its manager is connected
      // and all its children are inserted and theirs managers are connected
      // we just register new parent (parent has to be new).
      m_propertyToParents[property].append(parentProperty);

      // do not need to update m_managerToProperties map since
      // m_managerToProperties[manager] already contains property.
      return;
   }

   QtAbstractPropertyManager *manager = property->propertyManager();

   if (m_managerToProperties[manager].isEmpty()) {
      // connect manager's signals

      q_ptr->connect(manager, &QtAbstractPropertyManager::propertyInserted,
            q_ptr, &QtAbstractPropertyBrowser::slotPropertyInserted);

      q_ptr->connect(manager, &QtAbstractPropertyManager::propertyRemoved,
            q_ptr, &QtAbstractPropertyBrowser::slotPropertyRemoved);

      q_ptr->connect(manager, &QtAbstractPropertyManager::propertyDestroyed,
            q_ptr, &QtAbstractPropertyBrowser::slotPropertyDestroyed);

      q_ptr->connect(manager, &QtAbstractPropertyManager::propertyChanged,
            q_ptr, &QtAbstractPropertyBrowser::slotPropertyDataChanged);
   }

   m_managerToProperties[manager].append(property);
   m_propertyToParents[property].append(parentProperty);

   QList<QtProperty *> subList = property->subProperties();

   for (auto subProperty : subList) {
      insertSubTree(subProperty, property);
   }
}

void QtAbstractPropertyBrowserPrivate::removeSubTree(QtProperty *property, QtProperty *parentProperty)
{
   if (! m_propertyToParents.contains(property)) {
      // ASSERT
      return;
   }

   m_propertyToParents[property].removeAll(parentProperty);
   if (! m_propertyToParents[property].isEmpty()) {
      return;
   }

   m_propertyToParents.remove(property);
   QtAbstractPropertyManager *manager = property->propertyManager();
   m_managerToProperties[manager].removeAll(property);

   if (m_managerToProperties[manager].isEmpty()) {
      // disconnect manager's signals
      q_ptr->disconnect(manager, SIGNAL(propertyInserted(QtProperty *,
            QtProperty *, QtProperty *)), q_ptr, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));

      q_ptr->disconnect(manager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
            q_ptr, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));

      q_ptr->disconnect(manager, SIGNAL(propertyDestroyed(QtProperty *)),
            q_ptr, SLOT(slotPropertyDestroyed(QtProperty *)));

      q_ptr->disconnect(manager, SIGNAL(propertyChanged(QtProperty *)),
            q_ptr, SLOT(slotPropertyDataChanged(QtProperty *)));

      m_managerToProperties.remove(manager);
   }

   QList<QtProperty *> subList = property->subProperties();
   QListIterator<QtProperty *> itSub(subList);

   while (itSub.hasNext()) {
      QtProperty *subProperty = itSub.next();
      removeSubTree(subProperty, property);
   }
}

void QtAbstractPropertyBrowserPrivate::createBrowserIndexes(QtProperty *property, QtProperty *parentProperty,
   QtProperty *afterProperty)
{
   QMap<QtBrowserItem *, QtBrowserItem *> parentToAfter;

   if (afterProperty) {
      auto it = m_propertyToIndexes.find(afterProperty);

      if (it == m_propertyToIndexes.constEnd()) {
         return;
      }

      QList<QtBrowserItem *> indexes = it.value();
      QListIterator<QtBrowserItem *> itIndex(indexes);

      while (itIndex.hasNext()) {
         QtBrowserItem *idx = itIndex.next();
         QtBrowserItem *parentIdx = idx->parent();

         if ((parentProperty && parentIdx && parentIdx->property() == parentProperty) || (!parentProperty && !parentIdx)) {
            parentToAfter[idx->parent()] = idx;
         }
      }

   } else if (parentProperty) {
      auto it = m_propertyToIndexes.find(parentProperty);

      if (it == m_propertyToIndexes.constEnd()) {
         return;
      }

      QList<QtBrowserItem *> indexes = it.value();
      QListIterator<QtBrowserItem *> itIndex(indexes);

      while (itIndex.hasNext()) {
         QtBrowserItem *idx = itIndex.next();
         parentToAfter[idx] = nullptr;
      }

   } else {
      parentToAfter[nullptr] = nullptr;
   }

   auto pcend = parentToAfter.constEnd();

   for (auto it = parentToAfter.constBegin(); it != pcend; ++it) {
      createBrowserIndex(property, it.key(), it.value());
   }
}

QtBrowserItem *QtAbstractPropertyBrowserPrivate::createBrowserIndex(QtProperty *property,
      QtBrowserItem *parentIndex, QtBrowserItem *afterIndex)
{
   QtBrowserItem *newIndex = new QtBrowserItem(q_ptr, property, parentIndex);

   if (parentIndex) {
      parentIndex->d_ptr->addChild(newIndex, afterIndex);
   } else {
      m_topLevelPropertyToIndex[property] = newIndex;
      m_topLevelIndexes.insert(m_topLevelIndexes.indexOf(afterIndex) + 1, newIndex);
   }
   m_propertyToIndexes[property].append(newIndex);

   q_ptr->itemInserted(newIndex, afterIndex);

   QList<QtProperty *> subItems = property->subProperties();
   QListIterator<QtProperty *> itChild(subItems);

   QtBrowserItem *afterChild = nullptr;

   while (itChild.hasNext()) {
      QtProperty *child = itChild.next();
      afterChild = createBrowserIndex(child, newIndex, afterChild);
   }

   return newIndex;
}

void QtAbstractPropertyBrowserPrivate::removeBrowserIndexes(QtProperty *property, QtProperty *parentProperty)
{
   QList<QtBrowserItem *> toRemove;
   auto it = m_propertyToIndexes.find(property);

   if (it == m_propertyToIndexes.constEnd()) {
      return;
   }

   QList<QtBrowserItem *> indexes = it.value();
   QListIterator<QtBrowserItem *> itIndex(indexes);

   while (itIndex.hasNext()) {
      QtBrowserItem *idx = itIndex.next();
      QtBrowserItem *parentIdx = idx->parent();

      if ((parentProperty && parentIdx && parentIdx->property() == parentProperty) || (!parentProperty && !parentIdx)) {
         toRemove.append(idx);
      }
   }

   QListIterator<QtBrowserItem *> itRemove(toRemove);

   while (itRemove.hasNext()) {
      QtBrowserItem *index = itRemove.next();
      removeBrowserIndex(index);
   }
}

void QtAbstractPropertyBrowserPrivate::removeBrowserIndex(QtBrowserItem *index)
{
   QList<QtBrowserItem *> children = index->children();

   for (int i = children.count(); i > 0; i--) {
      removeBrowserIndex(children.at(i - 1));
   }

   q_ptr->itemRemoved(index);

   if (index->parent()) {
      index->parent()->d_ptr->removeChild(index);
   } else {
      m_topLevelPropertyToIndex.remove(index->property());
      m_topLevelIndexes.removeAll(index);
   }

   QtProperty *property = index->property();

   m_propertyToIndexes[property].removeAll(index);
   if (m_propertyToIndexes[property].isEmpty()) {
      m_propertyToIndexes.remove(property);
   }

   delete index;
}

void QtAbstractPropertyBrowserPrivate::clearIndex(QtBrowserItem *index)
{
   QList<QtBrowserItem *> children = index->children();
   QListIterator<QtBrowserItem *> itChild(children);

   while (itChild.hasNext()) {
      clearIndex(itChild.next());
   }

   delete index;
}

void QtAbstractPropertyBrowserPrivate::slotPropertyInserted(QtProperty *property,
      QtProperty *parentProperty, QtProperty *afterProperty)
{
   if (! m_propertyToParents.contains(parentProperty)) {
      return;
   }

   createBrowserIndexes(property, parentProperty, afterProperty);
   insertSubTree(property, parentProperty);
}

void QtAbstractPropertyBrowserPrivate::slotPropertyRemoved(QtProperty *property,  QtProperty *parentProperty)
{
   if (! m_propertyToParents.contains(parentProperty)) {
      return;
   }

   // this line should be probably moved down after propertyRemoved call
   removeSubTree(property, parentProperty);

   removeBrowserIndexes(property, parentProperty);
}

void QtAbstractPropertyBrowserPrivate::slotPropertyDestroyed(QtProperty *property)
{
   if (! m_subItems.contains(property)) {
      return;
   }

   q_ptr->removeProperty(property);
}

void QtAbstractPropertyBrowserPrivate::slotPropertyDataChanged(QtProperty *property)
{
   if (! m_propertyToParents.contains(property)) {
      return;
   }

   auto it = m_propertyToIndexes.find(property);
   if (it == m_propertyToIndexes.constEnd()) {
      return;
   }

   QList<QtBrowserItem *> indexes = it.value();
   QListIterator<QtBrowserItem *> itIndex(indexes);

   while (itIndex.hasNext()) {
      QtBrowserItem *idx = itIndex.next();
      q_ptr->itemChanged(idx);
   }
}

QtAbstractPropertyBrowser::QtAbstractPropertyBrowser(QWidget *parent)
   : QWidget(parent), d_ptr(new QtAbstractPropertyBrowserPrivate)
{
   d_ptr->q_ptr = this;

}

QtAbstractPropertyBrowser::~QtAbstractPropertyBrowser()
{
   QList<QtBrowserItem *> indexes = topLevelItems();
   QListIterator<QtBrowserItem *> itItem(indexes);
   while (itItem.hasNext()) {
      d_ptr->clearIndex(itItem.next());
   }
}

QList<QtProperty *> QtAbstractPropertyBrowser::properties() const
{
   return d_ptr->m_subItems;
}

QList<QtBrowserItem *> QtAbstractPropertyBrowser::items(QtProperty *property) const
{
   return d_ptr->m_propertyToIndexes.value(property);
}


QtBrowserItem *QtAbstractPropertyBrowser::topLevelItem(QtProperty *property) const
{
   return d_ptr->m_topLevelPropertyToIndex.value(property);
}

QList<QtBrowserItem *> QtAbstractPropertyBrowser::topLevelItems() const
{
   return d_ptr->m_topLevelIndexes;
}

void QtAbstractPropertyBrowser::clear()
{
   QList<QtProperty *> subList = properties();
   QListIterator<QtProperty *> itSub(subList);
   itSub.toBack();

   while (itSub.hasPrevious()) {
      QtProperty *property = itSub.previous();
      removeProperty(property);
   }
}

QtBrowserItem *QtAbstractPropertyBrowser::addProperty(QtProperty *property)
{
   QtProperty *afterProperty = nullptr;

   if (d_ptr->m_subItems.count() > 0) {
      afterProperty = d_ptr->m_subItems.last();
   }

   return insertProperty(property, afterProperty);
}

QtBrowserItem *QtAbstractPropertyBrowser::insertProperty(QtProperty *property, QtProperty *afterProperty)
{
   if (! property) {
      return nullptr;
   }

   // if item is already inserted in this item then cannot add.
   QList<QtProperty *> pendingList = properties();
   int pos   = 0;
   int newPos = 0;

   while (pos < pendingList.count()) {
      QtProperty *prop = pendingList.at(pos);

      if (prop == property) {
         return nullptr;
      }

      if (prop == afterProperty) {
         newPos = pos + 1;
      }

      ++pos;
   }

   d_ptr->createBrowserIndexes(property, nullptr, afterProperty);

   // traverse inserted subtree and connect to manager's signals
   d_ptr->insertSubTree(property, nullptr);

   d_ptr->m_subItems.insert(newPos, property);
   // propertyInserted(property, 0, properAfterProperty);

   return topLevelItem(property);
}

void QtAbstractPropertyBrowser::removeProperty(QtProperty *property)
{
   if (! property) {
      return;
   }

   QList<QtProperty *> pendingList = properties();
   int pos = 0;

   while (pos < pendingList.count()) {
      if (pendingList.at(pos) == property) {
         d_ptr->m_subItems.removeAt(pos);           // perhaps these two linesshould be
         d_ptr->removeSubTree(property, nullptr);  // moved down after propertyRemoved call

         // propertyRemoved(property, 0);

         d_ptr->removeBrowserIndexes(property, nullptr);

         // when item is deleted, item will call removeItem for top level items,
         // and itemRemoved for nested items.

         return;
      }

      ++pos;
   }
}

QWidget *QtAbstractPropertyBrowser::createEditor(QtProperty *property,
   QWidget *parent)
{
   QtAbstractEditorFactoryBase *factory = nullptr;
   QtAbstractPropertyManager *manager = property->propertyManager();

   if (m_viewToManagerToFactory()->contains(this) && (*m_viewToManagerToFactory())[this].contains(manager)) {
      factory = (*m_viewToManagerToFactory())[this][manager];
   }

   if (!factory) {
      return nullptr;
   }

   QWidget *w = factory->createEditor(property, parent);

   // Since some editors can be QComboBoxes and we changed their focus policy
   // to make them feel more native on Mac, need to relax the focus policy to something
   // more permissive to keep the combo box from losing focus, allowing it to stay alive,
   // when the user clicks on it to show the popup.

   if (w) {
      w->setFocusPolicy(Qt::WheelFocus);
   }
   return w;
}

bool QtAbstractPropertyBrowser::addFactory(QtAbstractPropertyManager *abstractManager,
   QtAbstractEditorFactoryBase *abstractFactory)
{
   bool connectNeeded = false;
   if (!m_managerToFactoryToViews()->contains(abstractManager) ||
      !(*m_managerToFactoryToViews())[abstractManager].contains(abstractFactory)) {
      connectNeeded = true;
   } else if ((*m_managerToFactoryToViews())[abstractManager][abstractFactory]
      .contains(this)) {
      return connectNeeded;
   }

   if (m_viewToManagerToFactory()->contains(this) &&
      (*m_viewToManagerToFactory())[this].contains(abstractManager)) {
      unsetFactoryForManager(abstractManager);
   }

   (*m_managerToFactoryToViews())[abstractManager][abstractFactory].append(this);
   (*m_viewToManagerToFactory())[this][abstractManager] = abstractFactory;

   return connectNeeded;
}

void QtAbstractPropertyBrowser::unsetFactoryForManager(QtAbstractPropertyManager *manager)
{
   if (!m_viewToManagerToFactory()->contains(this) ||
      !(*m_viewToManagerToFactory())[this].contains(manager)) {
      return;
   }

   QtAbstractEditorFactoryBase *abstractFactory =
      (*m_viewToManagerToFactory())[this][manager];
   (*m_viewToManagerToFactory())[this].remove(manager);
   if ((*m_viewToManagerToFactory())[this].isEmpty()) {
      (*m_viewToManagerToFactory()).remove(this);
   }

   (*m_managerToFactoryToViews())[manager][abstractFactory].removeAll(this);
   if ((*m_managerToFactoryToViews())[manager][abstractFactory].isEmpty()) {
      (*m_managerToFactoryToViews())[manager].remove(abstractFactory);
      abstractFactory->breakConnection(manager);
      if ((*m_managerToFactoryToViews())[manager].isEmpty()) {
         (*m_managerToFactoryToViews()).remove(manager);
      }
   }
}

QtBrowserItem *QtAbstractPropertyBrowser::currentItem() const
{
   return d_ptr->m_currentItem;
}

void QtAbstractPropertyBrowser::setCurrentItem(QtBrowserItem *item)
{
   QtBrowserItem *oldItem = d_ptr->m_currentItem;
   d_ptr->m_currentItem = item;

   if (oldItem != item) {
      emit  currentItemChanged(item);
   }
}

void QtAbstractPropertyBrowser::slotPropertyInserted(QtProperty *un_named_arg1, QtProperty *un_named_arg2, QtProperty *un_named_arg3)
{
   Q_D(QtAbstractPropertyBrowser);
   d->slotPropertyInserted(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QtAbstractPropertyBrowser::slotPropertyRemoved(QtProperty *un_named_arg1, QtProperty *un_named_arg2)
{
   Q_D(QtAbstractPropertyBrowser);
   d->slotPropertyRemoved(un_named_arg1, un_named_arg2);
}

void QtAbstractPropertyBrowser::slotPropertyDestroyed(QtProperty *un_named_arg1)
{
   Q_D(QtAbstractPropertyBrowser);
   d->slotPropertyDestroyed(un_named_arg1);
}

void QtAbstractPropertyBrowser::slotPropertyDataChanged(QtProperty *un_named_arg1)
{
   Q_D(QtAbstractPropertyBrowser);
   d->slotPropertyDataChanged(un_named_arg1);
}
