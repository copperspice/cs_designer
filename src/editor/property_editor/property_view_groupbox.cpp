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

#include <property_view_groupbox.h>

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMap>
#include <QSet>
#include <QTimer>

class QtGroupBoxPropertyBrowserPrivate
{
   QtGroupBoxPropertyBrowser *q_ptr;

 public:
   void init(QWidget *parent);

   void propertyInserted(QtBrowserItem *index, QtBrowserItem *afterIndex);
   void propertyRemoved(QtBrowserItem *index);
   void propertyChanged(QtBrowserItem *index);

   QWidget *createEditor(QtProperty *property, QWidget *parent) const {
      return q_ptr->createEditor(property, parent);
   }

   void slotEditorDestroyed();
   void slotUpdate();

   struct WidgetItem {
      WidgetItem()
         : widget(nullptr), label(nullptr), widgetLabel(nullptr), groupBox(nullptr),
           layout(nullptr), line(nullptr), parent(nullptr)
      {
      }

      QWidget *widget; // can be null
      QLabel *label;
      QLabel *widgetLabel;
      QGroupBox *groupBox;
      QGridLayout *layout;
      QFrame *line;
      WidgetItem *parent;
      QList<WidgetItem *> children;
   };

 private:
   Q_DECLARE_PUBLIC(QtGroupBoxPropertyBrowser)

   void updateLater();
   void updateItem(WidgetItem *item);
   void insertRow(QGridLayout *layout, int row) const;
   void removeRow(QGridLayout *layout, int row) const;

   bool hasHeader(WidgetItem *item) const;

   QMap<QtBrowserItem *, WidgetItem *> m_indexToItem;
   QMap<WidgetItem *, QtBrowserItem *> m_itemToIndex;
   QMap<QWidget *, WidgetItem *> m_widgetToItem;
   QGridLayout *m_mainLayout;
   QList<WidgetItem *> m_children;
   QList<WidgetItem *> m_recreateQueue;
};

void QtGroupBoxPropertyBrowserPrivate::init(QWidget *parent)
{
   m_mainLayout = new QGridLayout();
   parent->setLayout(m_mainLayout);

   QLayoutItem *item = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding);
   m_mainLayout->addItem(item, 0, 0);
}

void QtGroupBoxPropertyBrowserPrivate::slotEditorDestroyed()
{
   QWidget *editor = dynamic_cast<QWidget *>(q_ptr->sender());

   if (! editor) {
      return;
   }

   if (! m_widgetToItem.contains(editor)) {
      return;
   }

   m_widgetToItem[editor]->widget = nullptr;
   m_widgetToItem.remove(editor);
}

void QtGroupBoxPropertyBrowserPrivate::slotUpdate()
{
   QListIterator<WidgetItem *> iterItem(m_recreateQueue);

   while (iterItem.hasNext()) {
      WidgetItem *item = iterItem.next();
      WidgetItem *parentItem = item->parent;

      QWidget *currentWidget  = nullptr;
      QGridLayout *gridLayout = nullptr;
      int oldRow = -1;

      if (parentItem == nullptr) {
         currentWidget = q_ptr;
         gridLayout    = m_mainLayout;

         oldRow = m_children.indexOf(item);

      } else {
         currentWidget = parentItem->groupBox;
         gridLayout    = parentItem->layout;

         oldRow = parentItem->children.indexOf(item);

         if (hasHeader(parentItem)) {
            oldRow += 2;
         }
      }

      if (item->widget) {
         item->widget->setParent(currentWidget);

      } else if (item->widgetLabel) {
         item->widgetLabel->setParent(currentWidget);

      } else {
         item->widgetLabel = new QLabel(currentWidget);
      }

      int span = 1;

      if (item->widget) {
         gridLayout->addWidget(item->widget, oldRow, 1, 1, 1);
      } else if (item->widgetLabel) {
         gridLayout->addWidget(item->widgetLabel, oldRow, 1, 1, 1);
      } else {
         span = 2;
      }

      item->label = new QLabel(currentWidget);
      item->label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
      gridLayout->addWidget(item->label, oldRow, 0, 1, span);

      updateItem(item);
   }

   m_recreateQueue.clear();
}

