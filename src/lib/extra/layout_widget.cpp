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

#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <abstract_widgetfactory.h>
#include <designer_propertysheet.h>
#include <designer_utils.h>
#include <designer_widgetitem.h>
#include <extension.h>
#include <layout.h>
#include <layout_info.h>
#include <layout_widget.h>

#include <invisible_widget_p.h>

#include <QApplication>
#include <QEvent>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMap>
#include <QPainter>
#include <QPair>
#include <QSet>
#include <QStack>
#include <QVBoxLayout>

#include <qalgorithms.h>

#include <algorithm>

constexpr const int MIN_MARGIN      = 1;
constexpr const int COLUMNS_IN_FORM = 2;
constexpr const int INDICATOR_SIZE  = 2;

// Grid/form Helpers: get info (overloads to make templates work)

namespace {

// overloads to make templates over QGridLayout/QFormLayout work
inline int gridRowCount(const QGridLayout *gridLayout)
{
   return  gridLayout->rowCount();
}

inline int gridColumnCount(const QGridLayout *gridLayout)
{
   return  gridLayout->columnCount();
}

// QGridLayout/QFormLayout Helpers: get item position (overloads to make templates work)
inline void getGridItemPosition(QGridLayout *gridLayout, int index,
   int *row, int *column, int *rowspan, int *colspan)
{
   gridLayout->getItemPosition(index, row, column, rowspan, colspan);
}

QRect gridItemInfo(QGridLayout *grid, int index)
{
   int row, column, rowSpan, columnSpan;
   // getItemPosition is not const, grmbl..
   grid->getItemPosition(index, &row, &column, &rowSpan, &columnSpan);
   return QRect(column, row, columnSpan, rowSpan);
}

inline int gridRowCount(const QFormLayout *formLayout)
{
   return  formLayout->rowCount();
}

inline int gridColumnCount(const QFormLayout *)
{
   return COLUMNS_IN_FORM;
}

inline void getGridItemPosition(QFormLayout *formLayout, int index, int *row, int *column, int *rowspan, int *colspan)
{
   qdesigner_internal::getFormLayoutItemPosition(formLayout, index, row, column, rowspan, colspan);
}

/*
QRect gridItemInfo(const QFormLayout *form, int index)
{
   int row;
   int column;
   int colspan;
   qdesigner_internal::getFormLayoutItemPosition(form, index, &row, &column, nullptr, &colspan);

   return QRect(column, row, colspan, 1);
}
*/

}   // end namespace anonymous


static const QString objectNameC     = "objectName";
static const QString sizeConstraintC = "sizeConstraint";

/* A padding spacer element that is used to represent an empty form layout cell. It should grow with its cell.
 * Should not be used on a grid as it causes resizing inconsistencies */
namespace qdesigner_internal {
class PaddingSpacerItem : public QSpacerItem
{
 public:
   PaddingSpacerItem()
      : QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding)
   {
   }

   Qt::Orientations expandingDirections () const override {
      return Qt::Vertical | Qt::Horizontal;
   }
};

}

static inline QSpacerItem *createGridSpacer()
{
   return new QSpacerItem(0, 0);
}

static inline QSpacerItem *createFormSpacer()
{
   return new qdesigner_internal::PaddingSpacerItem;
}

static inline bool isEmptyFormLayoutRow(const QFormLayout *fl, int row)
{
   // Spanning can never be empty
   if (fl->itemAt(row, QFormLayout::SpanningRole)) {
      return false;
   }

   return qdesigner_internal::LayoutInfo::isEmptyItem(fl->itemAt(row, QFormLayout::LabelRole)) &&
      qdesigner_internal::LayoutInfo::isEmptyItem(fl->itemAt(row, QFormLayout::FieldRole));
}

static inline bool canSimplifyFormLayout(const QFormLayout *formLayout, const QRect &restrictionArea)
{
   if (restrictionArea.x() >= COLUMNS_IN_FORM) {
      return false;
   }

   // Try to find empty rows
   const int bottomCheckRow = qMin(formLayout->rowCount(), restrictionArea.top() + restrictionArea.height());

   for (int r = restrictionArea.y(); r < bottomCheckRow; ++r) {
      if (isEmptyFormLayoutRow(formLayout, r)) {
         return true;
      }
   }

   return false;
}

// recreate a managed layout (which does not automagically remove
// empty rows/columns like grid or form layout) in case it needs to shrink

static QLayout *recreateManagedLayout(const QDesignerFormEditorInterface *core, QWidget *w, QLayout *lt)
{
   const qdesigner_internal::LayoutInfo::Type t = qdesigner_internal::LayoutInfo::layoutType(core, lt);
   qdesigner_internal::LayoutProperties properties;

   const int mask = properties.fromPropertySheet(core, lt, qdesigner_internal::LayoutProperties::AllProperties);
   qdesigner_internal::LayoutInfo::deleteLayout(core, w);

   QLayout *rc = core->widgetFactory()->createLayout(w, nullptr, t);
   properties.toPropertySheet(core, rc, mask, true);

   return rc;
}

// QGridLayout/QFormLayout Helpers: find an item on a form/grid. Return index
template <class GridLikeLayout>
int findGridItemAt(GridLikeLayout *gridLayout, int at_row, int at_column)
{
   Q_ASSERT(gridLayout);
   const int count = gridLayout->count();

   for (int index = 0; index <  count; ++index) {
      int row, column, rowspan, colspan;
      getGridItemPosition(gridLayout, index, &row, &column, &rowspan, &colspan);

      if (at_row >= row && at_row < (row + rowspan)
         && at_column >= column && at_column < (column + colspan)) {
         return index;
      }
   }

   return -1;
}
// QGridLayout/QFormLayout  Helpers: remove dummy spacers on form/grid
template <class GridLikeLayout>
static bool removeEmptyCellsOnGrid(GridLikeLayout *grid, const QRect &area)
{
   // check if there are any items in the way. Should be only spacers
   // Unique out items that span rows/columns.

   QVector<int> indexesToBeRemoved;
   indexesToBeRemoved.reserve(grid->count());

   const int rightColumn = area.x() + area.width();
   const int bottomRow = area.y() + area.height();

   for (int c = area.x(); c < rightColumn; ++c) {
      for (int r = area.y(); r < bottomRow; ++r) {
         const int index = findGridItemAt(grid, r, c);

         if (index != -1)
            if (QLayoutItem *item = grid->itemAt(index)) {
               if (qdesigner_internal::LayoutInfo::isEmptyItem(item)) {
                  if (indexesToBeRemoved.indexOf(index) == -1) {
                     indexesToBeRemoved.push_back(index);
                  }
               } else {
                  return false;
               }
            }
      }
   }

   // remove, starting from last
   if (!indexesToBeRemoved.empty()) {
      std::stable_sort(indexesToBeRemoved.begin(), indexesToBeRemoved.end());
      for (int i = indexesToBeRemoved.size() - 1; i >= 0; i--) {
         delete grid->takeAt(indexesToBeRemoved[i]);
      }
   }
   return true;
}

