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

#include <abstract_actioneditor.h>
#include <abstract_formbuilder.h>
#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <abstract_formwindowcursor.h>
#include <abstract_integration.h>
#include <abstract_propertyeditor.h>
#include <abstract_widgetfactory.h>
#include <designer_command.h>
#include <designer_command.h>
#include <designer_menu.h>
#include <designer_object_inspector.h>
#include <designer_property_command.h>
#include <designer_propertysheet.h>
#include <designer_utils.h>
#include <designer_widget.h>
#include <extension.h>
#include <layout.h>
#include <layout_widget.h>
#include <widgetfactory.h>

#include <formwindowbase_p.h>
#include <metadatabase_p.h>

#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QFormLayout>
#include <QLayout>
#include <QListWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QQueue>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextStream>
#include <QToolBar>
#include <QToolBox>
#include <QTreeWidget>
#include <QWizardPage>

CS_DECLARE_METATYPE(QWidgetList)

static inline void setPropertySheetWindowTitle(const QDesignerFormEditorInterface *core, QObject *o, const QString &t)
{
   if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), o)) {
      const int idx = sheet->indexOf(QString("windowTitle"));
      if (idx != -1) {
         sheet->setProperty(idx, t);
         sheet->setChanged(idx, true);
      }
   }
}

namespace qdesigner_internal {

// Helpers for the dynamic properties that store Z/Widget order
static const QString widgetOrderPropertyC = "_q_widgetOrder";
static const QString zOrderPropertyC      = "_q_zOrder";

static void addToWidgetListDynamicProperty(QWidget *parentWidget, QWidget *widget, const QString &name, int index = -1)
{
   QVariant data = parentWidget->property(name);
   QWidgetList list = data.value<QWidgetList>();

   list.removeAll(widget);

   if (index >= 0 && index < list.size()) {
      list.insert(index, widget);
   } else {
      list.append(widget);
   }

   parentWidget->setProperty(name, QVariant::fromValue(list));
}

static int removeFromWidgetListDynamicProperty(QWidget *parentWidget, QWidget *widget, const QString &name)
{
   QVariant data = parentWidget->property(name);
   QWidgetList list = data.value<QWidgetList>();

   const int firstIndex = list.indexOf(widget);

   if (firstIndex != -1) {
      list.removeAll(widget);
      parentWidget->setProperty(name, QVariant::fromValue(list));
   }

   return firstIndex;
}

InsertWidgetCommand::InsertWidgetCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QString(), formWindow),
     m_insertMode(QDesignerLayoutDecorationExtension::InsertWidgetMode),
     m_layoutHelper(nullptr), m_widgetWasManaged(false)
{
}

InsertWidgetCommand::~InsertWidgetCommand()
{
   delete m_layoutHelper;
}

void InsertWidgetCommand::init(QWidget *widget, bool already_in_form, int layoutRow, int layoutColumn)
{
   m_widget = widget;

   setText(QApplication::translate("Command", "Insert '%1'").formatArg(widget->objectName()));

   QWidget *parentWidget = m_widget->parentWidget();
   QDesignerFormEditorInterface *core = formWindow()->core();
   QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension *>(core->extensionManager(), parentWidget);

   m_insertMode = deco ? deco->currentInsertMode() : QDesignerLayoutDecorationExtension::InsertWidgetMode;

   if (layoutRow >= 0 && layoutColumn >= 0) {
      m_cell.first = layoutRow;
      m_cell.second = layoutColumn;
   } else {
      m_cell = deco ? deco->currentCell() : qMakePair(0, 0);
   }

   m_widgetWasManaged = already_in_form;
}

static void recursiveUpdate(QWidget *w)
{
   w->update();

   const QObjectList &l = w->children();
   const QObjectList::const_iterator cend = l.end();

   for (QObjectList::const_iterator iter = l.begin(); iter != cend; ++iter) {
      if (QWidget *item_widget = dynamic_cast<QWidget *>(*iter)) {
         recursiveUpdate(item_widget);
      }
   }
}

void InsertWidgetCommand::redo()
{
   QWidget *parentWidget = m_widget->parentWidget();
   Q_ASSERT(parentWidget);

   addToWidgetListDynamicProperty(parentWidget, m_widget, widgetOrderPropertyC);
   addToWidgetListDynamicProperty(parentWidget, m_widget, zOrderPropertyC);

   QDesignerFormEditorInterface *core = formWindow()->core();
   QDesignerLayoutDecorationExtension *deco =
         qt_extension<QDesignerLayoutDecorationExtension *>(core->extensionManager(), parentWidget);

   if (deco != nullptr) {
      const LayoutInfo::Type type = LayoutInfo::layoutType(core, LayoutInfo::managedLayout(core, parentWidget));
      m_layoutHelper = LayoutHelper::createLayoutHelper(type);
      m_layoutHelper->pushState(core, parentWidget);

      if (type == LayoutInfo::Grid) {
         switch (m_insertMode) {
            case QDesignerLayoutDecorationExtension::InsertRowMode: {
               deco->insertRow(m_cell.first);
            }
            break;

            case QDesignerLayoutDecorationExtension::InsertColumnMode: {
               deco->insertColumn(m_cell.second);
            }
            break;

            default:
               break;
         } // end switch
      }
      deco->insertWidget(m_widget, m_cell);
   }

   if (!m_widgetWasManaged) {
      formWindow()->manageWidget(m_widget);
   }
   m_widget->show();
   formWindow()->emitSelectionChanged();

   if (parentWidget && parentWidget->layout()) {
      recursiveUpdate(parentWidget);
      parentWidget->layout()->invalidate();
   }

   refreshBuddyLabels();
}

void InsertWidgetCommand::undo()
{
   QWidget *parentWidget = m_widget->parentWidget();

   QDesignerFormEditorInterface *core = formWindow()->core();
   QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension *>(core->extensionManager(), parentWidget);

   if (deco) {
      deco->removeWidget(m_widget);
      m_layoutHelper->popState(core, parentWidget);
   }

   if (!m_widgetWasManaged) {
      formWindow()->unmanageWidget(m_widget);
      m_widget->hide();
   }

   removeFromWidgetListDynamicProperty(parentWidget, m_widget, widgetOrderPropertyC);
   removeFromWidgetListDynamicProperty(parentWidget, m_widget, zOrderPropertyC);

   formWindow()->emitSelectionChanged();

   refreshBuddyLabels();
}

void InsertWidgetCommand::refreshBuddyLabels()
{
   typedef QList<QLabel *> LabelList;

   const LabelList label_list = formWindow()->findChildren<QLabel *>();
   if (label_list.empty()) {
      return;
   }

   const QString buddyProperty = QString("buddy");
   const QByteArray objectNameU8 = m_widget->objectName().toUtf8();

   // Re-set the buddy (The sheet locates the object by name and sets it)
   const LabelList::const_iterator cend = label_list.constEnd();

   for (LabelList::const_iterator iter = label_list.constBegin(); iter != cend; ++iter ) {
      if (QDesignerPropertySheetExtension *sheet = propertySheet(*iter)) {
         const int idx = sheet->indexOf(buddyProperty);

         if (idx != -1) {
            const QVariant value = sheet->property(idx);
            if (value.toByteArray() == objectNameU8) {
               sheet->setProperty(idx, value);
            }
         }
      }
   }
}

// ---- ChangeZOrderCommand ----
ChangeZOrderCommand::ChangeZOrderCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QString(), formWindow)
{
}

void ChangeZOrderCommand::init(QWidget *widget)
{
   Q_ASSERT(widget);

   m_widget = widget;

   setText(QApplication::translate("Command", "Change Z-order of '%1'").formatArg(widget->objectName()));

   QVariant data = widget->parentWidget()->property("_q_zOrder");
   m_oldParentZOrder = data.value<QWidgetList>();

   const int index = m_oldParentZOrder.indexOf(m_widget);
   if (index != -1 && index + 1 < m_oldParentZOrder.count()) {
      m_oldPreceding = m_oldParentZOrder.at(index + 1);
   }
}

void ChangeZOrderCommand::redo()
{
   m_widget->parentWidget()->setProperty("_q_zOrder", QVariant::fromValue(reorderWidget(m_oldParentZOrder, m_widget)));

   reorder(m_widget);
}

void ChangeZOrderCommand::undo()
{
   m_widget->parentWidget()->setProperty("_q_zOrder", QVariant::fromValue(m_oldParentZOrder));

   if (m_oldPreceding) {
      m_widget->stackUnder(m_oldPreceding);
   } else {
      m_widget->raise();
   }
}

// ---- RaiseWidgetCommand ----
RaiseWidgetCommand::RaiseWidgetCommand(QDesignerFormWindowInterface *formWindow)
   : ChangeZOrderCommand(formWindow)
{
}

void RaiseWidgetCommand::init(QWidget *widget)
{
   ChangeZOrderCommand::init(widget);
   setText(QApplication::translate("Command", "Raise '%1'").formatArg(widget->objectName()));
}

QWidgetList RaiseWidgetCommand::reorderWidget(const QWidgetList &list, QWidget *widget) const
{
   QWidgetList l = list;
   l.removeAll(widget);
   l.append(widget);
   return l;
}

void RaiseWidgetCommand::reorder(QWidget *widget) const
{
   widget->raise();
}

// ---- LowerWidgetCommand ----
LowerWidgetCommand::LowerWidgetCommand(QDesignerFormWindowInterface *formWindow)
   : ChangeZOrderCommand(formWindow)
{
}

QWidgetList LowerWidgetCommand::reorderWidget(const QWidgetList &list, QWidget *widget) const
{
   QWidgetList l = list;
   l.removeAll(widget);
   l.prepend(widget);
   return l;
}

void LowerWidgetCommand::init(QWidget *widget)
{
   ChangeZOrderCommand::init(widget);
   setText(QApplication::translate("Command", "Lower '%1'").formatArg(widget->objectName()));
}

void LowerWidgetCommand::reorder(QWidget *widget) const
{
   widget->lower();
}

// ---- ManageWidgetCommandHelper
ManageWidgetCommandHelper::ManageWidgetCommandHelper()
   : m_widget(nullptr)
{
}

void ManageWidgetCommandHelper::init(const QDesignerFormWindowInterface *fw, QWidget *widget)
{
   m_widget = widget;
   m_managedChildren.clear();

   const QWidgetList children = m_widget->findChildren<QWidget *>();
   if (children.empty()) {
      return;
   }

   m_managedChildren.reserve(children.size());
   const QWidgetList::const_iterator lcend = children.constEnd();

   for (QWidgetList::const_iterator iter = children.constBegin(); iter != lcend; ++iter)
      if (fw->isManaged(*iter)) {
         m_managedChildren.push_back(*iter);
      }
}

void ManageWidgetCommandHelper::init(QWidget *widget, const QVector<QWidget *> &managedChildren)
{
   m_widget = widget;
   m_managedChildren = managedChildren;
}

void ManageWidgetCommandHelper::manage(QDesignerFormWindowInterface *fw)
{
   // Manage the managed children after parent
   fw->manageWidget(m_widget);

   if (! m_managedChildren.empty()) {
      for (auto item : m_managedChildren) {
         fw->manageWidget(item);
      }
   }
}

void ManageWidgetCommandHelper::unmanage(QDesignerFormWindowInterface *fw)
{
   // Unmanage the managed children first
   if (! m_managedChildren.empty()) {
      for (auto item : m_managedChildren) {
         fw->unmanageWidget(item);
      }
   }

   fw->unmanageWidget(m_widget);
}

DeleteWidgetCommand::DeleteWidgetCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QString(), formWindow), m_layoutType(LayoutInfo::NoLayout),
     m_layoutHelper(nullptr), m_flags(0), m_splitterIndex(-1), m_layoutSimplified(false),
     m_formItem(nullptr), m_tabOrderIndex(-1), m_widgetOrderIndex(-1), m_zOrderIndex(-1)
{
}

DeleteWidgetCommand::~DeleteWidgetCommand()
{
   delete  m_layoutHelper;
}

void DeleteWidgetCommand::init(QWidget *widget, unsigned flags)
{
   m_widget        = widget;
   m_parentWidget  = widget->parentWidget();
   m_geometry      = widget->geometry();
   m_flags         = flags;
   m_layoutType    = LayoutInfo::NoLayout;
   m_splitterIndex = -1;

   bool isManaged; // Check for a managed layout

   QLayout *layout;
   m_layoutType = LayoutInfo::laidoutWidgetType(formWindow()->core(), m_widget, &isManaged, &layout);

   if (!isManaged) {
      m_layoutType = LayoutInfo::NoLayout;
   }

   switch (m_layoutType) {
      case LayoutInfo::HSplitter:
      case LayoutInfo::VSplitter: {
         QSplitter *splitter = dynamic_cast<QSplitter *>(m_parentWidget.data());
         Q_ASSERT(splitter);
         m_splitterIndex = splitter->indexOf(widget);
      }
      break;

      case LayoutInfo::NoLayout:
         break;

      default:
         m_layoutHelper = LayoutHelper::createLayoutHelper(m_layoutType);
         m_layoutPosition = m_layoutHelper->itemInfo(layout, m_widget);
         break;
   }

   m_formItem = formWindow()->core()->metaDataBase()->item(formWindow());
   m_tabOrderIndex = m_formItem->tabOrder().indexOf(widget);

   // Build the list of managed children
   m_manageHelper.init(formWindow(), m_widget);

   setText(QApplication::translate("Command", "Delete '%1'").formatArg(widget->objectName()));
}

