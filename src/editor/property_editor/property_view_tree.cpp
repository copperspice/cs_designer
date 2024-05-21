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

#include <property_view_tree.h>

#include <QApplication>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QItemDelegate>
#include <QPainter>
#include <QPalette>
#include <QSet>
#include <QStyle>
#include <QTreeWidget>

class QtPropertyEditorView;

class QtTreePropertyBrowserPrivate
{
   QtTreePropertyBrowser *q_ptr;
   Q_DECLARE_PUBLIC(QtTreePropertyBrowser)

 public:
   QtTreePropertyBrowserPrivate();
   void init(QWidget *parent);

   void propertyInserted(QtBrowserItem *index, QtBrowserItem *afterIndex);
   void propertyRemoved(QtBrowserItem *index);
   void propertyChanged(QtBrowserItem *index);

   QWidget *createEditor(QtProperty *property, QWidget *parent) const {
      return q_ptr->createEditor(property, parent);
   }

   QtProperty *indexToProperty(const QModelIndex &index) const;
   QTreeWidgetItem *indexToItem(const QModelIndex &index) const;
   QtBrowserItem *indexToBrowserItem(const QModelIndex &index) const;
   bool lastColumn(int column) const;
   void disableItem(QTreeWidgetItem *item) const;
   void enableItem(QTreeWidgetItem *item) const;
   bool hasValue(QTreeWidgetItem *item) const;

   void slotCollapsed(const QModelIndex &index);
   void slotExpanded(const QModelIndex &index);

   QColor calculatedBackgroundColor(QtBrowserItem *item) const;

   QtPropertyEditorView *treeWidget() const {
      return m_treeWidget;
   }
   bool markPropertiesWithoutValue() const {
      return m_markPropertiesWithoutValue;
   }

   QtBrowserItem *currentItem() const;
   void setCurrentItem(QtBrowserItem *browserItem, bool block);
   void editItem(QtBrowserItem *browserItem);

   void slotCurrentBrowserItemChanged(QtBrowserItem *item);
   void slotCurrentTreeItemChanged(QTreeWidgetItem *newItem, QTreeWidgetItem *);

   QTreeWidgetItem *editedItem() const;

 private:
   void updateItem(QTreeWidgetItem *item);

   QMap<QtBrowserItem *, QTreeWidgetItem *> m_indexToItem;
   QMap<QTreeWidgetItem *, QtBrowserItem *> m_itemToIndex;

   QMap<QtBrowserItem *, QColor> m_indexToBackgroundColor;

   QtPropertyEditorView *m_treeWidget;

   bool m_headerVisible;
   QtTreePropertyBrowser::ResizeMode m_resizeMode;
   class QtPropertyEditorDelegate *m_delegate;
   bool m_markPropertiesWithoutValue;
   bool m_browserChangedBlocked;
   QIcon m_expandIcon;
};

class QtPropertyEditorView : public QTreeWidget
{
   CS_OBJECT(QtPropertyEditorView)

 public:
   QtPropertyEditorView(QWidget *parent = nullptr);

   void setEditorPrivate(QtTreePropertyBrowserPrivate *editorPrivate) {
      m_editorPrivate = editorPrivate;
   }

   QTreeWidgetItem *indexToItem(const QModelIndex &index) const {
      return itemFromIndex(index);
   }

 protected:
   void keyPressEvent(QKeyEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

 private:
   QtTreePropertyBrowserPrivate *m_editorPrivate;
};

QtPropertyEditorView::QtPropertyEditorView(QWidget *parent)
   : QTreeWidget(parent), m_editorPrivate(nullptr)
{
   connect(header(), &QHeaderView::sectionDoubleClicked, this, &QtPropertyEditorView::resizeColumnToContents);
}

void QtPropertyEditorView::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   QStyleOptionViewItem opt = option;
   bool hasValue = true;