namespace qdesigner_internal {
// --------- LayoutProperties

LayoutProperties::LayoutProperties()
{
   clear();
}

void LayoutProperties::clear()
{
   std::fill(m_margins, m_margins + MarginCount, 0);
   std::fill(m_marginsChanged, m_marginsChanged + MarginCount, false);
   std::fill(m_spacings, m_spacings + SpacingsCount, 0);
   std::fill(m_spacingsChanged, m_spacingsChanged + SpacingsCount, false);

   m_objectName = QVariant();
   m_objectNameChanged = false;
   m_sizeConstraint = QVariant(QLayout::SetDefaultConstraint);
   m_sizeConstraintChanged = false;

   m_fieldGrowthPolicyChanged = m_rowWrapPolicyChanged =  m_labelAlignmentChanged = m_formAlignmentChanged = false;
   m_fieldGrowthPolicy =  m_rowWrapPolicy =  m_formAlignment = QVariant();

   m_boxStretchChanged = m_gridRowStretchChanged = m_gridColumnStretchChanged = m_gridRowMinimumHeightChanged = false;
   m_boxStretch = m_gridRowStretch =  m_gridColumnStretch =  m_gridRowMinimumHeight = QVariant();
}

int LayoutProperties::visibleProperties(const  QLayout *layout)
{
   // Grid like layout have 2 spacings.
   const bool isFormLayout = dynamic_cast<const QFormLayout *>(layout);
   const bool isGridLike = dynamic_cast<const QGridLayout *>(layout) || isFormLayout;
   int rc = ObjectNameProperty | LeftMarginProperty | TopMarginProperty | RightMarginProperty | BottomMarginProperty |
      SizeConstraintProperty;

   rc |= isGridLike ? (HorizSpacingProperty | VertSpacingProperty) : SpacingProperty;
   if (isFormLayout) {
      rc |= FieldGrowthPolicyProperty | RowWrapPolicyProperty | LabelAlignmentProperty | FormAlignmentProperty;
   } else {
      if (isGridLike) {
         rc |=  GridRowStretchProperty | GridColumnStretchProperty | GridRowMinimumHeightProperty | GridColumnMinimumWidthProperty;
      } else {
         rc |=  BoxStretchProperty;
      }
   }
   return rc;
}

static const QString marginPropertyNamesC[]          = {"leftMargin", "topMargin", "rightMargin", "bottomMargin"};
static const QString spacingPropertyNamesC[]         = {"spacing", "horizontalSpacing", "verticalSpacing" };
static const QString fieldGrowthPolicyPropertyC      = "fieldGrowthPolicy";
static const QString rowWrapPolicyPropertyC          = "rowWrapPolicy";
static const QString labelAlignmentPropertyC         = "labelAlignment";
static const QString formAlignmentPropertyC          = "formAlignment";
static const QString boxStretchPropertyC             = "stretch";
static const QString gridRowStretchPropertyC         = "rowStretch";
static const QString gridColumnStretchPropertyC      =  "columnStretch";
static const QString gridRowMinimumHeightPropertyC   = "rowMinimumHeight";
static const QString gridColumnMinimumWidthPropertyC = "columnMinimumWidth";

static bool intValueFromSheet(const QDesignerPropertySheetExtension *sheet, const QString &name, int *value, bool *changed)
{
   const int sheetIndex = sheet->indexOf(name);
   if (sheetIndex == -1) {
      return false;
   }
   *value = sheet->property(sheetIndex).toInt();
   *changed = sheet->isChanged(sheetIndex);
   return true;
}

static void variantPropertyFromSheet(int mask, int flag, const QDesignerPropertySheetExtension *sheet, const QString &name,
   QVariant *value, bool *changed, int *returnMask)
{
   if (mask & flag) {
      const int sIndex = sheet->indexOf(name);

      if (sIndex != -1) {
         *value = sheet->property(sIndex);
         *changed = sheet->isChanged(sIndex);
         *returnMask |= flag;
      }
   }
}

int LayoutProperties::fromPropertySheet(const QDesignerFormEditorInterface *core, QLayout *l, int mask)
{
   int rc = 0;
   const QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), l);
   Q_ASSERT(sheet);

   // name
   if (mask & ObjectNameProperty) {
      const int nameIndex = sheet->indexOf(objectNameC);
      Q_ASSERT(nameIndex != -1);

      m_objectName = sheet->property(nameIndex);
      m_objectNameChanged =  sheet->isChanged(nameIndex);
      rc |= ObjectNameProperty;
   }

   // -- Margins
   const int marginFlags[MarginCount] = { LeftMarginProperty, TopMarginProperty, RightMarginProperty, BottomMarginProperty};

   for (int i = 0; i < MarginCount; ++i) {
      if (mask & marginFlags[i]) {
         if (intValueFromSheet(sheet, marginPropertyNamesC[i], m_margins + i, m_marginsChanged + i)) {
            rc |= marginFlags[i];
         }
      }
   }

   const int spacingFlags[] = { SpacingProperty, HorizSpacingProperty, VertSpacingProperty};

   for (int i = 0; i < SpacingsCount; i++) {
      if (mask & spacingFlags[i]) {
         if (intValueFromSheet(sheet, spacingPropertyNamesC[i], m_spacings + i, m_spacingsChanged + i)) {
            rc |= spacingFlags[i];
         }
      }
   }

   // sizeConstraint, flags
   variantPropertyFromSheet(mask, SizeConstraintProperty, sheet, sizeConstraintC, &m_sizeConstraint, &m_sizeConstraintChanged, &rc);
   variantPropertyFromSheet(mask, FieldGrowthPolicyProperty, sheet, fieldGrowthPolicyPropertyC, &m_fieldGrowthPolicy,
      &m_fieldGrowthPolicyChanged, &rc);

   variantPropertyFromSheet(mask, RowWrapPolicyProperty, sheet, rowWrapPolicyPropertyC, &m_rowWrapPolicy, &m_rowWrapPolicyChanged, &rc);
   variantPropertyFromSheet(mask, LabelAlignmentProperty, sheet, labelAlignmentPropertyC, &m_labelAlignment, &m_labelAlignmentChanged,
      &rc);

   variantPropertyFromSheet(mask, FormAlignmentProperty, sheet, formAlignmentPropertyC, &m_formAlignment, &m_formAlignmentChanged, &rc);
   variantPropertyFromSheet(mask, BoxStretchProperty, sheet, boxStretchPropertyC, &m_boxStretch, & m_boxStretchChanged, &rc);
   variantPropertyFromSheet(mask, GridRowStretchProperty, sheet, gridRowStretchPropertyC, &m_gridRowStretch, &m_gridRowStretchChanged,
      &rc);

   variantPropertyFromSheet(mask, GridColumnStretchProperty, sheet, gridColumnStretchPropertyC, &m_gridColumnStretch,
      &m_gridColumnStretchChanged, &rc);

   variantPropertyFromSheet(mask, GridRowMinimumHeightProperty, sheet, gridRowMinimumHeightPropertyC, &m_gridRowMinimumHeight,
      &m_gridRowMinimumHeightChanged, &rc);

   variantPropertyFromSheet(mask, GridColumnMinimumWidthProperty, sheet, gridColumnMinimumWidthPropertyC, &m_gridColumnMinimumWidth,
      &m_gridColumnMinimumWidthChanged, &rc);

   return rc;
}

static bool intValueToSheet(QDesignerPropertySheetExtension *sheet, const QString &name, int value, bool changed, bool applyChanged)
{
   const int sheetIndex = sheet->indexOf(name);

   if (sheetIndex == -1) {
      qWarning() << "intValueToSheet() Unable to set property " << name << ", does not exist in the current layout";
      return false;
   }

   sheet->setProperty(sheetIndex, QVariant(value));

   if (applyChanged) {
      sheet->setChanged(sheetIndex, changed);
   }

   return true;
}

static void variantPropertyToSheet(int mask, int flag, bool applyChanged, QDesignerPropertySheetExtension *sheet,
      const QString &name, const QVariant &value, bool changed, int *returnMask)
{
   if (mask & flag) {
      const int sIndex = sheet->indexOf(name);

      if (sIndex != -1) {
         sheet->setProperty(sIndex, value);

         if (applyChanged) {
            sheet->setChanged(sIndex, changed);
         }
         *returnMask |= flag;
      }
   }
}

int LayoutProperties::toPropertySheet(const QDesignerFormEditorInterface *core, QLayout *l, int mask, bool applyChanged) const
{
   int rc = 0;
   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), l);
   Q_ASSERT(sheet);

   // name
   if (mask & ObjectNameProperty) {
      const int nameIndex = sheet->indexOf(objectNameC);
      Q_ASSERT(nameIndex != -1);
      sheet->setProperty(nameIndex, m_objectName);
      if (applyChanged) {
         sheet->setChanged(nameIndex, m_objectNameChanged);
      }
      rc |= ObjectNameProperty;
   }

   // margins
   const int marginFlags[MarginCount] = { LeftMarginProperty, TopMarginProperty, RightMarginProperty, BottomMarginProperty};
   for (int i = 0; i < MarginCount; i++) {
      if (mask & marginFlags[i]) {
         if (intValueToSheet(sheet, marginPropertyNamesC[i], m_margins[i], m_marginsChanged[i], applyChanged)) {
            rc |= marginFlags[i];
         }
      }
   }

   const int spacingFlags[] = { SpacingProperty, HorizSpacingProperty, VertSpacingProperty};
   for (int i = 0; i < SpacingsCount; i++) {
      if (mask & spacingFlags[i]) {
         if (intValueToSheet(sheet, spacingPropertyNamesC[i], m_spacings[i], m_spacingsChanged[i], applyChanged)) {
            rc |= spacingFlags[i];
         }
      }
   }

   // sizeConstraint
   variantPropertyToSheet(mask, SizeConstraintProperty, applyChanged, sheet, sizeConstraintC, m_sizeConstraint,
         m_sizeConstraintChanged, &rc);

   variantPropertyToSheet(mask, FieldGrowthPolicyProperty, applyChanged, sheet, fieldGrowthPolicyPropertyC, m_fieldGrowthPolicy,
         m_fieldGrowthPolicyChanged, &rc);
   variantPropertyToSheet(mask, RowWrapPolicyProperty, applyChanged, sheet, rowWrapPolicyPropertyC, m_rowWrapPolicy,
         m_rowWrapPolicyChanged, &rc);
   variantPropertyToSheet(mask, LabelAlignmentProperty, applyChanged, sheet, labelAlignmentPropertyC, m_labelAlignment,
         m_labelAlignmentChanged, &rc);
   variantPropertyToSheet(mask, FormAlignmentProperty, applyChanged, sheet, formAlignmentPropertyC, m_formAlignment,
         m_formAlignmentChanged, &rc);

   variantPropertyToSheet(mask, BoxStretchProperty, applyChanged, sheet, boxStretchPropertyC, m_boxStretch,
         m_boxStretchChanged, &rc);
   variantPropertyToSheet(mask, GridRowStretchProperty, applyChanged, sheet, gridRowStretchPropertyC, m_gridRowStretch,
         m_gridRowStretchChanged, &rc);

   variantPropertyToSheet(mask, GridColumnStretchProperty, applyChanged, sheet, gridColumnStretchPropertyC, m_gridColumnStretch,
         m_gridColumnStretchChanged, &rc);

   variantPropertyToSheet(mask, GridRowMinimumHeightProperty, applyChanged, sheet, gridRowMinimumHeightPropertyC,
         m_gridRowMinimumHeight, m_gridRowMinimumHeightChanged, &rc);

   variantPropertyToSheet(mask, GridColumnMinimumWidthProperty, applyChanged, sheet, gridColumnMinimumWidthPropertyC,
         m_gridColumnMinimumWidth, m_gridColumnMinimumWidthChanged, &rc);

   return rc;
}

LayoutHelper::LayoutHelper()
{
}

LayoutHelper::~LayoutHelper()
{
}

int LayoutHelper::indexOf(const QLayout *lt, const QWidget *widget)
{
   if (!lt) {
      return -1;
   }

   const int itemCount = lt->count();

   for (int i = 0; i < itemCount; i++) {
      if (lt->itemAt(i)->widget() == widget) {
         return i;
      }
   }

   return -1;
}

QRect LayoutHelper::itemInfo(QLayout *lt, const QWidget *widget) const
{
   const int index = indexOf(lt, widget);

   if (index == -1) {
      qWarning() << "LayoutHelper::itemInfo() Widget " << widget << " does not exist in the current layout " << lt;
      return QRect(0, 0, 1, 1);
   }

   return itemInfo(lt, index);
}


class BoxLayoutHelper : public  LayoutHelper
{
 public:
   BoxLayoutHelper(const Qt::Orientation orientation) : m_orientation(orientation)
   {
   }

   QRect itemInfo(QLayout *lt, int index) const override;
   void insertWidget(QLayout *lt, const QRect &info, QWidget *w) override;
   void removeWidget(QLayout *lt, QWidget *widget) override;
   void replaceWidget(QLayout *lt, QWidget *before, QWidget *after) override;

   void pushState(const QDesignerFormEditorInterface *, const QWidget *) override;
   void popState(const QDesignerFormEditorInterface *, QWidget *) override;

