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

#include <abstract_widgetdatabase.h>

#include <qalgorithms.h>

QDesignerWidgetDataBaseInterface::QDesignerWidgetDataBaseInterface(QObject *parent)
   : QObject(parent)
{
}

QDesignerWidgetDataBaseInterface::~QDesignerWidgetDataBaseInterface()
{
   qDeleteAll(m_items);
}

int QDesignerWidgetDataBaseInterface::count() const
{
   return m_items.count();
}

QDesignerWidgetDataBaseItemInterface *QDesignerWidgetDataBaseInterface::item(int index) const
{
   return index != -1 ? m_items.at(index) : nullptr;
}

int QDesignerWidgetDataBaseInterface::indexOf(QDesignerWidgetDataBaseItemInterface *item) const
{
   return m_items.indexOf(item);
}

void QDesignerWidgetDataBaseInterface::insert(int index, QDesignerWidgetDataBaseItemInterface *item)
{
   m_items.insert(index, item);
}

void QDesignerWidgetDataBaseInterface::append(QDesignerWidgetDataBaseItemInterface *item)
{
   m_items.append(item);
}

QDesignerFormEditorInterface *QDesignerWidgetDataBaseInterface::core() const
{
   return nullptr;
}

int QDesignerWidgetDataBaseInterface::indexOfClassName(const QString &name, bool) const
{
   const int itemCount = count();

   for (int i = 0; i < itemCount; ++i) {
      const QDesignerWidgetDataBaseItemInterface *entry = item(i);
      if (entry->name() == name) {
         return i;
      }
   }

   return -1;
}

int QDesignerWidgetDataBaseInterface::indexOfObject(QObject *object, bool) const
{
   if (! object) {
      return -1;
   }

   const QString className = object->metaObject()->className();

   return indexOfClassName(className);
}

bool QDesignerWidgetDataBaseInterface::isContainer(QObject *object, bool resolveName) const
{
   if (const QDesignerWidgetDataBaseItemInterface *i = item(indexOfObject(object, resolveName))) {
      return i->isContainer();
   }

   return false;
}

bool QDesignerWidgetDataBaseInterface::isCustom(QObject *object, bool resolveName) const
{
   if (const QDesignerWidgetDataBaseItemInterface *i = item(indexOfObject(object, resolveName))) {
      return i->isCustom();
   }

   return false;
}