   if (m_editorPrivate) {
      QtProperty *property = m_editorPrivate->indexToProperty(index);

      if (property) {
         hasValue = property->hasValue();
      }
   }
   if (!hasValue && m_editorPrivate->markPropertiesWithoutValue()) {
      const QColor c = option.palette.color(QPalette::Dark);
      painter->fillRect(option.rect, c);
      opt.palette.setColor(QPalette::AlternateBase, c);
   } else {
      const QColor c = m_editorPrivate->calculatedBackgroundColor(m_editorPrivate->indexToBrowserItem(index));
      if (c.isValid()) {
         painter->fillRect(option.rect, c);
         opt.palette.setColor(QPalette::AlternateBase, c.lighter(112));
      }
   }
   QTreeWidget::drawRow(painter, opt, index);
   QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
   painter->save();
   painter->setPen(QPen(color));
   painter->drawLine(opt.rect.x(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
   painter->restore();
}

void QtPropertyEditorView::keyPressEvent(QKeyEvent *event)
{
   switch (event->key()) {
      case Qt::Key_Return:
      case Qt::Key_Enter:
      case Qt::Key_Space: // Trigger Edit
         if (!m_editorPrivate->editedItem())
            if (const QTreeWidgetItem *item = currentItem())
               if (item->columnCount() >= 2 &&
                  ((item->flags() & (Qt::ItemIsEditable | Qt::ItemIsEnabled)) == (Qt::ItemIsEditable | Qt::ItemIsEnabled))) {
                  event->accept();
                  // If the current position is at column 0, move to 1.
                  QModelIndex index = currentIndex();
                  if (index.column() == 0) {
                     index = index.sibling(index.row(), 1);
                     setCurrentIndex(index);
                  }
                  edit(index);
                  return;
               }
         break;

      default:
         break;
   }

   QTreeWidget::keyPressEvent(event);
}

void QtPropertyEditorView::mousePressEvent(QMouseEvent *event)
{
   QTreeWidget::mousePressEvent(event);
   QTreeWidgetItem *item = itemAt(event->pos());

   if (item) {
      if ((item != m_editorPrivate->editedItem()) && (event->button() == Qt::LeftButton)
         && (header()->logicalIndexAt(event->pos().x()) == 1)
         && ((item->flags() & (Qt::ItemIsEditable | Qt::ItemIsEnabled)) == (Qt::ItemIsEditable | Qt::ItemIsEnabled))) {
         editItem(item, 1);

      } else if (!m_editorPrivate->hasValue(item) && m_editorPrivate->markPropertiesWithoutValue() && !rootIsDecorated()) {
         if (event->pos().x() + header()->offset() < 20) {
            item->setExpanded(!item->isExpanded());
         }
      }
   }
}

class QtPropertyEditorDelegate : public QItemDelegate
{
   CS_OBJECT(QtPropertyEditorDelegate)

 public:
   QtPropertyEditorDelegate(QObject *parent = nullptr)
      : QItemDelegate(parent), m_editorPrivate(nullptr), m_editedItem(nullptr), m_editedWidget(nullptr)
   {
   }

   void setEditorPrivate(QtTreePropertyBrowserPrivate *editorPrivate) {
      m_editorPrivate = editorPrivate;
   }

   QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
      const QModelIndex &index) const override;

   void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
      const QModelIndex &index) const override;

   void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

   QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

   void setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const override
   {
   }

   void setEditorData(QWidget *, const QModelIndex &) const override
   {
   }

   bool eventFilter(QObject *object, QEvent *event) override;
   void closeEditor(QtProperty *property);

   QTreeWidgetItem *editedItem() const {
      return m_editedItem;
   }

 private:
   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *object))
   CS_SLOT_2(slotEditorDestroyed)

   int indentation(const QModelIndex &index) const;

   typedef QMap<QWidget *, QtProperty *> EditorToPropertyMap;
   mutable EditorToPropertyMap m_editorToProperty;

   typedef QMap<QtProperty *, QWidget *> PropertyToEditorMap;
   mutable PropertyToEditorMap m_propertyToEditor;
   QtTreePropertyBrowserPrivate *m_editorPrivate;
   mutable QTreeWidgetItem *m_editedItem;
   mutable QWidget *m_editedWidget;
};