   bool canSimplify(const QDesignerFormEditorInterface *, const QWidget *, const QRect &) const override {
      return false;
   }
   void simplify(const QDesignerFormEditorInterface *, QWidget *, const QRect &) override {}

   // Helper for restoring layout states
   typedef QVector <QLayoutItem *> LayoutItemVector;
   static LayoutItemVector disassembleLayout(QLayout *lt);
   static QLayoutItem *findItemOfWidget(const LayoutItemVector &lv, QWidget *w);

 private:
   typedef QVector<QWidget *> BoxLayoutState;

   static BoxLayoutState state(const QBoxLayout *lt);

   QStack<BoxLayoutState> m_states;
   const Qt::Orientation m_orientation;
};

QRect BoxLayoutHelper::itemInfo(QLayout * /*lt*/, int index) const
{
   return m_orientation == Qt::Horizontal ?  QRect(index, 0, 1, 1) : QRect(0, index, 1, 1);
}

void BoxLayoutHelper::insertWidget(QLayout *lt, const QRect &info, QWidget *w)
{
   QDesignerWidgetItemInstaller wii; // Make sure we use QDesignerWidgetItem.
   QBoxLayout *boxLayout = dynamic_cast<QBoxLayout *>(lt);
   Q_ASSERT(boxLayout);
   boxLayout->insertWidget(m_orientation == Qt::Horizontal ? info.x() : info.y(), w);
}

void BoxLayoutHelper::removeWidget(QLayout *lt, QWidget *widget)
{
   QBoxLayout *boxLayout = dynamic_cast<QBoxLayout *>(lt);
   Q_ASSERT(boxLayout);
   boxLayout->removeWidget(widget);
}

void BoxLayoutHelper::replaceWidget(QLayout *lt, QWidget *before, QWidget *after)
{
   bool ok = false;
   QDesignerWidgetItemInstaller wii; // Make sure we use QDesignerWidgetItem.

   if (QBoxLayout *boxLayout = dynamic_cast<QBoxLayout *>(lt)) {
      const int index = boxLayout->indexOf(before);
      if (index != -1) {
         const bool visible = before->isVisible();
         delete boxLayout->takeAt(index);

         if (visible) {
            before->hide();
         }

         before->setParent(nullptr);
         boxLayout->insertWidget(index, after);
         ok = true;
      }
   }

   if (!ok) {
      qWarning() << "BoxLayoutHelper::replaceWidget() Unable to replace " << before << " with " << after
            << " in the current layout " << lt;
   }
}

BoxLayoutHelper::BoxLayoutState BoxLayoutHelper::state(const QBoxLayout *lt)
{
   BoxLayoutState rc;
   if (const int count = lt->count()) {
      rc.reserve(count);
      for (int i = 0; i < count; i++)
         if (QWidget *w = lt->itemAt(i)->widget()) {
            rc.push_back(w);
         }
   }
   return rc;
}

void BoxLayoutHelper::pushState(const QDesignerFormEditorInterface *core, const QWidget *w)
{
   const QBoxLayout *boxLayout = dynamic_cast<const QBoxLayout *>(LayoutInfo::managedLayout(core, w));
   Q_ASSERT(boxLayout);
   m_states.push(state(boxLayout));
}

QLayoutItem *BoxLayoutHelper::findItemOfWidget(const LayoutItemVector &lv, QWidget *w)
{
   const LayoutItemVector::const_iterator cend = lv.constEnd();

   for (LayoutItemVector::const_iterator it = lv.constBegin(); it != cend; ++it) {
      if ( (*it)->widget() == w) {
         return *it;
      }
   }

   return nullptr;
}

BoxLayoutHelper::LayoutItemVector BoxLayoutHelper::disassembleLayout(QLayout *lt)
{
   // Take items
   const int count = lt->count();
   if (count == 0) {
      return LayoutItemVector();
   }

   LayoutItemVector rc;
   rc.reserve(count);
   for (int i = count - 1; i >= 0; i--) {
      rc.push_back(lt->takeAt(i));
   }

   return rc;
}

void BoxLayoutHelper::popState(const QDesignerFormEditorInterface *core, QWidget *w)
{
   QBoxLayout *boxLayout = dynamic_cast<QBoxLayout *>(LayoutInfo::managedLayout(core, w));
   Q_ASSERT(boxLayout);
   const BoxLayoutState savedState = m_states.pop();
   const BoxLayoutState currentState = state(boxLayout);

   // Check for equality/empty. Note that this will currently
   // always trigger as box layouts do not have a state apart from
   // the order and there is no layout order editor yet.
   if (savedState == state(boxLayout)) {
      return;
   }

   const int count = savedState.size();
   Q_ASSERT(count == currentState.size());

   // Take items and reassemble in saved order
   const LayoutItemVector items = disassembleLayout(boxLayout);
   for (int i = 0; i < count; i++) {
      QLayoutItem *item = findItemOfWidget(items, savedState[i]);
      Q_ASSERT(item);
      boxLayout->addItem(item);
   }
}

// Grid Layout state. Datatype storing the state of a GridLayout as a map of
// widgets to QRect(columns, rows) and size. Used to store the state for undo operations
// that do not change the widgets within the layout; also provides some manipulation
// functions and ability to apply the state to a layout provided its widgets haven't changed.
struct GridLayoutState {
   GridLayoutState();

   void fromLayout(QGridLayout *l);
   void applyToLayout(const QDesignerFormEditorInterface *core, QWidget *w) const;

   void insertRow(int row);
   void insertColumn(int column);

   bool simplify(const QRect &r, bool testOnly);
   void removeFreeRow(int row);
   void removeFreeColumn(int column);


   // State of a cell in one dimension
   enum DimensionCellState {
      Free,
      Spanned,  // Item spans it
      Occupied  // Item bordering on it
   };
   // Horiontal, Vertical pair of state
   typedef QPair<DimensionCellState, DimensionCellState> CellState;
   typedef QVector<CellState> CellStates;

   // Figure out states of a cell and return as a flat vector of
   // [column1, column2,...] (address as  row * columnCount + col)
   static CellStates cellStates(const QList<QRect> &rects, int numRows, int numColumns);

   typedef QMap<QWidget *, QRect> WidgetItemMap;
   typedef QMap<QWidget *, Qt::Alignment> WidgetAlignmentMap;

   WidgetItemMap widgetItemMap;
   WidgetAlignmentMap widgetAlignmentMap;

   int rowCount;
   int colCount;
};

static inline bool needsSpacerItem(const GridLayoutState::CellState &cs)
{
   return cs.first == GridLayoutState::Free && cs.second == GridLayoutState::Free;
}

GridLayoutState::GridLayoutState()
   : rowCount(0), colCount(0)
{
}

GridLayoutState::CellStates GridLayoutState::cellStates(const QList<QRect> &rects, int numRows, int numColumns)
{
   CellStates rc = CellStates(numRows * numColumns, CellState(Free, Free));
   const QList<QRect>::const_iterator rcend = rects.constEnd();

   for (QList<QRect>::const_iterator it = rects.constBegin(); it != rcend; ++it) {
      const int leftColumn = it->x();
      const int topRow = it->y();
      const int rightColumn = leftColumn + it->width() - 1;
      const int bottomRow = topRow + it->height() - 1;

      for (int r = topRow; r <= bottomRow; r++) {
         for (int c = leftColumn; c <= rightColumn; c++) {
            const int flatIndex = r * numColumns + c;

            // Bordering horizontally?
            DimensionCellState &horizState = rc[flatIndex].first;

            if (c == leftColumn || c == rightColumn) {
               horizState = Occupied;
            } else {
               if (horizState < Spanned) {
                  horizState = Spanned;
               }
            }

            // Bordering vertically?
            DimensionCellState &vertState = rc[flatIndex].second;

            if (r == topRow || r == bottomRow) {
               vertState = Occupied;
            } else {
               if (vertState < Spanned) {
                  vertState = Spanned;
               }
            }
         }
      }
   }

   return rc;
}

void GridLayoutState::fromLayout(QGridLayout *l)
{
   rowCount = l->rowCount();
   colCount = l->columnCount();
   const int count = l->count();

   for (int i = 0; i < count; i++) {
      QLayoutItem *item = l->itemAt(i);

      if (!LayoutInfo::isEmptyItem(item)) {
         widgetItemMap.insert(item->widget(), gridItemInfo(l, i));
         if (item->alignment()) {
            widgetAlignmentMap.insert(item->widget(), item->alignment());
         }
      }
   }
}

void GridLayoutState::applyToLayout(const QDesignerFormEditorInterface *core, QWidget *w) const
{
   typedef QMap<QLayoutItem *, QRect> LayoutItemRectMap;
   QGridLayout *grid = dynamic_cast<QGridLayout *>(LayoutInfo::managedLayout(core, w));

   Q_ASSERT(grid);

   const bool shrink = grid->rowCount() > rowCount || grid->columnCount() > colCount;

   // Build a map of existing items to rectangles via widget map, delete spacers
   LayoutItemRectMap itemMap;

   while (grid->count()) {
      QLayoutItem *item = grid->takeAt(0);

      if (! LayoutInfo::isEmptyItem(item)) {
         QWidget *itemWidget = item->widget();
         const WidgetItemMap::const_iterator it = widgetItemMap.constFind(itemWidget);

         if (it == widgetItemMap.constEnd())
            qFatal("GridLayoutState::applyToLayout() Unable to apply a layout which has a new widget "
                  "added after saving the state  %s %s",
                  csPrintable(itemWidget->metaObject()->className()), csPrintable(itemWidget->objectName()) );

         itemMap.insert(item, it.value());

      } else {
         delete item;
      }
   }

   Q_ASSERT(itemMap.size() == widgetItemMap.size());

   // recreate if shrink
   if (shrink) {
      grid = static_cast<QGridLayout *>(recreateManagedLayout(core, w, grid));
   }

   // Add widgets items
   const LayoutItemRectMap::const_iterator icend = itemMap.constEnd();

   for (LayoutItemRectMap::const_iterator it = itemMap.constBegin(); it != icend; ++it) {
      const QRect info = it.value();
      const Qt::Alignment alignment = widgetAlignmentMap.value(it.key()->widget(), Qt::Alignment(Qt::EmptyFlag));

      grid->addItem(it.key(), info.y(), info.x(), info.height(), info.width(), alignment);
   }

   // create spacers
   const CellStates cs = cellStates(itemMap.values(), rowCount, colCount);

   for (int r = 0; r < rowCount; r++) {
      for (int c = 0; c < colCount; c++) {
         if (needsSpacerItem(cs[r * colCount  + c])) {
            grid->addItem(createGridSpacer(), r, c);
         }
      }
   }

   grid->activate();
}

