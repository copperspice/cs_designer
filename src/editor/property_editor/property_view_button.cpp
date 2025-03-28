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

#include <property_view_button.h>

#include <QGridLayout>
#include <QLabel>
#include <QMap>
#include <QSet>
#include <QStyle>
#include <QTimer>
#include <QToolButton>

class QtButtonPropertyBrowserPrivate
{
   QtButtonPropertyBrowser *q_ptr;
   Q_DECLARE_PUBLIC(QtButtonPropertyBrowser)

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
   void slotToggled(bool checked);

   struct WidgetItem {
      WidgetItem()
         : widget(nullptr), label(nullptr), widgetLabel(nullptr), button(nullptr),
           container(nullptr), layout(nullptr), parent(nullptr), expanded(false)
      {
      }

      QWidget *widget;        // can be null
      QLabel *label;          // main label with property name
      QLabel *widgetLabel;    // label substitute showing the current value if there is no widget
      QToolButton *button;    // expandable button for items with children
      QWidget *container;     // container which is expanded when the button is clicked
      QGridLayout *layout;    // layout in container
      WidgetItem *parent;

      QList<WidgetItem *> children;
      bool expanded;
   };

 private:
   void updateLater();
   void updateItem(WidgetItem *item);
   void insertRow(QGridLayout *layout, int row) const;
   void removeRow(QGridLayout *layout, int row) const;
   int gridRow(WidgetItem *item) const;
   int gridSpan(WidgetItem *item) const;
   void setExpanded(WidgetItem *item, bool expanded);
   QToolButton *createButton(QWidget *panret = nullptr) const;

   QMap<QtBrowserItem *, WidgetItem *> m_indexToItem;
   QMap<WidgetItem *, QtBrowserItem *> m_itemToIndex;
   QMap<QWidget *, WidgetItem *> m_widgetToItem;
   QMap<QObject *, WidgetItem *> m_buttonToItem;
   QGridLayout *m_mainLayout;
   QList<WidgetItem *> m_children;
   QList<WidgetItem *> m_recreateQueue;
};

QToolButton *QtButtonPropertyBrowserPrivate::createButton(QWidget *parent) const
{
   QToolButton *button = new QToolButton(parent);
   button->setCheckable(true);
   button->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
   button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
   button->setArrowType(Qt::DownArrow);
   button->setIconSize(QSize(3, 16));

   /*
   QIcon icon;
   icon.addPixmap(q_ptr->style()->standardPixmap(QStyle::SP_ArrowDown), QIcon::Normal, QIcon::Off);
   icon.addPixmap(q_ptr->style()->standardPixmap(QStyle::SP_ArrowUp), QIcon::Normal, QIcon::On);
   button->setIcon(icon);
   */

   return button;
}

int QtButtonPropertyBrowserPrivate::gridRow(WidgetItem *item) const
{
   QList<WidgetItem *> siblings;

   if (item->parent) {
      siblings = item->parent->children;
   } else {
      siblings = m_children;
   }

   int row = 0;
   QListIterator<WidgetItem *> iter(siblings);

   while (iter.hasNext()) {
      WidgetItem *sibling = iter.next();
      if (sibling == item) {
         return row;
      }

      row += gridSpan(sibling);
   }

   return -1;
}

int QtButtonPropertyBrowserPrivate::gridSpan(WidgetItem *item) const
{
   if (item->container && item->expanded) {
      return 2;
   }

   return 1;
}

void QtButtonPropertyBrowserPrivate::init(QWidget *parent)
{
   m_mainLayout = new QGridLayout();
   parent->setLayout(m_mainLayout);

   QLayoutItem *item = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding);
   m_mainLayout->addItem(item, 0, 0);
}

void QtButtonPropertyBrowserPrivate::slotEditorDestroyed()
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

void QtButtonPropertyBrowserPrivate::slotUpdate()
{
   QListIterator<WidgetItem *> itItem(m_recreateQueue);

   while (itItem.hasNext()) {
      WidgetItem *item = itItem.next();

      WidgetItem *parent = item->parent;
      QWidget *widget    = nullptr;

      QGridLayout *gridLayout = nullptr;

      const int oldRow = gridRow(item);

      if (parent) {
         widget = parent->container;
         gridLayout = parent->layout;
      } else {
         widget = q_ptr;
         gridLayout = m_mainLayout;
      }

      int span = 1;
      if (! item->widget && ! item->widgetLabel) {
         span = 2;
      }

      item->label = new QLabel(widget);
      item->label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
      gridLayout->addWidget(item->label, oldRow, 0, 1, span);

      updateItem(item);
   }

   m_recreateQueue.clear();
}