int QtPropertyEditorDelegate::indentation(const QModelIndex &index) const
{
   if (! m_editorPrivate) {
      return 0;
   }

   QTreeWidgetItem *item = m_editorPrivate->indexToItem(index);
   int indent = 0;

   while (item->parent()) {
      item = item->parent();
      ++indent;
   }

   if (m_editorPrivate->treeWidget()->rootIsDecorated()) {
      ++indent;
   }

   return indent * m_editorPrivate->treeWidget()->indentation();
}

void QtPropertyEditorDelegate::slotEditorDestroyed(QObject *object)
{
   if (QWidget *w = dynamic_cast<QWidget *>(object)) {
      const EditorToPropertyMap::iterator it = m_editorToProperty.find(w);

      if (it != m_editorToProperty.end()) {
         m_propertyToEditor.remove(it.value());
         m_editorToProperty.erase(it);
      }

      if (m_editedWidget == w) {
         m_editedWidget = nullptr;
         m_editedItem   = nullptr;
      }
   }
}

void QtPropertyEditorDelegate::closeEditor(QtProperty *property)
{
   if (QWidget *w = m_propertyToEditor.value(property, nullptr)) {
      (void) w;

      // temporarily removed
      // w->deleteLater()
   }
}

QWidget *QtPropertyEditorDelegate::createEditor(QWidget *parent,
   const QStyleOptionViewItem &, const QModelIndex &index) const
{
   if (index.column() == 1 && m_editorPrivate) {
      QtProperty *property  = m_editorPrivate->indexToProperty(index);
      QTreeWidgetItem *item = m_editorPrivate->indexToItem(index);

      if (property && item && (item->flags() & Qt::ItemIsEnabled)) {
         QWidget *editor = m_editorPrivate->createEditor(property, parent);

         if (editor) {
            editor->setAutoFillBackground(true);
            editor->installEventFilter(const_cast<QtPropertyEditorDelegate *>(this));

            connect(editor, &QWidget::destroyed, this, &QtPropertyEditorDelegate::slotEditorDestroyed);

            m_propertyToEditor[property] = editor;
            m_editorToProperty[editor] = property;
            m_editedItem = item;
            m_editedWidget = editor;
         }

         return editor;
      }
   }

   return nullptr;
}

void QtPropertyEditorDelegate::updateEditorGeometry(QWidget *editor,
   const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   (void) index;
   editor->setGeometry(option.rect.adjusted(0, 0, 0, -1));
}

void QtPropertyEditorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
   const QModelIndex &index) const
{
   bool hasValue = true;

   if (m_editorPrivate != nullptr) {
      QtProperty *property = m_editorPrivate->indexToProperty(index);

      if (property != nullptr) {
         hasValue = property->hasValue();
      }
   }

   QStyleOptionViewItem opt = option;
   if ((m_editorPrivate && index.column() == 0) || !hasValue) {
      QtProperty *property = m_editorPrivate->indexToProperty(index);
      if (property && property->isModified()) {
         opt.font.setBold(true);
         opt.fontMetrics = QFontMetrics(opt.font);
      }
   }

   QColor c;
   if (!hasValue && m_editorPrivate->markPropertiesWithoutValue()) {
      c = opt.palette.color(QPalette::Dark);
      opt.palette.setColor(QPalette::Text, opt.palette.color(QPalette::BrightText));
   } else {
      c = m_editorPrivate->calculatedBackgroundColor(m_editorPrivate->indexToBrowserItem(index));
      if (c.isValid() && (opt.features & QStyleOptionViewItem::Alternate)) {
         c = c.lighter(112);
      }
   }
   if (c.isValid()) {
      painter->fillRect(option.rect, c);
   }
   opt.state &= ~QStyle::State_HasFocus;
   QItemDelegate::paint(painter, opt, index);

   opt.palette.setCurrentColorGroup(QPalette::Active);
   QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
   painter->save();
   painter->setPen(QPen(color));
   if (!m_editorPrivate || (!m_editorPrivate->lastColumn(index.column()) && hasValue)) {
      int right = (option.direction == Qt::LeftToRight) ? option.rect.right() : option.rect.left();
      painter->drawLine(right, option.rect.y(), right, option.rect.bottom());
   }
   painter->restore();
}

