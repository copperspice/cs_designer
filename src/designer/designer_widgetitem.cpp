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
#include <abstract_formwindow.h>
#include <abstract_widgetdatabase.h>
#include <container.h>
#include <designer_widget.h>
#include <designer_widgetitem.h>
#include <extension.h>
#include <widgetfactory.h>

#include <QApplication>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QTextStream>
#include <QVBoxLayout>

constexpr const int MIN_LENGTH = 10;

// Widget item creation function to be registered as factory method with QLayoutPrivate

static QWidgetItem *createDesignerWidgetItem(const QLayout *layout, QWidget *widget)
{
   Qt::Orientations orientations;

   if (qdesigner_internal::QDesignerWidgetItem::check(layout, widget, &orientations)) {
      return new qdesigner_internal::QDesignerWidgetItem(layout, widget, orientations);
   }

   return nullptr;
}

// Find the layout the item is contained in, recursing over child layouts
static const QLayout *findLayoutOfItem(const QLayout *haystack, const QLayoutItem *needle)
{
   const int count = haystack->count();

   for (int i = 0; i < count; i++) {
      QLayoutItem *item = haystack->itemAt(i);
      if (item == needle) {
         return haystack;
      }

      if (QLayout *childLayout =  item->layout())
         if (const QLayout *containing = findLayoutOfItem(childLayout, needle)) {
            return containing;
         }
   }
   return nullptr;
}