void QtButtonPropertyBrowserPrivate::setExpanded(WidgetItem *item, bool expanded)
{
   if (item->expanded == expanded) {
      return;
   }

   if (! item->container) {
      return;
   }

   item->expanded = expanded;
   const int row  = gridRow(item);

   WidgetItem *parent = item->parent;
   QGridLayout *gridLayout = nullptr;

   if (parent) {
      gridLayout = parent->layout;
   } else {
      gridLayout = m_mainLayout;
   }

   if (expanded) {
      insertRow(gridLayout, row + 1);
      gridLayout->addWidget(item->container, row + 1, 0, 1, 2);
      item->container->show();
   } else {
      gridLayout->removeWidget(item->container);
      item->container->hide();
      removeRow(gridLayout, row + 1);
   }

   item->button->setChecked(expanded);
   item->button->setArrowType(expanded ? Qt::UpArrow : Qt::DownArrow);
}

void QtButtonPropertyBrowserPrivate::slotToggled(bool checked)
{
   WidgetItem *item = m_buttonToItem.value(q_ptr->sender());

   if (! item) {
      return;
   }

   setExpanded(item, checked);

   if (checked) {
      emit q_ptr->expanded(m_itemToIndex.value(item));
   } else {
      emit q_ptr->collapsed(m_itemToIndex.value(item));
   }
}

void QtButtonPropertyBrowserPrivate::updateLater()
{
   QTimer::singleShot(0, q_ptr, SLOT(slotUpdate()));
}

void QtButtonPropertyBrowserPrivate::propertyInserted(QtBrowserItem *index, QtBrowserItem *afterIndex)
{
   WidgetItem *afterItem  = m_indexToItem.value(afterIndex);
   WidgetItem *parentItem = m_indexToItem.value(index->parent());

   WidgetItem *newItem = new WidgetItem();
   newItem->parent = parentItem;

   QGridLayout *topLayout = nullptr;
   QWidget *parentWidget  = nullptr;

   int row = -1;

   if (! afterItem) {
      row = 0;

      if (parentItem) {
         parentItem->children.insert(0, newItem);
      } else {
         m_children.insert(0, newItem);
      }

   } else {
      row = gridRow(afterItem) + gridSpan(afterItem);

      if (parentItem) {
         parentItem->children.insert(parentItem->children.indexOf(afterItem) + 1, newItem);
      } else {
         m_children.insert(m_children.indexOf(afterItem) + 1, newItem);
      }
   }

   if (! parentItem) {
      topLayout    = m_mainLayout;
      parentWidget = q_ptr;

   } else {
      if (! parentItem->container) {
         m_recreateQueue.removeAll(parentItem);
         WidgetItem *grandParent = parentItem->parent;

         QGridLayout *gridLayout = nullptr;
         const int oldRow = gridRow(parentItem);

         if (grandParent != nullptr) {
            gridLayout = grandParent->layout;
         } else {
            gridLayout = m_mainLayout;
         }

         QFrame *container = new QFrame();
         container->setFrameShape(QFrame::Panel);
         container->setFrameShadow(QFrame::Raised);

         parentItem->container = container;
         parentItem->button    = createButton();
         m_buttonToItem[parentItem->button] = parentItem;

         q_ptr->connect(parentItem->button, &QToolButton::toggled, q_ptr, &QtButtonPropertyBrowser::slotToggled);

         parentItem->layout = new QGridLayout();
         container->setLayout(parentItem->layout);

         if (parentItem->label) {
            gridLayout->removeWidget(parentItem->label);
            delete parentItem->label;
            parentItem->label = nullptr;
         }

         int span = 1;
         if (! parentItem->widget && ! parentItem->widgetLabel) {
            span = 2;
         }

         gridLayout->addWidget(parentItem->button, oldRow, 0, 1, span);
         updateItem(parentItem);
      }

      topLayout    = parentItem->layout;
      parentWidget = parentItem->container;
   }

   newItem->label = new QLabel(parentWidget);
   newItem->label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
   newItem->widget = createEditor(index->property(), parentWidget);

   if (newItem->widget) {
      QObject::connect(newItem->widget, &QWidget::destroyed,
            q_ptr, &QtButtonPropertyBrowser::slotEditorDestroyed);

      m_widgetToItem[newItem->widget] = newItem;

   } else if (index->property()->hasValue()) {
      newItem->widgetLabel = new QLabel(parentWidget);
      newItem->widgetLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
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

   topLayout->addWidget(newItem->label, row, 0, span, 1);

   m_itemToIndex[newItem] = index;
   m_indexToItem[index]   = newItem;

   updateItem(newItem);
}

void QtButtonPropertyBrowserPrivate::propertyRemoved(QtBrowserItem *index)
{
   WidgetItem *item = m_indexToItem.value(index);

   m_indexToItem.remove(index);
   m_itemToIndex.remove(item);

   WidgetItem *parentItem = item->parent;

   const int row = gridRow(item);

   if (parentItem) {
      parentItem->children.removeAt(parentItem->children.indexOf(item));
   } else {
      m_children.removeAt(m_children.indexOf(item));
   }

   const int colSpan = gridSpan(item);

   m_buttonToItem.remove(item->button);

   if (item->widget) {
      delete item->widget;
   }

   if (item->label) {
      delete item->label;
   }

   if (item->widgetLabel) {
      delete item->widgetLabel;
   }

   if (item->button) {
      delete item->button;
   }

   if (item->container) {
      delete item->container;
   }

   if (! parentItem) {
      removeRow(m_mainLayout, row);

      if (colSpan > 1) {
         removeRow(m_mainLayout, row);
      }

   } else if (parentItem->children.count() != 0) {
      removeRow(parentItem->layout, row);

      if (colSpan > 1) {
         removeRow(parentItem->layout, row);
      }

   } else {
      const WidgetItem *grandParent = parentItem->parent;
      QGridLayout *gridLayout = nullptr;

      if (grandParent != nullptr) {
         gridLayout = grandParent->layout;
      } else {
         gridLayout = m_mainLayout;
      }

      const int parentRow  = gridRow(parentItem);
      const int parentSpan = gridSpan(parentItem);

      gridLayout->removeWidget(parentItem->button);
      gridLayout->removeWidget(parentItem->container);

      delete parentItem->button;
      delete parentItem->container;

      parentItem->button    = nullptr;
      parentItem->container = nullptr;
      parentItem->layout    = nullptr;

      if (! m_recreateQueue.contains(parentItem)) {
         m_recreateQueue.append(parentItem);
      }

      if (parentSpan > 1) {
         removeRow(gridLayout, parentRow + 1);
      }

      updateLater();
   }

   m_recreateQueue.removeAll(item);

   delete item;
}

void QtButtonPropertyBrowserPrivate::insertRow(QGridLayout *layout, int row) const
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

   auto icend =  itemToPos.constEnd();

   for (auto iter = itemToPos.constBegin(); iter != icend; ++iter) {
      const QRect r = iter.value();
      layout->addItem(iter.key(), r.x(), r.y(), r.width(), r.height());
   }
}