QSize QtPropertyEditorDelegate::sizeHint(const QStyleOptionViewItem &option,
   const QModelIndex &index) const
{
   return QItemDelegate::sizeHint(option, index) + QSize(3, 4);
}

bool QtPropertyEditorDelegate::eventFilter(QObject *object, QEvent *event)
{
   if (event->type() == QEvent::FocusOut) {
      QFocusEvent *fe = static_cast<QFocusEvent *>(event);
      if (fe->reason() == Qt::ActiveWindowFocusReason) {
         return false;
      }
   }
   return QItemDelegate::eventFilter(object, event);
}

QtTreePropertyBrowserPrivate::QtTreePropertyBrowserPrivate()
   : m_treeWidget(nullptr), m_headerVisible(true), m_resizeMode(QtTreePropertyBrowser::Stretch),
     m_delegate(nullptr), m_markPropertiesWithoutValue(false), m_browserChangedBlocked(false)
{
}

// Draw an icon indicating opened/closing branches
static QIcon drawIndicatorIcon(const QPalette &palette, QStyle *style)
{
   QPixmap pix(14, 14);
   pix.fill(Qt::transparent);
   QStyleOption branchOption;
   QRect r(QPoint(0, 0), pix.size());
   branchOption.rect = QRect(2, 2, 9, 9); // ### hardcoded in qcommonstyle.cpp
   branchOption.palette = palette;
   branchOption.state = QStyle::State_Children;

   QPainter p;
   // Draw closed state
   p.begin(&pix);
   style->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, &p);
   p.end();
   QIcon rc = pix;
   rc.addPixmap(pix, QIcon::Selected, QIcon::Off);
   // Draw opened state
   branchOption.state |= QStyle::State_Open;
   pix.fill(Qt::transparent);
   p.begin(&pix);
   style->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, &p);
   p.end();

   rc.addPixmap(pix, QIcon::Normal, QIcon::On);
   rc.addPixmap(pix, QIcon::Selected, QIcon::On);
   return rc;
}

void QtTreePropertyBrowserPrivate::init(QWidget *parent)
{
   QHBoxLayout *layout = new QHBoxLayout(parent);
   layout->setMargin(0);

   m_treeWidget = new QtPropertyEditorView(parent);
   m_treeWidget->setEditorPrivate(this);
   m_treeWidget->setIconSize(QSize(18, 18));
   layout->addWidget(m_treeWidget);

   m_treeWidget->setColumnCount(2);

   QStringList labels;
   labels.append(QCoreApplication::translate("QtTreePropertyBrowser", "Property"));
   labels.append(QCoreApplication::translate("QtTreePropertyBrowser", "Value"));

   m_treeWidget->setHeaderLabels(labels);
   m_treeWidget->setAlternatingRowColors(true);
   m_treeWidget->setEditTriggers(QAbstractItemView::EditKeyPressed);

   m_delegate = new QtPropertyEditorDelegate(parent);
   m_delegate->setEditorPrivate(this);
   m_treeWidget->setItemDelegate(m_delegate);
   m_treeWidget->header()->setSectionsMovable(false);
   m_treeWidget->header()->setSectionResizeMode(QHeaderView::Stretch);

   m_expandIcon = drawIndicatorIcon(q_ptr->palette(), q_ptr->style());

   QObject::connect(m_treeWidget, &QtPropertyEditorView::collapsed,          q_ptr, &QtTreePropertyBrowser::slotCollapsed);
   QObject::connect(m_treeWidget, &QtPropertyEditorView::expanded,           q_ptr, &QtTreePropertyBrowser::slotExpanded);
   QObject::connect(m_treeWidget, &QtPropertyEditorView::currentItemChanged, q_ptr, &QtTreePropertyBrowser::slotCurrentTreeItemChanged);
}