void GridLayoutState::insertRow(int row)
{
   ++rowCount;
   const WidgetItemMap::iterator iend = widgetItemMap.end();

   for (WidgetItemMap::iterator it = widgetItemMap.begin(); it != iend; ++it) {
      const int topRow = it.value().y();

      if (topRow >= row) {
         it.value().translate(0, 1);

      } else {
         // Over  it: Does it span it -> widen?
         const int rowSpan = it.value().height();

         if (rowSpan > 1 && topRow + rowSpan > row) {
            it.value().setHeight(rowSpan + 1);
         }
      }
   }
}

void GridLayoutState::insertColumn(int column)
{
   ++colCount;
   const WidgetItemMap::iterator iend = widgetItemMap.end();

   for (WidgetItemMap::iterator it = widgetItemMap.begin(); it != iend; ++it) {
      const int leftColumn = it.value().x();

      if (leftColumn >= column) {
         it.value().translate(1, 0);

      } else {
         // Left of it: Does it span it -> widen?
         const int colSpan = it.value().width();

         if (colSpan  > 1 &&  leftColumn + colSpan > column) {
            it.value().setWidth(colSpan + 1);
         }
      }
   }
}

// Simplify: Remove empty columns/rows and such ones that are only spanned (shrink
// spanning items).
// 'AB.C.'           'ABC'
// 'DDDD.'     ==>   'DDD'
// 'EF.G.'           'EFG'
bool GridLayoutState::simplify(const QRect &rect, bool testOnly)
{
   // figure out free rows/columns
   QVector<bool> occupiedRows(rowCount, false);
   QVector<bool> occupiedColumns(colCount, false);

   // Mark everything outside restriction rectangle as occupied
   const int restrictionLeftColumn  = rect.x();
   const int restrictionRightColumn = restrictionLeftColumn + rect.width();
   const int restrictionTopRow      = rect.y();
   const int restrictionBottomRow   = restrictionTopRow + rect.height();

   if (restrictionLeftColumn > 0 || restrictionRightColumn < colCount ||
         restrictionTopRow   > 0 || restrictionBottomRow   < rowCount) {

      for (int row = 0; row < rowCount; row++) {
         if (row < restrictionTopRow || row >= restrictionBottomRow) {
            occupiedRows[row] = true;
         }
      }

      for (int column = 0; column < colCount; column++) {
         if (column < restrictionLeftColumn || column >= restrictionRightColumn) {
            occupiedColumns[column] = true;
         }
      }
   }

   // figure out free fields and tick off occupied rows and columns
   const CellStates cs = cellStates(widgetItemMap.values(), rowCount, colCount);

   for (int row = 0; row < rowCount; row++) {
      for (int column = 0; column < colCount; column++) {
         const CellState &state = cs[row * colCount  + column];

         if (state.first == Occupied) {
            occupiedColumns[column] = true;
         }

         if (state.second == Occupied) {
            occupiedRows[row] = true;
         }
      }
   }

   // Any free rows/columns?
   if (occupiedRows.indexOf(false) ==  -1 && occupiedColumns.indexOf(false) == -1) {
      return false;
   }

   if (testOnly) {
      return true;
   }

   // remove rows
   for (int row = rowCount - 1; row >= 0; row--)
      if (! occupiedRows[row]) {
         removeFreeRow(row);
      }

   // remove columns
   for (int column = colCount - 1; column >= 0; column--)
      if (! occupiedColumns[column]) {
         removeFreeColumn(column);
      }
   return true;
}

void GridLayoutState::removeFreeRow(int removeRow)
{
   const WidgetItemMap::iterator iend = widgetItemMap.end();

   for (WidgetItemMap::iterator it = widgetItemMap.begin(); it != iend; ++it) {
      const int r = it.value().y();
      Q_ASSERT(r != removeRow); // Free rows only

      if (r < removeRow) { // Does the item span it? - shrink it
         const int rowSpan = it.value().height();
         if (rowSpan > 1) {
            const int bottomRow = r + rowSpan;
            if (bottomRow > removeRow) {
               it.value().setHeight(rowSpan - 1);
            }
         }
      } else if (r > removeRow) { // Item below it? - move.
         it.value().translate(0, -1);
      }
   }

   --rowCount;
}

void GridLayoutState::removeFreeColumn(int removeColumn)
{
   const WidgetItemMap::iterator iend = widgetItemMap.end();

   for (WidgetItemMap::iterator it = widgetItemMap.begin(); it != iend; ++it) {
      const int c = it.value().x();
      Q_ASSERT(c != removeColumn); // Free columns only
      if (c < removeColumn) { // Does the item span it? - shrink it
         const int colSpan = it.value().width();
         if (colSpan > 1) {
            const int rightColumn = c + colSpan;
            if (rightColumn > removeColumn) {
               it.value().setWidth(colSpan - 1);
            }
         }

      } else if (c > removeColumn) { // Item to the right of it?  - move.
         it.value().translate(-1, 0);
      }
   }

   --colCount;
}

class GridLayoutHelper : public  LayoutHelper
{
 public:
   GridLayoutHelper() {}

   QRect itemInfo(QLayout *lt, int index) const override;
   void insertWidget(QLayout *lt, const QRect &info, QWidget *w) override;
   void removeWidget(QLayout *lt, QWidget *widget) override;
   void replaceWidget(QLayout *lt, QWidget *before, QWidget *after) override;

   void pushState(const QDesignerFormEditorInterface *core, const QWidget *widgetWithManagedLayout) override;
   void popState(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout) override;

   bool canSimplify(const QDesignerFormEditorInterface *core, const QWidget *widgetWithManagedLayout,
      const QRect &restrictionArea) const override;
   void simplify(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout, const QRect &restrictionArea) override;

   static void insertRow(QGridLayout *grid, int row);

 private:
   QStack<GridLayoutState> m_states;
};

void GridLayoutHelper::insertRow(QGridLayout *grid, int row)
{
   GridLayoutState state;
   state.fromLayout(grid);
   state.insertRow(row);
   QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(grid);
   state.applyToLayout(fw->core(), grid->parentWidget());
}

QRect GridLayoutHelper::itemInfo(QLayout *lt, int index) const
{
   QGridLayout *grid = dynamic_cast<QGridLayout *>(lt);
   Q_ASSERT(grid);
   return gridItemInfo(grid, index);
}

void GridLayoutHelper::insertWidget(QLayout *lt, const QRect &info, QWidget *w)
{
   QDesignerWidgetItemInstaller wii; // Make sure we use QDesignerWidgetItem.
   QGridLayout *gridLayout = dynamic_cast<QGridLayout *>(lt);
   Q_ASSERT(gridLayout);

   // check if there are any items. Should be only spacers, else something is wrong
   const int row = info.y();
   int column = info.x();
   int colSpan = info.width();
   int rowSpan = info.height();

   // If not empty: A multiselection was dropped on an empty item, insert row
   // and spread items along new row
   if (!removeEmptyCellsOnGrid(gridLayout, info)) {
      int freeColumn = -1;
      colSpan = rowSpan = 1;
      // First look to the right for a free column
      const int columnCount = gridLayout->columnCount();
      for (int c = column; c <  columnCount; c++) {
         const int idx = findGridItemAt(gridLayout, row, c);
         if (idx != -1 && LayoutInfo::isEmptyItem(gridLayout->itemAt(idx))) {
            freeColumn = c;
            break;
         }
      }

      if (freeColumn != -1) {
         removeEmptyCellsOnGrid(gridLayout, QRect(freeColumn, row, 1, 1));
         column = freeColumn;
      } else {
         GridLayoutHelper::insertRow(gridLayout, row);
         column = 0;
      }
   }
   gridLayout->addWidget(w, row, column, rowSpan, colSpan);
}

void GridLayoutHelper::removeWidget(QLayout *lt, QWidget *widget)
{
   QGridLayout *gridLayout = dynamic_cast<QGridLayout *>(lt);
   Q_ASSERT(gridLayout);
   const int index = gridLayout->indexOf(widget);

  if (index == -1) {
      qWarning() << "GridLayoutHelper::removeWidget() Unable to remove " << widget <<  ", not in the current layout";
      return;
   }

   // delete old item and pad with  by spacer items
   int row, column, rowspan, colspan;
   gridLayout->getItemPosition(index, &row, &column, &rowspan, &colspan);
   delete gridLayout->takeAt(index);
   const int rightColumn = column + colspan;
   const int bottomRow = row +  rowspan;
   for (int c = column; c < rightColumn; c++)
      for (int r = row; r < bottomRow; r++) {
         gridLayout->addItem(createGridSpacer(), r, c);
      }
}

void GridLayoutHelper::replaceWidget(QLayout *lt, QWidget *before, QWidget *after)
{
   bool ok = false;
   QDesignerWidgetItemInstaller wii; // Make sure we use QDesignerWidgetItem.
   if (QGridLayout *gridLayout = dynamic_cast<QGridLayout *>(lt)) {
      const int index = gridLayout->indexOf(before);

      if (index != -1) {
         int row, column, rowSpan, columnSpan;
         gridLayout->getItemPosition (index,  &row, &column, &rowSpan, &columnSpan);

         const bool visible = before->isVisible();
         delete gridLayout->takeAt(index);

         if (visible) {
            before->hide();
         }

         before->setParent(nullptr);
         gridLayout->addWidget(after, row, column, rowSpan, columnSpan);
         ok = true;
      }
   }

   if (!ok) {
      qWarning() << "GridLayoutHelper::replaceWidget() Unable to replace " << before << " with " << after
         << " in the current layout " << lt;
   }
}

