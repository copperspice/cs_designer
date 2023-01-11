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

#ifndef QLAYOUT_WIDGET_H
#define QLAYOUT_WIDGET_H

#include <layout_widget.h>
#include <layoutdecoration.h>

#include <QLayout>
#include <QPointer>
#include <QVariant>
#include <QWidget>

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;
class QGridLayout;
class QFormLayout;

namespace qdesigner_internal {

//  LayoutProperties: Helper struct that stores all layout-relevant properties
//  with functions to retrieve and apply to property sheets. Can be used to store the state
//  for undo commands and while rebuilding layouts.

struct LayoutProperties {
   LayoutProperties();
   void clear();

   enum Margins { LeftMargin, TopMargin, RightMargin, BottomMargin, MarginCount };
   enum Spacings { Spacing, HorizSpacing, VertSpacing, SpacingsCount };

   enum PropertyMask {
      ObjectNameProperty  = 0x1,
      LeftMarginProperty = 0x2, TopMarginProperty = 0x4, RightMarginProperty = 0x8, BottomMarginProperty = 0x10,
      SpacingProperty = 0x20, HorizSpacingProperty = 0x40, VertSpacingProperty = 0x80,
      SizeConstraintProperty = 0x100,
      FieldGrowthPolicyProperty = 0x200, RowWrapPolicyProperty = 0x400, LabelAlignmentProperty = 0x0800, FormAlignmentProperty = 0x1000,
      BoxStretchProperty = 0x2000, GridRowStretchProperty = 0x4000, GridColumnStretchProperty = 0x8000,
      GridRowMinimumHeightProperty = 0x10000, GridColumnMinimumWidthProperty = 0x20000,
      AllProperties = 0xFFFF
   };

   // return a PropertyMask of visible properties
   static int visibleProperties(const QLayout *layout);

   // Retrieve from /apply to sheet: A property mask is returned indicating the properties found in the sheet
   int fromPropertySheet(const QDesignerFormEditorInterface *core, QLayout *l, int mask = AllProperties);
   int toPropertySheet(const QDesignerFormEditorInterface *core, QLayout *l, int mask = AllProperties, bool applyChanged = true) const;

   int m_margins[MarginCount];
   bool m_marginsChanged[MarginCount];

   int m_spacings[SpacingsCount];
   bool m_spacingsChanged[SpacingsCount];

   QVariant m_objectName; // receives a PropertySheetStringValue
   bool m_objectNameChanged;
   QVariant m_sizeConstraint;
   bool m_sizeConstraintChanged;

   bool m_fieldGrowthPolicyChanged;
   QVariant m_fieldGrowthPolicy;
   bool m_rowWrapPolicyChanged;
   QVariant m_rowWrapPolicy;
   bool m_labelAlignmentChanged;
   QVariant m_labelAlignment;
   bool m_formAlignmentChanged;
   QVariant m_formAlignment;

   bool m_boxStretchChanged;
   QVariant m_boxStretch;

   bool m_gridRowStretchChanged;
   QVariant m_gridRowStretch;

   bool m_gridColumnStretchChanged;
   QVariant m_gridColumnStretch;

   bool m_gridRowMinimumHeightChanged;
   QVariant m_gridRowMinimumHeight;

   bool m_gridColumnMinimumWidthChanged;
   QVariant m_gridColumnMinimumWidth;
};

// -- LayoutHelper: For use with the 'insert widget'/'delete widget' command,
//    able to store and restore states.
//    This could become part of 'QDesignerLayoutDecorationExtensionV2',
//    but to keep any existing old extensions working, it is provided as
//    separate class with a factory function.
class LayoutHelper
{

 public:
   virtual ~LayoutHelper();

   static LayoutHelper *createLayoutHelper(int type);

   static int indexOf(const QLayout *lt, const QWidget *widget);

   // Return area of an item (x == columns)
   QRect itemInfo(QLayout *lt, const QWidget *widget) const;

   virtual QRect itemInfo(QLayout *lt, int index) const = 0;
   virtual void insertWidget(QLayout *lt, const QRect &info, QWidget *w) = 0;
   virtual void removeWidget(QLayout *lt, QWidget *widget) = 0;

   // The 'morphing' feature requires an API for replacing widgets on layouts.
   virtual void replaceWidget(QLayout *lt, QWidget *before, QWidget *after) = 0;

   // Simplify a grid, remove empty columns, rows within the rectangle
   // The DeleteWidget command restricts the area to be simplified.
   virtual bool canSimplify(const QDesignerFormEditorInterface *core, const QWidget *widgetWithManagedLayout,
      const QRect &restrictionArea) const = 0;
   virtual void simplify(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout, const QRect &restrictionArea) = 0;