void DeleteWidgetCommand::redo()
{
   formWindow()->clearSelection();
   QDesignerFormEditorInterface *core = formWindow()->core();

   if (QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension *>(core->extensionManager(), m_parentWidget)) {
      const int count = c->count();
      for (int i = 0; i < count; ++i) {
         if (c->widget(i) == m_widget) {
            c->remove(i);
            return;
         }
      }
   }

   m_widgetOrderIndex = removeFromWidgetListDynamicProperty(m_parentWidget, m_widget, widgetOrderPropertyC);
   m_zOrderIndex = removeFromWidgetListDynamicProperty(m_parentWidget, m_widget, zOrderPropertyC);

   if (QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension *>(core->extensionManager(),
            m_parentWidget)) {
      deco->removeWidget(m_widget);
   }

   if (m_layoutHelper)
      switch (m_layoutType) {
         case LayoutInfo::NoLayout:
         case LayoutInfo::HSplitter:
         case LayoutInfo::VSplitter:
            break;

         default:
            // Attempt to simplify grids if a row/column becomes empty
            m_layoutSimplified = (m_flags & DoNotSimplifyLayout) ? false : m_layoutHelper->canSimplify(core, m_parentWidget, m_layoutPosition);
            if (m_layoutSimplified) {
               m_layoutHelper->pushState(core, m_parentWidget);
               m_layoutHelper->simplify(core, m_parentWidget, m_layoutPosition);
            }
            break;
      }

   if (!(m_flags & DoNotUnmanage)) {
      m_manageHelper.unmanage(formWindow());
   }

   m_widget->setParent(formWindow());
   m_widget->hide();

   if (m_tabOrderIndex != -1) {
      QList<QWidget *> tab_order = m_formItem->tabOrder();
      tab_order.removeAt(m_tabOrderIndex);
      m_formItem->setTabOrder(tab_order);
   }
}

void DeleteWidgetCommand::undo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   formWindow()->clearSelection();

   m_widget->setParent(m_parentWidget);

   if (QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension *>(core->extensionManager(), m_parentWidget)) {
      c->addWidget(m_widget);
      return;
   }

   addToWidgetListDynamicProperty(m_parentWidget, m_widget, widgetOrderPropertyC, m_widgetOrderIndex);
   addToWidgetListDynamicProperty(m_parentWidget, m_widget, zOrderPropertyC, m_zOrderIndex);

   m_widget->setGeometry(m_geometry);

   if (!(m_flags & DoNotUnmanage)) {
      m_manageHelper.manage(formWindow());
   }
   // ### set up alignment
   switch (m_layoutType) {
      case LayoutInfo::NoLayout:
         break;
      case LayoutInfo::HSplitter:
      case LayoutInfo::VSplitter: {
         QSplitter *splitter = dynamic_cast<QSplitter *>(m_widget->parent());
         Q_ASSERT(splitter);
         splitter->insertWidget(m_splitterIndex, m_widget);
      }
      break;
      default: {
         Q_ASSERT(m_layoutHelper);
         if (m_layoutSimplified) {
            m_layoutHelper->popState(core, m_parentWidget);
         }
         QLayout *layout = LayoutInfo::managedLayout(core, m_parentWidget);
         Q_ASSERT(m_layoutType == LayoutInfo::layoutType(core, layout));
         m_layoutHelper->insertWidget(layout, m_layoutPosition, m_widget);
      }
      break;
   }

   m_widget->show();

   if (m_tabOrderIndex != -1) {
      QList<QWidget *> tab_order = m_formItem->tabOrder();
      tab_order.insert(m_tabOrderIndex, m_widget);
      m_formItem->setTabOrder(tab_order);
   }
}

ReparentWidgetCommand::ReparentWidgetCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QString(), formWindow)
{
}

void ReparentWidgetCommand::init(QWidget *widget, QWidget *parentWidget)
{
   Q_ASSERT(widget);

   m_widget = widget;
   m_oldParentWidget = widget->parentWidget();
   m_newParentWidget = parentWidget;

   m_oldPos = m_widget->pos();
   m_newPos = m_newParentWidget->mapFromGlobal(m_oldParentWidget->mapToGlobal(m_oldPos));

   setText(QApplication::translate("Command", "Reparent '%1'").formatArg(widget->objectName()));

   QVariant data   = m_oldParentWidget->property("_q_widgetOrder");
   m_oldParentList = data.value<QWidgetList>();

   data = m_oldParentWidget->property("_q_zOrder");
   m_oldParentZOrder = data.value<QWidgetList>();
}

void ReparentWidgetCommand::redo()
{
   m_widget->setParent(m_newParentWidget);
   m_widget->move(m_newPos);

   QWidgetList oldList = m_oldParentList;
   oldList.removeAll(m_widget);
   m_oldParentWidget->setProperty("_q_widgetOrder", QVariant::fromValue(oldList));

   QVariant data = m_newParentWidget->property("_q_widgetOrder");
   QWidgetList newList = data.value<QWidgetList>();

   newList.append(m_widget);
   m_newParentWidget->setProperty("_q_widgetOrder", QVariant::fromValue(newList));

   QWidgetList oldZOrder = m_oldParentZOrder;
   oldZOrder.removeAll(m_widget);
   m_oldParentWidget->setProperty("_q_zOrder", QVariant::fromValue(oldZOrder));

   data = m_newParentWidget->property("_q_zOrder");
   QWidgetList newZOrder = data.value<QWidgetList>();

   newZOrder.append(m_widget);
   m_newParentWidget->setProperty("_q_zOrder", QVariant::fromValue(newZOrder));

   m_widget->show();
   core()->objectInspector()->setFormWindow(formWindow());
}

void ReparentWidgetCommand::undo()
{
   m_widget->setParent(m_oldParentWidget);
   m_widget->move(m_oldPos);

   m_oldParentWidget->setProperty("_q_widgetOrder", QVariant::fromValue(m_oldParentList));

   QVariant data = m_newParentWidget->property("_q_widgetOrder");
   QWidgetList newList = data.value<QWidgetList>();

   newList.removeAll(m_widget);
   m_newParentWidget->setProperty("_q_widgetOrder", QVariant::fromValue(newList));

   m_oldParentWidget->setProperty("_q_zOrder", QVariant::fromValue(m_oldParentZOrder));

   data = m_newParentWidget->property("_q_zOrder");
   QWidgetList newZOrder = data.value<QWidgetList>();

   m_newParentWidget->setProperty("_q_zOrder", QVariant::fromValue(newZOrder));

   m_widget->show();
   core()->objectInspector()->setFormWindow(formWindow());
}

PromoteToCustomWidgetCommand::PromoteToCustomWidgetCommand
(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Promote to custom widget"), formWindow)
{
}

void PromoteToCustomWidgetCommand::init(const WidgetList &widgets, const QString &customClassName)
{
   m_widgets = widgets;
   m_customClassName = customClassName;
}

void PromoteToCustomWidgetCommand::redo()
{
   for (QWidget *w : m_widgets) {
      if (w) {
         promoteWidget(core(), w, m_customClassName);
      }
   }
   updateSelection();
}

void PromoteToCustomWidgetCommand::updateSelection()
{
   // Update class names in ObjectInspector, PropertyEditor
   QDesignerFormWindowInterface *fw = formWindow();
   QDesignerFormEditorInterface *core = fw->core();
   core->objectInspector()->setFormWindow(fw);
   if (QObject *o = core->propertyEditor()->object()) {
      core->propertyEditor()->setObject(o);
   }
}

void PromoteToCustomWidgetCommand::undo()
{
   for (QWidget *w : m_widgets) {
      if (w) {
         demoteWidget(core(), w);
      }
   }
   updateSelection();
}

// ---- DemoteFromCustomWidgetCommand ----

DemoteFromCustomWidgetCommand::DemoteFromCustomWidgetCommand
(QDesignerFormWindowInterface *formWindow) :
   QDesignerFormWindowCommand(QApplication::translate("Command", "Demote from custom widget"), formWindow),
   m_promote_cmd(formWindow)
{
}

void DemoteFromCustomWidgetCommand::init(const WidgetList &promoted)
{
   m_promote_cmd.init(promoted, promotedCustomClassName(core(), promoted.front()));
}

void DemoteFromCustomWidgetCommand::redo()
{
   m_promote_cmd.undo();
}

void DemoteFromCustomWidgetCommand::undo()
{
   m_promote_cmd.redo();
}

CursorSelectionState::CursorSelectionState()
{
}

void CursorSelectionState::save(const QDesignerFormWindowInterface *formWindow)
{
   const QDesignerFormWindowCursorInterface *cursor = formWindow->cursor();
   m_selection.clear();
   m_current = cursor->current();

   if (cursor->hasSelection()) {
      const int count = cursor->selectedWidgetCount();
      for (int i = 0; i < count; i++) {
         m_selection.push_back(cursor->selectedWidget(i));
      }
   }
}

void CursorSelectionState::restore(QDesignerFormWindowInterface *formWindow) const
{
   if (m_selection.empty()) {
      formWindow->clearSelection(true);

   } else {
      // Select current as last
      formWindow->clearSelection(false);
      const WidgetPointerList::const_iterator cend = m_selection.constEnd();

      for (WidgetPointerList::const_iterator iter = m_selection.constBegin(); iter != cend; ++iter)
         if (QWidget *w = *iter) {
            if (w != m_current) {
               formWindow->selectWidget(*iter, true);
            }
         }

      if (m_current) {
         formWindow->selectWidget(m_current, true);
      }
   }
}

// ---- LayoutCommand ----

LayoutCommand::LayoutCommand(QDesignerFormWindowInterface *formWindow) :
   QDesignerFormWindowCommand(QString(), formWindow),
   m_setup(false)
{
}

LayoutCommand::~LayoutCommand()
{
   delete m_layout;
}

void LayoutCommand::init(QWidget *parentWidget, const QWidgetList &widgets,
   LayoutInfo::Type layoutType, QWidget *layoutBase,
   bool reparentLayoutWidget)
{
   m_parentWidget = parentWidget;
   m_widgets = widgets;
   formWindow()->simplifySelection(&m_widgets);
   m_layout = Layout::createLayout(widgets, parentWidget, formWindow(), layoutBase, layoutType);
   m_layout->setReparentLayoutWidget(reparentLayoutWidget);

   switch (layoutType) {
      case LayoutInfo::Grid:
         setText(QApplication::translate("Command", "Lay out using grid"));
         break;
      case LayoutInfo::VBox:
         setText(QApplication::translate("Command", "Lay out vertically"));
         break;
      case LayoutInfo::HBox:
         setText(QApplication::translate("Command", "Lay out horizontally"));
         break;
      default:
         break;
   }
   // Delayed setup to avoid confusion in case we are chained
   // with a BreakLayout in a morph layout macro
   m_setup = false;
}

void LayoutCommand::redo()
{
   if (!m_setup) {
      m_layout->setup();
      m_cursorSelectionState.save(formWindow());
      m_setup = true;
   }
   m_layout->doLayout();
   core()->objectInspector()->setFormWindow(formWindow());
}

void LayoutCommand::undo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();

   QWidget *lb = m_layout->layoutBaseWidget();

   QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension *>(core->extensionManager(), lb);
   m_layout->undoLayout();
   delete deco; // release the extension

   // ### generalize (put in function)
   if (!m_layoutBase && lb != nullptr && !(dynamic_cast<QLayoutWidget *>(lb) || dynamic_cast<QSplitter *>(lb))) {
      core->metaDataBase()->add(lb);
      lb->show();
   }

   m_cursorSelectionState.restore(formWindow());
   core->objectInspector()->setFormWindow(formWindow());
}

BreakLayoutCommand::BreakLayoutCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Break layout"), formWindow),
     m_layoutHelper(nullptr), m_properties(nullptr), m_propertyMask(0)
{
}

BreakLayoutCommand::~BreakLayoutCommand()
{
   delete m_layoutHelper;
   delete m_layout;
   delete m_properties;
}

const LayoutProperties *BreakLayoutCommand::layoutProperties() const
{
   return m_properties;
}