void GridLayoutHelper::pushState(const QDesignerFormEditorInterface *core, const QWidget *widgetWithManagedLayout)
{
   QGridLayout *gridLayout = dynamic_cast<QGridLayout *>(LayoutInfo::managedLayout(core, widgetWithManagedLayout));
   Q_ASSERT(gridLayout);
   GridLayoutState gs;
   gs.fromLayout(gridLayout);
   m_states.push(gs);
}

void GridLayoutHelper::popState(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout)
{
   Q_ASSERT(!m_states.empty());
   const GridLayoutState state = m_states.pop();
   state.applyToLayout(core, widgetWithManagedLayout);
}

bool GridLayoutHelper::canSimplify(const QDesignerFormEditorInterface *core, const QWidget *widgetWithManagedLayout,
   const QRect &restrictionArea) const
{
   QGridLayout *gridLayout = dynamic_cast<QGridLayout *>(LayoutInfo::managedLayout(core, widgetWithManagedLayout));
   Q_ASSERT(gridLayout);
   GridLayoutState gs;
   gs.fromLayout(gridLayout);

   return gs.simplify(restrictionArea, true);
}

void GridLayoutHelper::simplify(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout,
   const QRect &restrictionArea)
{
   QGridLayout *gridLayout = dynamic_cast<QGridLayout *>(LayoutInfo::managedLayout(core, widgetWithManagedLayout));
   Q_ASSERT(gridLayout);

   GridLayoutState gs;
   gs.fromLayout(gridLayout);

   if (gs.simplify(restrictionArea, false)) {
      gs.applyToLayout(core, widgetWithManagedLayout);
   }
}

class FormLayoutHelper : public  LayoutHelper
{
 public:
   typedef QPair<QWidget *, QWidget *> WidgetPair;
   typedef QVector<WidgetPair> FormLayoutState;

   FormLayoutHelper()
   { }

   QRect itemInfo(QLayout *lt, int index) const override;
   void insertWidget(QLayout *lt, const QRect &info, QWidget *w) override;
   void removeWidget(QLayout *lt, QWidget *widget) override;
   void replaceWidget(QLayout *lt, QWidget *before, QWidget *after) override;

   void pushState(const QDesignerFormEditorInterface *core, const QWidget *widgetWithManagedLayout) override;
   void popState(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout) override;

   bool canSimplify(const QDesignerFormEditorInterface *core, const QWidget *, const QRect &) const override;
   void simplify(const QDesignerFormEditorInterface *, QWidget *, const QRect &) override;

 private:
   static FormLayoutState state(const QFormLayout *lt);

   QStack<FormLayoutState> m_states;
};

QRect FormLayoutHelper::itemInfo(QLayout *lt, int index) const
{
   QFormLayout *form = dynamic_cast<QFormLayout *>(lt);
   Q_ASSERT(form);

   int row;
   int column;
   int colspan;

   getFormLayoutItemPosition(form, index, &row, &column, nullptr, &colspan);

   return QRect(column, row, colspan, 1);
}

void FormLayoutHelper::insertWidget(QLayout *lt, const QRect &info, QWidget *w)
{
   QDesignerWidgetItemInstaller wii;
   QFormLayout *formLayout = dynamic_cast<QFormLayout *>(lt);

   Q_ASSERT(formLayout);

   // check if there are any nonspacer items? (Drop on 3rd column or drop of a multiselection
   // on an empty item. As the Form layout does not have insert semantics; we need to manually insert a row
   const bool insert = !removeEmptyCellsOnGrid(formLayout, info);
   formLayoutAddWidget(formLayout, w, info, insert);
   QLayoutSupport::createEmptyCells(formLayout);
}

void FormLayoutHelper::removeWidget(QLayout *lt, QWidget *widget)
{
   QFormLayout *formLayout = dynamic_cast<QFormLayout *>(lt);
   Q_ASSERT(formLayout);
   const int index = formLayout->indexOf(widget);

   if (index == -1) {
      qWarning() << "FormLayoutHelper::removeWidget() Unable to remove " << widget <<  ", not in the current layout";
      return;
   }

   // delete old item and pad with  by spacer items
   int row, column, colspan;
   getFormLayoutItemPosition(formLayout, index, &row, &column, nullptr, &colspan);

   delete formLayout->takeAt(index);

   if (colspan > 1 || column == 0) {
      formLayout->setItem(row, QFormLayout::LabelRole, createFormSpacer());
   }

   if (colspan > 1 || column == 1) {
      formLayout->setItem(row, QFormLayout::FieldRole, createFormSpacer());
   }
}

void FormLayoutHelper::replaceWidget(QLayout *lt, QWidget *before, QWidget *after)
{
   bool ok = false;
   QDesignerWidgetItemInstaller wii; // Make sure we use QDesignerWidgetItem.

   if (QFormLayout *formLayout = dynamic_cast<QFormLayout *>(lt)) {
      const int index = formLayout->indexOf(before);

      if (index != -1) {
         int row;
         QFormLayout::ItemRole role;
         formLayout->getItemPosition (index, &row, &role);

         const bool visible = before->isVisible();
         delete formLayout->takeAt(index);

         if (visible) {
            before->hide();
         }

         before->setParent(nullptr);
         formLayout->setWidget(row, role, after);
         ok = true;
      }
   }
   if (!ok) {
      qWarning() << "FormLayoutHelper::replaceWidget() Unable to replace " << before << " with " << after
            << " in the current layout " << lt;
   }
}

FormLayoutHelper::FormLayoutState FormLayoutHelper::state(const QFormLayout *lt)
{
   const int rowCount = lt->rowCount();
   if (rowCount == 0) {
      return FormLayoutState();
   }
   FormLayoutState rc(rowCount, WidgetPair(nullptr, nullptr));

   const int count = lt->count();
   int row, column, colspan;

   for (int i = 0; i < count; i++) {
      QLayoutItem *item = lt->itemAt(i);

      if (!LayoutInfo::isEmptyItem(item)) {
         QWidget *w = item->widget();
         Q_ASSERT(w);

         getFormLayoutItemPosition(lt, i, &row, &column, nullptr, &colspan);

         if (colspan > 1 || column == 0) {
            rc[row].first = w;
         }

         if (colspan > 1 || column == 1) {
            rc[row].second = w;
         }
      }
   }

   return rc;
}

void FormLayoutHelper::pushState(const QDesignerFormEditorInterface *core, const QWidget *widgetWithManagedLayout)
{
   QFormLayout *formLayout = dynamic_cast<QFormLayout *>(LayoutInfo::managedLayout(core, widgetWithManagedLayout));
   Q_ASSERT(formLayout);
   m_states.push(state(formLayout));
}

void FormLayoutHelper::popState(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout)
{
   QFormLayout *formLayout = dynamic_cast<QFormLayout *>(LayoutInfo::managedLayout(core, widgetWithManagedLayout));
   Q_ASSERT(!m_states.empty() && formLayout);

   const FormLayoutState storedState = m_states.pop();
   const FormLayoutState currentState =  state(formLayout);

   if (currentState ==  storedState) {
      return;
   }

   const int rowCount = storedState.size();

   // clear out, shrink if required, but maintain items via map, pad spacers
   const BoxLayoutHelper::LayoutItemVector items = BoxLayoutHelper::disassembleLayout(formLayout);
   if (rowCount < formLayout->rowCount()) {
      formLayout = static_cast<QFormLayout *>(recreateManagedLayout(core, widgetWithManagedLayout, formLayout ));
   }

   for (int r = 0; r < rowCount; r++) {
      QWidget *widgets[COLUMNS_IN_FORM] = { storedState[r].first, storedState[r].second };
      const bool spanning = widgets[0] != nullptr && widgets[0] == widgets[1];

      if (spanning) {
         formLayout->setWidget(r, QFormLayout::SpanningRole, widgets[0]);

      } else {
         for (int c = 0; c < COLUMNS_IN_FORM; c++) {
            const QFormLayout::ItemRole role = c == 0 ? QFormLayout::LabelRole : QFormLayout::FieldRole;

            if (widgets[c] && BoxLayoutHelper::findItemOfWidget(items, widgets[c])) {
               formLayout->setWidget(r, role, widgets[c]);
            } else {
               formLayout->setItem(r, role, createFormSpacer());
            }
         }
      }
   }
}

bool FormLayoutHelper::canSimplify(const QDesignerFormEditorInterface *core, const QWidget *widgetWithManagedLayout,
   const QRect &restrictionArea) const
{
   const QFormLayout *formLayout = dynamic_cast<QFormLayout *>(LayoutInfo::managedLayout(core, widgetWithManagedLayout));
   Q_ASSERT(formLayout);
   return canSimplifyFormLayout(formLayout, restrictionArea);
}

void FormLayoutHelper::simplify(const QDesignerFormEditorInterface *core, QWidget *widgetWithManagedLayout,
   const QRect &restrictionArea)
{
   typedef QPair<QLayoutItem *, QLayoutItem *> LayoutItemPair;
   typedef QVector<LayoutItemPair> LayoutItemPairs;

   QFormLayout *formLayout = dynamic_cast<QFormLayout *>(LayoutInfo::managedLayout(core, widgetWithManagedLayout));

   Q_ASSERT(formLayout);

   // Transform into vector of item pairs
   const int rowCount = formLayout->rowCount();
   LayoutItemPairs pairs(rowCount, LayoutItemPair(nullptr, nullptr));

   for (int i =  formLayout->count() - 1; i >= 0; i--) {
      int row, col, colspan;
      getFormLayoutItemPosition(formLayout, i, &row, &col, nullptr, &colspan);

      if (colspan > 1) {
         pairs[row].first = pairs[row].second = formLayout->takeAt(i);
      } else {
         if (col == 0) {
            pairs[row].first = formLayout->takeAt(i);
         } else {
            pairs[row].second = formLayout->takeAt(i);
         }
      }
   }
   // Weed out empty ones
   const int bottomCheckRow = qMin(rowCount, restrictionArea.y() + restrictionArea.height());
   for (int r = bottomCheckRow - 1; r >= restrictionArea.y(); r--)
      if (LayoutInfo::isEmptyItem(pairs[r].first) && LayoutInfo::isEmptyItem(pairs[r].second)) {
         delete pairs[r].first;
         delete pairs[r].second;
         pairs.remove(r);
      }
   const int simpleRowCount = pairs.size();
   if (simpleRowCount < rowCount) {
      formLayout = static_cast<QFormLayout *>(recreateManagedLayout(core, widgetWithManagedLayout, formLayout));
   }
   // repopulate
   for (int r = 0; r < simpleRowCount; r++) {
      const bool spanning = pairs[r].first == pairs[r].second;
      if (spanning) {
         formLayout->setItem(r, QFormLayout::SpanningRole, pairs[r].first);
      } else {
         formLayout->setItem(r, QFormLayout::LabelRole, pairs[r].first);
         formLayout->setItem(r, QFormLayout::FieldRole, pairs[r].second);
      }
   }
}

