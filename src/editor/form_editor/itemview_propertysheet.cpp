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

#include <abstract_formeditor.h>
#include <itemview_propertysheet.h>

#include <QAbstractItemView>
#include <QHeaderView>

namespace qdesigner_internal {

struct Property {
   Property()
      : m_sheet(nullptr), m_id(-1) {
   }

   Property(QDesignerPropertySheetExtension *sheet, int id)
      : m_sheet(sheet), m_id(id) {
   }

   QDesignerPropertySheetExtension *m_sheet;
   int m_id;
};

using FakePropertyMap = QMap<int, Property>;

struct ItemViewPropertySheetPrivate {
   ItemViewPropertySheetPrivate(QDesignerFormEditorInterface *core, QHeaderView *horizontalHeader,
      QHeaderView *verticalHeader);

   inline QStringList realPropertyNames();
   inline QString fakePropertyName(const QString &prefix, const QString &realName);

   // Maps index of fake property to index of real property in respective sheet
   FakePropertyMap m_propertyIdMap;

   // Maps name of fake property to name of real property
   QHash<QString, QString> m_propertyNameMap;

   QHash<QHeaderView *, QDesignerPropertySheetExtension *> m_propertySheet;
   QStringList m_realPropertyNames;
};

// Name of the fake group
static const QString headerGroup = "Header";

// Name of the real properties
static const QString visibleProperty                 = "visible";
static const QString cascadingSectionResizesProperty = "cascadingSectionResizes";
static const QString defaultSectionSizeProperty      = "defaultSectionSize";
static const QString highlightSectionsProperty       = "highlightSections";
static const QString minimumSectionSizeProperty      = "minimumSectionSize";
static const QString showSortIndicatorProperty       = "showSortIndicator";
static const QString stretchLastSectionProperty      = "stretchLastSection";

} // namespace qdesigner_internal

using namespace qdesigner_internal;

ItemViewPropertySheetPrivate::ItemViewPropertySheetPrivate(QDesignerFormEditorInterface *core,
   QHeaderView *horizontalHeader, QHeaderView *verticalHeader)
{
   if (horizontalHeader)
      m_propertySheet.insert(horizontalHeader,
         qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), horizontalHeader));

   if (verticalHeader)
      m_propertySheet.insert(verticalHeader,
         qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), verticalHeader));
}

QStringList ItemViewPropertySheetPrivate::realPropertyNames()
{
   if (m_realPropertyNames.isEmpty()) {

      m_realPropertyNames = {
         visibleProperty,
         cascadingSectionResizesProperty,
         defaultSectionSizeProperty,
         highlightSectionsProperty,
         minimumSectionSizeProperty,
         showSortIndicatorProperty,
         stretchLastSectionProperty
      };
   }

   return m_realPropertyNames;
}

QString ItemViewPropertySheetPrivate::fakePropertyName(const QString &prefix, const QString &realName)
{
   // prefix = "header", realPropertyName = "isVisible" returns "headerIsVisible"
   QString fakeName = prefix + realName.at(0).toUpper() + realName.mid(1);
   m_propertyNameMap.insert(fakeName, realName);

   return fakeName;
}

ItemViewPropertySheet::ItemViewPropertySheet(QTreeView *treeViewObject, QObject *parent)
   : QDesignerPropertySheet(treeViewObject, parent),
     d(new ItemViewPropertySheetPrivate(core(), treeViewObject->header(), nullptr))
{
   initHeaderProperties(treeViewObject->header(), "header");
}

ItemViewPropertySheet::ItemViewPropertySheet(QTableView *tableViewObject, QObject *parent)
   : QDesignerPropertySheet(tableViewObject, parent),
     d(new ItemViewPropertySheetPrivate(core(), tableViewObject->horizontalHeader(), tableViewObject->verticalHeader()))
{
   initHeaderProperties(tableViewObject->horizontalHeader(), "horizontalHeader");
   initHeaderProperties(tableViewObject->verticalHeader(),   "verticalHeader");
}