void QtGroupBoxPropertyBrowserPrivate::updateLater()
{
   QTimer::singleShot(0, q_ptr, SLOT(slotUpdate()));
}

void QtGroupBoxPropertyBrowserPrivate::propertyInserted(QtBrowserItem *index, QtBrowserItem *afterIndex)
{
   WidgetItem *afterItem  = m_indexToItem.value(afterIndex);
   WidgetItem *parentItem = m_indexToItem.value(index->parent());

   WidgetItem *newItem = new WidgetItem();
   newItem->parent = parentItem;

   QWidget *parentWidget  = nullptr;
   QGridLayout *topLayout = nullptr;
   int row = -1;

   if (! afterItem) {
      row = 0;

      if (parentItem != nullptr) {
         parentItem->children.insert(0, newItem);
      } else {
         m_children.insert(0, newItem);
      }

   } else {
      if (parentItem != nullptr) {
         row = parentItem->children.indexOf(afterItem) + 1;
         parentItem->children.insert(row, newItem);
      } else {
         row = m_children.indexOf(afterItem) + 1;
         m_children.insert(row, newItem);
      }
   }

   if (parentItem != nullptr && hasHeader(parentItem)) {
      row += 2;
   }

   if (parentItem == nullptr) {
      topLayout = m_mainLayout;
      parentWidget = q_ptr;

   } else {
      if (parentItem->groupBox == nullptr) {
         m_recreateQueue.removeAll(parentItem);
         WidgetItem *grandParentItem = parentItem->parent;

         QWidget *currentWidget  = nullptr;
         QGridLayout *gridLayout = nullptr;

         int oldRow = -1;

         if (grandParentItem == nullptr) {
            currentWidget = q_ptr;
            gridLayout    = m_mainLayout;

            oldRow = m_children.indexOf(parentItem);

         } else {
            currentWidget = grandParentItem->groupBox;
            gridLayout    = grandParentItem->layout;

            oldRow = grandParentItem->children.indexOf(parentItem);

            if (hasHeader(grandParentItem)) {
               oldRow += 2;
            }
         }

         parentItem->groupBox = new QGroupBox(currentWidget);
         parentItem->layout   = new QGridLayout();
         parentItem->groupBox->setLayout(parentItem->layout);

         if (parentItem->label) {
            gridLayout->removeWidget(parentItem->label);
            delete parentItem->label;
            parentItem->label = nullptr;
         }

         if (parentItem->widget) {
            gridLayout->removeWidget(parentItem->widget);
            parentItem->widget->setParent(parentItem->groupBox);
            parentItem->layout->addWidget(parentItem->widget, 0, 0, 1, 2);
            parentItem->line = new QFrame(parentItem->groupBox);

         } else if (parentItem->widgetLabel) {
            gridLayout->removeWidget(parentItem->widgetLabel);
            delete parentItem->widgetLabel;
            parentItem->widgetLabel = nullptr;
         }

         if (parentItem->line) {
            parentItem->line->setFrameShape(QFrame::HLine);
            parentItem->line->setFrameShadow(QFrame::Sunken);
            parentItem->layout->addWidget(parentItem->line, 1, 0, 1, 2);
         }

         gridLayout->addWidget(parentItem->groupBox, oldRow, 0, 1, 2);
         updateItem(parentItem);
      }

      topLayout    = parentItem->layout;
      parentWidget = parentItem->groupBox;
   }

   newItem->label  = new QLabel(parentWidget);
   newItem->label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
   newItem->widget = createEditor(index->property(), parentWidget);

   if (! newItem->widget) {
      newItem->widgetLabel = new QLabel(parentWidget);

   } else {
      QObject::connect(newItem->widget, &QWidget::destroyed,
            q_ptr, &QtGroupBoxPropertyBrowser::slotEditorDestroyed);

      m_widgetToItem[newItem->widget] = newItem;
   }

   insertRow(topLayout, row);
   int span = 1;

   if (newItem->widget) {
      topLayout->addWidget(newItem->widget, row, 1);

   } else if (newItem->widgetLabel) {
      topLayout->addWidget(newItem->widgetLabel, row, 1);

   } else {
      span = 2;
   }

   topLayout->addWidget(newItem->label, row, 0, 1, span);

   m_itemToIndex[newItem] = index;
   m_indexToItem[index]   = newItem;

   updateItem(newItem);
}