int BreakLayoutCommand::propertyMask() const
{
   return m_propertyMask;
}

void BreakLayoutCommand::init(const QWidgetList &widgets, QWidget *layoutBase, bool reparentLayoutWidget)
{
   enum Type { SplitterLayout, LayoutHasMarginSpacing, LayoutHasState };

   const QDesignerFormEditorInterface *core = formWindow()->core();
   m_widgets = widgets;
   m_layoutBase = core->widgetFactory()->containerOfWidget(layoutBase);

   QLayout *layoutToBeBroken;
   const LayoutInfo::Type layoutType = LayoutInfo::managedLayoutType(core, m_layoutBase, &layoutToBeBroken);
   m_layout = Layout::createLayout(widgets, m_layoutBase, formWindow(), layoutBase, layoutType);
   m_layout->setReparentLayoutWidget(reparentLayoutWidget);

   Type type = LayoutHasState;

   switch (layoutType) {
      case LayoutInfo::NoLayout:
      case LayoutInfo::HSplitter:
      case LayoutInfo::VSplitter:
         type = SplitterLayout;
         break;

      case LayoutInfo::HBox:
      case LayoutInfo::VBox: // Margin/spacing need to be saved
         type = LayoutHasMarginSpacing;
         break;

      default: // Margin/spacing need to be saved + has a state (empty rows/columns of a grid)
         type = LayoutHasState;
         break;
   }

   Q_ASSERT(m_layout != nullptr);
   m_layout->sort();

   if (type >= LayoutHasMarginSpacing) {
      m_properties = new LayoutProperties;
      m_propertyMask = m_properties->fromPropertySheet(core, layoutToBeBroken, LayoutProperties::AllProperties);
   }

   if (type >= LayoutHasState) {
      m_layoutHelper = LayoutHelper::createLayoutHelper(layoutType);
   }
   m_cursorSelectionState.save(formWindow());
}

void BreakLayoutCommand::redo()
{
   if (!m_layout) {
      return;
   }

   QDesignerFormEditorInterface *core = formWindow()->core();
   QWidget *lb = m_layout->layoutBaseWidget();
   QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension *>(core->extensionManager(), lb);
   formWindow()->clearSelection(false);
   if (m_layoutHelper) {
      m_layoutHelper->pushState(core, m_layoutBase);
   }
   m_layout->breakLayout();
   delete deco; // release the extension

   for (QWidget *widget : m_widgets) {
      widget->resize(widget->size().expandedTo(QSize(16, 16)));
   }
   // Update unless we are in an intermediate state of morphing layout
   // in which a QLayoutWidget will have no layout at all.
   if (m_layout->reparentLayoutWidget()) {
      core->objectInspector()->setFormWindow(formWindow());
   }
}

void BreakLayoutCommand::undo()
{
   if (!m_layout) {
      return;
   }

   formWindow()->clearSelection(false);
   m_layout->doLayout();

   if (m_layoutHelper) {
      m_layoutHelper->popState(formWindow()->core(), m_layoutBase);
   }

   QLayout *layoutToRestored = LayoutInfo::managedLayout(formWindow()->core(), m_layoutBase);

   if (m_properties && m_layoutBase && layoutToRestored) {
      m_properties->toPropertySheet(formWindow()->core(), layoutToRestored, m_propertyMask);
   }

   m_cursorSelectionState.restore(formWindow());
   core()->objectInspector()->setFormWindow(formWindow());
}

SimplifyLayoutCommand::SimplifyLayoutCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Simplify Grid Layout"), formWindow),
     m_area(0, 0, 32767, 32767), m_layoutBase(nullptr), m_layoutHelper(nullptr),
     m_layoutSimplified(false)
{
}

SimplifyLayoutCommand::~SimplifyLayoutCommand()
{
   delete m_layoutHelper;
}

bool SimplifyLayoutCommand::canSimplify(QDesignerFormEditorInterface *core, const QWidget *w, int *layoutType)
{
   if (! w) {
      return false;
   }

   QLayout *layout;
   const LayoutInfo::Type type = LayoutInfo::managedLayoutType(core, w, &layout);

   if (layoutType) {
      *layoutType = type;
   }
   if (!layout) {
      return false;
   }
   switch (type) { // Known negatives
      case LayoutInfo::NoLayout:
      case LayoutInfo::UnknownLayout:
      case LayoutInfo::HSplitter:
      case LayoutInfo::VSplitter:
      case LayoutInfo::HBox:
      case LayoutInfo::VBox:
         return false;
      default:
         break;
   }
   switch (type) {
      case LayoutInfo::Grid:
         return QLayoutSupport::canSimplifyQuickCheck(dynamic_cast<QGridLayout *>(layout));
      case LayoutInfo::Form:
         return QLayoutSupport::canSimplifyQuickCheck(dynamic_cast<const QFormLayout *>(layout));
      default:
         break;
   }
   return false;
}

bool SimplifyLayoutCommand::init(QWidget *layoutBase)
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   m_layoutSimplified = false;
   int type;
   if (canSimplify(core, layoutBase, &type)) {
      m_layoutBase = layoutBase;
      m_layoutHelper = LayoutHelper::createLayoutHelper(type);
      m_layoutSimplified = m_layoutHelper->canSimplify(core, layoutBase, m_area);
   }
   return m_layoutSimplified;
}

void SimplifyLayoutCommand::redo()
{
   const QDesignerFormEditorInterface *core = formWindow()->core();
   if (m_layoutSimplified) {
      m_layoutHelper->pushState(core, m_layoutBase);
      m_layoutHelper->simplify(core, m_layoutBase, m_area);
   }
}
void SimplifyLayoutCommand::undo()
{
   if (m_layoutSimplified) {
      m_layoutHelper->popState(formWindow()->core(), m_layoutBase);
   }
}

// ---- ToolBoxCommand ----
ToolBoxCommand::ToolBoxCommand(QDesignerFormWindowInterface *formWindow)  :
   QDesignerFormWindowCommand(QString(), formWindow),
   m_index(-1)
{
}

ToolBoxCommand::~ToolBoxCommand()
{
}

void ToolBoxCommand::init(QToolBox *toolBox)
{
   m_toolBox = toolBox;
   m_index = m_toolBox->currentIndex();
   m_widget = m_toolBox->widget(m_index);
   m_itemText = m_toolBox->itemText(m_index);
   m_itemIcon = m_toolBox->itemIcon(m_index);
}

void ToolBoxCommand::removePage()
{
   m_toolBox->removeItem(m_index);

   m_widget->hide();
   m_widget->setParent(formWindow());
   formWindow()->clearSelection();
   formWindow()->selectWidget(m_toolBox, true);

}

void ToolBoxCommand::addPage()
{
   m_widget->setParent(m_toolBox);
   m_toolBox->insertItem(m_index, m_widget, m_itemIcon, m_itemText);
   m_toolBox->setCurrentIndex(m_index);

   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(formWindow()->core()->extensionManager(),
         m_toolBox);
   if (sheet) {
      qdesigner_internal::PropertySheetStringValue itemText(m_itemText);
      sheet->setProperty(sheet->indexOf(QString("currentItemText")), QVariant::fromValue(itemText));
   }

   m_widget->show();
   formWindow()->clearSelection();
   formWindow()->selectWidget(m_toolBox, true);
}

// ---- MoveToolBoxPageCommand ----
MoveToolBoxPageCommand::MoveToolBoxPageCommand(QDesignerFormWindowInterface *formWindow) :
   ToolBoxCommand(formWindow),
   m_newIndex(-1),
   m_oldIndex(-1)
{
}

MoveToolBoxPageCommand::~MoveToolBoxPageCommand()
{
}

void MoveToolBoxPageCommand::init(QToolBox *toolBox, QWidget *page, int newIndex)
{
   ToolBoxCommand::init(toolBox);
   setText(QApplication::translate("Command", "Move Page"));

   m_widget = page;
   m_oldIndex = m_toolBox->indexOf(m_widget);
   m_itemText = m_toolBox->itemText(m_oldIndex);
   m_itemIcon = m_toolBox->itemIcon(m_oldIndex);
   m_newIndex = newIndex;
}

void MoveToolBoxPageCommand::redo()
{
   m_toolBox->removeItem(m_oldIndex);
   m_toolBox->insertItem(m_newIndex, m_widget, m_itemIcon, m_itemText);
}

void MoveToolBoxPageCommand::undo()
{
   m_toolBox->removeItem(m_newIndex);
   m_toolBox->insertItem(m_oldIndex, m_widget, m_itemIcon, m_itemText);
}

// ---- DeleteToolBoxPageCommand ----
DeleteToolBoxPageCommand::DeleteToolBoxPageCommand(QDesignerFormWindowInterface *formWindow)
   : ToolBoxCommand(formWindow)
{
}

DeleteToolBoxPageCommand::~DeleteToolBoxPageCommand()
{
}

void DeleteToolBoxPageCommand::init(QToolBox *toolBox)
{
   ToolBoxCommand::init(toolBox);
   setText(QApplication::translate("Command", "Delete Page"));
}

void DeleteToolBoxPageCommand::redo()
{
   removePage();
   cheapUpdate();
}

void DeleteToolBoxPageCommand::undo()
{
   addPage();
   cheapUpdate();
}

// ---- AddToolBoxPageCommand ----
AddToolBoxPageCommand::AddToolBoxPageCommand(QDesignerFormWindowInterface *formWindow)
   : ToolBoxCommand(formWindow)
{
}

AddToolBoxPageCommand::~AddToolBoxPageCommand()
{
}

void AddToolBoxPageCommand::init(QToolBox *toolBox)
{
   init(toolBox, InsertBefore);
}

void AddToolBoxPageCommand::init(QToolBox *toolBox, InsertionMode mode)
{
   m_toolBox = toolBox;

   m_index = m_toolBox->currentIndex();
   if (mode == InsertAfter) {
      ++m_index;
   }
   m_widget = new QDesignerWidget(formWindow(), m_toolBox);
   m_itemText = QApplication::translate("Command", "Page");
   m_itemIcon = QIcon();
   m_widget->setObjectName(QString("page"));
   formWindow()->ensureUniqueObjectName(m_widget);

   setText(QApplication::translate("Command", "Insert Page"));

   QDesignerFormEditorInterface *core = formWindow()->core();
   core->metaDataBase()->add(m_widget);
}

void AddToolBoxPageCommand::redo()
{
   addPage();
   cheapUpdate();
}

void AddToolBoxPageCommand::undo()
{
   removePage();
   cheapUpdate();
}

// ---- TabWidgetCommand ----
TabWidgetCommand::TabWidgetCommand(QDesignerFormWindowInterface *formWindow) :
   QDesignerFormWindowCommand(QString(), formWindow),
   m_index(-1)
{
}

TabWidgetCommand::~TabWidgetCommand()
{
}

void TabWidgetCommand::init(QTabWidget *tabWidget)
{
   m_tabWidget = tabWidget;
   m_index = m_tabWidget->currentIndex();
   m_widget = m_tabWidget->widget(m_index);
   m_itemText = m_tabWidget->tabText(m_index);
   m_itemIcon = m_tabWidget->tabIcon(m_index);
}

void TabWidgetCommand::removePage()
{
   m_tabWidget->removeTab(m_index);

   m_widget->hide();
   m_widget->setParent(formWindow());
   m_tabWidget->setCurrentIndex(qMin(m_index, m_tabWidget->count()));

   formWindow()->clearSelection();
   formWindow()->selectWidget(m_tabWidget, true);
}

void TabWidgetCommand::addPage()
{
   m_widget->setParent(nullptr);
   m_tabWidget->insertTab(m_index, m_widget, m_itemIcon, m_itemText);
   m_widget->show();
   m_tabWidget->setCurrentIndex(m_index);

   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(formWindow()->core()->extensionManager(),
         m_tabWidget);
   if (sheet) {
      qdesigner_internal::PropertySheetStringValue itemText(m_itemText);
      sheet->setProperty(sheet->indexOf(QString("currentTabText")), QVariant::fromValue(itemText));
   }

   formWindow()->clearSelection();
   formWindow()->selectWidget(m_tabWidget, true);
}

// ---- DeleteTabPageCommand ----
DeleteTabPageCommand::DeleteTabPageCommand(QDesignerFormWindowInterface *formWindow)
   : TabWidgetCommand(formWindow)
{
}

DeleteTabPageCommand::~DeleteTabPageCommand()
{
}

void DeleteTabPageCommand::init(QTabWidget *tabWidget)
{
   TabWidgetCommand::init(tabWidget);
   setText(QApplication::translate("Command", "Delete Page"));
}

void DeleteTabPageCommand::redo()
{
   removePage();
   cheapUpdate();
}