QtBrowserItem *QtTreePropertyBrowserPrivate::currentItem() const
{
   if (QTreeWidgetItem *treeItem = m_treeWidget->currentItem()) {
      return m_itemToIndex.value(treeItem);
   }
   return nullptr;
}

void QtTreePropertyBrowserPrivate::setCurrentItem(QtBrowserItem *browserItem, bool block)
{
   const bool blocked = block ? m_treeWidget->blockSignals(true) : false;

   if (browserItem == nullptr) {
      m_treeWidget->setCurrentItem(nullptr);

   } else {
      m_treeWidget->setCurrentItem(m_indexToItem.value(browserItem));
   }

   if (block) {
      m_treeWidget->blockSignals(blocked);
   }
}

QtProperty *QtTreePropertyBrowserPrivate::indexToProperty(const QModelIndex &index) const
{
   QTreeWidgetItem *item = m_treeWidget->indexToItem(index);
   QtBrowserItem *idx = m_itemToIndex.value(item);
   if (idx != nullptr) {

      return idx->property();
   }

   return nullptr;
}

QtBrowserItem *QtTreePropertyBrowserPrivate::indexToBrowserItem(const QModelIndex &index) const
{
   QTreeWidgetItem *item = m_treeWidget->indexToItem(index);
   return m_itemToIndex.value(item);
}

QTreeWidgetItem *QtTreePropertyBrowserPrivate::indexToItem(const QModelIndex &index) const
{
   return m_treeWidget->indexToItem(index);
}

bool QtTreePropertyBrowserPrivate::lastColumn(int column) const
{
   return m_treeWidget->header()->visualIndex(column) == m_treeWidget->columnCount() - 1;
}

void QtTreePropertyBrowserPrivate::disableItem(QTreeWidgetItem *item) const
{
   (void) item;

/* temporarily removed, concern with closeEditor()

   Qt::ItemFlags flags = item->flags();

   if (flags & Qt::ItemIsEnabled) {
      flags &= ~Qt::ItemIsEnabled;
      item->setFlags(flags);

      m_delegate->closeEditor(m_itemToIndex[item]->property());
      const int childCount = item->childCount();

      for (int i = 0; i < childCount; ++i) {
         QTreeWidgetItem *child = item->child(i);
         disableItem(child);
      }
   }
*/

}

void QtTreePropertyBrowserPrivate::enableItem(QTreeWidgetItem *item) const
{
   Qt::ItemFlags flags = item->flags();

   flags |= Qt::ItemIsEnabled;
   item->setFlags(flags);

   const int childCount = item->childCount();

   for (int i = 0; i < childCount; i++) {
      QTreeWidgetItem *child = item->child(i);
      QtProperty *property = m_itemToIndex[child]->property();

      if (property->isEnabled()) {
         enableItem(child);
      }
   }
}

bool QtTreePropertyBrowserPrivate::hasValue(QTreeWidgetItem *item) const
{
   QtBrowserItem *browserItem = m_itemToIndex.value(item);

   if (browserItem) {
      return browserItem->property()->hasValue();
   }

   return false;
}

void QtTreePropertyBrowserPrivate::propertyInserted(QtBrowserItem *index, QtBrowserItem *afterIndex)
{
   QTreeWidgetItem *afterItem = m_indexToItem.value(afterIndex);
   QTreeWidgetItem *parentItem = m_indexToItem.value(index->parent());

   QTreeWidgetItem *newItem = nullptr;

   if (parentItem) {
      newItem = new QTreeWidgetItem(parentItem, afterItem);
   } else {
      newItem = new QTreeWidgetItem(m_treeWidget, afterItem);
   }
   m_itemToIndex[newItem] = index;
   m_indexToItem[index] = newItem;

   newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
   m_treeWidget->setItemExpanded(newItem, true);

   updateItem(newItem);
}

