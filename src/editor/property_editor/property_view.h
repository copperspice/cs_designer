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

#ifndef PROPERTY_VIEW_H
#define PROPERTY_VIEW_H

class QtAbstractPropertyBrowserPrivate;
class QtAbstractPropertyManager;
class QtPropertyPrivate;
class QtAbstractPropertyManagerPrivate;
class QtAbstractPropertyBrowser;
class QtBrowserItemPrivate;

#include <QWidget>
#include <QSet>

class QtProperty
{
 public:
   virtual ~QtProperty();

   QList<QtProperty *> subProperties() const;

   QtAbstractPropertyManager *propertyManager() const;

   QString toolTip() const {
      return valueToolTip();   // Compatibility
   }

   QString valueToolTip() const;
   QString descriptionToolTip() const;
   QString statusTip() const;
   QString whatsThis() const;
   QString propertyName() const;
   bool isEnabled() const;
   bool isModified() const;

   bool hasValue() const;
   QIcon valueIcon() const;
   QString valueText() const;

   void setToolTip(const QString &text) {
      setValueToolTip(text);   // Compatibility
   }

   void setValueToolTip(const QString &text);
   void setDescriptionToolTip(const QString &text);
   void setStatusTip(const QString &text);
   void setWhatsThis(const QString &text);
   void setPropertyName(const QString &text);
   void setEnabled(bool enable);
   void setModified(bool modified);

   void addSubProperty(QtProperty *property);
   void insertSubProperty(QtProperty *property, QtProperty *afterProperty);
   void removeSubProperty(QtProperty *property);

 protected:
   explicit QtProperty(QtAbstractPropertyManager *manager);
   void propertyChanged();

 private:
   friend class QtAbstractPropertyManager;
   QScopedPointer<QtPropertyPrivate> d_ptr;
};

class QtAbstractPropertyManager : public QObject
{
   CS_OBJECT(QtAbstractPropertyManager)

 public:
   explicit QtAbstractPropertyManager(QObject *parent = nullptr);
   ~QtAbstractPropertyManager();

   QSet<QtProperty *> properties() const;
   void clear() const;

   QtProperty *addProperty(const QString &name = QString());

   CS_SIGNAL_1(Public, void propertyInserted(QtProperty *property, QtProperty *parent, QtProperty *after))
   CS_SIGNAL_2(propertyInserted, property, parent, after)

   CS_SIGNAL_1(Public, void propertyChanged(QtProperty *property))
   CS_SIGNAL_2(propertyChanged, property)

   CS_SIGNAL_1(Public, void propertyRemoved(QtProperty *property, QtProperty *parent))
   CS_SIGNAL_2(propertyRemoved, property, parent)

   CS_SIGNAL_1(Public, void propertyDestroyed(QtProperty *property))
   CS_SIGNAL_2(propertyDestroyed, property)

 protected:
   virtual bool hasValue(const QtProperty *property) const;
   virtual QIcon valueIcon(const QtProperty *property) const;
   virtual QString valueText(const QtProperty *property) const;
   virtual void initializeProperty(QtProperty *property) = 0;
   virtual void uninitializeProperty(QtProperty *property);
   virtual QtProperty *createProperty();

 private:
   friend class QtProperty;
   QScopedPointer<QtAbstractPropertyManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtAbstractPropertyManager)
   Q_DISABLE_COPY(QtAbstractPropertyManager)
};

class QtAbstractEditorFactoryBase : public QObject
{
   CS_OBJECT(QtAbstractEditorFactoryBase)

 public:
   virtual QWidget *createEditor(QtProperty *property, QWidget *parent) = 0;

 protected:
   explicit QtAbstractEditorFactoryBase(QObject *parent = nullptr)
      : QObject(parent)
   {
   }

   virtual void breakConnection(QtAbstractPropertyManager *manager) = 0;

   CS_SLOT_1(Protected, virtual void managerDestroyed(QObject *manager) = 0)
   CS_SLOT_2(managerDestroyed)

   friend class QtAbstractPropertyBrowser;
};

template <class PropertyManager>
class QtAbstractEditorFactory : public QtAbstractEditorFactoryBase
{
 public:
   explicit QtAbstractEditorFactory(QObject *parent) : QtAbstractEditorFactoryBase(parent)
   {
   }

   QWidget *createEditor(QtProperty *property, QWidget *parent) {
      QSetIterator<PropertyManager *> it(m_managers);
      while (it.hasNext()) {
         PropertyManager *manager = it.next();
         if (manager == property->propertyManager()) {
            return createEditor(manager, property, parent);
         }
      }

      return nullptr;
   }

   void addPropertyManager(PropertyManager *manager) {
      if (m_managers.contains(manager)) {
         return;
      }
      m_managers.insert(manager);
      connectPropertyManager(manager);
      connect(manager, SIGNAL(destroyed(QObject *)),
         this, SLOT(managerDestroyed(QObject *)));
   }