void DeleteTabPageCommand::undo()
{
   addPage();
   cheapUpdate();
}

// ---- AddTabPageCommand ----
AddTabPageCommand::AddTabPageCommand(QDesignerFormWindowInterface *formWindow)
   : TabWidgetCommand(formWindow)
{
}

AddTabPageCommand::~AddTabPageCommand()
{
}

void AddTabPageCommand::init(QTabWidget *tabWidget)
{
   init(tabWidget, InsertBefore);
}

void AddTabPageCommand::init(QTabWidget *tabWidget, InsertionMode mode)
{
   m_tabWidget = tabWidget;

   m_index = m_tabWidget->currentIndex();
   if (mode == InsertAfter) {
      ++m_index;
   }
   m_widget = new QDesignerWidget(formWindow(), m_tabWidget);
   m_itemText = QApplication::translate("Command", "Page");
   m_itemIcon = QIcon();
   m_widget->setObjectName(QString("tab"));
   formWindow()->ensureUniqueObjectName(m_widget);

   setText(QApplication::translate("Command", "Insert Page"));

   QDesignerFormEditorInterface *core = formWindow()->core();
   core->metaDataBase()->add(m_widget);
}

void AddTabPageCommand::redo()
{
   addPage();
   cheapUpdate();
}

void AddTabPageCommand::undo()
{
   removePage();
   cheapUpdate();
}

// ---- MoveTabPageCommand ----
MoveTabPageCommand::MoveTabPageCommand(QDesignerFormWindowInterface *formWindow) :
   TabWidgetCommand(formWindow),
   m_newIndex(-1),
   m_oldIndex(-1)
{
}

MoveTabPageCommand::~MoveTabPageCommand()
{
}

void MoveTabPageCommand::init(QTabWidget *tabWidget, QWidget *page,
   const QIcon &icon, const QString &label,
   int index, int newIndex)
{
   TabWidgetCommand::init(tabWidget);
   setText(QApplication::translate("Command", "Move Page"));

   m_page = page;
   m_newIndex = newIndex;
   m_oldIndex = index;
   m_label = label;
   m_icon = icon;
}

void MoveTabPageCommand::redo()
{
   m_tabWidget->removeTab(m_oldIndex);
   m_tabWidget->insertTab(m_newIndex, m_page, m_icon, m_label);
   m_tabWidget->setCurrentIndex(m_newIndex);
}

void MoveTabPageCommand::undo()
{
   m_tabWidget->removeTab(m_newIndex);
   m_tabWidget->insertTab(m_oldIndex, m_page, m_icon, m_label);
   m_tabWidget->setCurrentIndex(m_oldIndex);
}

// ---- StackedWidgetCommand ----
StackedWidgetCommand::StackedWidgetCommand(QDesignerFormWindowInterface *formWindow) :
   QDesignerFormWindowCommand(QString(), formWindow),
   m_index(-1)
{
}

StackedWidgetCommand::~StackedWidgetCommand()
{
}

void StackedWidgetCommand::init(QStackedWidget *stackedWidget)
{
   m_stackedWidget = stackedWidget;
   m_index = m_stackedWidget->currentIndex();
   m_widget = m_stackedWidget->widget(m_index);
}

void StackedWidgetCommand::removePage()
{
   m_stackedWidget->removeWidget(m_stackedWidget->widget(m_index));

   m_widget->hide();
   m_widget->setParent(formWindow());

   formWindow()->clearSelection();
   formWindow()->selectWidget(m_stackedWidget, true);
}

void StackedWidgetCommand::addPage()
{
   m_stackedWidget->insertWidget(m_index, m_widget);

   m_widget->show();
   m_stackedWidget->setCurrentIndex(m_index);

   formWindow()->clearSelection();
   formWindow()->selectWidget(m_stackedWidget, true);
}

// ---- MoveStackedWidgetCommand ----
MoveStackedWidgetCommand::MoveStackedWidgetCommand(QDesignerFormWindowInterface *formWindow) :
   StackedWidgetCommand(formWindow),
   m_newIndex(-1),
   m_oldIndex(-1)
{
}

MoveStackedWidgetCommand::~MoveStackedWidgetCommand()
{
}

void MoveStackedWidgetCommand::init(QStackedWidget *stackedWidget, QWidget *page, int newIndex)
{
   StackedWidgetCommand::init(stackedWidget);
   setText(QApplication::translate("Command", "Move Page"));

   m_widget = page;
   m_newIndex = newIndex;
   m_oldIndex = m_stackedWidget->indexOf(m_widget);
}

void MoveStackedWidgetCommand::redo()
{
   m_stackedWidget->removeWidget(m_widget);
   m_stackedWidget->insertWidget(m_newIndex, m_widget);
}

void MoveStackedWidgetCommand::undo()
{
   m_stackedWidget->removeWidget(m_widget);
   m_stackedWidget->insertWidget(m_oldIndex, m_widget);
}

// ---- DeleteStackedWidgetPageCommand ----
DeleteStackedWidgetPageCommand::DeleteStackedWidgetPageCommand(QDesignerFormWindowInterface *formWindow)
   : StackedWidgetCommand(formWindow)
{
}

DeleteStackedWidgetPageCommand::~DeleteStackedWidgetPageCommand()
{
}

void DeleteStackedWidgetPageCommand::init(QStackedWidget *stackedWidget)
{
   StackedWidgetCommand::init(stackedWidget);
   setText(QApplication::translate("Command", "Delete Page"));
}

void DeleteStackedWidgetPageCommand::redo()
{
   removePage();
   cheapUpdate();
}

void DeleteStackedWidgetPageCommand::undo()
{
   addPage();
   cheapUpdate();
}

// ---- AddStackedWidgetPageCommand ----
AddStackedWidgetPageCommand::AddStackedWidgetPageCommand(QDesignerFormWindowInterface *formWindow)
   : StackedWidgetCommand(formWindow)
{
}

AddStackedWidgetPageCommand::~AddStackedWidgetPageCommand()
{
}

void AddStackedWidgetPageCommand::init(QStackedWidget *stackedWidget)
{
   init(stackedWidget, InsertBefore);
}

void AddStackedWidgetPageCommand::init(QStackedWidget *stackedWidget, InsertionMode mode)
{
   m_stackedWidget = stackedWidget;

   m_index = m_stackedWidget->currentIndex();
   if (mode == InsertAfter) {
      ++m_index;
   }
   m_widget = new QDesignerWidget(formWindow(), m_stackedWidget);
   m_widget->setObjectName(QString("page"));
   formWindow()->ensureUniqueObjectName(m_widget);

   setText(QApplication::translate("Command", "Insert Page"));

   QDesignerFormEditorInterface *core = formWindow()->core();
   core->metaDataBase()->add(m_widget);
}

void AddStackedWidgetPageCommand::redo()
{
   addPage();
   cheapUpdate();
}

void AddStackedWidgetPageCommand::undo()
{
   removePage();
   cheapUpdate();
}

// ---- TabOrderCommand ----
TabOrderCommand::TabOrderCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Change Tab order"), formWindow),
     m_widgetItem(nullptr)
{
}

void TabOrderCommand::init(const QList<QWidget *> &newTabOrder)
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   Q_ASSERT(core);

   m_widgetItem = core->metaDataBase()->item(formWindow());
   Q_ASSERT(m_widgetItem);
   m_oldTabOrder = m_widgetItem->tabOrder();
   m_newTabOrder = newTabOrder;
}

void TabOrderCommand::redo()
{
   m_widgetItem->setTabOrder(m_newTabOrder);
}

void TabOrderCommand::undo()
{
   m_widgetItem->setTabOrder(m_oldTabOrder);
}

CreateMenuBarCommand::CreateMenuBarCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Create Menu Bar"), formWindow)
{
}

void CreateMenuBarCommand::init(QMainWindow *mainWindow)
{
   m_mainWindow = mainWindow;
   QDesignerFormEditorInterface *core = formWindow()->core();
   m_menuBar = dynamic_cast<QMenuBar *>(core->widgetFactory()->createWidget(QString("QMenuBar"), m_mainWindow));
   core->widgetFactory()->initialize(m_menuBar);
}

void CreateMenuBarCommand::redo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   QDesignerContainerExtension *c;
   c = qt_extension<QDesignerContainerExtension *>(core->extensionManager(), m_mainWindow);
   c->addWidget(m_menuBar);

   m_menuBar->setObjectName(QString("menuBar"));
   formWindow()->ensureUniqueObjectName(m_menuBar);
   core->metaDataBase()->add(m_menuBar);
   formWindow()->emitSelectionChanged();
   m_menuBar->setFocus();
}

void CreateMenuBarCommand::undo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   QDesignerContainerExtension *c;
   c = qt_extension<QDesignerContainerExtension *>(core->extensionManager(), m_mainWindow);

   for (int i = 0; i < c->count(); ++i) {
      if (c->widget(i) == m_menuBar) {
         c->remove(i);
         break;
      }
   }

   core->metaDataBase()->remove(m_menuBar);
   formWindow()->emitSelectionChanged();
}

// ---- DeleteMenuBarCommand ----
DeleteMenuBarCommand::DeleteMenuBarCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Delete Menu Bar"), formWindow)
{
}

void DeleteMenuBarCommand::init(QMenuBar *menuBar)
{
   m_menuBar = menuBar;
   m_mainWindow = dynamic_cast<QMainWindow *>(menuBar->parentWidget());
}

void DeleteMenuBarCommand::redo()
{
   if (m_mainWindow) {
      QDesignerContainerExtension *c;
      c = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), m_mainWindow);
      Q_ASSERT(c != nullptr);

      for (int i = 0; i < c->count(); ++i) {
         if (c->widget(i) == m_menuBar) {
            c->remove(i);
            break;
         }
      }
   }

   core()->metaDataBase()->remove(m_menuBar);
   m_menuBar->hide();
   m_menuBar->setParent(formWindow());
   formWindow()->emitSelectionChanged();
}

void DeleteMenuBarCommand::undo()
{
   if (m_mainWindow) {
      m_menuBar->setParent(m_mainWindow);
      QDesignerContainerExtension *c;
      c = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), m_mainWindow);

      c->addWidget(m_menuBar);

      core()->metaDataBase()->add(m_menuBar);
      m_menuBar->show();
      formWindow()->emitSelectionChanged();
   }
}

// ---- CreateStatusBarCommand ----
CreateStatusBarCommand::CreateStatusBarCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Create Status Bar"), formWindow)
{
}

void CreateStatusBarCommand::init(QMainWindow *mainWindow)
{
   m_mainWindow = mainWindow;
   QDesignerFormEditorInterface *core = formWindow()->core();
   m_statusBar = dynamic_cast<QStatusBar *>(core->widgetFactory()->createWidget(QString("QStatusBar"), m_mainWindow));
   core->widgetFactory()->initialize(m_statusBar);
}

void CreateStatusBarCommand::redo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   QDesignerContainerExtension *c;
   c = qt_extension<QDesignerContainerExtension *>(core->extensionManager(), m_mainWindow);
   c->addWidget(m_statusBar);

   m_statusBar->setObjectName(QString("statusBar"));
   formWindow()->ensureUniqueObjectName(m_statusBar);
   core->metaDataBase()->add(m_statusBar);
   formWindow()->emitSelectionChanged();
}

void CreateStatusBarCommand::undo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension *>(core->extensionManager(), m_mainWindow);

   for (int i = 0; i < c->count(); ++i) {
      if (c->widget(i) == m_statusBar) {
         c->remove(i);
         break;
      }
   }

   core->metaDataBase()->remove(m_statusBar);
   formWindow()->emitSelectionChanged();
}

// ---- DeleteStatusBarCommand ----
DeleteStatusBarCommand::DeleteStatusBarCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Delete Status Bar"), formWindow)
{
}

void DeleteStatusBarCommand::init(QStatusBar *statusBar)
{
   m_statusBar = statusBar;
   m_mainWindow = dynamic_cast<QMainWindow *>(statusBar->parentWidget());
}

void DeleteStatusBarCommand::redo()
{
   if (m_mainWindow) {
      QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), m_mainWindow);
      Q_ASSERT(c != nullptr);

      for (int i = 0; i < c->count(); ++i) {
         if (c->widget(i) == m_statusBar) {
            c->remove(i);
            break;
         }
      }
   }

   core()->metaDataBase()->remove(m_statusBar);
   m_statusBar->hide();
   m_statusBar->setParent(formWindow());
   formWindow()->emitSelectionChanged();
}

void DeleteStatusBarCommand::undo()
{
   if (m_mainWindow) {
      m_statusBar->setParent(m_mainWindow);
      QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), m_mainWindow);

      c->addWidget(m_statusBar);

      core()->metaDataBase()->add(m_statusBar);
      m_statusBar->show();
      formWindow()->emitSelectionChanged();
   }
}