void QtTreePropertyBrowserPrivate::propertyRemoved(QtBrowserItem *index)
{
   QTreeWidgetItem *item = m_indexToItem.value(index);

   if (m_treeWidget->currentItem() == item) {
      m_treeWidget->setCurrentItem(nullptr);
   }

   delete item;

   m_indexToItem.remove(index);
   m_itemToIndex.remove(item);
   m_indexToBackgroundColor.remove(index);
}

void QtTreePropertyBrowserPrivate::propertyChanged(QtBrowserItem *index)
{
   QTreeWidgetItem *item = m_indexToItem.value(index);
   updateItem(item);
}

void QtTreePropertyBrowserPrivate::updateItem(QTreeWidgetItem *item)
{
   QtProperty *property = m_itemToIndex[item]->property();
   QIcon expandIcon;

   if (property->hasValue()) {
      const QString valueToolTip = property->valueToolTip();
      const QString valueText = property->valueText();

      item->setToolTip(1, valueToolTip.isEmpty() ? valueText : valueToolTip);
      item->setIcon(1, property->valueIcon());
      item->setText(1, valueText);

   } else if (markPropertiesWithoutValue() && !m_treeWidget->rootIsDecorated()) {
      expandIcon = m_expandIcon;
   }

   item->setIcon(0, expandIcon);

   if (! property->hasValue()) {
      item->setFirstColumnSpanned(true);
   }

   const QString descriptionToolTip = property->descriptionToolTip();
   const QString propertyName = property->propertyName();

   if (! descriptionToolTip.isEmpty()) {
      // does not show when all we have is the propertyName
      item->setToolTip(0, descriptionToolTip);
   }

   item->setStatusTip(0, property->statusTip());
   item->setWhatsThis(0, property->whatsThis());
   item->setText(0, propertyName);

   bool wasEnabled = item->flags() & Qt::ItemIsEnabled;
   bool isEnabled  = wasEnabled;

   if (property->isEnabled()) {
      QTreeWidgetItem *parent = item->parent();

      if (! parent || (parent->flags() & Qt::ItemIsEnabled)) {
         isEnabled = true;
      } else {
         isEnabled = false;
      }

   } else {
      isEnabled = false;
   }

   if (wasEnabled != isEnabled) {
      if (isEnabled) {
         enableItem(item);
      } else {
         disableItem(item);
      }
   }

   m_treeWidget->viewport()->update();
}

QColor QtTreePropertyBrowserPrivate::calculatedBackgroundColor(QtBrowserItem *item) const
{
   QtBrowserItem *i = item;
   auto itEnd = m_indexToBackgroundColor.constEnd();

   while (i) {
      QMap<QtBrowserItem *, QColor>::const_iterator it = m_indexToBackgroundColor.constFind(i);

      if (it != itEnd) {
         return it.value();
      }

      i = i->parent();
   }

   return QColor();
}

void QtTreePropertyBrowserPrivate::slotCollapsed(const QModelIndex &index)
{
   QTreeWidgetItem *item = indexToItem(index);
   QtBrowserItem *idx = m_itemToIndex.value(item);
   if (item) {
      emit q_ptr->collapsed(idx);
   }
}

void QtTreePropertyBrowserPrivate::slotExpanded(const QModelIndex &index)
{
   QTreeWidgetItem *item = indexToItem(index);
   QtBrowserItem *idx = m_itemToIndex.value(item);

   if (item) {
      emit q_ptr->expanded(idx);
   }
}

void QtTreePropertyBrowserPrivate::slotCurrentBrowserItemChanged(QtBrowserItem *item)
{
   if (!m_browserChangedBlocked && item != currentItem()) {
      setCurrentItem(item, true);
   }
}

void QtTreePropertyBrowserPrivate::slotCurrentTreeItemChanged(QTreeWidgetItem *newItem, QTreeWidgetItem *)
{
   QtBrowserItem *browserItem = newItem ? m_itemToIndex.value(newItem) : nullptr;
   m_browserChangedBlocked = true;
   q_ptr->setCurrentItem(browserItem);
   m_browserChangedBlocked = false;
}