   // Push and pop a state. Can be used for implementing undo for simplify/row, column
   // insertion commands, provided that the widgets remain the same.
   virtual void pushState(const QDesignerFormEditorInterface *core, const QWidget *widgetWithManagedLayout)  = 0;
   virtual void popState(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout) = 0;

 protected:
   LayoutHelper();

};

// Base class for layout decoration extensions.
class QLayoutSupport: public QObject, public QDesignerLayoutDecorationExtension
{
   CS_OBJECT(QLayoutSupport)

   CS_INTERFACES(QDesignerLayoutDecorationExtension)

 public:
   virtual ~QLayoutSupport();

   inline QDesignerFormWindowInterface *formWindow() const   {
      return m_formWindow;
   }

   // DecorationExtension V2
   LayoutHelper *helper() const                              {
      return m_helper;
   }

   // DecorationExtension
   int currentIndex() const override                  {
      return m_currentIndex;
   }

   InsertMode currentInsertMode() const override      {
      return m_currentInsertMode;
   }

   QPair<int, int> currentCell() const  override      {
      return m_currentCell;
   }

   int findItemAt(const QPoint &pos) const override;
   int indexOf(QWidget *widget) const override;
   int indexOf(QLayoutItem *item) const override;

   void adjustIndicator(const QPoint &pos, int index) override;

   QList<QWidget *> widgets(QLayout *layout) const override;

   // Pad empty cells with dummy spacers. Called by layouting commands.
   static void createEmptyCells(QGridLayout *gridLayout);
   // remove dummy spacers in the area. Returns false if there are non-empty items in the way
   static bool removeEmptyCells(QGridLayout *gridLayout, const QRect &area);
   static void createEmptyCells(QFormLayout *formLayout); // ditto.
   static bool removeEmptyCells(QFormLayout *formLayout, const QRect &area);

   // grid helpers: find item index
   static int findItemAt(QGridLayout *, int row, int column);
   // grid helpers: Quick check whether simplify should be enabled for grids. May return false positives.
   static bool canSimplifyQuickCheck(const QGridLayout *);
   static bool canSimplifyQuickCheck(const QFormLayout *fl);
   // Factory function, create layout support according to layout type of widget
   static QLayoutSupport *createLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, QObject *parent = nullptr);

 protected:
   QLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, LayoutHelper *helper, QObject *parent = nullptr);

   // figure out insertion position and mode from indicator on empty cell if supported
   virtual void setCurrentCellFromIndicatorOnEmptyCell(int index) = 0;

   // figure out insertion position and mode from indicator
   virtual void setCurrentCellFromIndicator(Qt::Orientation indicatorOrientation, int index, int increment) = 0;

   // Overwrite to return the extended geometry of an item, that is,
   // if it is a border item, include the widget border for the indicator to work correctly
   virtual QRect extendedGeometry(int index) const = 0;
   virtual bool supportsIndicatorOrientation(Qt::Orientation indicatorOrientation) const = 0;

   QRect itemInfo(int index) const override;
   QLayout *layout() const;
   QGridLayout *gridLayout() const;
   QWidget *widget() const              {
      return m_widget;
   }

   void setInsertMode(InsertMode im);
   void setCurrentCell(const QPair<int, int> &cell);

 private:
   enum Indicator { LeftIndicator, TopIndicator, RightIndicator, BottomIndicator, NumIndicators };

   void hideIndicator(Indicator i);
   void showIndicator(Indicator i, const QRect &geometry, const QPalette &);

   QDesignerFormWindowInterface *m_formWindow;
   LayoutHelper *m_helper;

   QPointer<QWidget> m_widget;
   QPointer<QWidget> m_indicators[NumIndicators];
   int m_currentIndex;
   InsertMode m_currentInsertMode;
   QPair<int, int> m_currentCell;
};
} // namespace qdesigner_internal

// Red layout widget.
class QLayoutWidget: public QWidget
{
   CS_OBJECT(QLayoutWidget)

 public:
   explicit QLayoutWidget(QDesignerFormWindowInterface *formWindow, QWidget *parent = nullptr);

   int layoutLeftMargin() const;
   void setLayoutLeftMargin(int layoutMargin);

   int layoutTopMargin() const;
   void setLayoutTopMargin(int layoutMargin);

   int layoutRightMargin() const;
   void setLayoutRightMargin(int layoutMargin);

   int layoutBottomMargin() const;
   void setLayoutBottomMargin(int layoutMargin);

   inline QDesignerFormWindowInterface *formWindow() const    {
      return m_formWindow;
   }

 protected:
   bool event(QEvent *e) override;
   void paintEvent(QPaintEvent *e) override;

 private:
   QDesignerFormWindowInterface *m_formWindow;
   int m_leftMargin;
   int m_topMargin;
   int m_rightMargin;
   int m_bottomMargin;
};

#endif