LayoutHelper *LayoutHelper::createLayoutHelper(int type)
{
   LayoutHelper *rc = nullptr;
   switch (type) {
      case LayoutInfo::HBox:
         rc = new BoxLayoutHelper(Qt::Horizontal);
         break;
      case LayoutInfo::VBox:
         rc = new BoxLayoutHelper(Qt::Vertical);
         break;
      case LayoutInfo::Grid:
         rc = new GridLayoutHelper;
         break;
      case LayoutInfo::Form:
         return new FormLayoutHelper;
      default:
         break;
   }
   Q_ASSERT(rc);
   return rc;
}

// ---- QLayoutSupport (LayoutDecorationExtension)
QLayoutSupport::QLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, LayoutHelper *helper, QObject *parent)  :
   QObject(parent),
   m_formWindow(formWindow),
   m_helper(helper),
   m_widget(widget),
   m_currentIndex(-1),
   m_currentInsertMode(QDesignerLayoutDecorationExtension::InsertWidgetMode)
{
}

QLayout *QLayoutSupport::layout() const
{
   return LayoutInfo::managedLayout(m_formWindow->core(), m_widget);
}

void QLayoutSupport::hideIndicator(Indicator i)
{
   if (m_indicators[i]) {
      m_indicators[i]->hide();
   }
}

void QLayoutSupport::showIndicator(Indicator i, const QRect &geometry, const QPalette &p)
{
   if (!m_indicators[i]) {
      m_indicators[i] = new qdesigner_internal::InvisibleWidget(m_widget);
   }
   QWidget *indicator = m_indicators[i];
   indicator->setAutoFillBackground(true);
   indicator->setPalette(p);
   indicator->setGeometry(geometry);
   indicator->show();
   indicator->raise();
}

QLayoutSupport::~QLayoutSupport()
{
   delete m_helper;
   for (int i = 0; i < NumIndicators; i++)
      if (m_indicators[i]) {
         m_indicators[i]->deleteLater();
      }
}

QGridLayout *QLayoutSupport::gridLayout() const
{
   return dynamic_cast<QGridLayout *>(LayoutInfo::managedLayout(m_formWindow->core(), m_widget));
}

QRect QLayoutSupport::itemInfo(int index) const
{
   return m_helper->itemInfo(LayoutInfo::managedLayout(m_formWindow->core(), m_widget), index);
}

void QLayoutSupport::setInsertMode(InsertMode im)
{
   m_currentInsertMode = im;
}

void QLayoutSupport::setCurrentCell(const QPair<int, int> &cell)
{
   m_currentCell = cell;
}

void QLayoutSupport::adjustIndicator(const QPoint &pos, int index)
{
   if (index == -1) {
      // first item goes anywhere
      hideIndicator(LeftIndicator);
      hideIndicator(TopIndicator);
      hideIndicator(RightIndicator);
      hideIndicator(BottomIndicator);

      return;
   }

   m_currentIndex = index;
   m_currentInsertMode = QDesignerLayoutDecorationExtension::InsertWidgetMode;

   QLayoutItem *item = layout()->itemAt(index);
   const QRect g     = extendedGeometry(index);

   if (LayoutInfo::isEmptyItem(item)) {
      // Empty grid/form cell. Draw a rectangle

      QPalette redPalette;
      redPalette.setColor(QPalette::Window, Qt::red);

      showIndicator(LeftIndicator,   QRect(g.x(),     g.y(),      INDICATOR_SIZE, g.height()),     redPalette);
      showIndicator(TopIndicator,    QRect(g.x(),     g.y(),      g.width(),      INDICATOR_SIZE), redPalette);
      showIndicator(RightIndicator,  QRect(g.right(), g.y(),      INDICATOR_SIZE, g.height()),     redPalette);
      showIndicator(BottomIndicator, QRect(g.x(),     g.bottom(), g.width(),      INDICATOR_SIZE), redPalette);

      setCurrentCellFromIndicatorOnEmptyCell(m_currentIndex);

   } else {
      // Append/Insert. Draw a bar left/right or above/below

      QPalette bluePalette;
      bluePalette.setColor(QPalette::Window, Qt::blue);
      hideIndicator(LeftIndicator);
      hideIndicator(TopIndicator);

      const int fromRight = g.right() - pos.x();
      const int fromBottom = g.bottom() - pos.y();

      const int fromLeft = pos.x() - g.x();
      const int fromTop = pos.y() - g.y();

      const int fromLeftRight = qMin(fromRight, fromLeft );
      const int fromBottomTop = qMin(fromBottom, fromTop);

      const Qt::Orientation indicatorOrientation =  fromLeftRight < fromBottomTop ? Qt::Vertical :  Qt::Horizontal;

      if (supportsIndicatorOrientation(indicatorOrientation)) {
         const QRect r(layout()->geometry().topLeft(), layout()->parentWidget()->size());

         switch (indicatorOrientation) {
            case  Qt::Vertical: {
               hideIndicator(BottomIndicator);
               const bool closeToLeft = fromLeftRight == fromLeft;
               showIndicator(RightIndicator, QRect(closeToLeft ? g.x() : g.right() + 1 - INDICATOR_SIZE,
                     0, INDICATOR_SIZE, r.height()), bluePalette);

               const QWidget *parent = layout()->parentWidget();
               const bool leftToRight = Qt::LeftToRight == (parent ? parent->layoutDirection() : QApplication::layoutDirection());
               const int incr = leftToRight == closeToLeft ? 0 : +1;
               setCurrentCellFromIndicator(indicatorOrientation, m_currentIndex, incr);
            }
            break;

            case  Qt::Horizontal: {
               hideIndicator(RightIndicator);
               const bool closeToTop = fromBottomTop == fromTop;

               showIndicator(BottomIndicator, QRect(r.x(), closeToTop ? g.y() : g.bottom() + 1 - INDICATOR_SIZE,
                     r.width(), INDICATOR_SIZE), bluePalette);

               const int incr = closeToTop ? 0 : +1;
               setCurrentCellFromIndicator(indicatorOrientation, m_currentIndex, incr);
            }
            break;
         }

      } else {
         hideIndicator(RightIndicator);
         hideIndicator(BottomIndicator);

      }


   }
}

int QLayoutSupport::indexOf(QLayoutItem *i) const
{
   const QLayout *lt = layout();
   if (! lt) {
      return -1;
   }

   int index = 0;

   while (QLayoutItem *item = lt->itemAt(index)) {
      if (item == i) {
         return index;
      }

      ++index;
   }

   return -1;
}

int QLayoutSupport::indexOf(QWidget *widget) const
{
   const QLayout *lt = layout();
   if (! lt) {
      return -1;
   }

   int index = 0;
   while (QLayoutItem *item = lt->itemAt(index)) {
      if (item->widget() == widget) {
         return index;
      }

      ++index;
   }

   return -1;
}

QList<QWidget *> QLayoutSupport::widgets(QLayout *layout) const
{
   if (!layout) {
      return QList<QWidget *>();
   }

   QList<QWidget *> lst;
   int index = 0;

   while (QLayoutItem *item = layout->itemAt(index)) {
      ++index;

      QWidget *widget = item->widget();
      if (widget && formWindow()->isManaged(widget)) {
         lst.append(widget);
      }
   }

   return lst;
}

int QLayoutSupport::findItemAt_GridCell(QGridLayout *gridLayout, int at_row, int at_column)
{
   return findGridItemAt(gridLayout, at_row, at_column);
}

// Quick check whether simplify should be enabled for grids. May return false positives.
// Note: Calculating the occupied area does not work as spanning items may also be simplified.

bool QLayoutSupport::canSimplifyQuickCheck(const QGridLayout *gl)
{
   if (!gl) {
      return false;
   }

   const int colCount = gl->columnCount();
   const int rowCount = gl->rowCount();

   if (colCount < 2 || rowCount < 2) {
      return false;
   }

   // try to find a spacer
   const int count = gl->count();

   for (int index = 0; index < count; ++index) {
      if (LayoutInfo::isEmptyItem(gl->itemAt(index))) {
         return true;
      }
   }

   return false;
}

bool QLayoutSupport::canSimplifyQuickCheck(const QFormLayout *fl)
{
   return canSimplifyFormLayout(fl, QRect(QPoint(0, 0), QSize(32767, 32767)));
}

// remove dummy spacers
bool QLayoutSupport::removeEmptyCells(QGridLayout *grid, const QRect &area)
{
   return removeEmptyCellsOnGrid(grid, area);
}

void QLayoutSupport::createEmptyCells(QGridLayout *gridLayout)
{
   Q_ASSERT(gridLayout);
   GridLayoutState gs;
   gs.fromLayout(gridLayout);

   const GridLayoutState::CellStates cs = GridLayoutState::cellStates(gs.widgetItemMap.values(), gs.rowCount, gs.colCount);
   for (int c = 0; c < gs.colCount; c++) {
      for (int r = 0; r < gs.rowCount; r++) {

         if (needsSpacerItem(cs[r * gs.colCount + c])) {
            const int existingItemIndex = findItemAt_GridCell(gridLayout, r, c);

            if (existingItemIndex == -1) {
               gridLayout->addItem(createGridSpacer(), r, c);
            }
         }
      }
   }
}

bool QLayoutSupport::removeEmptyCells(QFormLayout *formLayout, const QRect &area)
{
   return removeEmptyCellsOnGrid(formLayout, area);
}