QTreeWidgetItem *QtTreePropertyBrowserPrivate::editedItem() const
{
   return m_delegate->editedItem();
}

void QtTreePropertyBrowserPrivate::editItem(QtBrowserItem *browserItem)
{
   if (QTreeWidgetItem *treeItem = m_indexToItem.value(browserItem, nullptr)) {
      m_treeWidget->setCurrentItem (treeItem, 1);
      m_treeWidget->editItem(treeItem, 1);
   }
}

QtTreePropertyBrowser::QtTreePropertyBrowser(QWidget *parent)
   : QtAbstractPropertyBrowser(parent), d_ptr(new QtTreePropertyBrowserPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->init(this);
   connect(this, &QtTreePropertyBrowser::currentItemChanged, this, &QtTreePropertyBrowser::slotCurrentBrowserItemChanged);
}

QtTreePropertyBrowser::~QtTreePropertyBrowser()
{
}

int QtTreePropertyBrowser::indentation() const
{
   return d_ptr->m_treeWidget->indentation();
}

void QtTreePropertyBrowser::setIndentation(int i)
{
   d_ptr->m_treeWidget->setIndentation(i);
}

bool QtTreePropertyBrowser::rootIsDecorated() const
{
   return d_ptr->m_treeWidget->rootIsDecorated();
}

void QtTreePropertyBrowser::setRootIsDecorated(bool show)
{
   d_ptr->m_treeWidget->setRootIsDecorated(show);
   QMapIterator<QTreeWidgetItem *, QtBrowserItem *> it(d_ptr->m_itemToIndex);

   while (it.hasNext()) {
      QtProperty *property = it.next().value()->property();

      if (!property->hasValue()) {
         d_ptr->updateItem(it.key());
      }
   }
}

bool QtTreePropertyBrowser::alternatingRowColors() const
{
   return d_ptr->m_treeWidget->alternatingRowColors();
}

void QtTreePropertyBrowser::setAlternatingRowColors(bool enable)
{
   d_ptr->m_treeWidget->setAlternatingRowColors(enable);
   QMapIterator<QTreeWidgetItem *, QtBrowserItem *> it(d_ptr->m_itemToIndex);
}

bool QtTreePropertyBrowser::isHeaderVisible() const
{
   return d_ptr->m_headerVisible;
}

void QtTreePropertyBrowser::setHeaderVisible(bool visible)
{
   if (d_ptr->m_headerVisible == visible) {
      return;
   }

   d_ptr->m_headerVisible = visible;
   d_ptr->m_treeWidget->header()->setVisible(visible);
}

QtTreePropertyBrowser::ResizeMode QtTreePropertyBrowser::resizeMode() const
{
   return d_ptr->m_resizeMode;
}

void QtTreePropertyBrowser::setResizeMode(QtTreePropertyBrowser::ResizeMode mode)
{
   if (d_ptr->m_resizeMode == mode) {
      return;
   }

   d_ptr->m_resizeMode = mode;
   QHeaderView::ResizeMode m = QHeaderView::Stretch;

   switch (mode) {
      case QtTreePropertyBrowser::Interactive:
         m = QHeaderView::Interactive;
         break;

      case QtTreePropertyBrowser::Fixed:
         m = QHeaderView::Fixed;
         break;

      case QtTreePropertyBrowser::ResizeToContents:
         m = QHeaderView::ResizeToContents;
         break;

      case QtTreePropertyBrowser::Stretch:
      default:
         m = QHeaderView::Stretch;
         break;
   }

   d_ptr->m_treeWidget->header()->setSectionResizeMode(m);
}

int QtTreePropertyBrowser::splitterPosition() const
{
   return d_ptr->m_treeWidget->header()->sectionSize(0);
}

void QtTreePropertyBrowser::setSplitterPosition(int position)
{
   d_ptr->m_treeWidget->header()->resizeSection(0, position);
}

