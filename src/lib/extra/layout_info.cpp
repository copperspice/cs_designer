/***********************************************************************
*
* Copyright (c) 2021-2023 Barbara Geller
* Copyright (c) 2021-2023 Ansel Sermersheim
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
#include <container.h>
#include <extension.h>
#include <layout_info.h>

#include <metadatabase_p.h>

#include <QDebug>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHash>
#include <QRect>
#include <QSplitter>

namespace qdesigner_internal {

LayoutInfo::Type LayoutInfo::layoutType(const QDesignerFormEditorInterface *core, const QLayout *layout)
{
   Q_UNUSED(core)

   if (!layout) {
      return NoLayout;
   } else if (dynamic_cast<const QHBoxLayout *>(layout)) {
      return HBox;
   } else if (dynamic_cast<const QVBoxLayout *>(layout)) {
      return VBox;
   } else if (dynamic_cast<const QGridLayout *>(layout)) {
      return Grid;
   } else if (dynamic_cast<const QFormLayout *>(layout)) {
      return Form;
   }
   return UnknownLayout;
}

static const QHash<QString, LayoutInfo::Type> &layoutNameTypeMap()
{
   static QHash<QString, LayoutInfo::Type> nameTypeMap;
   if (nameTypeMap.empty()) {
      nameTypeMap.insert(QString("QVBoxLayout"), LayoutInfo::VBox);
      nameTypeMap.insert(QString("QHBoxLayout"), LayoutInfo::HBox);
      nameTypeMap.insert(QString("QGridLayout"), LayoutInfo::Grid);
      nameTypeMap.insert(QString("QFormLayout"), LayoutInfo::Form);
   }
   return nameTypeMap;
}

LayoutInfo::Type LayoutInfo::layoutType(const QString &typeName)
{
   return layoutNameTypeMap().value(typeName, NoLayout);
}

QString LayoutInfo::layoutName(Type t)
{
   return layoutNameTypeMap().key(t);
}

/*!
  \overload
*/
LayoutInfo::Type LayoutInfo::layoutType(const QDesignerFormEditorInterface *core, const QWidget *w)
{
   if (const QSplitter *splitter = dynamic_cast<const QSplitter *>(w)) {
      return  splitter->orientation() == Qt::Horizontal ? HSplitter : VSplitter;
   }
   return layoutType(core, w->layout());
}

LayoutInfo::Type LayoutInfo::managedLayoutType(const QDesignerFormEditorInterface *core,
      const QWidget *w, QLayout **ptrToLayout)
{
   if (ptrToLayout) {
      *ptrToLayout = nullptr;
   }

   if (const QSplitter *splitter = dynamic_cast<const QSplitter *>(w)) {
      return  splitter->orientation() == Qt::Horizontal ? HSplitter : VSplitter;
   }

   QLayout *layout = managedLayout(core, w);
   if (!layout) {
      return NoLayout;
   }

   if (ptrToLayout) {
      *ptrToLayout = layout;
   }
   return layoutType(core, layout);
}

QWidget *LayoutInfo::layoutParent(const QDesignerFormEditorInterface *core, QLayout *layout)
{
   Q_UNUSED(core)

   QObject *o = layout;

   while (o) {
      if (QWidget *widget = dynamic_cast<QWidget *>(o)) {
         return widget;
      }

      o = o->parent();
   }

   return nullptr;
}

void LayoutInfo::deleteLayout(const QDesignerFormEditorInterface *core, QWidget *widget)
{
   if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension *>(core->extensionManager(), widget)) {
      widget = container->widget(container->currentIndex());
   }

   Q_ASSERT(widget != nullptr);

   QLayout *layout = managedLayout(core, widget);

   if (layout == nullptr || core->metaDataBase()->item(layout) != nullptr) {
      delete layout;

      widget->updateGeometry();
      return;
   }

   qDebug() << "trying to delete an unmanaged layout:" << "widget:" << widget << "layout:" << layout;
}