void QtGroupBoxPropertyBrowserPrivate::propertyRemoved(QtBrowserItem *index)
{
   WidgetItem *item = m_indexToItem.value(index);

   m_indexToItem.remove(index);
   m_itemToIndex.remove(item);

   WidgetItem *parentItem = item->parent;

   int row = -1;

   if (parentItem) {
      row = parentItem->children.indexOf(item);
      parentItem->children.removeAt(row);

      if (hasHeader(parentItem)) {
         row += 2;
      }

   } else {
      row = m_children.indexOf(item);
      m_children.removeAt(row);
   }

   if (item->widget) {
      delete item->widget;
   }

   if (item->label) {
      delete item->label;
   }

   if (item->widgetLabel) {
      delete item->widgetLabel;
   }

   if (item->groupBox) {
      delete item->groupBox;
   }

   if (! parentItem) {
      removeRow(m_mainLayout, row);

   } else if (parentItem->children.count() != 0) {
      removeRow(parentItem->layout, row);

   } else {
      WidgetItem *grandParentItem = parentItem->parent;
      QGridLayout *gridLayout = nullptr;

      if (grandParentItem == nullptr) {
         gridLayout  = m_mainLayout;

      } else {
         gridLayout  = grandParentItem->layout;

      }

      if (parentItem->widget) {
         parentItem->widget->hide();
         parentItem->widget->setParent(nullptr);

      } else if (parentItem->widgetLabel) {
         parentItem->widgetLabel->hide();
         parentItem->widgetLabel->setParent(nullptr);

      } else {
         // parentItem->widgetLabel = new QLabel(currentWidget);
      }

      gridLayout ->removeWidget(parentItem->groupBox);
      delete parentItem->groupBox;

      parentItem->groupBox = nullptr;
      parentItem->line     = nullptr;
      parentItem->layout   = nullptr;

      if (! m_recreateQueue.contains(parentItem)) {
         m_recreateQueue.append(parentItem);
      }

      updateLater();
   }

   m_recreateQueue.removeAll(item);

   delete item;
}

void QtGroupBoxPropertyBrowserPrivate::insertRow(QGridLayout *layout, int row) const
{
   QMap<QLayoutItem *, QRect> itemToPos;
   int idx = 0;

   while (idx < layout->count()) {
      int r, c, rs, cs;
      layout->getItemPosition(idx, &r, &c, &rs, &cs);

      if (r >= row) {
         itemToPos[layout->takeAt(idx)] = QRect(r + 1, c, rs, cs);
      } else {
         ++idx;
      }
   }

   auto icend = itemToPos.constEnd();

   for (auto iter = itemToPos.constBegin(); iter != icend; ++iter) {
      const QRect r = iter.value();
      layout->addItem(iter.key(), r.x(), r.y(), r.width(), r.height());
   }
}

