/***********************************************************************
*
* Copyright (c) 2021-2022 Barbara Geller
* Copyright (c) 2021-2022 Ansel Sermersheim
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

#ifndef DESIGNERWIDGETITEM_H
#define DESIGNERWIDGETITEM_H

#include <QLayoutItem>
#include <QObject>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

// QDesignerWidgetItem: A Layout Item that is used for non-containerextension-
// containers (QFrame, etc) on Designer forms. It prevents its widget
// from being slammed to size 0 if the widget has no layout:

// Pre 4.5, this item ensured only that QWidgets and QFrames were not squeezed
// to size 0 since they have an invalid size hint when non-laid out.

// Since 4.5, the item is used for every  non-containerextension-container.
// In case the container has itself a layout, it merely tracks the minimum
// size. If the container has no layout and is not subject to some stretch
// factor, it will return the last valid size. The effect is that after
// breaking a layout on a container within a layout, it just maintains its
// last size and is not slammed to 0,0. In addition, it can be resized.
// The class keeps track of the containing layout by tracking widget reparent
// and destroyed slots as Designer will for example re-create grid layouts to
// shrink them.

class QDesignerWidgetItem : public QObject, public QWidgetItemV2
{
   Q_DISABLE_COPY(QDesignerWidgetItem)
   CS_OBJECT(QDesignerWidgetItem)

 public:
   explicit QDesignerWidgetItem(const QLayout *containingLayout, QWidget *w, Qt::Orientations o = Qt::Horizontal | Qt::Vertical);

   const QLayout *containingLayout() const;

   inline QWidget *constWidget() const {
      return const_cast<QDesignerWidgetItem *>(this)->widget();
   }

   QSize minimumSize() const override;
   QSize sizeHint()    const override;

   // Resize: Takes effect if the contained widget does not have a layout
   QSize nonLaidOutMinSize() const;
   void setNonLaidOutMinSize(const QSize &s);

   QSize nonLaidOutSizeHint() const;
   void setNonLaidOutSizeHint(const QSize &s);

   // Check whether a QDesignerWidgetItem should be installed
   static bool check(const QLayout *layout, QWidget *w, Qt::Orientations *ptrToOrientations = 0);

   // Register itself using QLayoutPrivate's widget item factory method hook
   static void install();
   static void deinstall();

   // Check for a non-container extension container
   static bool isContainer(const QDesignerFormEditorInterface *core, QWidget *w);

   static bool subjectToStretch(const QLayout *layout, QWidget *w);

   bool eventFilter(QObject *watched, QEvent *event) override;

 private:
   CS_SLOT_1(Private, void layoutChanged())
   CS_SLOT_2(layoutChanged)

   void expand(QSize *s) const;
   bool subjectToStretch() const;

   const Qt::Orientations m_orientations;
   mutable QSize m_nonLaidOutMinSize;
   mutable QSize m_nonLaidOutSizeHint;
   mutable const QLayout *m_cachedContainingLayout;
};

// Helper class that ensures QDesignerWidgetItem is installed while an
// instance is in scope.

class QDesignerWidgetItemInstaller
{
   Q_DISABLE_COPY(QDesignerWidgetItemInstaller)

 public:
   QDesignerWidgetItemInstaller();
   ~QDesignerWidgetItemInstaller();

 private:
   static int m_instanceCount;
};

} // namespace qdesigner_internal

#endif