// ---- AddToolBarCommand ----
AddToolBarCommand::AddToolBarCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Add Tool Bar"), formWindow)
{
}

void AddToolBarCommand::init(QMainWindow *mainWindow)
{
   m_mainWindow = mainWindow;
   QDesignerWidgetFactoryInterface *wf =  formWindow()->core()->widgetFactory();
   // Pass on 0 parent first to avoid reparenting flicker.
   m_toolBar = dynamic_cast<QToolBar *>(wf->createWidget(QString("QToolBar"), nullptr));
   wf->initialize(m_toolBar);
   m_toolBar->hide();
}

void AddToolBarCommand::redo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   core->metaDataBase()->add(m_toolBar);

   QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension *>(core->extensionManager(), m_mainWindow);
   c->addWidget(m_toolBar);

   m_toolBar->setObjectName(QString("toolBar"));
   formWindow()->ensureUniqueObjectName(m_toolBar);
   setPropertySheetWindowTitle(core, m_toolBar, m_toolBar->objectName());
   formWindow()->emitSelectionChanged();
}

void AddToolBarCommand::undo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   core->metaDataBase()->remove(m_toolBar);
   QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension *>(core->extensionManager(), m_mainWindow);
   for (int i = 0; i < c->count(); ++i) {
      if (c->widget(i) == m_toolBar) {
         c->remove(i);
         break;
      }
   }
   formWindow()->emitSelectionChanged();
}

// ---- DockWidgetCommand:: ----
DockWidgetCommand::DockWidgetCommand(const QString &description, QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(description, formWindow)
{
}

DockWidgetCommand::~DockWidgetCommand()
{
}

void DockWidgetCommand::init(QDockWidget *dockWidget)
{
   m_dockWidget = dockWidget;
}

// ---- AddDockWidgetCommand ----
AddDockWidgetCommand::AddDockWidgetCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Add Dock Window"), formWindow)
{
}

void AddDockWidgetCommand::init(QMainWindow *mainWindow, QDockWidget *dockWidget)
{
   m_mainWindow = mainWindow;
   m_dockWidget = dockWidget;
}

void AddDockWidgetCommand::init(QMainWindow *mainWindow)
{
   m_mainWindow = mainWindow;
   QDesignerFormEditorInterface *core = formWindow()->core();
   m_dockWidget = dynamic_cast<QDockWidget *>(core->widgetFactory()->createWidget(QString("QDockWidget"), m_mainWindow));
}

void AddDockWidgetCommand::redo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension *>(core->extensionManager(), m_mainWindow);
   c->addWidget(m_dockWidget);

   m_dockWidget->setObjectName(QString("dockWidget"));
   formWindow()->ensureUniqueObjectName(m_dockWidget);
   formWindow()->manageWidget(m_dockWidget);
   formWindow()->emitSelectionChanged();
}

void AddDockWidgetCommand::undo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension *>(core->extensionManager(), m_mainWindow);
   for (int i = 0; i < c->count(); ++i) {
      if (c->widget(i) == m_dockWidget) {
         c->remove(i);
         break;
      }
   }

   formWindow()->unmanageWidget(m_dockWidget);
   formWindow()->emitSelectionChanged();
}

// ---- AdjustWidgetSizeCommand ----
AdjustWidgetSizeCommand::AdjustWidgetSizeCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QString(), formWindow)
{
}

void AdjustWidgetSizeCommand::init(QWidget *widget)
{
   m_widget = widget;
   setText(QApplication::translate("Command", "Adjust Size of '%1'").formatArg(widget->objectName()));
}

QWidget *AdjustWidgetSizeCommand::widgetForAdjust() const
{
   QDesignerFormWindowInterface *fw = formWindow();

   // Return the outer, embedding widget if it is the main container
   if (Utils::isCentralWidget(fw, m_widget)) {
      return fw->core()->integration()->containerWindow(m_widget);
   }

   return m_widget;
}

void AdjustWidgetSizeCommand::redo()
{
   QWidget *aw = widgetForAdjust();
   m_geometry = aw->geometry();

   QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
   aw->adjustSize();
   const bool isMainContainer = aw != m_widget;

   if (!isMainContainer) {
      /* When doing adjustsize on a selected non-laid out child that has been enlarged
       * and pushed partially over the top/left edge[s], it is possible that it "disappears"
       * when shrinking. In that case, move it back so that it remains visible. */
      if (aw->parentWidget()->layout() == nullptr) {
         const QRect contentsRect = aw->parentWidget()->contentsRect();
         const QRect newGeometry = aw->geometry();
         QPoint newPos = m_geometry.topLeft();

         if (newGeometry.bottom() <= contentsRect.y()) {
            newPos.setY(contentsRect.y());
         }

         if (newGeometry.right() <= contentsRect.x()) {
            newPos.setX(contentsRect.x());
         }

         if (newPos != m_geometry.topLeft()) {
            aw->move(newPos);
         }
      }
   }
   updatePropertyEditor();
}

void AdjustWidgetSizeCommand::undo()
{
   QWidget *aw = widgetForAdjust();
   aw->resize(m_geometry.size());
   if (m_geometry.topLeft() != aw->geometry().topLeft()) {
      aw->move(m_geometry.topLeft());
   }
   updatePropertyEditor();
}

void AdjustWidgetSizeCommand::updatePropertyEditor() const
{
   if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
      if (propertyEditor->object() == m_widget) {
         propertyEditor->setPropertyValue(QString("geometry"), m_widget->geometry(), true);
      }
   }
}
// ------------  ChangeFormLayoutItemRoleCommand

ChangeFormLayoutItemRoleCommand::ChangeFormLayoutItemRoleCommand(QDesignerFormWindowInterface *formWindow) :
   QDesignerFormWindowCommand(QApplication::translate("Command", "Change Form Layout Item Geometry"), formWindow),
   m_operation(SpanningToLabel)
{
}

void ChangeFormLayoutItemRoleCommand::init(QWidget *widget, Operation op)
{
   m_widget = widget;
   m_operation = op;
}

void ChangeFormLayoutItemRoleCommand::redo()
{
   doOperation(m_operation);
}

void ChangeFormLayoutItemRoleCommand::undo()
{
   doOperation(reverseOperation(m_operation));
}

ChangeFormLayoutItemRoleCommand::Operation ChangeFormLayoutItemRoleCommand::reverseOperation(Operation op)
{
   switch (op) {
      case SpanningToLabel:
         return LabelToSpanning;
      case SpanningToField:
         return FieldToSpanning;
      case LabelToSpanning:
         return SpanningToLabel;
      case FieldToSpanning:
         return SpanningToField;
   }
   return SpanningToField;
}

void ChangeFormLayoutItemRoleCommand::doOperation(Operation op)
{
   QFormLayout *fl = ChangeFormLayoutItemRoleCommand::managedFormLayoutOf(formWindow()->core(), m_widget);
   const int index = fl->indexOf(m_widget);

   Q_ASSERT(index != -1);

   int row;
   QFormLayout::ItemRole role;
   fl->getItemPosition (index, &row, &role);

   Q_ASSERT(index != -1);

   QLayoutItem *item = fl->takeAt(index);
   const QRect area = QRect(0, row, 2, 1);

   switch (op) {
      case SpanningToLabel:
         fl->setItem(row, QFormLayout::LabelRole, item);
         QLayoutSupport::createEmptyCells(fl);
         break;

      case SpanningToField:
         fl->setItem(row, QFormLayout::FieldRole, item);
         QLayoutSupport::createEmptyCells(fl);
         break;

      case LabelToSpanning:
      case FieldToSpanning:
         QLayoutSupport::removeEmptyCells(fl, area);
         fl->setItem(row, QFormLayout::SpanningRole, item);
         break;
   }
}

unsigned ChangeFormLayoutItemRoleCommand::possibleOperations(QDesignerFormEditorInterface *core, QWidget *w)
{
   QFormLayout *fl = managedFormLayoutOf(core, w);

   if (! fl) {
      return 0;
   }

   const int index = fl->indexOf(w);
   if (index == -1) {
      return 0;
   }

   int row, col, colspan;
   getFormLayoutItemPosition(fl, index, &row, &col, nullptr, &colspan);

  // Spanning item?
   if (colspan > 1) {
      return SpanningToLabel | SpanningToField;
   }

   // Is the neighbouring column free, that is, can the current item be expanded?
   const QFormLayout::ItemRole neighbouringRole = col == 0 ? QFormLayout::FieldRole : QFormLayout::LabelRole;
   const bool empty = LayoutInfo::isEmptyItem(fl->itemAt(row, neighbouringRole));

  if (empty) {
      return col == 0 ? LabelToSpanning : FieldToSpanning;
   }

   return 0;
}

QFormLayout *ChangeFormLayoutItemRoleCommand::managedFormLayoutOf(QDesignerFormEditorInterface *core, QWidget *w)
{
   if (QLayout *layout = LayoutInfo::managedLayout(core, w->parentWidget()))
      if (QFormLayout *fl = dynamic_cast<QFormLayout *>(layout)) {
         return fl;
      }
   return nullptr;
}

// ---- ChangeLayoutItemGeometry ----
ChangeLayoutItemGeometry::ChangeLayoutItemGeometry(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Change Layout Item Geometry"), formWindow)
{
}

void ChangeLayoutItemGeometry::init(QWidget *widget, int row, int column, int rowspan, int colspan)
{
   m_widget = widget;
   Q_ASSERT(m_widget->parentWidget() != nullptr);

   QLayout *layout = LayoutInfo::managedLayout(formWindow()->core(), m_widget->parentWidget());
   Q_ASSERT(layout != nullptr);

   QGridLayout *grid = dynamic_cast<QGridLayout *>(layout);
   Q_ASSERT(grid != nullptr);

   const int itemIndex = grid->indexOf(m_widget);
   Q_ASSERT(itemIndex != -1);

   int current_row, current_column, current_rowspan, current_colspan;
   grid->getItemPosition(itemIndex, &current_row, &current_column, &current_rowspan, &current_colspan);

   m_oldInfo.setRect(current_column, current_row, current_colspan, current_rowspan);
   m_newInfo.setRect(column, row, colspan, rowspan);
}

void ChangeLayoutItemGeometry::changeItemPosition(const QRect &g)
{
   QLayout *layout = LayoutInfo::managedLayout(formWindow()->core(), m_widget->parentWidget());
   Q_ASSERT(layout != nullptr);

   QGridLayout *grid = dynamic_cast<QGridLayout *>(layout);
   Q_ASSERT(grid != nullptr);

   const int itemIndex = grid->indexOf(m_widget);
   Q_ASSERT(itemIndex != -1);

   QLayoutItem *item = grid->takeAt(itemIndex);
   delete item;

   if (! QLayoutSupport::removeEmptyCells(grid, g)) {
      qWarning() << "ChangeLayoutItemGeometry::changeItemPosition() Unable to remove cell at location " << g
             << ", contents was not empty";
   }

   grid->addWidget(m_widget, g.top(), g.left(), g.height(), g.width());

   grid->invalidate();
   grid->activate();

   QLayoutSupport::createEmptyCells(grid);

   formWindow()->clearSelection(false);
   formWindow()->selectWidget(m_widget, true);
}

void ChangeLayoutItemGeometry::redo()
{
   changeItemPosition(m_newInfo);
}

void ChangeLayoutItemGeometry::undo()
{
   changeItemPosition(m_oldInfo);
}

// ---- ContainerWidgetCommand ----
ContainerWidgetCommand::ContainerWidgetCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QString(), formWindow), m_index(-1)
{
}

ContainerWidgetCommand::~ContainerWidgetCommand()
{
}

QDesignerContainerExtension *ContainerWidgetCommand::containerExtension() const
{
   QExtensionManager *mgr = core()->extensionManager();
   return qt_extension<QDesignerContainerExtension *>(mgr, m_containerWidget);
}

void ContainerWidgetCommand::init(QWidget *containerWidget)
{
   m_containerWidget = containerWidget;

   if (QDesignerContainerExtension *c = containerExtension()) {
      m_index = c->currentIndex();
      m_widget = c->widget(m_index);
   }
}

void ContainerWidgetCommand::removePage()
{
   if (QDesignerContainerExtension *c = containerExtension()) {
      if (const int count = c->count()) {
         // Undo add after last?
         const int deleteIndex = m_index >= 0 ? m_index : count - 1;
         c->remove(deleteIndex);
         m_widget->hide();
         m_widget->setParent(formWindow());
      }
   }
}

void ContainerWidgetCommand::addPage()
{
   if (QDesignerContainerExtension *c = containerExtension()) {
      int newCurrentIndex;

      if (m_index >= 0) {
         c->insertWidget(m_index, m_widget);
         newCurrentIndex = m_index;
      } else {
         c->addWidget(m_widget);
         newCurrentIndex = c->count() - 1 ;
      }

      m_widget->show();
      c->setCurrentIndex(newCurrentIndex);
   }
}

