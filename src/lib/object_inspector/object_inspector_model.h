/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#ifndef OBJECT_INSPECTOR_MODEL_H
#define OBJECT_INSPECTOR_MODEL_H

#include <layout_info.h>

#include <QIcon>
#include <QList>
#include <QModelIndex>
#include <QMultiMap>
#include <QPointer>
#include <QStandardItemModel>
#include <QString>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

// Data structure containing the fixed item type icons
struct ObjectInspectorIcons {
   QIcon layoutIcons[LayoutInfo::UnknownLayout + 1];
};

struct ModelRecursionContext;

// Data structure representing one item of the object inspector.
class ObjectData
{
 public:
   enum Type {
      Object,
      Action,
      SeparatorAction,
      ChildWidget,         // A child widget
      LayoutableContainer, // A container that can be laid out
      LayoutWidget,        // A QLayoutWidget
      ExtensionContainer   // QTabWidget and the like, container extension
   };

   typedef QList<QStandardItem *> StandardItemList;

   explicit ObjectData(QObject *parent, QObject *object, const ModelRecursionContext &ctx);
   ObjectData();

   inline Type     type()       const {
      return m_type;
   }
   inline QObject *object()     const {
      return m_object;
   }
   inline QObject *parent()     const {
      return m_parent;
   }
   inline QString  objectName() const {
      return m_objectName;
   }

   bool equals(const ObjectData &me) const;

   enum ChangedMask { ClassNameChanged = 1, ObjectNameChanged = 2,
      ClassIconChanged = 4, TypeChanged = 8,
      LayoutTypeChanged = 16
   };

   unsigned compare(const ObjectData &me) const;

   // Initially set up a row
   void setItems(const StandardItemList &row, const ObjectInspectorIcons &icons) const;
   // Update row data according to change mask
   void setItemsDisplayData(const StandardItemList &row, const ObjectInspectorIcons &icons, unsigned mask) const;

 private:
   void initObject(const ModelRecursionContext &ctx);
   void initWidget(QWidget *w, const ModelRecursionContext &ctx);

   QObject *m_parent;
   QObject *m_object;
   Type m_type;
   QString m_className;
   QString m_objectName;
   QIcon m_classIcon;
   LayoutInfo::Type m_managedLayoutType;
};

inline bool operator==(const ObjectData &e1, const ObjectData &e2)
{
   return e1.equals(e2);
}
inline bool operator!=(const ObjectData &e1, const ObjectData &e2)
{
   return !e1.equals(e2);
}

typedef QList<ObjectData> ObjectModel;

// QStandardItemModel for ObjectInspector. Uses ObjectData/ObjectModel
// internally for its updates.
class ObjectInspectorModel : public QStandardItemModel
{
 public:
   typedef QList<QStandardItem *> StandardItemList;
   enum { ObjectNameColumn, ClassNameColumn, NumColumns };

   explicit ObjectInspectorModel(QObject *parent);

   enum UpdateResult { NoForm, Rebuilt, Updated };
   UpdateResult update(QDesignerFormWindowInterface *fw);

   const QModelIndexList indexesOf(QObject *o) const {
      return m_objectIndexMultiMap.values(o);
   }
   QObject *objectAt(const QModelIndex &index) const;

   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

 private:
   typedef QMultiMap<QObject *, QModelIndex> ObjectIndexMultiMap;

   void rebuild(const ObjectModel &newModel);
   void updateItemContents(ObjectModel &oldModel, const ObjectModel &newModel);
   void clearItems();
   StandardItemList rowAt(QModelIndex index) const;

   ObjectInspectorIcons m_icons;
   ObjectIndexMultiMap m_objectIndexMultiMap;
   ObjectModel m_model;
   QPointer<QDesignerFormWindowInterface> m_formWindow;
};

}   // end namespace qdesigner_internal

#endif