void QLayoutSupport::createEmptyCells(QFormLayout *formLayout)
{
   // No spanning items here..
   if (const int rowCount = formLayout->rowCount()) {
      for (int c = 0; c < COLUMNS_IN_FORM; ++c) {
         for (int r = 0; r < rowCount; ++r) {
            if (findGridItemAt(formLayout, r, c) == -1) {
               formLayout->setItem(r, c == 0 ? QFormLayout::LabelRole : QFormLayout::FieldRole, createFormSpacer());
            }
         }
      }
   }
}

int QLayoutSupport::findItemAt(const QPoint &pos) const
{
   if (! layout()) {
      return -1;
   }

   const QLayout *lt = layout();
   const int count   = lt->count();

   if (count == 0) {
      return -1;
   }

   int best = -1;
   int bestIndex = -1;

   for (int index = 0;  index < count;  index++) {
      QLayoutItem *item = lt->itemAt(index);
      bool visible = true;

      // When dragging widgets within layout, the source widget is invisible and must not be hit
      if (const QWidget *w = item->widget()) {
         visible = w->isVisible();
      }

      if (visible) {
         const QRect g = item->geometry();

         const int dist = (g.center() - pos).manhattanLength();
         if (best == -1 || dist < best) {
            best = dist;
            bestIndex = index;
         }
      }
   }
   return bestIndex;
}

namespace {

class QBoxLayoutSupport: public QLayoutSupport
{
 public:
   QBoxLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, Qt::Orientation orientation, QObject *parent = nullptr);

   void insertWidget(QWidget *widget, const QPair<int, int> &cell) override;
   void removeWidget(QWidget *widget) override;
   void simplify() override {}
   void insertRow(int /*row*/) override {}
   void insertColumn(int /*column*/) override {}

   int findItemAt(int /*at_row*/, int /*at_column*/) const override {
      return -1;
   }
   using QLayoutSupport::findItemAt;

 private:
   void setCurrentCellFromIndicatorOnEmptyCell(int index) override;
   void setCurrentCellFromIndicator(Qt::Orientation indicatorOrientation, int index, int increment) override;
   bool supportsIndicatorOrientation(Qt::Orientation indicatorOrientation) const override;
   QRect extendedGeometry(int index) const override;

   const Qt::Orientation m_orientation;
};

void QBoxLayoutSupport::removeWidget(QWidget *widget)
{
   QLayout *lt = layout();
   const int index = lt->indexOf(widget);

   // Adjust the current cell in case a widget was dragged within the same layout to a position
   // of higher index, which happens as follows:
   // Drag start: The widget is hidden
   // Drop: Current cell is stored, widget is removed and re-added, causing an index offset that needs to be compensated

   QPair<int, int> currCell = currentCell();

   switch (m_orientation) {
      case Qt::Horizontal:
         if (currCell.second > 0 && index < currCell.second ) {
            --currCell.second;
            setCurrentCell(currCell);
         }
         break;

      case Qt::Vertical:
         if (currCell.first > 0 && index < currCell.first) {
            --currCell.first;
            setCurrentCell(currCell);
         }
         break;
   }

   helper()->removeWidget(lt, widget);
}

QBoxLayoutSupport::QBoxLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget,
      Qt::Orientation orientation, QObject *parent)
   : QLayoutSupport(formWindow, widget, new BoxLayoutHelper(orientation), parent),
     m_orientation(orientation)
{
}

void QBoxLayoutSupport::setCurrentCellFromIndicatorOnEmptyCell(int index)
{
   (void) index;
   setCurrentCell(qMakePair(0, 0));
}

void QBoxLayoutSupport::insertWidget(QWidget *widget, const QPair<int, int> &cell)
{
   switch (m_orientation) {
      case  Qt::Horizontal:
         helper()->insertWidget(layout(), QRect(cell.second, 0, 1, 1), widget);
         break;

      case  Qt::Vertical:
         helper()->insertWidget(layout(), QRect(0, cell.first, 1, 1), widget);
         break;
   }
}

void QBoxLayoutSupport::setCurrentCellFromIndicator(Qt::Orientation indicatorOrientation, int index, int increment)
{
   if (m_orientation == Qt::Horizontal && indicatorOrientation == Qt::Vertical) {
      setCurrentCell(qMakePair(0, index + increment));

   } else if (m_orientation == Qt::Vertical && indicatorOrientation == Qt::Horizontal) {
      setCurrentCell(qMakePair(index + increment, 0));
   }
}

bool QBoxLayoutSupport::supportsIndicatorOrientation(Qt::Orientation indicatorOrientation) const
{
   return m_orientation != indicatorOrientation;
}

QRect QBoxLayoutSupport::extendedGeometry(int index) const
{
   QLayoutItem *item = layout()->itemAt(index);

   // start off with item geometry
   QRect g = item->geometry();

   const QRect info = itemInfo(index);

   // On left border: extend to widget border
   if (info.x() == 0) {
      QPoint topLeft = g.topLeft();
      topLeft.rx() = layout()->geometry().left();
      g.setTopLeft(topLeft);
   }

   // On top border: extend to widget border
   if (info.y() == 0) {
      QPoint topLeft = g.topLeft();
      topLeft.ry() = layout()->geometry().top();
      g.setTopLeft(topLeft);
   }

   // is this the last item?
   const QBoxLayout *box = static_cast<const QBoxLayout *>(layout());
   if (index < box->count() - 1) {
      // no
      return g;
   }

   // extend to widget border
   QPoint bottomRight = g.bottomRight();
   switch (m_orientation) {
      case Qt::Vertical:
         bottomRight.ry() = layout()->geometry().bottom();
         break;

      case Qt::Horizontal:
         bottomRight.rx() = layout()->geometry().right();
         break;
   }
   g.setBottomRight(bottomRight);
   return g;
}

template <class GridLikeLayout>
class GridLikeLayoutSupportBase: public QLayoutSupport
{
 public:

   GridLikeLayoutSupportBase(QDesignerFormWindowInterface *formWindow, QWidget *widget, LayoutHelper *helper, QObject *parent = nullptr) :
      QLayoutSupport(formWindow, widget, helper, parent)
   {
   }

   void insertWidget(QWidget *widget, const QPair<int, int> &cell) override;

   void removeWidget(QWidget *widget) override {
      helper()->removeWidget(layout(), widget);
   }

   int findItemAt(int row, int column) const override;
   using QLayoutSupport::findItemAt;

 protected:
   GridLikeLayout *gridLikeLayout() const {
      return dynamic_cast<GridLikeLayout *>(LayoutInfo::managedLayout(formWindow()->core(), widget()));
   }

 private:
   void setCurrentCellFromIndicatorOnEmptyCell(int index) override;
   void setCurrentCellFromIndicator(Qt::Orientation indicatorOrientation, int index, int increment) override;
   bool supportsIndicatorOrientation(Qt::Orientation) const override {
      return true;
   }

   QRect extendedGeometry(int index) const override;

   // Overwrite to check the insertion position (if there are limits)
   virtual void checkCellForInsertion(int * /*row*/, int * /*col*/) const {}
};

template <class GridLikeLayout>
void GridLikeLayoutSupportBase<GridLikeLayout>::setCurrentCellFromIndicatorOnEmptyCell(int index)
{
   GridLikeLayout *grid = gridLikeLayout();
   Q_ASSERT(grid);

   setInsertMode(InsertWidgetMode);
   int row, column, rowspan, colspan;

   getGridItemPosition(grid, index, &row, &column, &rowspan, &colspan);
   setCurrentCell(qMakePair(row, column));
}

template <class GridLikeLayout>
void GridLikeLayoutSupportBase<GridLikeLayout>::setCurrentCellFromIndicator(Qt::Orientation indicatorOrientation, int index,
   int increment)
{
   const QRect info = itemInfo(index);

   switch (indicatorOrientation) {
      case Qt::Vertical: {
         setInsertMode(InsertColumnMode);
         int row = info.top();
         int column = increment ? info.right() + 1 : info.left();
         checkCellForInsertion(&row, &column);
         setCurrentCell(qMakePair(row, column));
      }
      break;

      case Qt::Horizontal: {
         setInsertMode(InsertRowMode);
         int row = increment ? info.bottom() + 1 : info.top();
         int column = info.left();
         checkCellForInsertion(&row, &column);
         setCurrentCell(qMakePair(row, column));
      }
      break;
   }
}

template <class GridLikeLayout>
void GridLikeLayoutSupportBase<GridLikeLayout>::insertWidget(QWidget *widget, const QPair<int, int> &cell)
{
   helper()->insertWidget(layout(), QRect(cell.second, cell.first, 1, 1), widget);
}

template <class GridLikeLayout>
int GridLikeLayoutSupportBase<GridLikeLayout>::findItemAt(int at_row, int at_column) const
{
   GridLikeLayout *grid = gridLikeLayout();
   Q_ASSERT(grid);
   return findGridItemAt(grid, at_row, at_column);
}

template <class GridLikeLayout>
QRect GridLikeLayoutSupportBase<GridLikeLayout>::extendedGeometry(int index) const
{
   QLayoutItem *item = layout()->itemAt(index);
   // start off with item geometry
   QRect g = item->geometry();

   const QRect info = itemInfo(index);

   // On left border: extend to widget border
   if (info.x() == 0) {
      QPoint topLeft = g.topLeft();
      topLeft.rx() = layout()->geometry().left();
      g.setTopLeft(topLeft);
   }

   // On top border: extend to widget border
   if (info.y() == 0) {
      QPoint topLeft = g.topLeft();
      topLeft.ry() = layout()->geometry().top();
      g.setTopLeft(topLeft);
   }

   const GridLikeLayout *grid = gridLikeLayout();
   Q_ASSERT(grid);

   // extend to widget border
   QPoint bottomRight = g.bottomRight();
   if (gridRowCount(grid) == info.y()) {
      bottomRight.ry() = layout()->geometry().bottom();
   }

   if (gridColumnCount(grid) == info.x()) {
      bottomRight.rx() = layout()->geometry().right();
   }

   g.setBottomRight(bottomRight);

   return g;
}

class QGridLayoutSupport: public GridLikeLayoutSupportBase<QGridLayout>
{
 public:
   QGridLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, QObject *parent = nullptr);

   void simplify() override;
   void insertRow(int row) override;
   void insertColumn(int column) override;
};