   void removePropertyManager(PropertyManager *manager) {
      if (!m_managers.contains(manager)) {
         return;
      }
      disconnect(manager, SIGNAL(destroyed(QObject *)),
         this, SLOT(managerDestroyed(QObject *)));
      disconnectPropertyManager(manager);
      m_managers.remove(manager);
   }

   QSet<PropertyManager *> propertyManagers() const {
      return m_managers;
   }

   PropertyManager *propertyManager(QtProperty *property) const {
      QtAbstractPropertyManager *manager = property->propertyManager();
      QSetIterator<PropertyManager *> itManager(m_managers);
      while (itManager.hasNext()) {
         PropertyManager *m = itManager.next();
         if (m == manager) {
            return m;
         }
      }
      return nullptr;
   }

 protected:
   virtual void connectPropertyManager(PropertyManager *manager) = 0;
   virtual QWidget *createEditor(PropertyManager *manager, QtProperty *property, QWidget *parent) = 0;
   virtual void disconnectPropertyManager(PropertyManager *manager) = 0;

   void managerDestroyed(QObject *manager) {
      QSetIterator<PropertyManager *> it(m_managers);

      while (it.hasNext()) {
         PropertyManager *m = it.next();
         if (m == manager) {
            m_managers.remove(m);
            return;
         }
      }
   }

 private:
   void breakConnection(QtAbstractPropertyManager *manager) {
      QSetIterator<PropertyManager *> it(m_managers);
      while (it.hasNext()) {
         PropertyManager *m = it.next();
         if (m == manager) {
            removePropertyManager(m);
            return;
         }
      }
   }

 private:
   QSet<PropertyManager *> m_managers;
   friend class QtAbstractPropertyEditor;
};

class QtBrowserItem
{
 public:
   QtProperty *property() const;
   QtBrowserItem *parent() const;
   QList<QtBrowserItem *> children() const;
   QtAbstractPropertyBrowser *browser() const;

 private:
   explicit QtBrowserItem(QtAbstractPropertyBrowser *browser, QtProperty *property, QtBrowserItem *parent);
   ~QtBrowserItem();
   QScopedPointer<QtBrowserItemPrivate> d_ptr;

   friend class QtAbstractPropertyBrowserPrivate;
};

class QtAbstractPropertyBrowser : public QWidget
{
   CS_OBJECT(QtAbstractPropertyBrowser)

 public:
   explicit QtAbstractPropertyBrowser(QWidget *parent = nullptr);
   ~QtAbstractPropertyBrowser();

   QList<QtProperty *> properties() const;
   QList<QtBrowserItem *> items(QtProperty *property) const;
   QtBrowserItem *topLevelItem(QtProperty *property) const;
   QList<QtBrowserItem *> topLevelItems() const;
   void clear();

   template <class PropertyManager>
   void setFactoryForManager(PropertyManager *manager,
      QtAbstractEditorFactory<PropertyManager> *factory) {
      QtAbstractPropertyManager *abstractManager = manager;
      QtAbstractEditorFactoryBase *abstractFactory = factory;

      if (addFactory(abstractManager, abstractFactory)) {
         factory->addPropertyManager(manager);
      }
   }

   void unsetFactoryForManager(QtAbstractPropertyManager *manager);

   QtBrowserItem *currentItem() const;
   void setCurrentItem(QtBrowserItem *);

   CS_SIGNAL_1(Public, void currentItemChanged(QtBrowserItem *un_named_arg1))
   CS_SIGNAL_2(currentItemChanged, un_named_arg1)

   CS_SLOT_1(Public, QtBrowserItem *addProperty(QtProperty *property))
   CS_SLOT_2(addProperty)

   CS_SLOT_1(Public, QtBrowserItem *insertProperty(QtProperty *property, QtProperty *afterProperty))
   CS_SLOT_2(insertProperty)

   CS_SLOT_1(Public, void removeProperty(QtProperty *property))
   CS_SLOT_2(removeProperty)

 protected:
   virtual void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem) = 0;
   virtual void itemRemoved(QtBrowserItem *item) = 0;

   // can be tooltip, statustip, whatsthis, name, icon, text.
   virtual void itemChanged(QtBrowserItem *item) = 0;

   virtual QWidget *createEditor(QtProperty *property, QWidget *parent);

 private:
   bool addFactory(QtAbstractPropertyManager *abstractManager, QtAbstractEditorFactoryBase *abstractFactory);

   QScopedPointer<QtAbstractPropertyBrowserPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtAbstractPropertyBrowser)
   Q_DISABLE_COPY(QtAbstractPropertyBrowser)

   CS_SLOT_1(Private, void slotPropertyInserted(QtProperty *un_named_arg1, QtProperty *un_named_arg2, QtProperty *un_named_arg3))
   CS_SLOT_2(slotPropertyInserted)

   CS_SLOT_1(Private, void slotPropertyRemoved(QtProperty *un_named_arg1, QtProperty *un_named_arg2))
   CS_SLOT_2(slotPropertyRemoved)

   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDestroyed)

   CS_SLOT_1(Private, void slotPropertyDataChanged(QtProperty *un_named_arg1))
   CS_SLOT_2(slotPropertyDataChanged)
};

#endif