void QtTreePropertyBrowser::setExpanded(QtBrowserItem *item, bool expanded)
{
   QTreeWidgetItem *treeItem = d_ptr->m_indexToItem.value(item);

   if (treeItem) {
      treeItem->setExpanded(expanded);
   }
}

bool QtTreePropertyBrowser::isExpanded(QtBrowserItem *item) const
{
   QTreeWidgetItem *treeItem = d_ptr->m_indexToItem.value(item);

   if (treeItem) {
      return treeItem->isExpanded();
   }

   return false;
}

bool QtTreePropertyBrowser::isItemVisible(QtBrowserItem *item) const
{
   if (const QTreeWidgetItem *treeItem = d_ptr->m_indexToItem.value(item)) {
      return !treeItem->isHidden();
   }

   return false;
}

void QtTreePropertyBrowser::setItemVisible(QtBrowserItem *item, bool visible)
{
   if (QTreeWidgetItem *treeItem = d_ptr->m_indexToItem.value(item)) {
      treeItem->setHidden(!visible);
   }
}

void QtTreePropertyBrowser::setBackgroundColor(QtBrowserItem *item, const QColor &color)
{
   if (! d_ptr->m_indexToItem.contains(item)) {
      return;
   }

   if (color.isValid()) {
      d_ptr->m_indexToBackgroundColor[item] = color;
   } else {
      d_ptr->m_indexToBackgroundColor.remove(item);
   }

   d_ptr->m_treeWidget->viewport()->update();
}

QColor QtTreePropertyBrowser::backgroundColor(QtBrowserItem *item) const
{
   return d_ptr->m_indexToBackgroundColor.value(item);
}

QColor QtTreePropertyBrowser::calculatedBackgroundColor(QtBrowserItem *item) const
{
   return d_ptr->calculatedBackgroundColor(item);
}

void QtTreePropertyBrowser::setPropertiesWithoutValueMarked(bool mark)
{
   if (d_ptr->m_markPropertiesWithoutValue == mark) {
      return;
   }

   d_ptr->m_markPropertiesWithoutValue = mark;
   QMapIterator<QTreeWidgetItem *, QtBrowserItem *> it(d_ptr->m_itemToIndex);

   while (it.hasNext()) {
      QtProperty *property = it.next().value()->property();

      if (!property->hasValue()) {
         d_ptr->updateItem(it.key());
      }
   }
   d_ptr->m_treeWidget->viewport()->update();
}

bool QtTreePropertyBrowser::propertiesWithoutValueMarked() const
{
   return d_ptr->m_markPropertiesWithoutValue;
}

void QtTreePropertyBrowser::itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem)
{
   d_ptr->propertyInserted(item, afterItem);
}

void QtTreePropertyBrowser::itemRemoved(QtBrowserItem *item)
{
   d_ptr->propertyRemoved(item);
}

void QtTreePropertyBrowser::itemChanged(QtBrowserItem *item)
{
   d_ptr->propertyChanged(item);
}

void QtTreePropertyBrowser::editItem(QtBrowserItem *item)
{
   d_ptr->editItem(item);
}

void QtTreePropertyBrowser::slotCollapsed(const QModelIndex &un_named_arg1)
{
   Q_D(QtTreePropertyBrowser);
   d->slotCollapsed(un_named_arg1);
}

void QtTreePropertyBrowser::slotExpanded(const QModelIndex &un_named_arg1)
{
   Q_D(QtTreePropertyBrowser);
   d->slotExpanded(un_named_arg1);
}

void QtTreePropertyBrowser::slotCurrentBrowserItemChanged(QtBrowserItem *un_named_arg1)
{
   Q_D(QtTreePropertyBrowser);
   d->slotCurrentBrowserItemChanged(un_named_arg1);
}

void QtTreePropertyBrowser::slotCurrentTreeItemChanged(QTreeWidgetItem *un_named_arg1, QTreeWidgetItem *un_named_arg2)
{
   Q_D(QtTreePropertyBrowser);
   d->slotCurrentTreeItemChanged(un_named_arg1, un_named_arg2);
}