ItemViewPropertySheet::~ItemViewPropertySheet()
{
   delete d;
}

void ItemViewPropertySheet::initHeaderProperties(QHeaderView *hv, const QString &prefix)
{
   QDesignerPropertySheetExtension *headerSheet = d->m_propertySheet.value(hv);
   Q_ASSERT(headerSheet);

   const QString headerGroupS = headerGroup;

   for (const QString &realPropertyName : d->realPropertyNames()) {
      const int headerIndex = headerSheet->indexOf(realPropertyName);
      Q_ASSERT(headerIndex != -1);

      const QVariant defaultValue = realPropertyName == visibleProperty ?
         QVariant(true) : headerSheet->property(headerIndex);

      const QString fakePropertyName = d->fakePropertyName(prefix, realPropertyName);
      const int fakeIndex = createFakeProperty(fakePropertyName, defaultValue);

      d->m_propertyIdMap.insert(fakeIndex, Property(headerSheet, headerIndex));
      setAttribute(fakeIndex, true);
      setPropertyGroup(fakeIndex, headerGroupS);
   }
}

QHash<QString, QString> ItemViewPropertySheet::propertyNameMap() const
{
   return d->m_propertyNameMap;
}

QVariant ItemViewPropertySheet::property(int index) const
{
   const FakePropertyMap::const_iterator it = d->m_propertyIdMap.constFind(index);

   if (it != d->m_propertyIdMap.constEnd()) {
      return it.value().m_sheet->property(it.value().m_id);
   }

   return QDesignerPropertySheet::property(index);
}

void ItemViewPropertySheet::setProperty(int index, const QVariant &value)
{
   const FakePropertyMap::iterator it = d->m_propertyIdMap.find(index);

   if (it != d->m_propertyIdMap.end()) {
      it.value().m_sheet->setProperty(it.value().m_id, value);
   } else {
      QDesignerPropertySheet::setProperty(index, value);
   }
}

void ItemViewPropertySheet::setChanged(int index, bool changed)
{
   const FakePropertyMap::iterator it = d->m_propertyIdMap.find(index);

   if (it != d->m_propertyIdMap.end()) {
      it.value().m_sheet->setChanged(it.value().m_id, changed);
   } else {
      QDesignerPropertySheet::setChanged(index, changed);
   }
}

bool ItemViewPropertySheet::isChanged(int index) const
{
   const FakePropertyMap::const_iterator it = d->m_propertyIdMap.constFind(index);
   if (it != d->m_propertyIdMap.constEnd()) {
      return it.value().m_sheet->isChanged(it.value().m_id);
   }
   return QDesignerPropertySheet::isChanged(index);
}

bool ItemViewPropertySheet::hasReset(int index) const
{
   const FakePropertyMap::const_iterator it = d->m_propertyIdMap.constFind(index);
   if (it != d->m_propertyIdMap.constEnd()) {
      return it.value().m_sheet->hasReset(it.value().m_id);
   }
   return QDesignerPropertySheet::hasReset(index);
}

bool ItemViewPropertySheet::reset(int index)
{
   const FakePropertyMap::iterator it = d->m_propertyIdMap.find(index);
   if (it != d->m_propertyIdMap.end()) {
      QDesignerPropertySheetExtension *headerSheet = it.value().m_sheet;
      const int headerIndex = it.value().m_id;
      const bool resetRC = headerSheet->reset(headerIndex);
      // Resetting for "visible" might fail and the stored default
      // of the Widget database is "false" due to the widget not being
      // visible at the time it was determined. Reset to "true" manually.
      if (!resetRC && headerSheet->propertyName(headerIndex) == visibleProperty) {
         headerSheet->setProperty(headerIndex, QVariant(true));
         headerSheet->setChanged(headerIndex, false);
         return true;
      }
      return resetRC;
   } else {
      return QDesignerPropertySheet::reset(index);
   }
}