QGridLayoutSupport::QGridLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, QObject *parent)
   : GridLikeLayoutSupportBase<QGridLayout>(formWindow, widget, new GridLayoutHelper, parent)
{
}

void QGridLayoutSupport::insertRow(int row)
{
   QGridLayout *grid = gridLayout();
   Q_ASSERT(grid);
   GridLayoutHelper::insertRow(grid, row);
}

void QGridLayoutSupport::insertColumn(int column)
{
   QGridLayout *grid = gridLayout();
   Q_ASSERT(grid);
   GridLayoutState state;
   state.fromLayout(grid);
   state.insertColumn(column);
   state.applyToLayout(formWindow()->core(), widget());
}

void QGridLayoutSupport::simplify()
{
   QGridLayout *grid = gridLayout();
   Q_ASSERT(grid);
   GridLayoutState state;
   state.fromLayout(grid);

   const QRect fullArea = QRect(0, 0, state.colCount, state.rowCount);

   if (state.simplify(fullArea, false)) {
      state.applyToLayout(formWindow()->core(), widget());
   }
}

class QFormLayoutSupport: public GridLikeLayoutSupportBase<QFormLayout>
{
 public:
   QFormLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, QObject *parent = nullptr);

   void simplify() override
   {
   }

   void insertRow(int) override
   {
   }

   void insertColumn(int) override
   {
   }

 private:
   void checkCellForInsertion(int *row, int *col) const override;
};

QFormLayoutSupport::QFormLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, QObject *parent) :
   GridLikeLayoutSupportBase<QFormLayout>(formWindow, widget, new FormLayoutHelper, parent)
{
}

void QFormLayoutSupport::checkCellForInsertion(int *row, int *col) const
{
   if (*col >= COLUMNS_IN_FORM) {
      // Clamp to 2 columns
      *col = 1;
      (*row)++;
   }
}

}   // end  anonymous namespace

QLayoutSupport *QLayoutSupport::createLayoutSupport(QDesignerFormWindowInterface *formWindow, QWidget *widget, QObject *parent)
{
   const QLayout *layout = LayoutInfo::managedLayout(formWindow->core(), widget);
   Q_ASSERT(layout);
   QLayoutSupport *rc = nullptr;

   switch (LayoutInfo::layoutType(formWindow->core(), layout)) {
      case LayoutInfo::HBox:
         rc = new QBoxLayoutSupport(formWindow, widget, Qt::Horizontal, parent);
         break;

      case LayoutInfo::VBox:
         rc = new QBoxLayoutSupport(formWindow, widget, Qt::Vertical, parent);
         break;

      case LayoutInfo::Grid:
         rc = new QGridLayoutSupport(formWindow, widget, parent);
         break;

      case LayoutInfo::Form:
         rc = new QFormLayoutSupport(formWindow, widget, parent);
         break;

      default:
         break;
   }

   Q_ASSERT(rc);

   return rc;
}

}   // end namespace qdesigner_internal

QLayoutWidget::QLayoutWidget(QDesignerFormWindowInterface *formWindow, QWidget *parent)
   : QWidget(parent), m_formWindow(formWindow),
     m_leftMargin(0), m_topMargin(0), m_rightMargin(0), m_bottomMargin(0)
{
}

void QLayoutWidget::paintEvent(QPaintEvent *)
{
   if (m_formWindow->currentTool() != 0) {
      return;
   }

   // only draw red borders when editing widgets
   QPainter p(this);

   QMap<int, QMap<int, bool>> excludedRowsForColumn;
   QMap<int, QMap<int, bool>> excludedColumnsForRow;

   QLayout *lt = layout();
   QGridLayout *grid = dynamic_cast<QGridLayout *>(lt);

   if (lt != nullptr) {
      if (const int count = lt->count()) {
         p.setPen(QPen(QColor(255, 0, 0, 35), 1));

         for (int i = 0; i < count; i++) {
            QLayoutItem *item = lt->itemAt(i);

            if (grid != nullptr) {
               int row, column, rowSpan, columnSpan;
               grid->getItemPosition(i, &row, &column, &rowSpan, &columnSpan);

               QMap<int, bool> rows;
               QMap<int, bool> columns;

               for (int rowCnt = rowSpan; rowCnt > 1; rowCnt--) {
                  rows[row + rowCnt - 2] = true;
               }

               for (int colCnt = columnSpan; colCnt > 1; colCnt--) {
                  columns[column + colCnt - 2] = true;
               }

               while (rowSpan > 0) {
                  excludedColumnsForRow[row + rowSpan - 1].unite(columns);
                  --rowSpan;
               }

               while (columnSpan > 0) {
                  excludedRowsForColumn[column + columnSpan - 1].unite(rows);
                  --columnSpan;
               }
            }

            if (item->spacerItem()) {
               const QRect geometry = item->geometry();

               if (! geometry.isNull()) {
                  p.drawRect(geometry.adjusted(1, 1, -2, -2));
               }
            }
         }
      }
   }

   if (grid) {
      p.setPen(QPen(QColor(0, 0x80, 0, 0x80), 1));

      const int rowCount = grid->rowCount();
      const int columnCount = grid->columnCount();

      for (int i = 0; i < rowCount; i++) {
         for (int j = 0; j < columnCount; j++) {
            const QRect cellRect = grid->cellRect(i, j);

            if (j < columnCount - 1 && excludedColumnsForRow.value(i).value(j, false) == false) {
               const double y0 = (i == 0)
                  ? 0 : (grid->cellRect(i - 1, j).bottom() + cellRect.top()) / 2.0;

               const double y1 = (i == rowCount - 1)
                  ? height() - 1 : (cellRect.bottom() + grid->cellRect(i + 1, j).top()) / 2.0;

               const double x = (cellRect.right() + grid->cellRect(i, j + 1).left()) / 2.0;
               p.drawLine(QPointF(x, y0), QPointF(x, y1));
            }

            if (i < rowCount - 1 && excludedRowsForColumn.value(j).value(i, false) == false) {
               const double x0 = (j == 0)
                  ? 0 : (grid->cellRect(i, j - 1).right() + cellRect.left()) / 2.0;

               const double x1 = (j == columnCount - 1)
                  ? width() - 1 : (cellRect.right() + grid->cellRect(i, j + 1).left()) / 2.0;

               const double y = (cellRect.bottom() + grid->cellRect(i + 1, j).top()) / 2.0;
               p.drawLine(QPointF(x0, y), QPointF(x1, y));
            }
         }
      }
   }

   p.setPen(QPen(QColor(255, 0, 0, 128), 1));
   p.drawRect(0, 0, width() - 1, height() - 1);
}

bool QLayoutWidget::event(QEvent *e)
{
   switch (e->type()) {
      case QEvent::LayoutRequest: {
         (void) QWidget::event(e);

         // current widget has a layout, parent does not
         if (layout() && qdesigner_internal::LayoutInfo::layoutType(formWindow()->core(),
                  parentWidget()) == qdesigner_internal::LayoutInfo::NoLayout) {
            resize(layout()->totalMinimumSize().expandedTo(size()));
         }

         update();

         return true;
      }

      default:
         break;
   }

   return QWidget::event(e);
}

int QLayoutWidget::layoutLeftMargin() const
{
   if (m_leftMargin < 0 && layout()) {
      int margin;
      layout()->getContentsMargins(&margin, nullptr, nullptr, nullptr);

      return margin;
   }

   return m_leftMargin;
}

void QLayoutWidget::setLayoutLeftMargin(int layoutMargin)
{
   m_leftMargin = layoutMargin;

   if (layout()) {
      int newMargin = m_leftMargin;

      if (newMargin >= 0 && newMargin < MIN_MARGIN) {
         newMargin = MIN_MARGIN;
      }

      int left;
      int top;
      int right;
      int bottom;

      layout()->getContentsMargins(&left, &top, &right, &bottom);
      layout()->setContentsMargins(newMargin, top, right, bottom);
   }
}

int QLayoutWidget::layoutTopMargin() const
{
   if (m_topMargin < 0 && layout()) {
      int margin;
      layout()->getContentsMargins(nullptr, &margin, nullptr, nullptr);
      return margin;
   }

   return m_topMargin;
}

void QLayoutWidget::setLayoutTopMargin(int layoutMargin)
{
   m_topMargin = layoutMargin;

   if (layout()) {
      int newMargin = m_topMargin;
      if (newMargin >= 0 && newMargin < MIN_MARGIN) {
         newMargin = MIN_MARGIN;
      }

      int left;
      int top;
      int right;
      int bottom;

      layout()->getContentsMargins(&left, &top, &right, &bottom);
      layout()->setContentsMargins(left, newMargin, right, bottom);
   }
}

int QLayoutWidget::layoutRightMargin() const
{
   if (m_rightMargin < 0 && layout()) {
      int margin;
      layout()->getContentsMargins(nullptr, nullptr, &margin, nullptr);
      return margin;
   }

   return m_rightMargin;
}

void QLayoutWidget::setLayoutRightMargin(int layoutMargin)
{
   m_rightMargin = layoutMargin;

   if (layout()) {
      int newMargin = m_rightMargin;
      if (newMargin >= 0 && newMargin < MIN_MARGIN) {
         newMargin = MIN_MARGIN;
      }

      int left;
      int top;
      int right;
      int bottom;

      layout()->getContentsMargins(&left, &top, &right, &bottom);
      layout()->setContentsMargins(left, top, newMargin, bottom);
   }
}

int QLayoutWidget::layoutBottomMargin() const
{
   if (m_bottomMargin < 0 && layout()) {
      int margin;
      layout()->getContentsMargins(nullptr, nullptr, nullptr, &margin);
      return margin;
   }

   return m_bottomMargin;
}

void QLayoutWidget::setLayoutBottomMargin(int layoutMargin)
{
   m_bottomMargin = layoutMargin;

   if (layout()) {

      int newMargin = m_bottomMargin;
      if (newMargin >= 0 && newMargin < MIN_MARGIN) {
         newMargin = MIN_MARGIN;
      }

      int left;
      int top;
      int right;
      int bottom;

      layout()->getContentsMargins(&left, &top, &right, &bottom);
      layout()->setContentsMargins(left, top, right, newMargin);
   }
}