void QtGroupBoxPropertyBrowserPrivate::removeRow(QGridLayout *layout, int row) const
{
   QMap<QLayoutItem *, QRect> itemToPos;
   int idx = 0;

   while (idx < layout->count()) {
      int r, c, rs, cs;
      layout->getItemPosition(idx, &r, &c, &rs, &cs);

      if (r > row) {
         itemToPos[layout->takeAt(idx)] = QRect(r - 1, c, rs, cs);
      } else {
         ++idx;
      }
   }

   auto icend = itemToPos.constEnd();

   for (auto iter = itemToPos.constBegin(); iter != icend; ++iter) {
      const QRect r = iter.value();
      layout->addItem(iter.key(), r.x(), r.y(), r.width(), r.height());
   }
}

bool QtGroupBoxPropertyBrowserPrivate::hasHeader(WidgetItem *item) const
{
   if (item->widget) {
      return true;
   }

   return false;
}

void QtGroupBoxPropertyBrowserPrivate::propertyChanged(QtBrowserItem *index)
{
   WidgetItem *item = m_indexToItem.value(index);

   updateItem(item);
}

void QtGroupBoxPropertyBrowserPrivate::updateItem(WidgetItem *item)
{
   QtProperty *property = m_itemToIndex[item]->property();

   if (item->groupBox) {
      QFont font = item->groupBox->font();
      font.setUnderline(property->isModified());
      item->groupBox->setFont(font);
      item->groupBox->setTitle(property->propertyName());
      item->groupBox->setToolTip(property->descriptionToolTip());
      item->groupBox->setStatusTip(property->statusTip());
      item->groupBox->setWhatsThis(property->whatsThis());
      item->groupBox->setEnabled(property->isEnabled());
   }
   if (item->label) {
      QFont font = item->label->font();
      font.setUnderline(property->isModified());
      item->label->setFont(font);
      item->label->setText(property->propertyName());
      item->label->setToolTip(property->descriptionToolTip());
      item->label->setStatusTip(property->statusTip());
      item->label->setWhatsThis(property->whatsThis());
      item->label->setEnabled(property->isEnabled());
   }

   if (item->widgetLabel) {
      QFont font = item->widgetLabel->font();
      font.setUnderline(false);
      item->widgetLabel->setFont(font);
      item->widgetLabel->setText(property->valueText());
      item->widgetLabel->setEnabled(property->isEnabled());
   }

   if (item->widget) {
      QFont font = item->widget->font();
      font.setUnderline(false);
      item->widget->setFont(font);
      item->widget->setEnabled(property->isEnabled());
      const QString valueToolTip = property->valueToolTip();
      item->widget->setToolTip(valueToolTip.isEmpty() ? property->valueText() : valueToolTip);
   }

   //item->setIcon(1, property->valueIcon());
}

QtGroupBoxPropertyBrowser::QtGroupBoxPropertyBrowser(QWidget *parent)
   : QtAbstractPropertyBrowser(parent), d_ptr(new QtGroupBoxPropertyBrowserPrivate)
{
   d_ptr->q_ptr = this;
   d_ptr->init(this);
}

QtGroupBoxPropertyBrowser::~QtGroupBoxPropertyBrowser()
{
   auto icend = d_ptr->m_itemToIndex.constEnd();

   for (auto iter = d_ptr->m_itemToIndex.constBegin(); iter != icend; ++iter) {
      delete iter.key();
   }
}

void QtGroupBoxPropertyBrowser::itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem)
{
   d_ptr->propertyInserted(item, afterItem);
}

void QtGroupBoxPropertyBrowser::itemRemoved(QtBrowserItem *item)
{
   d_ptr->propertyRemoved(item);
}

void QtGroupBoxPropertyBrowser::itemChanged(QtBrowserItem *item)
{
   d_ptr->propertyChanged(item);
}

// private slots
void QtGroupBoxPropertyBrowser::slotUpdate()
{
   Q_D(QtGroupBoxPropertyBrowser);
   d->slotUpdate();
}

void QtGroupBoxPropertyBrowser::slotEditorDestroyed()
{
   Q_D(QtGroupBoxPropertyBrowser);
   d->slotEditorDestroyed();
}