LayoutInfo::Type LayoutInfo::laidoutWidgetType(const QDesignerFormEditorInterface *core,
      QWidget *widget, bool *isManaged, QLayout **ptrToLayout)
{
   if (isManaged) {
      *isManaged = false;
   }

   if (ptrToLayout) {
      *ptrToLayout = nullptr;
   }

   QWidget *parent = widget->parentWidget();
   if (!parent) {
      return NoLayout;
   }

   // 1) Splitter
   if (QSplitter *splitter  = dynamic_cast<QSplitter *>(parent)) {
      if (isManaged) {
         *isManaged = core->metaDataBase()->item(splitter);
      }
      return  splitter->orientation() == Qt::Horizontal ? HSplitter : VSplitter;
   }

   // 2) Layout of parent
   QLayout *parentLayout = parent->layout();
   if (!parentLayout) {
      return NoLayout;
   }

   if (parentLayout->indexOf(widget) != -1) {
      if (isManaged) {
         *isManaged = core->metaDataBase()->item(parentLayout);
      }
      if (ptrToLayout) {
         *ptrToLayout = parentLayout;
      }
      return layoutType(core, parentLayout);
   }

   // 3) Some child layout (see below comment about Q3GroupBox)
   const QList<QLayout *> childLayouts = parentLayout->findChildren<QLayout *>();
   if (childLayouts.empty()) {
      return NoLayout;
   }
   const QList<QLayout *>::const_iterator lcend = childLayouts.constEnd();
   for (QList<QLayout *>::const_iterator it = childLayouts.constBegin(); it != lcend; ++it) {
      QLayout *layout = *it;
      if (layout->indexOf(widget) != -1) {
         if (isManaged) {
            *isManaged = core->metaDataBase()->item(layout);
         }
         if (ptrToLayout) {
            *ptrToLayout = layout;
         }
         return layoutType(core, layout);
      }
   }

   return NoLayout;
}

QLayout *LayoutInfo::internalLayout(const QWidget *widget)
{
   return widget->layout();
}

QLayout *LayoutInfo::managedLayout(const QDesignerFormEditorInterface *core, const QWidget *widget)
{
   if (widget == nullptr) {
      return nullptr;
   }

   QLayout *layout = widget->layout();
   if (! layout) {
      return nullptr;
   }

   return managedLayout(core, layout);
}

QLayout *LayoutInfo::managedLayout(const QDesignerFormEditorInterface *formEditor, QLayout *layout)
{
   QDesignerMetaDataBaseInterface *metaDataBase = formEditor->metaDataBase();

   if (metaDataBase == nullptr) {
      return layout;
   }

   // following code exists mainly for the Q3GroupBox class, for which
   // widget->layout() returns an internal VBoxLayout
   const QDesignerMetaDataBaseItemInterface *item = metaDataBase->item(layout);

   if (item == nullptr && layout != nullptr) {

      layout = layout->findChild<QLayout *>();
      item   = metaDataBase->item(layout);

      if (item == nullptr) {
         return nullptr;
      }
   }

   return layout;
}

// Is it a a dummy grid placeholder created by Designer?
bool LayoutInfo::isEmptyItem(QLayoutItem *item)
{
   if (item == nullptr) {
      qDebug() << "** WARNING Zero-item passed on to isEmptyItem(). This indicates a layout inconsistency.";
      return true;
   }

   return item->spacerItem() != nullptr;
}

void getFormLayoutItemPosition(const QFormLayout *formLayout, int index, int *rowPtr, int *columnPtr, int *rowspanPtr, int *colspanPtr)
{
   int row;

   QFormLayout::ItemRole role;
   formLayout->getItemPosition(index, &row, &role);
   const int columnspan = role == QFormLayout::SpanningRole ? 2 : 1;
   const int column = (columnspan > 1 || role == QFormLayout::LabelRole) ? 0 : 1;

   if (rowPtr) {
      *rowPtr = row;
   }
   if (columnPtr) {
      *columnPtr = column;
   }
   if (rowspanPtr) {
      *rowspanPtr = 1;
   }
   if (colspanPtr) {
      *colspanPtr = columnspan;
   }
}

static inline QFormLayout::ItemRole formLayoutRole(int column, int colspan)
{
   if (colspan > 1) {
      return QFormLayout::SpanningRole;
   }

   return column == 0 ? QFormLayout::LabelRole : QFormLayout::FieldRole;
}

void formLayoutAddWidget(QFormLayout *formLayout, QWidget *w, const QRect &r, bool insert)
{
   // Consistent API galore...
   if (insert) {
      const bool spanning = r.width() > 1;

      if (spanning) {
         formLayout->insertRow(r.y(), w);

      } else {
         QWidget *label = nullptr;
         QWidget *field = nullptr;

         if (r.x() == 0) {
            label = w;
         } else {
            field = w;
         }

         formLayout->insertRow(r.y(), label, field);
      }

   } else {
      formLayout->setWidget(r.y(), formLayoutRole(r.x(), r.width()), w);
   }
}

}   // end namespace qdesigner_internal