// ---- DeleteContainerWidgetPageCommand ----
DeleteContainerWidgetPageCommand::DeleteContainerWidgetPageCommand(QDesignerFormWindowInterface *formWindow)
   : ContainerWidgetCommand(formWindow)
{
}

DeleteContainerWidgetPageCommand::~DeleteContainerWidgetPageCommand()
{
}

void DeleteContainerWidgetPageCommand::init(QWidget *containerWidget, ContainerType ct)
{
   ContainerWidgetCommand::init(containerWidget);
   switch (ct) {
      case WizardContainer:
      case PageContainer:
         setText(QApplication::translate("Command", "Delete Page"));
         break;
      case MdiContainer:
         setText(QApplication::translate("Command", "Delete Subwindow"));
         break;
   }
}

void DeleteContainerWidgetPageCommand::redo()
{
   removePage();
   cheapUpdate();
}

void DeleteContainerWidgetPageCommand::undo()
{
   addPage();
   cheapUpdate();
}

// ---- AddContainerWidgetPageCommand ----
AddContainerWidgetPageCommand::AddContainerWidgetPageCommand(QDesignerFormWindowInterface *formWindow)
   : ContainerWidgetCommand(formWindow)
{
}

AddContainerWidgetPageCommand::~AddContainerWidgetPageCommand()
{
}

void AddContainerWidgetPageCommand::init(QWidget *containerWidget, ContainerType ct, InsertionMode mode)
{
   m_containerWidget = containerWidget;

   if (QDesignerContainerExtension *c = containerExtension()) {

      m_index = c->currentIndex();
      if (m_index >= 0 && mode == InsertAfter) {
         ++m_index;
      }

      m_widget = nullptr;

      const QDesignerFormEditorInterface *core = formWindow()->core();

      switch (ct) {
         case PageContainer:
            setText(QApplication::translate("Command", "Insert Page"));
            m_widget = new QDesignerWidget(formWindow(), m_containerWidget);
            m_widget->setObjectName(QString("page"));
            break;

         case MdiContainer:
            setText(QApplication::translate("Command", "Insert Subwindow"));
            m_widget = new QDesignerWidget(formWindow(), m_containerWidget);
            m_widget->setObjectName(QString("subwindow"));
            setPropertySheetWindowTitle(core, m_widget, QApplication::translate("Command", "Subwindow"));
            break;

         case WizardContainer: // Apply style, don't manage
            m_widget = core->widgetFactory()->createWidget(QString("QWizardPage"), nullptr);
            break;
      }

      formWindow()->ensureUniqueObjectName(m_widget);
      core->metaDataBase()->add(m_widget);
   }
}

void AddContainerWidgetPageCommand::redo()
{
   addPage();
   cheapUpdate();
}

void AddContainerWidgetPageCommand::undo()
{
   removePage();
   cheapUpdate();
}

ChangeCurrentPageCommand::ChangeCurrentPageCommand(QDesignerFormWindowInterface *formWindow)
   :
   QDesignerFormWindowCommand(QString(), formWindow), m_oldIndex(0), m_newIndex(0)
{
}

ChangeCurrentPageCommand::~ChangeCurrentPageCommand()
{
}

QDesignerContainerExtension *ChangeCurrentPageCommand::containerExtension() const
{
   QExtensionManager *mgr = core()->extensionManager();
   return qt_extension<QDesignerContainerExtension *>(mgr, m_containerWidget);
}

void ChangeCurrentPageCommand::init(QWidget *containerWidget, int newIndex)
{
   m_containerWidget = containerWidget;

   if (QDesignerContainerExtension *c = containerExtension()) {
      m_newIndex = newIndex;
      m_oldIndex = c->currentIndex();
      m_widget = c->widget(m_oldIndex);
   }
}

void ChangeCurrentPageCommand::redo()
{
   containerExtension()->setCurrentIndex(m_newIndex);
}

void ChangeCurrentPageCommand::undo()
{
   containerExtension()->setCurrentIndex(m_oldIndex);
}

static int itemRoles[] = {
   Qt::DecorationPropertyRole,
   Qt::DisplayPropertyRole,
   Qt::ToolTipPropertyRole,
   Qt::StatusTipPropertyRole,
   Qt::WhatsThisPropertyRole,
   Qt::FontRole,
   Qt::TextAlignmentRole,
   Qt::BackgroundRole,
   Qt::ForegroundRole,
   Qt::CheckStateRole,
   -1
};

template<class T>
static void copyRoleFromItem(ItemData *id, int role, const T *item)
{
   QVariant v = item->data(role);
   if (v.isValid()) {
      id->m_properties.insert(role, v);
   }
}

template<class T>
static void copyRolesFromItem(ItemData *id, const T *item, bool editor)
{
   static const uint defaultFlags = T().flags();

   for (int i = 0; itemRoles[i] != -1; i++) {
      copyRoleFromItem<T>(id, itemRoles[i], item);
   }

   if (editor) {
      copyRoleFromItem<T>(id, ItemFlagsShadowRole, item);

   } else if (item->flags() != defaultFlags) {
      id->m_properties.insert(ItemFlagsShadowRole, QVariant::fromValue((int)item->flags()));
   }
}

template<class T>
static void copyRolesToItem(const ItemData *id, T *item, DesignerIconCache *iconCache, bool editor)
{
   auto iter_end = id->m_properties.constEnd();

   for (auto iter = id->m_properties.constBegin(); iter != iter_end; ++iter)

      if (iter.value().isValid()) {
         if (! editor && iter.key() == ItemFlagsShadowRole) {
            item->setFlags((Qt::ItemFlags)iter.value().toInt());

         } else {
            item->setData(iter.key(), iter.value());

            switch (iter.key()) {
               case Qt::DecorationPropertyRole:
                  if (iconCache) {
                     QVariant data = iter.value();
                     item->setIcon(iconCache->icon(data.value<PropertySheetIconValue>()));
                  }
                  break;

               case Qt::DisplayPropertyRole: {
                  QVariant data = iter.value();
                  item->setText(data.value<PropertySheetStringValue>().value());
                  break;
               }

               case Qt::ToolTipPropertyRole: {
                  QVariant data = iter.value();
                  item->setToolTip(data.value<PropertySheetStringValue>().value());
                  break;
               }

               case Qt::StatusTipPropertyRole: {
                  QVariant data = iter.value();
                  item->setStatusTip(data.value<PropertySheetStringValue>().value());
                  break;
               }

               case Qt::WhatsThisPropertyRole: {
                  QVariant data = iter.value();
                  item->setWhatsThis(data.value<PropertySheetStringValue>().value());
                  break;
               }
            }
         }
      }

   if (editor) {
      item->setFlags(item->flags() | Qt::ItemIsEditable);
   }
}

ItemData::ItemData(const QListWidgetItem *item, bool editor)
{
   copyRolesFromItem<QListWidgetItem>(this, item, editor);
}

QListWidgetItem *ItemData::createListItem(DesignerIconCache *iconCache, bool editor) const
{
   QListWidgetItem *item = new QListWidgetItem();
   copyRolesToItem(this, item, iconCache, editor);
   return item;
}

ItemData::ItemData(const QTableWidgetItem *item, bool editor)
{
   copyRolesFromItem(this, item, editor);
}

QTableWidgetItem *ItemData::createTableItem(DesignerIconCache *iconCache, bool editor) const
{
   QTableWidgetItem *item = new QTableWidgetItem;
   copyRolesToItem(this, item, iconCache, editor);

   return item;
}

static void copyRoleFromItem(ItemData *id, int role, const QTreeWidgetItem *item, int column)
{
   QVariant v = item->data(column, role);

   if (v.isValid()) {
      id->m_properties.insert(role, v);
   }
}

ItemData::ItemData(const QTreeWidgetItem *item, int column)
{
   copyRoleFromItem(this, Qt::EditRole, item, column);
   PropertySheetStringValue str(item->text(column));
   m_properties.insert(Qt::DisplayPropertyRole, QVariant::fromValue(str));

   for (int i = 0; itemRoles[i] != -1; i++) {
      copyRoleFromItem(this, itemRoles[i], item, column);
   }
}

void ItemData::fillTreeItemColumn(QTreeWidgetItem *item, int column, DesignerIconCache *iconCache) const
{
   auto iter = m_properties.constBegin();
   auto end  = m_properties.constEnd();

   for (; iter != end; ++iter) {
      if (iter.value().isValid()) {
         item->setData(column, iter.key(), iter.value());

         switch (iter.key()) {
            case Qt::DecorationPropertyRole:
               if (iconCache) {
                  QVariant data = iter.value();
                  item->setIcon(column, iconCache->icon(data.value<PropertySheetIconValue>()));
               }
               break;

            case Qt::DisplayPropertyRole:  {
               QVariant data = iter.value();
               item->setText(column, data.value<PropertySheetStringValue>().value());
               break;
            }

            case Qt::ToolTipPropertyRole:  {
               QVariant data = iter.value();
               item->setToolTip(column, data.value<PropertySheetStringValue>().value());
               break;
            }

            case Qt::StatusTipPropertyRole: {
               QVariant data = iter.value();
               item->setStatusTip(column, data.value<PropertySheetStringValue>().value());
               break;
            }

            case Qt::WhatsThisPropertyRole: {
               QVariant data = iter.value();
               item->setWhatsThis(column, data.value<PropertySheetStringValue>().value());
               break;
            }
         }
      }
   }
}

ListData::ListData(const QTreeWidgetItem *item)
{
   for (int i = 0; i < item->columnCount(); i++) {
      m_items.append(ItemData(item, i));
   }
}

QTreeWidgetItem *ListData::createTreeItem(DesignerIconCache *iconCache) const
{
   QTreeWidgetItem *item = new QTreeWidgetItem;
   int i = 0;

   for (const ItemData &id : m_items) {
      id.fillTreeItemColumn(item, i++, iconCache);
   }

   return item;
}

void ListData::createFromListWidget(const QListWidget *listWidget, bool editor)
{
   m_items.clear();

   for (int i = 0; i < listWidget->count(); i++) {
      m_items.append(ItemData(listWidget->item(i), editor));
   }
}

void ListData::applyToListWidget(QListWidget *listWidget, DesignerIconCache *iconCache, bool editor) const
{
   listWidget->clear();

   int i = 0;

   for (const ItemData &entry : m_items) {
      if (! entry.isValid()) {
         new QListWidgetItem(TableWidgetData::defaultHeaderText(i), listWidget);
      } else {
         listWidget->addItem(entry.createListItem(iconCache, editor));
      }

      ++i;
   }
}

void ListData::createFromComboBox(const QComboBox *comboBox)
{
   m_items.clear();

   const int count = comboBox->count();
   for (int i = 0; i < count; i++) {
      // We might encounter items added in a custom combo
      // constructor. Ignore those.
      const QVariant textValue = comboBox->itemData(i, Qt::DisplayPropertyRole);

      if (textValue.isValid()) {
         ItemData entry;
         entry.m_properties.insert(Qt::DisplayPropertyRole, textValue);
         const QVariant iconValue =  comboBox->itemData(i, Qt::DecorationPropertyRole);

         if (iconValue.isValid()) {
            entry.m_properties.insert(Qt::DecorationPropertyRole, iconValue);
         }
         m_items.append(entry);
      }
   }
}

void ListData::applyToComboBox(QComboBox *comboBox, DesignerIconCache *iconCache) const
{
   comboBox->clear();

   for (const ItemData &hash : m_items) {
      QIcon icon;

      if (iconCache) {
         QVariant data = hash.m_properties[Qt::DecorationPropertyRole];
         icon = iconCache->icon(data.value<PropertySheetIconValue>());
      }

      QVariant var = hash.m_properties[Qt::DisplayPropertyRole];
      PropertySheetStringValue str = var.value<PropertySheetStringValue>();

      comboBox->addItem(icon, str.value());
      comboBox->setItemData(comboBox->count() - 1, var, Qt::DisplayPropertyRole);
      comboBox->setItemData(comboBox->count() - 1,
            hash.m_properties[Qt::DecorationPropertyRole], Qt::DecorationPropertyRole);
   }
}

TableWidgetData::TableWidgetData()
   : m_columnCount(0), m_rowCount(0)
{
}

void TableWidgetData::clear()
{
   m_horizontalHeader.m_items.clear();
   m_verticalHeader.m_items.clear();
   m_items.clear();
   m_columnCount = 0;
   m_rowCount    = 0;
}

QString TableWidgetData::defaultHeaderText(int i)
{
   return QString::number(i + 1);
}