void QtButtonPropertyBrowserPrivate::removeRow(QGridLayout *layout, int row) const
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

   auto icend =  itemToPos.constEnd();

   for (auto iter = itemToPos.constBegin(); iter != icend; ++iter) {
      const QRect r = iter.value();
      layout->addItem(iter.key(), r.x(), r.y(), r.width(), r.height());
   }
}

void QtButtonPropertyBrowserPrivate::propertyChanged(QtBrowserItem *index)
{
   WidgetItem *item = m_indexToItem.value(index);
   updateItem(item);
}

void QtButtonPropertyBrowserPrivate::updateItem(WidgetItem *item)
{
   QtProperty *property = m_itemToIndex[item]->property();

   if (item->button) {
      QFont font = item->button->font();
      font.setUnderline(property->isModified());
      item->button->setFont(font);
      item->button->setText(property->propertyName());
      item->button->setToolTip(property->descriptionToolTip());
      item->button->setStatusTip(property->statusTip());
      item->button->setWhatsThis(property->whatsThis());
      item->button->setEnabled(property->isEnabled());
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
      item->widgetLabel->setToolTip(property->valueText());
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
}

QtButtonPropertyBrowser::QtButtonPropertyBrowser(QWidget *parent)
   : QtAbstractPropertyBrowser(parent), d_ptr(new QtButtonPropertyBrowserPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->init(this);
}

QtButtonPropertyBrowser::~QtButtonPropertyBrowser()
{
   auto icend = d_ptr->m_itemToIndex.constEnd();

   for (auto iter =  d_ptr->m_itemToIndex.constBegin(); iter != icend; ++iter) {
      delete iter.key();
   }
}

void QtButtonPropertyBrowser::itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem)
{
   d_ptr->propertyInserted(item, afterItem);
}

void QtButtonPropertyBrowser::itemRemoved(QtBrowserItem *item)
{
   d_ptr->propertyRemoved(item);
}

void QtButtonPropertyBrowser::itemChanged(QtBrowserItem *item)
{
   d_ptr->propertyChanged(item);
}

void QtButtonPropertyBrowser::setExpanded(QtBrowserItem *browserItem, bool expanded)
{
   QtButtonPropertyBrowserPrivate::WidgetItem *widgetItem = d_ptr->m_indexToItem.value(browserItem);

   if (widgetItem != nullptr) {
      d_ptr->setExpanded(widgetItem, expanded);
   }
}

bool QtButtonPropertyBrowser::isExpanded(QtBrowserItem *browserItem) const
{
   QtButtonPropertyBrowserPrivate::WidgetItem *widgetItem = d_ptr->m_indexToItem.value(browserItem);

   if (widgetItem != nullptr) {
      return widgetItem->expanded;
   }

   return false;
}

void QtButtonPropertyBrowser::slotUpdate()
{
   Q_D(QtButtonPropertyBrowser);
   d->slotUpdate();
}

void QtButtonPropertyBrowser::slotEditorDestroyed()
{
   Q_D(QtButtonPropertyBrowser);
   d->slotEditorDestroyed();
}

void QtButtonPropertyBrowser::slotToggled(bool un_named_arg1)
{
   Q_D(QtButtonPropertyBrowser);
   d->slotToggled(un_named_arg1);
}