namespace qdesigner_internal {

int QDesignerWidgetItemInstaller::m_instanceCount = 0;

QDesignerWidgetItem::QDesignerWidgetItem(const QLayout *containingLayout, QWidget *w, Qt::Orientations o)
   : QWidgetItemV2(w), m_orientations(o), m_nonLaidOutMinSize(w->minimumSizeHint()),
     m_nonLaidOutSizeHint(w->sizeHint()), m_cachedContainingLayout(containingLayout)
{
   // Initialize the minimum size to prevent nonlaid-out frames/widgets
   // from being slammed to zero
   const QSize minimumSize = w->minimumSize();

   if (! minimumSize.isEmpty()) {
      m_nonLaidOutMinSize = minimumSize;
   }

   expand(&m_nonLaidOutMinSize);
   expand(&m_nonLaidOutSizeHint);
   w->installEventFilter(this);

   connect(containingLayout, &QObject::destroyed, this, &QDesignerWidgetItem::layoutChanged);
}

void QDesignerWidgetItem::expand(QSize *s) const
{
   // Expand the size if its too small
   if (m_orientations & Qt::Horizontal && s->width() <= 0) {
      s->setWidth(MIN_LENGTH);
   }

   if (m_orientations & Qt::Vertical && s->height() <= 0) {
      s->setHeight(MIN_LENGTH);
   }
}

QSize QDesignerWidgetItem::minimumSize() const
{
   // Just track the size in case we are laid-out or stretched.
   const QSize baseMinSize = QWidgetItemV2::minimumSize();
   QWidget *w = constWidget();

   if (w->layout() || subjectToStretch(containingLayout(), w)) {
      m_nonLaidOutMinSize = baseMinSize;
      return baseMinSize;
   }

   // Nonlaid out: Maintain last laid-out size
   const QSize rc = baseMinSize.expandedTo(m_nonLaidOutMinSize);

   return rc;
}

QSize QDesignerWidgetItem::sizeHint()    const
{
   // Just track the size in case we are laid-out or stretched.
   const QSize baseSizeHint = QWidgetItemV2::sizeHint();
   QWidget *w = constWidget();

   if (w->layout() || subjectToStretch(containingLayout(), w)) {
      m_nonLaidOutSizeHint = baseSizeHint;
      return baseSizeHint;
   }

   // Nonlaid out: Maintain last laid-out size
   const QSize rc = baseSizeHint.expandedTo(m_nonLaidOutSizeHint);

   return rc;
}

bool QDesignerWidgetItem::subjectToStretch(const QLayout *layout, QWidget *w)
{
   if (!layout) {
      return false;
   }

   // Are we under some stretch factor?
   if (const QBoxLayout *bl = dynamic_cast<const QBoxLayout *>(layout)) {
      const int index = bl->indexOf(w);
      Q_ASSERT(index != -1);
      return bl->stretch(index) != 0;
   }

   if (const QGridLayout *cgl = dynamic_cast<const QGridLayout *>(layout)) {
      QGridLayout *gl = const_cast<QGridLayout *>(cgl);
      const int index = cgl->indexOf(w);
      Q_ASSERT(index != -1);
      int row, column, rowSpan, columnSpan;
      gl->getItemPosition (index, &row, &column, &rowSpan, &columnSpan);
      const int rend = row + rowSpan;
      const int cend = column + columnSpan;
      for (int r = row; r < rend; r++)
         if (cgl->rowStretch(r) != 0) {
            return true;
         }
      for (int c = column; c < cend; c++)
         if (cgl->columnStretch(c) != 0) {
            return true;
         }
   }
   return false;
}

/* Return the orientations mask for a layout, specifying
 * in which directions squeezing should be prevented. */
static Qt::Orientations layoutOrientation(const QLayout *layout)
{
   if (const QBoxLayout *bl = dynamic_cast<const QBoxLayout *>(layout)) {
      const QBoxLayout::Direction direction = bl->direction();
      return direction == QBoxLayout::LeftToRight || direction == QBoxLayout::RightToLeft ? Qt::Horizontal : Qt::Vertical;
   }
   if (dynamic_cast<const QFormLayout *>(layout)) {
      return  Qt::Vertical;
   }
   return Qt::Horizontal | Qt::Vertical;
}

// Check for a non-container extension container
bool  QDesignerWidgetItem::isContainer(const QDesignerFormEditorInterface *core, QWidget *w)
{
   if (!WidgetFactory::isFormEditorObject(w)) {
      return false;
   }
   const QDesignerWidgetDataBaseInterface *wdb = core->widgetDataBase();
   const int widx = wdb->indexOfObject(w);
   if (widx == -1 || !wdb->item(widx)->isContainer()) {
      return false;
   }
   if (qt_extension<QDesignerContainerExtension *>(core->extensionManager(), w)) {
      return false;
   }
   return true;
}

bool QDesignerWidgetItem::check(const QLayout *layout, QWidget *w, Qt::Orientations *ptrToOrientations)
{
   // Check for form-editor non-containerextension-containers (QFrame, etc)
   // within laid-out form editor widgets. No check for managed() here as we
   // want container pages and widgets in the process of being morphed as
   // well. Avoid nested layouts (as the effective stretch cannot be easily
   // computed and may mess things up).

   if (ptrToOrientations) {
      *ptrToOrientations = nullptr;
   }

   const QObject *layoutParent = layout->parent();
   if (! layoutParent || !layoutParent->isWidgetType() || !WidgetFactory::isFormEditorObject(layoutParent)) {
      return false;
   }

   QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(w);
   if (!fw || !isContainer(fw->core(), w)) {
      return false;
   }

   // If it is a box, restrict to its orientation
   if (ptrToOrientations) {
      *ptrToOrientations = layoutOrientation(layout);
   }

   return true;
}

QSize QDesignerWidgetItem::nonLaidOutMinSize() const
{
   return m_nonLaidOutMinSize;
}

void QDesignerWidgetItem::setNonLaidOutMinSize(const QSize &s)
{
   m_nonLaidOutMinSize = s;
}

QSize QDesignerWidgetItem::nonLaidOutSizeHint() const
{
   return m_nonLaidOutSizeHint;
}

void QDesignerWidgetItem::setNonLaidOutSizeHint(const QSize &s)
{
   m_nonLaidOutSizeHint = s;
}

void QDesignerWidgetItem::install()
{
   QLayout::setWidgetItemFactory(createDesignerWidgetItem);
}

void QDesignerWidgetItem::deinstall()
{
   QLayout::setWidgetItemFactory(nullptr);
}

const QLayout *QDesignerWidgetItem::containingLayout() const
{
   if (! m_cachedContainingLayout) {

      if (QWidget *parentWidget = constWidget()->parentWidget()) {
         if (QLayout *parentLayout = parentWidget->layout()) {
            m_cachedContainingLayout = findLayoutOfItem(parentLayout, this);

            if (m_cachedContainingLayout) {
               connect(m_cachedContainingLayout, &QObject::destroyed,
                  this, &QDesignerWidgetItem::layoutChanged);
            }
         }
      }
   }

   return m_cachedContainingLayout;
}

void QDesignerWidgetItem::layoutChanged()
{
   m_cachedContainingLayout = nullptr;
}

bool QDesignerWidgetItem::eventFilter(QObject *, QEvent *event)
{
   if (event->type() == QEvent::ParentChange) {
      layoutChanged();
   }

   return false;
}

QDesignerWidgetItemInstaller::QDesignerWidgetItemInstaller()
{
   if (m_instanceCount == 0) {
      QDesignerWidgetItem::install();
   }

   ++m_instanceCount;
}

QDesignerWidgetItemInstaller::~QDesignerWidgetItemInstaller()
{
   if (m_instanceCount == 0) {
      QDesignerWidgetItem::deinstall();
   }

   --m_instanceCount;
}

}   // end namespace - qdesigner_internal