bool TableWidgetData::nonEmpty(const QTableWidgetItem *item, int headerColumn)
{
   static uint defaultFlags = QTableWidgetItem().flags();

   if (item->flags() != defaultFlags) {
      return true;
   }

   QVariant data = item->data(Qt::DisplayPropertyRole);
   QString text  = data.value<PropertySheetStringValue>().value();

   if (! text.isEmpty()) {
      if (headerColumn < 0 || text != defaultHeaderText(headerColumn)) {
         return true;
      }

   } else {
      // FIXME: is this the correct return value?
      return true;
   }

   for (int i = 0; itemRoles[i] != -1; ++i) {
      if (itemRoles[i] != Qt::DisplayPropertyRole && item->data(itemRoles[i]).isValid()) {
         return true;
      }
   }

   return false;
}

void TableWidgetData::insertHeaderItem(const QTableWidgetItem *item, int i, ListData *header, bool editor)
{
   if (nonEmpty(item, i)) {
      header->m_items.append(ItemData(item, editor));
   } else {
      header->m_items.append(ItemData());
   }
}

void TableWidgetData::fromTableWidget(const QTableWidget *tableWidget, bool editor)
{
   clear();
   m_columnCount = tableWidget->columnCount();
   m_rowCount    = tableWidget->rowCount();

   // horiz header: Legacy behaviour: auto-generate number for empty items
   for (int col = 0; col <  m_columnCount; ++col) {
      if (const QTableWidgetItem *item = tableWidget->horizontalHeaderItem(col)) {
         insertHeaderItem(item, col, &m_horizontalHeader, editor);
      }
   }

   // vertical header: Legacy behaviour: auto-generate number for empty items
   for (int row = 0; row < m_rowCount; ++row) {
      if (const QTableWidgetItem *item = tableWidget->verticalHeaderItem(row)) {
         insertHeaderItem(item, row, &m_verticalHeader, editor);
      }
   }

   // cell data
   for (int col = 0; col < m_columnCount; ++col) {

      for (int row = 0; row < m_rowCount; ++row) {
         if (const QTableWidgetItem *item = tableWidget->item(row, col)) {
            if (nonEmpty(item, -1)) {
               m_items.insert( {row, col}, ItemData(item, editor) );
            }
         }
      }
   }
}

void TableWidgetData::applyToTableWidget(QTableWidget *tableWidget, DesignerIconCache *iconCache, bool editor) const
{
   tableWidget->clear();

   tableWidget->setColumnCount(m_columnCount);
   tableWidget->setRowCount(m_rowCount);

   // horiz header
   int col = 0;
   for (const ItemData &id : m_horizontalHeader.m_items) {
      if (id.isValid()) {
         tableWidget->setHorizontalHeaderItem(col, id.createTableItem(iconCache, editor));
      }

      ++col;
   }

   // vertical header
   int row = 0;
   for (const ItemData &id : m_verticalHeader.m_items) {
      if (id.isValid()) {
         tableWidget->setVerticalHeaderItem(row, id.createTableItem(iconCache, editor));
      }

      ++row;
   }

   // items
   auto iter_end = m_items.constEnd();

   for (auto iter = m_items.constBegin(); iter !=  iter_end; ++iter) {
      tableWidget->setItem(iter.key().first, iter.key().second, iter.value().createTableItem(iconCache, editor));
   }
}

bool TableWidgetData::operator==(const TableWidgetData &rhs) const
{
   if (m_columnCount != rhs.m_columnCount || m_rowCount !=  rhs.m_rowCount) {
      return false;
   }

   return m_horizontalHeader.m_items == rhs.m_horizontalHeader.m_items &&
      m_verticalHeader.m_items == rhs.m_verticalHeader.m_items &&
      m_items == rhs.m_items;
}

ChangeTableDataCommand::ChangeTableDataCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Change Table Contents"),
     formWindow), m_iconCache(nullptr)
{
   FormWindowBase *fwb = dynamic_cast<FormWindowBase *>(formWindow);

   if (fwb) {
      m_iconCache = fwb->iconCache();
   }
}

void ChangeTableDataCommand::init(QTableWidget *tableWidget,
         const TableWidgetData &oldCont, const TableWidgetData &newCont)
{
   m_tableWidget = tableWidget;
   m_oldContents = oldCont;
   m_newContents = newCont;
}

void ChangeTableDataCommand::redo()
{
   m_newContents.applyToTableWidget(m_tableWidget, m_iconCache, false);
   QMetaObject::invokeMethod(m_tableWidget, "updateGeometries");
}

void ChangeTableDataCommand::undo()
{
   m_oldContents.applyToTableWidget(m_tableWidget, m_iconCache, false);
   QMetaObject::invokeMethod(m_tableWidget, "updateGeometries");
}

TreeWidgetData::TreeNode::TreeNode(TreeWidgetData *owner, const QTreeWidgetItem *treeItem, bool editor)
   : ListData(treeItem), m_owner(owner)
{
   static const uint defaultFlags = QTreeWidgetItem().flags();
   static int currentId = 0;

   m_id = currentId;
   ++currentId;

   if (editor) {
      QVariant v = treeItem->data(0, ItemFlagsShadowRole);
      m_itemFlags = v.isValid() ? v.toInt() : -1;

   } else  {
      if (treeItem->flags() != defaultFlags) {
         m_itemFlags = (int)treeItem->flags();
      } else {
         m_itemFlags = -1;
      }
   }

   // hint ensures the newest item is last for a given m_id
   auto hint = m_owner->m_childNodes.upperBound(m_id);

   for (int i = 0; i < treeItem->childCount(); ++i) {
      TreeNode newNode = TreeNode(m_owner, treeItem->child(i), editor);
      m_owner->m_childNodes.insert(hint, m_id, std::move(newNode));
   }
}

QTreeWidgetItem *TreeWidgetData::TreeNode::createTreeItem(DesignerIconCache *iconCache, bool editor) const
{
   QTreeWidgetItem *treeItem = ListData::createTreeItem(iconCache);

   if (editor) {
      treeItem->setFlags(treeItem->flags() | Qt::ItemIsEditable);
   }

   if (m_itemFlags != -1) {
      if (editor) {
         treeItem->setData(0, ItemFlagsShadowRole, QVariant::fromValue(m_itemFlags));
      } else {
         treeItem->setFlags((Qt::ItemFlags)m_itemFlags);
      }
   }

   auto [iter, iter_end] = m_owner->m_childNodes.equal_range(m_id);

   while (iter != iter_end)  {
      treeItem->addChild(iter->createTreeItem(iconCache, editor));
      ++iter;
   }

   return treeItem;
}

bool TreeWidgetData::TreeNode::operator==(const TreeWidgetData::TreeNode &other) const
{
   if ((m_itemFlags != other.m_itemFlags) || (m_items != other.m_items)) {
      return false;
   }

   auto [iter, iter_end]             = m_owner->m_childNodes.equal_range(m_id);
   auto [other_iter, other_iter_end] = other.m_owner->m_childNodes.equal_range(other.m_id);

   // std::equal will dereference the iterators and compare the values in the map

   return std::equal(iter, iter_end, other_iter, other_iter_end);
}

void TreeWidgetData::clear()
{
   m_headerItem.m_items.clear();
   m_rootItems.clear();
}

void TreeWidgetData::fromTreeWidget(const QTreeWidget *treeWidget, bool editor)
{
   clear();
   m_headerItem = ListData(treeWidget->headerItem());

   for (int col = 0; col < treeWidget->topLevelItemCount(); ++col) {
      m_rootItems.append(TreeNode(this, treeWidget->topLevelItem(col), editor));
   }
}

void TreeWidgetData::applyToTreeWidget(QTreeWidget *treeWidget, DesignerIconCache *iconCache, bool editor) const
{
   treeWidget->clear();

   treeWidget->setColumnCount(m_headerItem.m_items.count());
   treeWidget->setHeaderItem(m_headerItem.createTreeItem(iconCache));

   for (const TreeNode &ic : m_rootItems) {
      treeWidget->addTopLevelItem(ic.createTreeItem(iconCache, editor));
   }

   treeWidget->expandAll();
}


bool TreeWidgetData::operator==(const TreeWidgetData &other) const
{
   return m_headerItem == other.m_headerItem && (m_rootItems == other.m_rootItems);
}

ChangeTreeDataCommand::ChangeTreeDataCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Change Tree Contents"), formWindow),
     m_iconCache(nullptr)
{
   FormWindowBase *fwb = dynamic_cast<FormWindowBase *>(formWindow);

   if (fwb) {
      m_iconCache = fwb->iconCache();
   }
}

void ChangeTreeDataCommand::init(QTreeWidget *treeWidget, const TreeWidgetData &oldState,
         const TreeWidgetData &newState)
{
   m_treeWidget = treeWidget;
   m_oldState = oldState;
   m_newState = newState;
}

void ChangeTreeDataCommand::redo()
{
   m_newState.applyToTreeWidget(m_treeWidget, m_iconCache, false);
}

void ChangeTreeDataCommand::undo()
{
   m_oldState.applyToTreeWidget(m_treeWidget, m_iconCache, false);
}

ChangeListDataCommand::ChangeListDataCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QString(), formWindow), m_iconCache(nullptr)
{
   FormWindowBase *fwb = dynamic_cast<FormWindowBase *>(formWindow);

   if (fwb) {
      m_iconCache = fwb->iconCache();
   }
}

void ChangeListDataCommand::init(QListWidget *listWidget, const ListData &oldItems, const ListData &items)
{
   m_listWidget = listWidget;
   m_comboBox   = nullptr;

   m_newItemsState = items;
   m_oldItemsState = oldItems;
}

void ChangeListDataCommand::init(QComboBox *comboBox, const ListData &oldItems, const ListData &items)
{
   m_listWidget = nullptr;
   m_comboBox   = comboBox;

   m_newItemsState = items;
   m_oldItemsState = oldItems;
}

void ChangeListDataCommand::redo()
{
   if (m_listWidget) {
      m_newItemsState.applyToListWidget(m_listWidget, m_iconCache, false);
   } else if (m_comboBox) {
      m_newItemsState.applyToComboBox(m_comboBox, m_iconCache);
   }
}

void ChangeListDataCommand::undo()
{
   if (m_listWidget) {
      m_oldItemsState.applyToListWidget(m_listWidget, m_iconCache, false);
   } else if (m_comboBox) {
      m_oldItemsState.applyToComboBox(m_comboBox, m_iconCache);
   }
}

AddActionCommand::AddActionCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Add action"), formWindow)
{
   m_action = nullptr;
}

void AddActionCommand::init(QAction *action)
{
   Q_ASSERT(m_action == nullptr);
   m_action = action;
}

void AddActionCommand::redo()
{
   core()->actionEditor()->setFormWindow(formWindow());
   core()->actionEditor()->manageAction(m_action);
}

void AddActionCommand::undo()
{
   core()->actionEditor()->setFormWindow(formWindow());
   core()->actionEditor()->unmanageAction(m_action);
}

// ---- RemoveActionCommand ----

RemoveActionCommand::RemoveActionCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Remove action"),
     formWindow), m_action(nullptr)
{
}

static RemoveActionCommand::ActionData findActionIn(QAction *action)
{
   RemoveActionCommand::ActionData result;

   // We only want menus and toolbars, no toolbuttons.
   for (QWidget *widget : action->associatedWidgets()) {
      if (dynamic_cast<const QMenu *>(widget) || dynamic_cast<const QToolBar *>(widget)) {
         const QList<QAction *> actionList = widget->actions();
         const int size = actionList.size();

         for (int i = 0; i < size; ++i) {
            if (actionList.at(i) == action) {
               QAction *before = nullptr;

               if (i + 1 < size) {
                  before = actionList.at(i + 1);
               }
               result.append(RemoveActionCommand::ActionDataItem(before, widget));
               break;
            }
         }
      }
   }

   return result;
}

void RemoveActionCommand::init(QAction *action)
{
   Q_ASSERT(m_action == nullptr);
   m_action = action;

   m_actionData = findActionIn(action);
}

void RemoveActionCommand::redo()
{
   QDesignerFormWindowInterface *fw = formWindow();
   for (const ActionDataItem &item : m_actionData) {
      item.widget->removeAction(m_action);
   }

   // Notify components (for example, signal slot editor)
   if (qdesigner_internal::FormWindowBase *fwb = dynamic_cast<qdesigner_internal::FormWindowBase *>(fw)) {
      fwb->emitObjectRemoved(m_action);
   }

   core()->actionEditor()->setFormWindow(fw);
   core()->actionEditor()->unmanageAction(m_action);
   if (!m_actionData.empty()) {
      core()->objectInspector()->setFormWindow(fw);
   }
}

void RemoveActionCommand::undo()
{
   core()->actionEditor()->setFormWindow(formWindow());
   core()->actionEditor()->manageAction(m_action);

   for (const ActionDataItem &item : m_actionData) {
      item.widget->insertAction(item.before, m_action);
   }
   if (!m_actionData.empty()) {
      core()->objectInspector()->setFormWindow(formWindow());
   }
}

ActionInsertionCommand::ActionInsertionCommand(const QString &text, QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(text, formWindow), m_parentWidget(nullptr),
     m_action(nullptr), m_beforeAction(nullptr), m_update(false)
{
}

void ActionInsertionCommand::init(QWidget *parentWidget, QAction *action, QAction *beforeAction, bool update)
{
   Q_ASSERT(m_parentWidget == nullptr);
   Q_ASSERT(m_action == nullptr);

   m_parentWidget = parentWidget;
   m_action = action;
   m_beforeAction = beforeAction;
   m_update = update;
}

void ActionInsertionCommand::insertAction()
{
   Q_ASSERT(m_action != nullptr);
   Q_ASSERT(m_parentWidget != nullptr);

   if (m_beforeAction) {
      m_parentWidget->insertAction(m_beforeAction, m_action);
   } else {
      m_parentWidget->addAction(m_action);
   }

   if (m_update) {
      cheapUpdate();
      if (QMenu *menu = m_action->menu()) {
         selectUnmanagedObject(menu);
      } else {
         selectUnmanagedObject(m_action);
      }
      PropertyHelper::triggerActionChanged(m_action); // Update Used column in action editor.
   }
}

void ActionInsertionCommand::removeAction()
{
   Q_ASSERT(m_action != nullptr);
   Q_ASSERT(m_parentWidget != nullptr);

   if (QDesignerMenu *menu = dynamic_cast<QDesignerMenu *>(m_parentWidget)) {
      menu->hideSubMenu();
   }

   m_parentWidget->removeAction(m_action);

   if (m_update) {
      cheapUpdate();
      selectUnmanagedObject(m_parentWidget);
      PropertyHelper::triggerActionChanged(m_action); // Update Used column in action editor.
   }
}

InsertActionIntoCommand::InsertActionIntoCommand(QDesignerFormWindowInterface *formWindow)
   : ActionInsertionCommand(QApplication::translate("Command", "Add action"), formWindow)
{
}

RemoveActionFromCommand::RemoveActionFromCommand(QDesignerFormWindowInterface *formWindow)
   : ActionInsertionCommand(QApplication::translate("Command", "Remove action"), formWindow)
{
}

MenuActionCommand::MenuActionCommand(const QString &text, QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(text, formWindow), m_action(nullptr), m_actionBefore(nullptr),
     m_menuParent(nullptr), m_associatedWidget(nullptr), m_objectToSelect(nullptr)
{
}

void MenuActionCommand::init(QAction *action, QAction *actionBefore,
   QWidget *associatedWidget, QWidget *objectToSelect)
{
   QMenu *menu = action->menu();
   Q_ASSERT(menu);
   m_menuParent = menu->parentWidget();
   m_action = action;
   m_actionBefore = actionBefore;
   m_associatedWidget = associatedWidget;
   m_objectToSelect = objectToSelect;
}

void MenuActionCommand::insertMenu()
{
   core()->metaDataBase()->add(m_action);
   QMenu *menu = m_action->menu();

   if (m_menuParent && menu->parentWidget() != m_menuParent) {
      menu->setParent(m_menuParent);
   }

   core()->metaDataBase()->add(menu);
   m_associatedWidget->insertAction(m_actionBefore, m_action);
   cheapUpdate();
   selectUnmanagedObject(menu);
}

void MenuActionCommand::removeMenu()
{
   m_action->menu()->setParent(nullptr);

   QMenu *menu = m_action->menu();
   core()->metaDataBase()->remove(menu);
   menu->setParent(nullptr);

   core()->metaDataBase()->remove(m_action);
   m_associatedWidget->removeAction(m_action);

   cheapUpdate();
   selectUnmanagedObject(m_objectToSelect);
}

AddMenuActionCommand::AddMenuActionCommand(QDesignerFormWindowInterface *formWindow)
   : MenuActionCommand(QApplication::translate("Command", "Add menu"), formWindow)
{
}

// ---- RemoveMenuActionCommand ----
RemoveMenuActionCommand::RemoveMenuActionCommand(QDesignerFormWindowInterface *formWindow)
   : MenuActionCommand(QApplication::translate("Command", "Remove menu"), formWindow)
{
}

// ---- CreateSubmenuCommand ----
CreateSubmenuCommand::CreateSubmenuCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Create submenu"), formWindow),
     m_action(nullptr), m_menu(nullptr), m_objectToSelect(nullptr)
{
}

void CreateSubmenuCommand::init(QDesignerMenu *menu, QAction *action, QObject *objectToSelect)
{
   m_menu = menu;
   m_action = action;
   m_objectToSelect = objectToSelect;
}

void CreateSubmenuCommand::redo()
{
   m_menu->createRealMenuAction(m_action);
   cheapUpdate();
   if (m_objectToSelect) {
      selectUnmanagedObject(m_objectToSelect);
   }
}

void CreateSubmenuCommand::undo()
{
   m_menu->removeRealMenu(m_action);
   cheapUpdate();
   selectUnmanagedObject(m_menu);
}

// ---- DeleteToolBarCommand ----
DeleteToolBarCommand::DeleteToolBarCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QApplication::translate("Command", "Delete Tool Bar"), formWindow)
{
}

void DeleteToolBarCommand::init(QToolBar *toolBar)
{
   m_toolBar = toolBar;
   m_mainWindow = dynamic_cast<QMainWindow *>(toolBar->parentWidget());
}

void DeleteToolBarCommand::redo()
{
   if (m_mainWindow) {
      QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), m_mainWindow);
      Q_ASSERT(c != nullptr);

      for (int i = 0; i < c->count(); ++i) {
         if (c->widget(i) == m_toolBar) {
            c->remove(i);
            break;
         }
      }
   }

   core()->metaDataBase()->remove(m_toolBar);
   m_toolBar->hide();
   m_toolBar->setParent(formWindow());
   formWindow()->emitSelectionChanged();
}

void DeleteToolBarCommand::undo()
{
   if (m_mainWindow) {
      m_toolBar->setParent(m_mainWindow);
      QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), m_mainWindow);

      c->addWidget(m_toolBar);

      core()->metaDataBase()->add(m_toolBar);
      m_toolBar->show();
      formWindow()->emitSelectionChanged();
   }
}

MorphLayoutCommand::MorphLayoutCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QString(), formWindow),
     m_breakLayoutCommand(new BreakLayoutCommand(formWindow)),
     m_layoutCommand(new LayoutCommand(formWindow)), m_newType(LayoutInfo::VBox), m_layoutBase(nullptr)
{
}

MorphLayoutCommand::~MorphLayoutCommand()
{
   delete m_layoutCommand;
   delete m_breakLayoutCommand;
}

bool MorphLayoutCommand::init(QWidget *w, int newType)
{
   int oldType;
   QDesignerFormWindowInterface *fw = formWindow();

   if (! canMorph(fw, w, &oldType) || oldType == newType) {
      return false;
   }

   m_layoutBase = w;
   m_newType = newType;

   // find all managed widgets
   m_widgets.clear();

   const QLayout *layout = LayoutInfo::managedLayout(fw->core(), w);
   const int count = layout->count();

   for (int i = 0; i < count ; i++) {
      if (QWidget *item_widget = layout->itemAt(i)->widget()) {
         if (fw->isManaged(item_widget)) {
            m_widgets.push_back(item_widget);
         }
      }
   }

   const bool reparentLayoutWidget = false; // leave QLayoutWidget intact
   m_breakLayoutCommand->init(m_widgets, m_layoutBase, reparentLayoutWidget);
   m_layoutCommand->init(m_layoutBase, m_widgets, static_cast<LayoutInfo::Type>(m_newType), m_layoutBase, reparentLayoutWidget);
   setText(formatDescription(core(), m_layoutBase, oldType, newType));

   return true;
}

bool MorphLayoutCommand::canMorph(const QDesignerFormWindowInterface *formWindow, QWidget *w, int *ptrToCurrentType)
{
   if (ptrToCurrentType) {
      *ptrToCurrentType = LayoutInfo::NoLayout;
   }
   // We want a managed widget or a container page
   // with a level-0 managed layout
   QDesignerFormEditorInterface *core = formWindow->core();
   QLayout *layout = LayoutInfo::managedLayout(core, w);
   if (!layout) {
      return false;
   }
   const LayoutInfo::Type type = LayoutInfo::layoutType(core, layout);
   if (ptrToCurrentType) {
      *ptrToCurrentType = type;
   }
   switch (type) {
      case LayoutInfo::HBox:
      case LayoutInfo::VBox:
      case LayoutInfo::Grid:
      case LayoutInfo::Form:
         return true;
         break;
      case LayoutInfo::NoLayout:
      case LayoutInfo::HSplitter: // Nothing doing
      case LayoutInfo::VSplitter:
      case LayoutInfo::UnknownLayout:
         break;
   }
   return false;
}

void MorphLayoutCommand::redo()
{
   m_breakLayoutCommand->redo();
   m_layoutCommand->redo();
   /* Transfer applicable properties which is a cross-section of the modified
    * properties except object name. */
   if (const LayoutProperties *properties = m_breakLayoutCommand->layoutProperties()) {
      const int oldMask = m_breakLayoutCommand->propertyMask();
      QLayout *newLayout = LayoutInfo::managedLayout(core(), m_layoutBase);
      const int newMask = LayoutProperties::visibleProperties(newLayout);
      const int applicableMask = (oldMask & newMask) & ~LayoutProperties::ObjectNameProperty;
      if (applicableMask) {
         properties->toPropertySheet(core(), newLayout, applicableMask);
      }
   }
}

void MorphLayoutCommand::undo()
{
   m_layoutCommand->undo();
   m_breakLayoutCommand->undo();
}

QString MorphLayoutCommand::formatDescription(QDesignerFormEditorInterface * /* core*/, const QWidget *w, int oldType, int newType)
{
   const QString oldName = LayoutInfo::layoutName(static_cast<LayoutInfo::Type>(oldType));
   const QString newName = LayoutInfo::layoutName(static_cast<LayoutInfo::Type>(newType));
   const QString widgetName = dynamic_cast<const QLayoutWidget *>(w) ? w->layout()->objectName() : w->objectName();

   return QApplication::translate("Command", "Change layout of '%1' from %2 to %3").formatArgs(widgetName, oldName, newName);
}

LayoutAlignmentCommand::LayoutAlignmentCommand(QDesignerFormWindowInterface *formWindow) :
   QDesignerFormWindowCommand(QApplication::translate("Command", "Change layout alignment"), formWindow),
   m_newAlignment(nullptr), m_oldAlignment(nullptr), m_widget(nullptr)
{
}

bool LayoutAlignmentCommand::init(QWidget *w, Qt::Alignment alignment)
{
   bool enabled;
   m_newAlignment = alignment;
   m_oldAlignment = LayoutAlignmentCommand::alignmentOf(core(), w, &enabled);
   m_widget = w;

   return enabled;
}

void LayoutAlignmentCommand::redo()
{
   LayoutAlignmentCommand::applyAlignment(core(), m_widget, m_newAlignment);
}

void LayoutAlignmentCommand::undo()
{
   LayoutAlignmentCommand::applyAlignment(core(), m_widget, m_oldAlignment);
}

// Find out alignment and return whether command is enabled.
Qt::Alignment LayoutAlignmentCommand::alignmentOf(const QDesignerFormEditorInterface *core, QWidget *w, bool *enabledIn)
{
   bool managed;
   QLayout *layout;

   if (enabledIn) {
      *enabledIn = false;
   }

   // Can only work on a managed layout
   const LayoutInfo::Type type = LayoutInfo::laidoutWidgetType(core, w, &managed, &layout);

   const bool enabled = layout && managed && (type == LayoutInfo::HBox || type == LayoutInfo::VBox
         || type == LayoutInfo::Grid);

   if (! enabled) {
      return Qt::Alignment(nullptr);
   }

   // Get alignment
   const int index = layout->indexOf(w);
   Q_ASSERT(index >= 0);

   if (enabledIn) {
      *enabledIn = true;
   }

   return layout->itemAt(index)->alignment();
}

void LayoutAlignmentCommand::applyAlignment(const QDesignerFormEditorInterface *core, QWidget *w, Qt::Alignment a)
{
   // Find layout and apply to item
   QLayout *layout;
   LayoutInfo::laidoutWidgetType(core, w, nullptr, &layout);

   if (layout) {
      const int index = layout->indexOf(w);

      if (index >= 0) {
         layout->itemAt(index)->setAlignment(a);
         layout->update();
      }
   }
}

}   // end namespace qdesigner_internal

