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

#include <abstract_dialoggui.h>
#include <action_editor.h>
#include <action_provider.h>
#include <connection_edit.h>
#include <container.h>
#include <designer_command.h>
#include <designer_object_inspector.h>
#include <designer_property_command.h>
#include <designer_resource.h>
#include <designer_stackedbox.h>
#include <designer_tabwidget.h>
#include <designer_taskmenu.h>
#include <designer_toolbox.h>
#include <designer_utils.h>
#include <designer_widget.h>
#include <designer_widgetbox.h>
#include <extension.h>
#include <formeditor.h>
#include <formwindow.h>
#include <formwindow_dnditem.h>
#include <formwindow_widgetstack.h>
#include <formwindowcursor.h>
#include <formwindowmanager.h>
#include <layout_info.h>
#include <layout_widget.h>
#include <propertysheet.h>
#include <resource_model.h>
#include <taskmenu.h>
#include <tool_widgeteditor.h>
#include <ui4.h>
#include <widgetfactory.h>
#include <widgetselection.h>

#include <deviceprofile_p.h>
#include <grid_p.h>
#include <invisible_widget_p.h>
#include <metadatabase_p.h>
#include <shared_settings_p.h>
#include <spacer_widget_p.h>
#include <widgetdatabase_p.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QBuffer>
#include <QButtonGroup>
#include <QClipboard>
#include <QDockWidget>
#include <QGroupBox>
#include <QMenu>
#include <QPainter>
#include <QRubberBand>
#include <QScrollArea>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTimer>
#include <QToolBox>
#include <QUndoGroup>
#include <QXmlStreamReader>

CS_DECLARE_METATYPE(QWidget *)

namespace {

class BlockSelection
{
 public:
   BlockSelection(qdesigner_internal::FormWindow *fw)
      : m_formWindow(fw), m_blocked(m_formWindow->blockSelectionChanged(true)) {
   }

   ~BlockSelection() {
      if (m_formWindow) {
         m_formWindow->blockSelectionChanged(m_blocked);
      }
   }

 private:
   QPointer<qdesigner_internal::FormWindow> m_formWindow;
   const bool m_blocked;
};

}   // end namespace - anonymous

namespace qdesigner_internal {

// ------------------------ FormWindow::Selection
// Maintains a pool of WidgetSelections to be used for selected widgets.

class FormWindow::Selection
{
 public:
   Selection();
   ~Selection();

   // Clear
   void clear();

   // Also clear out the pool. Call if reparenting of the main container occurs.
   void  clearSelectionPool();

   void repaintSelection(QWidget *w);
   void repaintSelection();

   bool isWidgetSelected(QWidget *w) const;
   QWidgetList selectedWidgets() const;

   WidgetSelection *addWidget(FormWindow *fw, QWidget *w);
   // remove widget, return new current widget or 0
   QWidget *removeWidget(QWidget *w);

   void raiseList(const QWidgetList &l);
   void raiseWidget(QWidget *w);

   void updateGeometry(QWidget *w);

   void hide(QWidget *w);
   void show(QWidget *w);

 private:
   typedef QList<WidgetSelection *> SelectionPool;
   SelectionPool m_selectionPool;

   typedef QHash<QWidget *, WidgetSelection *> SelectionHash;
   SelectionHash m_usedSelections;
};

FormWindow::Selection::Selection()
{
}

FormWindow::Selection::~Selection()
{
   clearSelectionPool();
}

void FormWindow::Selection::clear()
{
   if (! m_usedSelections.empty()) {
      const SelectionHash::iterator mend = m_usedSelections.end();

      for (SelectionHash::iterator it = m_usedSelections.begin(); it != mend; ++it) {
         it.value()->setWidget(nullptr);
      }

      m_usedSelections.clear();
   }
}

void  FormWindow::Selection::clearSelectionPool()
{
   clear();
   qDeleteAll(m_selectionPool);
   m_selectionPool.clear();
}

WidgetSelection *FormWindow::Selection::addWidget(FormWindow *fw, QWidget *w)
{
   WidgetSelection *rc = m_usedSelections.value(w);
   if (rc != nullptr) {
      rc->show();
      rc->updateActive();
      return rc;
   }

   // find a free one in the pool
   auto pend = m_selectionPool.end();

   for (SelectionPool::iterator it = m_selectionPool.begin(); it != pend; ++it) {
      if (! (*it)->isUsed()) {
         rc = *it;
         break;
      }
   }

   if (rc == nullptr) {
      rc = new WidgetSelection(fw);
      m_selectionPool.push_back(rc);
   }

   m_usedSelections.insert(w, rc);
   rc->setWidget(w);

   return rc;
}

QWidget *FormWindow::Selection::removeWidget(QWidget *w)
{
   WidgetSelection *s = m_usedSelections.value(w);
   if (!s) {
      return w;
   }

   s->setWidget(nullptr);
   m_usedSelections.remove(w);

   if (m_usedSelections.isEmpty()) {
      return nullptr;
   }

   return (*m_usedSelections.begin())->widget();
}

void FormWindow::Selection::repaintSelection(QWidget *w)
{
   if (WidgetSelection *s = m_usedSelections.value(w)) {
      s->update();
   }
}

void FormWindow::Selection::repaintSelection()
{
   const SelectionHash::iterator mend = m_usedSelections.end();
   for (SelectionHash::iterator it = m_usedSelections.begin(); it != mend; ++it) {
      it.value()->update();
   }
}

bool FormWindow::Selection::isWidgetSelected(QWidget *w) const
{
   return  m_usedSelections.contains(w);
}

QWidgetList FormWindow::Selection::selectedWidgets() const
{
   return m_usedSelections.keys();
}

void FormWindow::Selection::raiseList(const QWidgetList &l)
{
   const SelectionHash::iterator mend = m_usedSelections.end();
   for (SelectionHash::iterator it = m_usedSelections.begin(); it != mend; ++it) {
      WidgetSelection *w = it.value();
      if (l.contains(w->widget())) {
         w->show();
      }
   }
}

void FormWindow::Selection::raiseWidget(QWidget *w)
{
   if (WidgetSelection *s = m_usedSelections.value(w)) {
      s->show();
   }
}

void FormWindow::Selection::updateGeometry(QWidget *w)
{
   if (WidgetSelection *s = m_usedSelections.value(w)) {
      s->updateGeometry();
   }
}

void FormWindow::Selection::hide(QWidget *w)
{
   if (WidgetSelection *s = m_usedSelections.value(w)) {
      s->hide();
   }
}

void FormWindow::Selection::show(QWidget *w)
{
   if (WidgetSelection *s = m_usedSelections.value(w)) {
      s->show();
   }
}

// ------------------------ FormWindow
FormWindow::FormWindow(FormEditor *core, QWidget *parent, Qt::WindowFlags flags)
   : FormWindowBase(core, parent, flags), m_mouseState(NoMouseState), m_core(core),
     m_selection(new Selection), m_widgetStack(new FormWindowWidgetStack(this)), m_contextMenuPosition(-1, -1)
{
   // Apply settings to formcontainer
   deviceProfile().apply(core, m_widgetStack->formContainer(), qdesigner_internal::DeviceProfile::ApplyFormParent);

   setLayout(m_widgetStack->layout());
   init();

   m_cursor = new FormWindowCursor(this, this);
   core->formWindowManager()->addFormWindow(this);

   setDirty(false);
   setAcceptDrops(true);
}

FormWindow::~FormWindow()
{
   Q_ASSERT(core() != nullptr);
   Q_ASSERT(core()->metaDataBase() != nullptr);
   Q_ASSERT(core()->formWindowManager() != nullptr);

   core()->formWindowManager()->removeFormWindow(this);
   core()->metaDataBase()->remove(this);

   QWidgetList l = widgets();
   for (QWidget *w : l) {
      core()->metaDataBase()->remove(w);
   }

   m_widgetStack = nullptr;
   m_rubberBand  = nullptr;

   if (resourceSet()) {
      core()->resourceModel()->removeResourceSet(resourceSet());
   }

   delete m_selection;
}

QDesignerFormEditorInterface *FormWindow::core() const
{
   return m_core;
}

QDesignerFormWindowCursorInterface *FormWindow::cursor() const
{
   return m_cursor;
}

void FormWindow::updateWidgets()
{
   if (!m_mainContainer) {
      return;
   }
}

int FormWindow::widgetDepth(const QWidget *w)
{
   int d = -1;
   while (w && !w->isWindow()) {
      ++d;
      w = w->parentWidget();
   }

   return d;
}

bool FormWindow::isChildOf(const QWidget *c, const QWidget *p)
{
   while (c) {
      if (c == p) {
         return true;
      }
      c = c->parentWidget();
   }
   return false;
}

void FormWindow::setCursorToAll(const QCursor &c, QWidget *start)
{
   start->setCursor(c);
   const QWidgetList widgets = start->findChildren<QWidget *>();

   for (QWidget *widget : widgets) {
      if (! dynamic_cast<WidgetHandle *>(widget)) {
         widget->setCursor(c);
      }
   }
}

void FormWindow::init()
{
   if (FormWindowManager *manager = dynamic_cast<FormWindowManager *> (core()->formWindowManager())) {
      manager->undoGroup()->addStack(m_undoStack.qundoStack());
   }

   m_blockSelectionChanged = false;

   m_defaultMargin = INT_MIN;
   m_defaultSpacing = INT_MIN;

   connect(m_widgetStack, &FormWindowWidgetStack::currentToolChanged,
         this, &QDesignerFormWindowInterface::toolChanged);

   m_selectionChangedTimer = new QTimer(this);
   m_selectionChangedTimer->setSingleShot(true);

   connect(m_selectionChangedTimer, &QTimer::timeout, this,
         &FormWindow::selectionChangedTimerDone);

   m_checkSelectionTimer = new QTimer(this);
   m_checkSelectionTimer->setSingleShot(true);

   connect(m_checkSelectionTimer, &QTimer::timeout,
         this, &FormWindow::checkSelectionNow);

   m_geometryChangedTimer = new QTimer(this);
   m_geometryChangedTimer->setSingleShot(true);

   connect(m_geometryChangedTimer, &QTimer::timeout,
         this, &QDesignerFormWindowInterface::geometryChanged);

   m_rubberBand = nullptr;

   setFocusPolicy(Qt::StrongFocus);

   m_mainContainer = nullptr;
   m_currentWidget = nullptr;

   connect(&m_undoStack, &QDesignerUndoStack::changed,
         this, &QDesignerFormWindowInterface::changed);

   connect(&m_undoStack, &QDesignerUndoStack::changed,
         this, &FormWindow::checkSelection);

   core()->metaDataBase()->add(this);

   initializeCoreTools();

   QAction *a = new QAction(this);
   a->setText(tr("Edit contents"));
   a->setShortcut(tr("F2"));
   connect(a, &QAction::triggered, this, &FormWindow::editContents);
   addAction(a);
}

QWidget *FormWindow::mainContainer() const
{
   return m_mainContainer;
}


void FormWindow::clearMainContainer()
{
   if (m_mainContainer) {
      setCurrentTool(0);

      m_widgetStack->setMainContainer(nullptr);
      core()->metaDataBase()->remove(m_mainContainer);
      unmanageWidget(m_mainContainer);

      delete m_mainContainer;
      m_mainContainer = nullptr;
   }
}

void FormWindow::setMainContainer(QWidget *w)
{
   if (w == m_mainContainer) {
      // nothing to do
      return;
   }

   clearMainContainer();

   m_mainContainer = w;
   const QSize sz = m_mainContainer->size();

   m_widgetStack->setMainContainer(m_mainContainer);
   m_widgetStack->setCurrentTool(m_widgetEditor);

   setCurrentWidget(m_mainContainer);
   manageWidget(m_mainContainer);

   if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(
            core()->extensionManager(), m_mainContainer)) {

      sheet->setVisible(sheet->indexOf("windowTitle"),    true);
      sheet->setVisible(sheet->indexOf("windowIcon"),     true);
      sheet->setVisible(sheet->indexOf("windowModality"), true);
      sheet->setVisible(sheet->indexOf("windowOpacity"),  true);
      sheet->setVisible(sheet->indexOf("windowFilePath"), true);
   }

   m_mainContainer->setFocusPolicy(Qt::StrongFocus);
   m_mainContainer->resize(sz);

   emit mainContainerChanged(m_mainContainer);
}

QWidget *FormWindow::findTargetContainer(QWidget *widget) const
{
   Q_ASSERT(widget);

   while (QWidget *parentWidget = widget->parentWidget()) {
      if (LayoutInfo::layoutType(m_core, parentWidget) == LayoutInfo::NoLayout && isManaged(widget)) {
         return widget;
      }

      widget = parentWidget;
   }

   return mainContainer();
}

static inline void clearObjectInspectorSelection(const QDesignerFormEditorInterface *core)
{
   if (QDesignerObjectInspector *oi = dynamic_cast<QDesignerObjectInspector *>(core->objectInspector())) {
      oi->clearSelection();
   }
}

// Find a parent of a desired selection state
static QWidget *findSelectedParent(QDesignerFormWindowInterface *fw, const QWidget *w, bool selected)
{
   const QDesignerFormWindowCursorInterface *cursor = fw->cursor();
   QWidget *mainContainer = fw->mainContainer();
   for (QWidget *p = w->parentWidget(); p && p != mainContainer; p = p->parentWidget())
      if (fw->isManaged(p))
         if (cursor->isWidgetSelected(p) == selected) {
            return p;
         }
   return nullptr;
}

// Mouse modifiers.

enum MouseFlags { ToggleSelectionModifier = 0x1, CycleParentModifier = 0x2, CopyDragModifier = 0x4 };

static inline unsigned mouseFlags(Qt::KeyboardModifiers mod)
{
   switch (mod) {
      case Qt::ShiftModifier:
         return CycleParentModifier;
         break;

#ifdef Q_OS_DARWIN
      case Qt::AltModifier: // "Alt" or "option" key on Mac means copy
         return CopyDragModifier;
#endif

      case Qt::ControlModifier:
         return CopyDragModifier | ToggleSelectionModifier;
         break;

      default:
         break;
   }

   return 0;
}

// Handle the click selection: Do toggling/cycling
// of parents according to the modifiers.
void FormWindow::handleClickSelection(QWidget *managedWidget, unsigned mouseMode)
{
   const bool sameWidget = managedWidget == m_lastClickedWidget;
   m_lastClickedWidget = managedWidget;

   const bool selected = isWidgetSelected(managedWidget);

   // toggle selection state of widget
   if (mouseMode & ToggleSelectionModifier) {
      selectWidget(managedWidget, ! selected);
      return;
   }

   QWidget *selectionCandidate = nullptr;

   // Hierarchy cycling: If the same widget clicked again: Attempt to cycle
   // trough the hierarchy. Find the next currently selected parent
   if (sameWidget && (mouseMode & CycleParentModifier))
      if (QWidget *currentlySelectedParent = selected ? managedWidget : findSelectedParent(this, managedWidget, true)) {
         selectionCandidate = findSelectedParent(this, currentlySelectedParent, false);
      }

   // Not the same widget, list wrapped over or there was no unselected parent
   if (!selectionCandidate && !selected) {
      selectionCandidate = managedWidget;
   }

   if (selectionCandidate) {
      selectSingleWidget(selectionCandidate);
   }
}

void FormWindow::selectSingleWidget(QWidget *w)
{
   clearSelection(false);
   selectWidget(w, true);
   raiseChildSelections(w);
}

bool FormWindow::handleMousePressEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
   (void) widget;

   m_mouseState = NoMouseState;
   m_startPos = QPoint();

   e->accept();

   BlockSelection blocker(this);

   if (core()->formWindowManager()->activeFormWindow() != this) {
      core()->formWindowManager()->setActiveFormWindow(this);
   }

   const Qt::MouseButtons buttons = e->buttons();

   if (buttons != Qt::LeftButton && buttons != Qt::MiddleButton) {
      return true;
   }

   m_startPos = mapFromGlobal(e->globalPos());

   if (buttons == Qt::MiddleButton || isMainContainer(managedWidget) == true) { // press was on the formwindow
      clearObjectInspectorSelection(m_core);  // We might have a toolbar or non-widget selected in the object inspector.
      clearSelection(false);

      m_mouseState = MouseDrawRubber;
      m_currRect = QRect();
      startRectDraw(mapFromGlobal(e->globalPos()), this, Rubber);
      return true;
   }
   if (buttons != Qt::LeftButton) {
      return true;
   }

   const unsigned mouseMode = mouseFlags(e->modifiers());

   /* Normally, we want to be able to click /select-on-press to drag away
    * the widget in the next step. However, in the case of a widget which
    * itself or whose parent is selected, we defer the selection to the
    * release event.
    * This is to prevent children from being dragged away from layouts
    * when their layouts are selected and one wants to move the layout.
    * Note that toggle selection is only deferred if the widget is already
    * selected, so, it is still possible to just Ctrl+Click and CopyDrag. */
   const bool deferSelection = isWidgetSelected(managedWidget) || findSelectedParent(this, managedWidget, true);
   if (deferSelection) {
      m_mouseState = MouseDeferredSelection;
   } else {
      // Cycle the parent unless we explicitly want toggle
      const unsigned effectiveMouseMode = (mouseMode & ToggleSelectionModifier) ? mouseMode : static_cast<unsigned>(CycleParentModifier);
      handleClickSelection(managedWidget, effectiveMouseMode);
   }
   return true;
}

// We can drag widget in managed layouts except splitter.
static bool canDragWidgetInLayout(const QDesignerFormEditorInterface *core, QWidget *w)
{
   bool managed;
   const LayoutInfo::Type type = LayoutInfo::laidoutWidgetType(core, w, &managed);
   if (!managed) {
      return false;
   }

   switch (type) {
      case LayoutInfo::NoLayout:
      case LayoutInfo::HSplitter:
      case LayoutInfo::VSplitter:
         return false;

      default:
         break;
   }

   return true;
}

bool FormWindow::handleMouseMoveEvent(QWidget *, QWidget *, QMouseEvent *e)
{
   e->accept();
   if (m_startPos.isNull()) {
      return true;
   }

   const QPoint pos = mapFromGlobal(e->globalPos());

   switch (m_mouseState) {
      case MouseDrawRubber:  // Rubber band with left/middle mouse
         continueRectDraw(pos, this, Rubber);
         return true;
      case MouseMoveDrag: // Spurious move event after drag started?
         return true;
      default:
         break;
   }

   if (e->buttons() != Qt::LeftButton) {
      return true;
   }

   const bool canStartDrag = (m_startPos - pos).manhattanLength() > QApplication::startDragDistance();

   if (canStartDrag == false) {
      // nothing to do
      return true;
   }

   m_mouseState = MouseMoveDrag;
   const bool blocked = blockSelectionChanged(true);

   QWidgetList sel = selectedWidgets();
   simplifySelection(&sel);

   QSet<QWidget *> widget_set;

   for (QWidget *child : sel) {
      // Move parent layout or container?
      QWidget *current = child;

      bool done = false;
      while (!isMainContainer(current) && !done) {
         if (!isManaged(current)) {
            current = current->parentWidget();
            continue;
         } else if (LayoutInfo::isWidgetLaidout(core(), current)) {
            // Go up to parent of layout if shift pressed, else do that only for splitters
            if (!canDragWidgetInLayout(core(), current)) {
               current = current->parentWidget();
               continue;
            }
         }
         done = true;
      }

      if (current == mainContainer()) {
         continue;
      }

      widget_set.insert(current);
   }

   sel = widget_set.toList();
   QDesignerFormWindowCursorInterface *c = cursor();
   QWidget *current = c->current();
   if (sel.contains(current)) {
      sel.removeAll(current);
      sel.prepend(current);
   }

   QList<QDesignerDnDItemInterface *> item_list;
   const QPoint globalPos = mapToGlobal(m_startPos);
   const QDesignerDnDItemInterface::DropType dropType = (mouseFlags(e->modifiers()) & CopyDragModifier) ?
      QDesignerDnDItemInterface::CopyDrop : QDesignerDnDItemInterface::MoveDrop;

   for (QWidget *widget : sel) {
      item_list.append(new FormWindowDnDItem(dropType,  this, widget, globalPos));

      if (dropType == QDesignerDnDItemInterface::MoveDrop) {
         m_selection->hide(widget);
         widget->hide();
      }
   }

   blockSelectionChanged(blocked);

   if (! sel.empty()) {
      // reshow selection?

      if (QDesignerMimeData::execDrag(item_list, core()->topLevel()) == Qt::IgnoreAction && dropType == QDesignerDnDItemInterface::MoveDrop) {
         for (QWidget *widget : sel) {
            m_selection->show(widget);
         }
      }
   }

   m_startPos = QPoint();

   return true;
}

bool FormWindow::handleMouseReleaseEvent(QWidget *w, QWidget *mw, QMouseEvent *e)
{
   (void) w;

   const MouseState oldState = m_mouseState;
   m_mouseState = NoMouseState;

   if (oldState == MouseDoubleClicked) {
      return true;
   }

   e->accept();

   switch (oldState) {
      case MouseDrawRubber: {  // we were drawing a rubber selection
         endRectDraw(); // get rid of the rectangle
         const bool blocked = blockSelectionChanged(true);
         selectWidgets(); // select widgets which intersect the rect
         blockSelectionChanged(blocked);
      }
      break;

      // Deferred select: Select the child here unless the parent was moved.
      case MouseDeferredSelection:
         handleClickSelection(mw, mouseFlags(e->modifiers()));
         break;

      default:
         break;
   }

   m_startPos = QPoint();

   /* Inform about selection changes (left/mid or context menu). Also triggers
    * in the case of an empty rubber drag that cleared the selection in
    * MousePressEvent. */
   switch (e->button()) {

      case Qt::LeftButton:
      case Qt::MiddleButton:
      case Qt::RightButton:
         emitSelectionChanged();
         break;

      default:
         break;
   }

   return true;
}

void FormWindow::checkPreviewGeometry(QRect &r)
{
   if (!rect().contains(r)) {
      if (r.left() < rect().left()) {
         r.moveTopLeft(QPoint(0, r.top()));
      }
      if (r.right() > rect().right()) {
         r.moveBottomRight(QPoint(rect().right(), r.bottom()));
      }
      if (r.top() < rect().top()) {
         r.moveTopLeft(QPoint(r.left(), rect().top()));
      }
      if (r.bottom() > rect().bottom()) {
         r.moveBottomRight(QPoint(r.right(), rect().bottom()));
      }
   }
}

void FormWindow::startRectDraw(const QPoint &pos, QWidget *, RectType t)
{
   m_rectAnchor = (t == Insert) ? designerGrid().snapPoint(pos) : pos;

   m_currRect = QRect(m_rectAnchor, QSize(0, 0));
   if (!m_rubberBand) {
      m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
   }
   m_rubberBand->setGeometry(m_currRect);
   m_rubberBand->show();
}

void FormWindow::continueRectDraw(const QPoint &pos, QWidget *, RectType t)
{
   const QPoint p2 = (t == Insert) ? designerGrid().snapPoint(pos) : pos;

   QRect r(m_rectAnchor, p2);
   r = r.normalized();

   if (m_currRect == r) {
      return;
   }

   if (r.width() > 1 || r.height() > 1) {
      m_currRect = r;
      if (m_rubberBand) {
         m_rubberBand->setGeometry(m_currRect);
      }
   }
}

void FormWindow::endRectDraw()
{
   if (m_rubberBand) {
      delete m_rubberBand;
      m_rubberBand = nullptr;
   }
}

QWidget *FormWindow::currentWidget() const
{
   return m_currentWidget;
}

bool FormWindow::setCurrentWidget(QWidget *currentWidget)
{
   if (currentWidget == m_currentWidget) {
      return false;
   }

   // repaint the old widget unless it is the main window
   if (m_currentWidget && m_currentWidget != mainContainer()) {
      m_selection->repaintSelection(m_currentWidget);
   }

   // set new and repaint
   m_currentWidget = currentWidget;
   if (m_currentWidget && m_currentWidget != mainContainer()) {
      m_selection->repaintSelection(m_currentWidget);
   }
   return true;
}

void FormWindow::selectWidget(QWidget *w, bool select)
{
   if (trySelectWidget(w, select)) {
      emitSelectionChanged();
   }
}

// Selects a widget and determines the new current one. Returns true if a change occurs.
bool FormWindow::trySelectWidget(QWidget *w, bool select)
{
   if (!isManaged(w) && !isCentralWidget(w)) {
      return false;
   }

   if (!select && !isWidgetSelected(w)) {
      return false;
   }

   if (!mainContainer()) {
      return false;
   }

   if (isMainContainer(w) || isCentralWidget(w)) {
      setCurrentWidget(mainContainer());
      return true;
   }

   if (select) {
      setCurrentWidget(w);
      m_selection->addWidget(this, w);
   } else {
      QWidget *newCurrent = m_selection->removeWidget(w);
      if (!newCurrent) {
         newCurrent = mainContainer();
      }
      setCurrentWidget(newCurrent);
   }
   return true;
}

void FormWindow::clearSelection(bool changePropertyDisplay)
{
   // At all events, we need a current widget.
   m_selection->clear();
   setCurrentWidget(mainContainer());

   if (changePropertyDisplay) {
      emitSelectionChanged();
   }
}

void FormWindow::emitSelectionChanged()
{
   if (m_blockSelectionChanged == true) {
      // nothing to do
      return;
   }

   m_selectionChangedTimer->start(0);
}

void FormWindow::selectionChangedTimerDone()
{
   emit selectionChanged();
}

bool FormWindow::isWidgetSelected(QWidget *w) const
{
   return m_selection->isWidgetSelected(w);
}

bool FormWindow::isMainContainer(const QWidget *w) const
{
   return w && (w == this || w == mainContainer());
}

void FormWindow::updateChildSelections(QWidget *w)
{
   const QWidgetList list = w->findChildren<QWidget *>();

   for (auto item : list) {
      if (isManaged(item)) {
         updateSelection(item);
      }
   }
}

void FormWindow::repaintSelection()
{
   m_selection->repaintSelection();
}

void FormWindow::raiseSelection(QWidget *w)
{
   m_selection->raiseWidget(w);
}

void FormWindow::updateSelection(QWidget *w)
{
   if (! w->isVisibleTo(this)) {
      selectWidget(w, false);
   } else {
      m_selection->updateGeometry(w);
   }
}

QWidget *FormWindow::designerWidget(QWidget *w) const
{
   while ((w && !isMainContainer(w) && !isManaged(w)) || isCentralWidget(w)) {
      w = w->parentWidget();
   }

   return w;
}

bool FormWindow::isCentralWidget(QWidget *w) const
{
   if (QMainWindow *mainWindow = dynamic_cast<QMainWindow *>(mainContainer())) {
      return w == mainWindow->centralWidget();
   }

   return false;
}

void FormWindow::ensureUniqueObjectName(QObject *object)
{
   QString name = object->objectName();

   if (name.isEmpty()) {
      QDesignerWidgetDataBaseInterface *db = core()->widgetDataBase();

      if (QDesignerWidgetDataBaseItemInterface *item = db->item(db->indexOfObject(object))) {
         name = qdesigner_internal::refactorClassName(item->name());
      }
   }

   unify(object, name, true);
   object->setObjectName(name);
}

template <class Iterator>
static inline void insertNames(const QDesignerMetaDataBaseInterface *metaDataBase,
   Iterator it, const Iterator &end,
   QObject *excludedObject, QSet<QString> &nameSet)
{
   for ( ; it != end; ++it)
      if (excludedObject != *it && metaDataBase->item(*it)) {
         nameSet.insert((*it)->objectName());
      }
}

static QSet<QString> languageKeywords()
{
   static QSet<QString> keywords;

   if (keywords.isEmpty()) {
      // C++ keywords
      keywords.insert(QString("asm"));
      keywords.insert(QString("auto"));
      keywords.insert(QString("bool"));
      keywords.insert(QString("break"));
      keywords.insert(QString("case"));
      keywords.insert(QString("catch"));
      keywords.insert(QString("char"));
      keywords.insert(QString("class"));
      keywords.insert(QString("const"));
      keywords.insert(QString("const_cast"));
      keywords.insert(QString("continue"));
      keywords.insert(QString("default"));
      keywords.insert(QString("delete"));
      keywords.insert(QString("do"));
      keywords.insert(QString("double"));
      keywords.insert(QString("dynamic_cast"));
      keywords.insert(QString("else"));
      keywords.insert(QString("enum"));
      keywords.insert(QString("explicit"));
      keywords.insert(QString("export"));
      keywords.insert(QString("extern"));
      keywords.insert(QString("false"));
      keywords.insert(QString("float"));
      keywords.insert(QString("for"));
      keywords.insert(QString("friend"));
      keywords.insert(QString("goto"));
      keywords.insert(QString("if"));
      keywords.insert(QString("inline"));
      keywords.insert(QString("int"));
      keywords.insert(QString("long"));
      keywords.insert(QString("mutable"));
      keywords.insert(QString("namespace"));
      keywords.insert(QString("new"));
      keywords.insert(QString("NULL"));
      keywords.insert(QString("operator"));
      keywords.insert(QString("private"));
      keywords.insert(QString("protected"));
      keywords.insert(QString("public"));
      keywords.insert(QString("register"));
      keywords.insert(QString("reinterpret_cast"));
      keywords.insert(QString("return"));
      keywords.insert(QString("short"));
      keywords.insert(QString("signed"));
      keywords.insert(QString("sizeof"));
      keywords.insert(QString("static"));
      keywords.insert(QString("static_cast"));
      keywords.insert(QString("struct"));
      keywords.insert(QString("switch"));
      keywords.insert(QString("template"));
      keywords.insert(QString("this"));
      keywords.insert(QString("throw"));
      keywords.insert(QString("true"));
      keywords.insert(QString("try"));
      keywords.insert(QString("typedef"));
      keywords.insert(QString("typeid"));
      keywords.insert(QString("typename"));
      keywords.insert(QString("union"));
      keywords.insert(QString("unsigned"));
      keywords.insert(QString("using"));
      keywords.insert(QString("virtual"));
      keywords.insert(QString("void"));
      keywords.insert(QString("volatile"));
      keywords.insert(QString("wchar_t"));
      keywords.insert(QString("while"));

      // java keywords
      keywords.insert(QString("abstract"));
      keywords.insert(QString("assert"));
      keywords.insert(QString("boolean"));
      keywords.insert(QString("break"));
      keywords.insert(QString("byte"));
      keywords.insert(QString("case"));
      keywords.insert(QString("catch"));
      keywords.insert(QString("char"));
      keywords.insert(QString("class"));
      keywords.insert(QString("const"));
      keywords.insert(QString("continue"));
      keywords.insert(QString("default"));
      keywords.insert(QString("do"));
      keywords.insert(QString("double"));
      keywords.insert(QString("else"));
      keywords.insert(QString("enum"));
      keywords.insert(QString("extends"));
      keywords.insert(QString("false"));
      keywords.insert(QString("final"));
      keywords.insert(QString("finality"));
      keywords.insert(QString("float"));
      keywords.insert(QString("for"));
      keywords.insert(QString("goto"));
      keywords.insert(QString("if"));
      keywords.insert(QString("implements"));
      keywords.insert(QString("import"));
      keywords.insert(QString("instanceof"));
      keywords.insert(QString("int"));
      keywords.insert(QString("interface"));
      keywords.insert(QString("long"));
      keywords.insert(QString("native"));
      keywords.insert(QString("new"));
      keywords.insert(QString("null"));
      keywords.insert(QString("package"));
      keywords.insert(QString("private"));
      keywords.insert(QString("protected"));
      keywords.insert(QString("public"));
      keywords.insert(QString("return"));
      keywords.insert(QString("short"));
      keywords.insert(QString("static"));
      keywords.insert(QString("strictfp"));
      keywords.insert(QString("super"));
      keywords.insert(QString("switch"));
      keywords.insert(QString("synchronized"));
      keywords.insert(QString("this"));
      keywords.insert(QString("throw"));
      keywords.insert(QString("throws"));
      keywords.insert(QString("transient"));
      keywords.insert(QString("true"));
      keywords.insert(QString("try"));
      keywords.insert(QString("void"));
      keywords.insert(QString("volatile"));
      keywords.insert(QString("while"));
   }

   return keywords;
}

bool FormWindow::unify(QObject *w, QString &s, bool changeIt)
{
   QWidget *main = mainContainer();
   if (! main) {
      return true;
   }

   QSet<QString> existingNames = languageKeywords();

   // build a set of existing names of other widget excluding self
   if (! (w->isWidgetType() && isMainContainer(dynamic_cast<QWidget *>(w)))) {
      existingNames.insert(main->objectName());
   }

   const QDesignerMetaDataBaseInterface *metaDataBase = core()->metaDataBase();
   const QWidgetList widgetChildren = main->findChildren<QWidget *>();
   if (!widgetChildren.empty()) {
      insertNames(metaDataBase, widgetChildren.constBegin(), widgetChildren.constEnd(), w, existingNames);
   }

   const QList<QLayout *> layoutChildren = main->findChildren<QLayout *>();
   if (! layoutChildren.empty()) {
      insertNames(metaDataBase, layoutChildren.constBegin(), layoutChildren.constEnd(), w, existingNames);
   }

   const QList<QAction *> actionChildren = main->findChildren<QAction *>();
   if (! actionChildren.empty()) {
      insertNames(metaDataBase, actionChildren.constBegin(), actionChildren.constEnd(), w, existingNames);
   }

   const QList<QButtonGroup *> buttonGroupChildren = main->findChildren<QButtonGroup *>();
   if (! buttonGroupChildren.empty()) {
      insertNames(metaDataBase, buttonGroupChildren.constBegin(), buttonGroupChildren.constEnd(), w, existingNames);
   }

   const auto enEnd = existingNames.constEnd();
   if (existingNames.constFind(s) == enEnd) {
      return true;
   } else if (!changeIt) {
      return false;
   }

   // split 'name_number'
   qint64 num = 0;
   qint64 factor = 1;
   int idx = s.length() - 1;

   const ushort zeroUnicode = QLatin1Char('0').unicode();
   for ( ; idx > 0 && s.at(idx).isDigit(); --idx) {
      num += (s.at(idx).unicode() - zeroUnicode) * factor;
      factor *= 10;
   }

   // Position index past '_'.
   const QChar underscore = QLatin1Char('_');
   if (idx >= 0 && s.at(idx) == underscore) {
      ++idx;
   }  else {
      num = 1;
      s += underscore;
      idx = s.length();
   }
   // try 'name_n', 'name_n+1'
   for (num++ ; ; num++) {
      s.truncate(idx);
      s += QString::number(num);
      if (existingNames.constFind(s) == enEnd) {
         break;
      }
   }

   return false;
}

/* already_in_form is true when we are moving a widget from one parent to another inside the same
 * form. All this means is that InsertWidgetCommand::undo() must not unmanage it. */

void FormWindow::insertWidget(QWidget *w, const QRect &rect, QWidget *container, bool already_in_form)
{
   clearSelection(false);

   beginCommand(tr("Insert widget '%1'").formatArg(WidgetFactory::classNameOf(m_core, w))); // ### use the WidgetDatabaseItem

   /* Reparenting into a QSplitter automatically adjusts child's geometry. We create the geometry
    * command before we push the reparent command, so that the geometry command has the original
    * geometry of the widget. */

   QRect r = rect;
   Q_ASSERT(r.isValid());

   SetPropertyCommand *geom_cmd = new SetPropertyCommand(this);
   geom_cmd->init(w, QString("geometry"), r); // ### use rc.size()

   if (w->parentWidget() != container) {
      ReparentWidgetCommand *cmd = new ReparentWidgetCommand(this);
      cmd->init(w, container);
      m_undoStack.push(cmd);
   }

   m_undoStack.push(geom_cmd);

   InsertWidgetCommand *cmd = new InsertWidgetCommand(this);
   cmd->init(w, already_in_form);
   m_undoStack.push(cmd);

   endCommand();

   w->show();
}

QWidget *FormWindow::createWidget(DomUI *ui, const QRect &rc, QWidget *target)
{
   QWidget *container = findContainer(target, false);
   if (!container) {
      return nullptr;
   }
   if (isMainContainer(container)) {
      if (QMainWindow *mw = dynamic_cast<QMainWindow *>(container)) {
         Q_ASSERT(mw->centralWidget() != nullptr);
         container = mw->centralWidget();
      }
   }
   QDesignerResource resource(this);
   const FormBuilderClipboard clipboard = resource.paste(ui, container);
   if (clipboard.m_widgets.size() != 1) { // multiple-paste from DomUI not supported yet
      return nullptr;
   }
   QWidget *widget = clipboard.m_widgets.first();
   insertWidget(widget, rc, container);
   return widget;
}

static bool isDescendant(const QWidget *parent, const QWidget *child)
{
   while (child != nullptr) {
      if (child == parent) {
         return true;
      }

      child = child->parentWidget();
   }

   return false;
}

void FormWindow::resizeWidget(QWidget *widget, const QRect &geometry)
{
   Q_ASSERT(isDescendant(this, widget));

   QRect r = geometry;
   SetPropertyCommand *cmd = new SetPropertyCommand(this);
   cmd->init(widget, QString("geometry"), r);
   cmd->setText(tr("Resize"));
   m_undoStack.push(cmd);
}

void FormWindow::raiseChildSelections(QWidget *w)
{
   const QWidgetList l = w->findChildren<QWidget *>();
   if (l.isEmpty()) {
      return;
   }
   m_selection->raiseList(l);
}

QWidget *FormWindow::containerAt(const QPoint &pos, QWidget *notParentOf)
{
   QWidget *container = nullptr;
   int depth = -1;

   const QWidgetList selected = selectedWidgets();
   if (rect().contains(mapFromGlobal(pos))) {
      container = mainContainer();
      depth = widgetDepth(container);
   }

   QListIterator<QWidget *> it(m_widgets);
   while (it.hasNext()) {
      QWidget *wit = it.next();
      if (dynamic_cast<QLayoutWidget *>(wit) || dynamic_cast<QSplitter *>(wit)) {
         continue;
      }
      if (!wit->isVisibleTo(this)) {
         continue;
      }
      if (selected.indexOf(wit) != -1) {
         continue;
      }
      if (!core()->widgetDataBase()->isContainer(wit) &&
         wit != mainContainer()) {
         continue;
      }

      // the rectangles of all ancestors of the container must contain the insert position
      QWidget *w = wit;
      while (w && !w->isWindow()) {
         if (!w->rect().contains((w->mapFromGlobal(pos)))) {
            break;
         }
         w = w->parentWidget();
      }

      if (!(w == nullptr || w->isWindow())) {
         continue;   // we did not get through the full while loop
      }

      int wd = widgetDepth(wit);
      if (wd == depth && container) {
         if (wit->parentWidget()->children().indexOf(wit) >
            container->parentWidget()->children().indexOf(container)) {
            ++wd;
         }
      }
      if (wd > depth && !isChildOf(wit, notParentOf)) {
         depth = wd;
         container = wit;
      }
   }
   return container;
}

QWidgetList FormWindow::selectedWidgets() const
{
   return m_selection->selectedWidgets();
}

void FormWindow::selectWidgets()
{
   bool selectionChanged = false;
   const QWidgetList l = mainContainer()->findChildren<QWidget *>();
   QListIterator <QWidget *> it(l);
   const QRect selRect(mapToGlobal(m_currRect.topLeft()), m_currRect.size());
   while (it.hasNext()) {
      QWidget *w = it.next();
      if (w->isVisibleTo(this) && isManaged(w)) {
         const QPoint p = w->mapToGlobal(QPoint(0, 0));
         const QRect r(p, w->size());
         if (r.intersects(selRect) && !r.contains(selRect) && trySelectWidget(w, true)) {
            selectionChanged = true;
         }
      }
   }

   if (selectionChanged) {
      emitSelectionChanged();
   }
}

bool FormWindow::handleKeyPressEvent(QWidget *widget, QWidget *, QKeyEvent *e)
{
   if (dynamic_cast<const FormWindow *>(widget) || dynamic_cast<const QMenu *>(widget)) {
      return false;
   }

   e->accept(); // we always accept!

   switch (e->key()) {
      default:
         break; // we don't care about the other keys

      case Qt::Key_Delete:
      case Qt::Key_Backspace:
         if (e->modifiers() == Qt::NoModifier) {
            deleteWidgets();
         }
         break;

      case Qt::Key_Tab:
         if (e->modifiers() == Qt::NoModifier) {
            cursor()->movePosition(QDesignerFormWindowCursorInterface::Next);
         }
         break;

      case Qt::Key_Backtab:
         if (e->modifiers() == Qt::NoModifier) {
            cursor()->movePosition(QDesignerFormWindowCursorInterface::Prev);
         }
         break;

      case Qt::Key_Left:
      case Qt::Key_Right:
      case Qt::Key_Up:
      case Qt::Key_Down:
         handleArrowKeyEvent(e->key(), e->modifiers());
         break;
   }

   return true;
}

int FormWindow::getValue(const QRect &rect, int key, bool size) const
{
   if (size) {
      if (key == Qt::Key_Left || key == Qt::Key_Right) {
         return rect.width();
      }
      return rect.height();
   }
   if (key == Qt::Key_Left || key == Qt::Key_Right) {
      return rect.x();
   }
   return rect.y();
}

int FormWindow::calcValue(int val, bool forward, bool snap, int snapOffset) const
{
   if (snap) {
      const int rest = val % snapOffset;
      if (rest) {
         const int offset = forward ? snapOffset : 0;
         const int newOffset = rest < 0 ? offset - snapOffset : offset;
         return val + newOffset - rest;
      }
      return (forward ? val + snapOffset : val - snapOffset);
   }
   return (forward ? val + 1 : val - 1);
}

// ArrowKeyOperation: Stores a keyboard move or resize (Shift pressed)
// operation.
struct ArrowKeyOperation {
   ArrowKeyOperation() : resize(false), distance(0), arrowKey(Qt::Key_Left) {}

   QRect apply(const QRect &in) const;

   bool resize; // Resize: Shift-Key->drag bottom/right corner, else just move
   int distance;
   int arrowKey;
};

}   // end namespace - qdesigner_internal

CS_DECLARE_METATYPE(qdesigner_internal::ArrowKeyOperation)

namespace qdesigner_internal {

QRect ArrowKeyOperation::apply(const QRect &rect) const
{
   QRect retval = rect;

   if (resize) {
      if (arrowKey == Qt::Key_Left || arrowKey == Qt::Key_Right) {
         retval.setWidth(retval.width() + distance);
      } else {
         retval.setHeight(retval.height() + distance);
      }

   } else {
      if (arrowKey == Qt::Key_Left || arrowKey == Qt::Key_Right) {
         retval.moveLeft(retval.x() + distance);
      } else {
         retval.moveTop(retval.y() + distance);
      }
   }

   return retval;
}

// ArrowKeyPropertyHelper: Applies a struct ArrowKeyOperation (stored as new value) to a list of
// widgets using to calculate the changed geometry of the widget in setValue(). Thus, the 'newValue'
// of the property command is the relative move distance, which is the same for all widgets
// (although resulting in different geometries for the widgets). The command merging can then work
// as it would when applying the same text to all QLabels.

class ArrowKeyPropertyHelper : public PropertyHelper
{
 public:
   ArrowKeyPropertyHelper(QObject *o, SpecialProperty sp, QDesignerPropertySheetExtension *s, int i)
      : PropertyHelper(o, sp, s, i) {}

   Value setValue(QDesignerFormWindowInterface *fw, const QVariant &value, bool changed, unsigned subPropertyMask) override;
};

PropertyHelper::Value ArrowKeyPropertyHelper::setValue(QDesignerFormWindowInterface *fw, const QVariant &value, bool changed,
   unsigned subPropertyMask)
{
   // Apply operation to obtain the new geometry value.
   QWidget *w = dynamic_cast<QWidget *>(object());

   const ArrowKeyOperation operation = value.value<ArrowKeyOperation>();
   const QRect newGeom = operation.apply(w->geometry());

   return PropertyHelper::setValue(fw, QVariant(newGeom), changed, subPropertyMask);
}

// ArrowKeyPropertyCommand: Helper factory overwritten to create
// ArrowKeyPropertyHelper and a merge operation that merges values of the same direction.
class ArrowKeyPropertyCommand: public SetPropertyCommand
{
 public:
   explicit ArrowKeyPropertyCommand(QDesignerFormWindowInterface *fw, QUndoCommand *p = nullptr);

   void init(QWidgetList &l, const ArrowKeyOperation &op);

 protected:
   PropertyHelper *createPropertyHelper(QObject *o, SpecialProperty sp,
      QDesignerPropertySheetExtension *s, int i) const override {
      return new ArrowKeyPropertyHelper(o, sp, s, i);
   }

   QVariant mergeValue(const QVariant &newValue) override;
};

ArrowKeyPropertyCommand::ArrowKeyPropertyCommand(QDesignerFormWindowInterface *fw, QUndoCommand *p)
   : SetPropertyCommand(fw, p)
{
}

void ArrowKeyPropertyCommand::init(QWidgetList &l, const ArrowKeyOperation &op)
{
   QObjectList ol;

   for (QWidget *w : l) {
      ol.push_back(w);
   }

   SetPropertyCommand::init(ol, "geometry", QVariant::fromValue(op));

   setText(op.resize ? FormWindow::tr("Key Resize") : FormWindow::tr("Key Move"));
}

QVariant ArrowKeyPropertyCommand::mergeValue(const QVariant &newMergeValue)
{
   // Merge move operations of the same arrow key
   if (!newMergeValue.canConvert<ArrowKeyOperation>()) {
      return QVariant();
   }

   ArrowKeyOperation mergedOperation = newValue().value<ArrowKeyOperation>();
   const ArrowKeyOperation newMergeOperation = newMergeValue.value<ArrowKeyOperation>();

   if (mergedOperation.resize != newMergeOperation.resize || mergedOperation.arrowKey != newMergeOperation.arrowKey) {
      return QVariant();
   }

   mergedOperation.distance += newMergeOperation.distance;
   return QVariant::fromValue(mergedOperation);
}

void FormWindow::handleArrowKeyEvent(int key, Qt::KeyboardModifiers modifiers)
{
   const QDesignerFormWindowCursorInterface *c = cursor();
   if (! c->hasSelection()) {
      return;
   }

   QWidgetList selection;

   // check if a laid out widget is selected
   const int count = c->selectedWidgetCount();
   for (int index = 0; index < count; ++index) {
      QWidget *w = c->selectedWidget(index);

      if (!LayoutInfo::isWidgetLaidout(m_core, w)) {
         selection.append(w);
      }
   }

   simplifySelection(&selection);

   if (selection.isEmpty()) {
      return;
   }

   QWidget *current = c->current();
   if (!current || LayoutInfo::isWidgetLaidout(m_core, current)) {
      current = selection.first();
   }

   const bool size = modifiers & Qt::ShiftModifier;

   const bool snap = !(modifiers & Qt::ControlModifier);
   const bool forward = (key == Qt::Key_Right || key == Qt::Key_Down);
   const int snapPoint = (key == Qt::Key_Left || key == Qt::Key_Right) ? grid().x() : grid().y();

   const int oldValue = getValue(current->geometry(), key, size);

   const int newValue = calcValue(oldValue, forward, snap, snapPoint);

   ArrowKeyOperation operation;
   operation.resize = modifiers & Qt::ShiftModifier;
   operation.distance = newValue - oldValue;
   operation.arrowKey = key;

   ArrowKeyPropertyCommand *cmd = new ArrowKeyPropertyCommand(this);
   cmd->init(selection, operation);
   m_undoStack.push(cmd);
}

bool FormWindow::handleKeyReleaseEvent(QWidget *, QWidget *, QKeyEvent *e)
{
   e->accept();
   return true;
}

void FormWindow::selectAll()
{
   bool selectionChanged = false;
   for (QWidget *widget : m_widgets) {
      if (widget->isVisibleTo(this) && trySelectWidget(widget, true)) {
         selectionChanged = true;
      }
   }

   if (selectionChanged) {
      emitSelectionChanged();
   }
}

void FormWindow::createLayout(int type, QWidget *container)
{
   if (container) {
      layoutContainer(container, type);
   } else {
      LayoutCommand *cmd = new LayoutCommand(this);
      cmd->init(mainContainer(), selectedWidgets(), static_cast<LayoutInfo::Type>(type));
      commandHistory()->push(cmd);
   }
}

void FormWindow::morphLayout(QWidget *container, int newType)
{
   MorphLayoutCommand *cmd = new MorphLayoutCommand(this);

   if (cmd->init(container, newType)) {
      commandHistory()->push(cmd);
   } else {
      csWarning("Unable to morph layout");
      delete cmd;
   }
}

void FormWindow::deleteWidgets()
{
   QWidgetList selection = selectedWidgets();
   simplifySelection(&selection);

   deleteWidgetList(selection);
}

QString FormWindow::fileName() const
{
   return m_fileName;
}

void FormWindow::setFileName(const QString &fileName)
{
   if (m_fileName == fileName) {
      return;
   }

   m_fileName = fileName;
   emit fileNameChanged(fileName);
}

QString FormWindow::contents() const
{
   QBuffer b;
   if (!mainContainer() || !b.open(QIODevice::WriteOnly)) {
      return QString();
   }

   QDesignerResource resource(const_cast<FormWindow *>(this));
   resource.save(&b, mainContainer());

   return QString::fromUtf8(b.buffer());
}

void FormWindow::copy()
{
   QBuffer b;
   if (!b.open(QIODevice::WriteOnly)) {
      return;
   }

   FormBuilderClipboard clipboard;
   QDesignerResource resource(this);
   resource.setSaveRelative(false);
   clipboard.m_widgets = selectedWidgets();
   simplifySelection(&clipboard.m_widgets);
   resource.copy(&b, clipboard);

   qApp->clipboard()->setText(QString::fromUtf8(b.buffer()), QClipboard::Clipboard);
}

void FormWindow::cut()
{
   copy();
   deleteWidgets();
}

void FormWindow::paste()
{
   paste(PasteAll);
}

// for cases like QMainWindow (central widget is an inner container) or QStackedWidget (page is an inner container)
QWidget *FormWindow::innerContainer(QWidget *outerContainer) const
{
   if (m_core->widgetDataBase()->isContainer(outerContainer)) {
      if (const QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension *>(m_core->extensionManager(),
               outerContainer)) {
         const int currentIndex = container->currentIndex();

         return currentIndex >= 0 ? container->widget(currentIndex) : nullptr;
      }
   }

   return outerContainer;
}

QWidget *FormWindow::containerForPaste() const
{
   QWidget *w = mainContainer();

   if (! w) {
      return nullptr;
   }

   do {
      // Try to find a close parent, for example a non-laid-out
      // QFrame/QGroupBox when a widget within it is selected.
      QWidgetList selection = selectedWidgets();
      if (selection.empty()) {
         break;
      }
      simplifySelection(&selection);

      QWidget *containerOfW = findContainer(selection.first(), /* exclude layouts */ true);
      if (!containerOfW || containerOfW == mainContainer()) {
         break;
      }
      // No layouts, must be container. No empty page-based containers.
      containerOfW = innerContainer(containerOfW);
      if (!containerOfW) {
         break;
      }

      if (LayoutInfo::layoutType(m_core, containerOfW) != LayoutInfo::NoLayout || !m_core->widgetDataBase()->isContainer(containerOfW)) {
         break;
      }

      w = containerOfW;

   } while (false);

   // First check for layout (note that it does not cover QMainWindow
   // and the like as the central widget has the layout).

   w = innerContainer(w);
   if (!w) {
      return nullptr;
   }

   if (LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout) {
      return nullptr;
   }

   // Go up via container extension (also includes step from QMainWindow to its central widget)
   w = m_core->widgetFactory()->containerOfWidget(w);

   if (w == nullptr || LayoutInfo::layoutType(m_core, w) != LayoutInfo::NoLayout) {
      return nullptr;
   }

   return w;
}

// Construct DomUI from clipboard (paste) and determine number of widgets/actions.
static inline DomUI *domUIFromClipboard(int *widgetCount, int *actionCount)
{
   *widgetCount = 0;
   *actionCount = 0;

   const QString clipboardText = qApp->clipboard()->text();
   if (clipboardText.isEmpty() || clipboardText.indexOf('<') == -1) {
      return nullptr;
   }

   QXmlStreamReader reader(clipboardText);
   DomUI *ui = nullptr;
   const QString uiElement = "ui";

   while (!reader.atEnd()) {
      if (reader.readNext() == QXmlStreamReader::StartElement) {

         if (reader.name().compare(uiElement, Qt::CaseInsensitive) == 0 && !ui) {
            ui = new DomUI();
            ui->read(reader);
            break;

         } else {
            reader.raiseError(QCoreApplication::translate("FormWindow", "Unexpected element <%1>").formatArg(reader.name().toString()));
         }
      }
   }
   if (reader.hasError()) {
      delete ui;
      ui = nullptr;

      csWarning(QCoreApplication::translate("FormWindow", "Error while pasting clipboard contents at line %1, column %2: %3")
            .formatArg(reader.lineNumber()).formatArg(reader.columnNumber()).formatArg(reader.errorString()));

      return nullptr;
   }

   if (const DomWidget *topLevel = ui->elementWidget()) {
      *widgetCount = topLevel->elementWidget().size();
      *actionCount = topLevel->elementAction().size();
   }

   if (*widgetCount == 0 && *actionCount == 0) {
      delete ui;
      return nullptr;
   }

   return ui;
}

static inline QString pasteCommandDescription(int widgetCount, int actionCount)
{
   if (widgetCount == 0) {
      return FormWindow::tr("Paste %n action(s)", nullptr, actionCount);
   }

   if (actionCount == 0) {
      return FormWindow::tr("Paste %n widget(s)", nullptr, widgetCount);
   }

   return FormWindow::tr("Paste (%1 widgets, %2 actions)").formatArg(widgetCount).formatArg(actionCount);
}

static void positionPastedWidgetsAtMousePosition(FormWindow *fw, const QPoint &contextMenuPosition, QWidget *parent,
   const QWidgetList &l)
{
   // Try to position pasted widgets at mouse position (current mouse position for Ctrl-V or position of context menu)
   // if it fits. If it is completely outside, force it to 0,0
   // If it fails, the old coordinates relative to the previous parent will be used.
   QPoint currentPos = contextMenuPosition.x() >= 0 ? parent->mapFrom(fw, contextMenuPosition) : parent->mapFromGlobal(QCursor::pos());
   const Grid &grid = fw->designerGrid();

   QPoint cursorPos = grid.snapPoint(currentPos);
   const QRect parentGeometry = QRect(QPoint(0, 0), parent->size());
   const bool outside = !parentGeometry.contains(cursorPos);

   if (outside) {
      cursorPos = grid.snapPoint(QPoint(0, 0));
   }

   // Determine area of pasted widgets
   QRect pasteArea;

   const QWidgetList::const_iterator lcend = l.constEnd();
   for (QWidgetList::const_iterator it = l.constBegin(); it != lcend; ++it) {
      pasteArea = pasteArea.isNull() ? (*it)->geometry() : pasteArea.united((*it)->geometry());
   }

   // Mouse on some child? (try to position bottomRight on a free spot to
   // get the stacked-offset effect of Designer 4.3, that is, offset by grid if Ctrl-V is pressed continuously
   do {
      const QPoint bottomRight = cursorPos + QPoint(pasteArea.width(), pasteArea.height()) - QPoint(1, 1);
      if (bottomRight.y() > parentGeometry.bottom() || parent->childAt(bottomRight) == nullptr) {
         break;
      }
      cursorPos += QPoint(grid.deltaX(), grid.deltaY());
   } while (true);

   // Move
   const QPoint offset = cursorPos - pasteArea.topLeft();

   for (QWidgetList::const_iterator it = l.constBegin(); it != lcend; ++it) {
      (*it)->move((*it)->pos() + offset);
   }
}

void FormWindow::paste(PasteMode pasteMode)
{
   // Avoid QDesignerResource constructing widgets that are not used as
   // QDesignerResource manages the widgets it creates (creating havoc if one remains unused)
   DomUI *ui = nullptr;

   do {
      int widgetCount;
      int actionCount;

      ui = domUIFromClipboard(&widgetCount, &actionCount);
      if (!ui) {
         break;
      }

      // Check for actions
      if (pasteMode == PasteActionsOnly)
         if (widgetCount != 0 || actionCount == 0) {
            break;
         }

      // Check for widgets: need a container
      QWidget *pasteContainer = widgetCount ? containerForPaste() : nullptr;
      if (widgetCount && pasteContainer == nullptr) {

         const QString message = tr("Cannot paste widgets. Designer could not find a container "
               "without a layout to paste into.");

         const QString infoMessage = tr("Break the layout of the "
               "container you want to paste into, select this container and then paste again.");

         core()->dialogGui()->message(this, QDesignerDialogGuiInterface::FormEditorMessage, QMessageBox::Information,
            tr("Paste error"), message, infoMessage, QMessageBox::Ok);

         break;
      }

      QDesignerResource resource(this);
      // Note that the widget factory must be able to locate the
      // form window (us) via parent, otherwise, it will not able to construct QLayoutWidgets
      // (It will then default to widgets) among other issues.
      const FormBuilderClipboard clipboard = resource.paste(ui, pasteContainer, this);

      clearSelection(false);

      // Create command sequence
      beginCommand(pasteCommandDescription(widgetCount, actionCount));

      if (widgetCount) {
         positionPastedWidgetsAtMousePosition(this,  m_contextMenuPosition, pasteContainer, clipboard.m_widgets);
         for (QWidget *w : clipboard.m_widgets) {
            InsertWidgetCommand *cmd = new InsertWidgetCommand(this);
            cmd->init(w);
            m_undoStack.push(cmd);
            selectWidget(w);
         }
      }

      if (actionCount)
         for (QAction *a : clipboard.m_actions) {
            ensureUniqueObjectName(a);
            AddActionCommand *cmd = new AddActionCommand(this);
            cmd->init(a);
            m_undoStack.push(cmd);
         }
      endCommand();
   } while (false);

   delete ui;
}

// Draw a dotted frame around containers
bool FormWindow::frameNeeded(QWidget *w) const
{
   if (!core()->widgetDataBase()->isContainer(w)) {
      return false;
   }
   if (dynamic_cast<QGroupBox *>(w)) {
      return false;
   }
   if (dynamic_cast<QToolBox *>(w)) {
      return false;
   }
   if (dynamic_cast<QTabWidget *>(w)) {
      return false;
   }
   if (dynamic_cast<QStackedWidget *>(w)) {
      return false;
   }
   if (dynamic_cast<QDockWidget *>(w)) {
      return false;
   }
   if (dynamic_cast<QDesignerWidget *>(w)) {
      return false;
   }
   if (dynamic_cast<QMainWindow *>(w)) {
      return false;
   }
   if (dynamic_cast<QDialog *>(w)) {
      return false;
   }
   if (dynamic_cast<QLayoutWidget *>(w)) {
      return false;
   }
   return true;
}

bool FormWindow::eventFilter(QObject *watched, QEvent *event)
{
   const bool ret = FormWindowBase::eventFilter(watched, event);
   if (event->type() != QEvent::Paint) {
      return ret;
   }

   Q_ASSERT(watched->isWidgetType());

   QWidget *w = static_cast<QWidget *>(watched);
   QPaintEvent *pe = static_cast<QPaintEvent *>(event);
   const QRect widgetRect = w->rect();
   const QRect paintRect =  pe->rect();

   // Does the paint rectangle touch the borders of the widget rectangle
   if (paintRect.x()     > widgetRect.x()     && paintRect.y()      > widgetRect.y() &&
      paintRect.right() < widgetRect.right() && paintRect.bottom() < widgetRect.bottom()) {
      return ret;
   }
   QPainter p(w);
   const QPen pen(QColor(0, 0, 0, 32), 0, Qt::DotLine);
   p.setPen(pen);
   p.setBrush(QBrush(Qt::NoBrush));
   p.drawRect(widgetRect.adjusted(0, 0, -1, -1));
   return ret;
}

void FormWindow::manageWidget(QWidget *w)
{
   if (isManaged(w)) {
      return;
   }

   Q_ASSERT(dynamic_cast<QMenu *>(w) == nullptr);

   if (w->hasFocus()) {
      setFocus();
   }

   core()->metaDataBase()->add(w);

   m_insertedWidgets.insert(w);
   m_widgets.append(w);

   setCursorToAll(Qt::ArrowCursor, w);

   emit changed();
   emit widgetManaged(w);

   if (frameNeeded(w)) {
      w->installEventFilter(this);
   }
}

void FormWindow::unmanageWidget(QWidget *w)
{
   if (!isManaged(w)) {
      return;
   }

   m_selection->removeWidget(w);

   emit aboutToUnmanageWidget(w);

   if (w == m_currentWidget) {
      setCurrentWidget(mainContainer());
   }

   core()->metaDataBase()->remove(w);

   m_insertedWidgets.remove(w);
   m_widgets.removeAt(m_widgets.indexOf(w));

   emit changed();
   emit widgetUnmanaged(w);

   if (frameNeeded(w)) {
      w->removeEventFilter(this);
   }
}

bool FormWindow::isManaged(QWidget *w) const
{
   return m_insertedWidgets.contains(w);
}

void FormWindow::breakLayout(QWidget *w)
{
   if (w == this) {
      w = mainContainer();
   }

   // Find the first-order managed child widgets
   QWidgetList widgets;
   const QDesignerMetaDataBaseInterface *mdb = core()->metaDataBase();

   for (const auto item : w->children()) {

      if (item->isWidgetType())  {
         QWidget *widget = static_cast<QWidget *>(item);

         if (mdb->item(widget)) {
            widgets.push_back(widget);
         }
      }
   }

   BreakLayoutCommand *cmd = new BreakLayoutCommand(this);
   cmd->init(widgets, w);
   commandHistory()->push(cmd);
   clearSelection(false);
}

void FormWindow::beginCommand(const QString &description)
{
   m_undoStack.beginMacro(description);
}

void FormWindow::endCommand()
{
   m_undoStack.endMacro();
}

void FormWindow::raiseWidgets()
{
   QWidgetList widgets = selectedWidgets();
   simplifySelection(&widgets);

   if (widgets.isEmpty()) {
      return;
   }

   beginCommand(tr("Raise widgets"));
   for (QWidget *widget : widgets) {
      RaiseWidgetCommand *cmd = new RaiseWidgetCommand(this);
      cmd->init(widget);
      m_undoStack.push(cmd);
   }
   endCommand();
}

void FormWindow::lowerWidgets()
{
   QWidgetList widgets = selectedWidgets();
   simplifySelection(&widgets);

   if (widgets.isEmpty()) {
      return;
   }

   beginCommand(tr("Lower widgets"));

   for (QWidget *widget : widgets) {
      LowerWidgetCommand *cmd = new LowerWidgetCommand(this);
      cmd->init(widget);
      m_undoStack.push(cmd);
   }

   endCommand();
}

bool FormWindow::handleMouseButtonDblClickEvent(QWidget *w, QWidget *managedWidget, QMouseEvent *e)
{
   (void) w;

   e->accept();

   // Might be out of sync due cycling of the parent selection
   // In that case, do nothing
   if (isWidgetSelected(managedWidget)) {
      emit activated(managedWidget);
   }

   m_mouseState = MouseDoubleClicked;

   return true;
}

QMenu *FormWindow::initializePopupMenu(QWidget *managedWidget)
{
   if (! isManaged(managedWidget) || currentTool()) {
      return nullptr;
   }

   // Make sure the managedWidget is selected and current since
   // the SetPropertyCommands must use the right reference
   // object obtained from the property editor for the property group
   // of a multiselection to be correct.

   const bool selected = isWidgetSelected(managedWidget);
   bool update = false;

   if (selected == false) {
      clearObjectInspectorSelection(m_core); // We might have a toolbar or non-widget selected in the object inspector.
      clearSelection(false);
      update = trySelectWidget(managedWidget, true);
      raiseChildSelections(managedWidget);   // raise selections and select widget
   } else {
      update = setCurrentWidget(managedWidget);
   }

   if (update) {
      emitSelectionChanged();
      QMetaObject::invokeMethod(core()->formWindowManager(), "slotUpdateActions");
   }

   QWidget *contextMenuWidget = nullptr;

   if (isMainContainer(managedWidget)) { // press on a child widget
      contextMenuWidget = mainContainer();
   } else {  // press on a child widget
      // if widget is laid out, find the first non-laid out super-widget
      QWidget *realWidget = managedWidget; // but store the original one
      QMainWindow *mw = dynamic_cast<QMainWindow *>(mainContainer());

      if (mw && mw->centralWidget() == realWidget) {
         contextMenuWidget = managedWidget;
      } else {
         contextMenuWidget = realWidget;
      }
   }

   if (!contextMenuWidget) {
      return nullptr;
   }

   QMenu *contextMenu = createPopupMenu(contextMenuWidget);
   if (!contextMenu) {
      return nullptr;
   }

   emit contextMenuRequested(contextMenu, contextMenuWidget);
   return contextMenu;
}

bool FormWindow::handleContextMenu(QWidget *, QWidget *managedWidget, QContextMenuEvent *e)
{
   QMenu *contextMenu = initializePopupMenu(managedWidget);
   if (!contextMenu) {
      return false;
   }
   const QPoint globalPos = e->globalPos();
   m_contextMenuPosition = mapFromGlobal (globalPos);
   contextMenu->exec(globalPos);
   delete contextMenu;
   e->accept();
   m_contextMenuPosition = QPoint(-1, -1);

   return true;
}

bool FormWindow::setContents(QIODevice *dev, QString *errorMessageIn)
{
   UpdateBlocker ub(this);
   clearSelection();

   m_selection->clearSelectionPool();
   m_insertedWidgets.clear();
   m_widgets.clear();

   // The main container is cleared as otherwise
   // the names of the newly loaded objects will be unified.
   clearMainContainer();
   m_undoStack.clear();
   emit changed();

   QDesignerResource r(this);
   QWidget *w = r.load(dev, formContainer());

   if (w) {
      setMainContainer(w);
      emit changed();
   }

   if (errorMessageIn) {
      *errorMessageIn = r.errorString();
   }

   return w != nullptr;
}

bool FormWindow::setContents(const QString &contents)
{
   QString errorMessage;
   QByteArray arrayData = contents.toUtf8();

   QBuffer b(&arrayData);
   const bool success = b.open(QIODevice::ReadOnly) &&  setContents(&b, &errorMessage);

   if (! success && ! errorMessage.isEmpty()) {
      csWarning(errorMessage);
   }

   return success;
}

void FormWindow::layoutContainer(QWidget *w, int type)
{
   if (w == this) {
      w = mainContainer();
   }

   w = core()->widgetFactory()->containerOfWidget(w);

   const QObjectList l = w->children();
   // find managed widget children
   QWidgetList widgets;
   const QObjectList::const_iterator ocend = l.constEnd();
   for (QObjectList::const_iterator it = l.constBegin(); it != ocend; ++it)
      if ( (*it)->isWidgetType() ) {
         QWidget *widget = static_cast<QWidget *>(*it);
         if (widget->isVisibleTo(this) && isManaged(widget)) {
            widgets.append(widget);
         }
      }

   if (widgets.isEmpty()) { // QTBUG-50563, observed when using hand-edited forms.
      return;
   }

   LayoutCommand *cmd = new LayoutCommand(this);
   cmd->init(mainContainer(), widgets, static_cast<LayoutInfo::Type>(type), w);
   clearSelection(false);
   commandHistory()->push(cmd);
}

bool FormWindow::hasInsertedChildren(QWidget *widget) const // ### move
{
   if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), widget)) {
      const int index = container->currentIndex();
      if (index < 0) {
         return false;
      }
      widget = container->widget(index);
   }

   const QWidgetList l = widgets(widget);

   for (QWidget *child : l) {
      if (isManaged(child) && ! LayoutInfo::isWidgetLaidout(core(), child) && child->isVisibleTo(const_cast<FormWindow *>(this))) {
         return true;
      }
   }

   return false;
}

// "Select Ancestor" sub menu code
void FormWindow::slotSelectWidget(QAction *a)
{
   QWidget *w = a->data().value<QWidget *>();

   if (w != nullptr) {
      selectSingleWidget(w);
   }
}

static inline QString objectNameOf(const QWidget *w)
{
   if (const QLayoutWidget *lw = dynamic_cast<const QLayoutWidget *>(w)) {
      const QLayout *layout = lw->layout();
      const QString rc = layout->objectName();

      if (! rc.isEmpty()) {
         return rc;
      }

      // Fall thru for 4.3 forms which have a name on the widget: Display the class name
      return layout->metaObject()->className();
   }

   return  w->objectName();
}

QAction *FormWindow::createSelectAncestorSubMenu(QWidget *w)
{
   // Find the managed, unselected parents
   QWidgetList parents;
   QWidget *mc = mainContainer();

   for (QWidget *p = w->parentWidget(); p && p != mc; p = p->parentWidget()) {
      if (isManaged(p) && !isWidgetSelected(p)) {
         parents.push_back(p);
      }
   }

   if (parents.empty()) {
      return nullptr;
   }

   // Create a submenu listing the managed, unselected parents
   QMenu *menu = new QMenu;
   QActionGroup *ag = new QActionGroup(menu);
   QObject::connect(ag, &QActionGroup::triggered, this, &FormWindow::slotSelectWidget);

   const int size = parents.size();

   for (int i = 0; i < size; i++) {
      QWidget *widget = parents.at(i);

      QAction *action = ag->addAction(objectNameOf(widget));
      action->setData(QVariant::fromValue(widget));
      menu->addAction(action);
   }

   QAction *menuAction = new QAction(tr("Select Ancestor"), nullptr);
   menuAction->setMenu(menu);

   return menuAction;
}

QMenu *FormWindow::createPopupMenu(QWidget *w)
{
   QMenu *popup = createExtensionTaskMenu(this, w, true);
   if (!popup) {
      popup = new QMenu;
   }

   // if w doesn't have a QDesignerTaskMenu as a child create one and make it a child.
   // insert actions from QDesignerTaskMenu

   QDesignerFormWindowManagerInterface *manager = core()->formWindowManager();
   const bool isFormWindow = dynamic_cast<const FormWindow *>(w);

   // Check for special containers and obtain the page menu from them to add layout actions.
   if (!isFormWindow) {
      if (QStackedWidget *stackedWidget  = dynamic_cast<QStackedWidget *>(w)) {
         QStackedWidgetEventFilter::addStackedWidgetContextMenuActions(stackedWidget, popup);

      } else if (QTabWidget *tabWidget = dynamic_cast<QTabWidget *>(w)) {
         QTabWidgetEventFilter::addTabWidgetContextMenuActions(tabWidget, popup);

      } else if (QToolBox *toolBox = dynamic_cast<QToolBox *>(w)) {
         QToolBoxHelper::addToolBoxContextMenuActions(toolBox, popup);

      }

      if (manager->action(QDesignerFormWindowManagerInterface::LowerAction)->isEnabled()) {
         popup->addAction(manager->action(QDesignerFormWindowManagerInterface::LowerAction));
         popup->addAction(manager->action(QDesignerFormWindowManagerInterface::RaiseAction));
         popup->addSeparator();
      }

      popup->addAction(manager->action(QDesignerFormWindowManagerInterface::CutAction));
      popup->addAction(manager->action(QDesignerFormWindowManagerInterface::CopyAction));
   }

   popup->addAction(manager->action(QDesignerFormWindowManagerInterface::PasteAction));

   if (QAction *selectAncestorAction = createSelectAncestorSubMenu(w)) {
      popup->addAction(selectAncestorAction);
   }
   popup->addAction(manager->action(QDesignerFormWindowManagerInterface::SelectAllAction));

   if (!isFormWindow) {
      popup->addAction(manager->action(QDesignerFormWindowManagerInterface::DeleteAction));
   }

   popup->addSeparator();
   QMenu *layoutMenu = popup->addMenu(tr("Lay out"));
   layoutMenu->addAction(manager->action(QDesignerFormWindowManagerInterface::AdjustSizeAction));
   layoutMenu->addAction(manager->action(QDesignerFormWindowManagerInterface::HorizontalLayoutAction));
   layoutMenu->addAction(manager->action(QDesignerFormWindowManagerInterface::VerticalLayoutAction));

   if (!isFormWindow) {
      layoutMenu->addAction(manager->action(QDesignerFormWindowManagerInterface::SplitHorizontalAction));
      layoutMenu->addAction(manager->action(QDesignerFormWindowManagerInterface::SplitVerticalAction));
   }

   layoutMenu->addAction(manager->action(QDesignerFormWindowManagerInterface::GridLayoutAction));
   layoutMenu->addAction(manager->action(QDesignerFormWindowManagerInterface::FormLayoutAction));
   layoutMenu->addAction(manager->action(QDesignerFormWindowManagerInterface::BreakLayoutAction));
   layoutMenu->addAction(manager->action(QDesignerFormWindowManagerInterface::SimplifyLayoutAction));

   return popup;
}

void FormWindow::resizeEvent(QResizeEvent *e)
{
   m_geometryChangedTimer->start(10);

   QWidget::resizeEvent(e);
}

QPoint FormWindow::mapToForm(const QWidget *w, const QPoint &pos) const
{
   QPoint p = pos;
   const QWidget *i = w;

   while (i && ! i->isWindow() && ! isMainContainer(i)) {
      p = i->mapToParent(p);
      i = i->parentWidget();
   }

   return mapFromGlobal(w->mapToGlobal(pos));
}

bool FormWindow::canBeBuddy(QWidget *w) const
{
   if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), w)) {
      const int index = sheet->indexOf("focusPolicy");

      if (index != -1) {
         bool ok = false;
         const Qt::FocusPolicy q = static_cast<Qt::FocusPolicy>(Utils::valueOf(sheet->property(index), &ok));
         return ok && q != Qt::NoFocus;
      }
   }

   return false;
}

QWidget *FormWindow::findContainer(QWidget *w, bool excludeLayout) const
{
   if (! isChildOf(w, this) || const_cast<const QWidget *>(w) == this) {
      return nullptr;
   }

   QDesignerWidgetFactoryInterface *widgetFactory = core()->widgetFactory();
   QDesignerWidgetDataBaseInterface *widgetDataBase = core()->widgetDataBase();
   QDesignerMetaDataBaseInterface *metaDataBase = core()->metaDataBase();

   QWidget *container = widgetFactory->containerOfWidget(mainContainer()); // default parent for new widget is the formwindow

   if (! isMainContainer(w)) {
      // press was not on formwindow, check if we can find another parent

      while (w) {
         if (dynamic_cast<InvisibleWidget *>(w) || !metaDataBase->item(w)) {
            w = w->parentWidget();
            continue;
         }

         const bool isContainer =  widgetDataBase->isContainer(w, true) || w == mainContainer();

         if (!isContainer || (excludeLayout && dynamic_cast<QLayoutWidget *>(w))) { // ### skip QSplitter
            w = w->parentWidget();
         } else {
            container = w;
            break;
         }
      }
   }

   return container;
}

void FormWindow::simplifySelection(QWidgetList *sel) const
{
   if (sel->size() < 2) {
      return;
   }

   // Figure out which widgets should be removed from selection.
   // We want to remove those whose parent widget is also in the
   // selection (because the child widgets are contained by
   // their parent, they shouldn't be in the selection --
   // they are "implicitly" selected).

   QWidget *mainC = mainContainer(); // Quick check for main container first
   if (sel->contains(mainC)) {
      sel->clear();
      sel->push_back(mainC);
      return;
   }

   QVector<QWidget *> toBeRemoved;
   toBeRemoved.reserve(sel->size());
   const QWidgetList::const_iterator scend = sel->constEnd();

   for (QWidgetList::const_iterator it = sel->constBegin(); it != scend; ++it) {
      QWidget *child = *it;

      for (QWidget *w = child; true ; ) { // Is any of the parents also selected?
         QWidget *parent = w->parentWidget();
         if (!parent || parent == mainC) {
            break;
         }
         if (sel->contains(parent)) {
            toBeRemoved.append(child);
            break;
         }
         w = parent;
      }
   }

   // actually remove the widgets that were marked for removal in the previous pass.
   for (auto item : toBeRemoved) {
      sel->removeAll(item);
   }
}

FormWindow *FormWindow::findFormWindow(QWidget *w)
{
   return dynamic_cast<FormWindow *>(QDesignerFormWindowInterface::findFormWindow(w));
}

bool FormWindow::isDirty() const
{
   return m_undoStack.isDirty();
}

void FormWindow::setDirty(bool dirty)
{
   m_undoStack.setDirty(dirty);
}

QWidget *FormWindow::containerAt(const QPoint &pos)
{
   QWidget *widget = widgetAt(pos);
   return findContainer(widget, true);
}

static QWidget *childAt_SkipDropLine(QWidget *w, QPoint pos)
{
   const QObjectList child_list = w->children();

   for (int i = child_list.size() - 1; i >= 0; --i) {
      QObject *child_obj = child_list[i];
      if (dynamic_cast<WidgetHandle *>(child_obj) != nullptr) {
         continue;
      }

      QWidget *child = dynamic_cast<QWidget *>(child_obj);

      if (!child || child->isWindow() || !child->isVisible() ||
         !child->geometry().contains(pos) || child->testAttribute(Qt::WA_TransparentForMouseEvents)) {
         continue;
      }

      const QPoint childPos = child->mapFromParent(pos);
      if (QWidget *res = childAt_SkipDropLine(child, childPos)) {
         return res;
      }

      if (child->testAttribute(Qt::WA_MouseNoMask) || child->mask().contains(pos)
         || child->mask().isEmpty()) {
         return child;
      }
   }

   return nullptr;
}

QWidget *FormWindow::widgetAt(const QPoint &pos)
{
   QWidget *w = childAt(pos);

   if (dynamic_cast<const WidgetHandle *>(w) != nullptr) {
      w = childAt_SkipDropLine(this, pos);
   }

   return (w == nullptr || w == formContainer()) ? this : w;
}

void FormWindow::highlightWidget(QWidget *widget, const QPoint &pos, HighlightMode mode)
{
   Q_ASSERT(widget);

   if (QMainWindow *mainWindow = dynamic_cast<QMainWindow *> (widget)) {
      widget = mainWindow->centralWidget();
   }

   QWidget *container = findContainer(widget, false);

   if (container == nullptr || core()->metaDataBase()->item(container) == nullptr) {
      return;
   }

   if (QDesignerActionProviderExtension *actionExtension =
         qt_extension<QDesignerActionProviderExtension *>(core()->extensionManager(), container)) {

      if (mode == Restore) {
         actionExtension->adjustIndicator(QPoint());
      } else {
         const QPoint pt = widget->mapTo(container, pos);
         actionExtension->adjustIndicator(pt);
      }

   } else if (QDesignerLayoutDecorationExtension *layoutExtension =
         qt_extension<QDesignerLayoutDecorationExtension *>(core()->extensionManager(), container)) {

      if (mode == Restore) {
         layoutExtension->adjustIndicator(QPoint(), -1);
      } else {
         const QPoint pt = widget->mapTo(container, pos);
         const int index = layoutExtension->findItemAt(pt);
         layoutExtension->adjustIndicator(pt, index);
      }
   }

   QMainWindow *mw = dynamic_cast<QMainWindow *> (container);

   if (container == mainContainer() || (mw && mw->centralWidget() && mw->centralWidget() == container)) {
      return;
   }

   if (mode == Restore) {
      const WidgetPaletteMap::iterator pit = m_palettesBeforeHighlight.find(container);

      if (pit != m_palettesBeforeHighlight.end()) {
         container->setPalette(pit.value().first);
         container->setAutoFillBackground(pit.value().second);
         m_palettesBeforeHighlight.erase(pit);
      }

   } else {
      QPalette p = container->palette();

      if (! m_palettesBeforeHighlight.contains(container)) {
         PaletteAndFill paletteAndFill;

         if (container->testAttribute(Qt::WA_SetPalette)) {
            paletteAndFill.first = p;
         }
         paletteAndFill.second = container->autoFillBackground();
         m_palettesBeforeHighlight.insert(container, paletteAndFill);
      }

      p.setColor(backgroundRole(), p.midlight().color());
      container->setPalette(p);
      container->setAutoFillBackground(true);
   }
}

QWidgetList FormWindow::widgets(QWidget *widget) const
{
   const  QObjectList children = widget->children();
   if (children.empty()) {
      return QWidgetList();
   }
   QWidgetList rc;
   const QObjectList::const_iterator cend = children.constEnd();
   for (QObjectList::const_iterator it = children.constBegin(); it != cend; ++it)
      if ((*it)->isWidgetType()) {
         QWidget *w = dynamic_cast<QWidget *>(*it);
         if (isManaged(w)) {
            rc.push_back(w);
         }
      }
   return rc;
}

int FormWindow::toolCount() const
{
   return m_widgetStack->count();
}

QDesignerFormWindowToolInterface *FormWindow::tool(int index) const
{
   return m_widgetStack->tool(index);
}

void FormWindow::registerTool(QDesignerFormWindowToolInterface *tool)
{
   Q_ASSERT(tool != nullptr);

   m_widgetStack->addTool(tool);

   if (m_mainContainer) {
      m_mainContainer->update();
   }
}

void FormWindow::setCurrentTool(int index)
{
   m_widgetStack->setCurrentTool(index);
}

int FormWindow::currentTool() const
{
   return m_widgetStack->currentIndex();
}

bool FormWindow::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
   if (m_widgetStack == nullptr) {
      return false;
   }

   QDesignerFormWindowToolInterface *tool = m_widgetStack->currentTool();

   if (tool == nullptr) {
      return false;
   }

   return tool->handleEvent(widget, managedWidget, event);
}

void FormWindow::initializeCoreTools()
{
   m_widgetEditor = new WidgetEditorTool(this);
   registerTool(m_widgetEditor);
}

void FormWindow::checkSelection()
{
   m_checkSelectionTimer->start(0);
}

void FormWindow::checkSelectionNow()
{
   m_checkSelectionTimer->stop();

   for (QWidget *widget : selectedWidgets()) {
      updateSelection(widget);

      if (LayoutInfo::layoutType(core(), widget) != LayoutInfo::NoLayout) {
         updateChildSelections(widget);
      }
   }
}

QString FormWindow::author() const
{
   return m_author;
}

QString FormWindow::comment() const
{
   return m_comment;
}

void FormWindow::setAuthor(const QString &author)
{
   m_author = author;
}

void FormWindow::setComment(const QString &comment)
{
   m_comment = comment;
}

void FormWindow::editWidgets()
{
   m_widgetEditor->action()->trigger();
}

QStringList FormWindow::resourceFiles() const
{
   return m_resourceFiles;
}

void FormWindow::addResourceFile(const QString &path)
{
   if (!m_resourceFiles.contains(path)) {
      m_resourceFiles.append(path);
      setDirty(true);
      emit resourceFilesChanged();
   }
}

void FormWindow::removeResourceFile(const QString &path)
{
   if (m_resourceFiles.removeAll(path) > 0) {
      setDirty(true);
      emit resourceFilesChanged();
   }
}

bool FormWindow::blockSelectionChanged(bool b)
{
   const bool blocked = m_blockSelectionChanged;
   m_blockSelectionChanged = b;

   return blocked;
}

void FormWindow::editContents()
{
   const QWidgetList sel = selectedWidgets();

   if (sel.count() == 1) {
      QWidget *widget = sel.first();

      if (QAction *a = preferredEditAction(core(), widget)) {
         a->trigger();
      }
   }
}

void FormWindow::dragWidgetWithinForm(QWidget *widget, const QRect &targetGeometry, QWidget *targetContainer)
{
   const bool fromLayout = canDragWidgetInLayout(core(), widget);

   const QDesignerLayoutDecorationExtension *targetDeco =
         qt_extension<QDesignerLayoutDecorationExtension *>(core()->extensionManager(), targetContainer);

   const bool toLayout = targetDeco != nullptr;

   if (fromLayout) {
      // Drag from Layout: We need to delete the widget properly to store the layout state
      // Do not simplify the layout when dragging onto a layout
      // as this might invalidate the insertion position if it is the same layout
      DeleteWidgetCommand *cmd = new DeleteWidgetCommand(this);
      unsigned deleteFlags = DeleteWidgetCommand::DoNotUnmanage;
      if (toLayout) {
         deleteFlags |= DeleteWidgetCommand::DoNotSimplifyLayout;
      }
      cmd->init(widget, deleteFlags);
      commandHistory()->push(cmd);
   }

   if (toLayout) {
      // Drag from form to layout: just insert. Do not manage
      insertWidget(widget, targetGeometry, targetContainer, true);

   } else {
      // into container without layout
      if (targetContainer != widget->parent()) { // different parent
         ReparentWidgetCommand *cmd = new ReparentWidgetCommand(this);
         cmd->init(widget, targetContainer );
         commandHistory()->push(cmd);
      }
      resizeWidget(widget, targetGeometry);
      selectWidget(widget, true);
      widget->show();
   }
}

static Qt::DockWidgetArea detectDropArea(QMainWindow *mainWindow, const QRect &area, const QPoint &drop)
{
   QPoint offset = area.topLeft();

   QRect rect = area;
   rect.moveTopLeft(QPoint(0, 0));

   QPoint point = drop - offset;
   const int x = point.x();
   const int y = point.y();
   const int w = rect.width();
   const int h = rect.height();

   if (rect.contains(point)) {
      bool topRight = false;
      bool topLeft = false;

      if (w * y < h * x) { // top and right, oterwise bottom and left
         topRight = true;
      }
      if (w * y < h * (w - x)) { // top and left, otherwise bottom and right
         topLeft = true;
      }

      if (topRight && topLeft) {
         return Qt::TopDockWidgetArea;
      } else if (topRight && !topLeft) {
         return Qt::RightDockWidgetArea;
      } else if (!topRight && topLeft) {
         return Qt::LeftDockWidgetArea;
      }

      return Qt::BottomDockWidgetArea;
   }

   if (x < 0) {
      if (y < 0) {
         return mainWindow->corner(Qt::TopLeftCorner);
      } else if (y > h) {
         return mainWindow->corner(Qt::BottomLeftCorner);
      } else {
         return Qt::LeftDockWidgetArea;
      }

   } else if (x > w) {
      if (y < 0) {
         return mainWindow->corner(Qt::TopRightCorner);
      } else if (y > h) {
         return mainWindow->corner(Qt::BottomRightCorner);
      } else {
         return Qt::RightDockWidgetArea;
      }

   } else {
      if (y < 0) {
         return Qt::TopDockWidgetArea;
      } else {
         return Qt::BottomDockWidgetArea;
      }
   }

   return Qt::LeftDockWidgetArea;
}

bool FormWindow::dropDockWidget(QDesignerDnDItemInterface *item, const QPoint &global_mouse_pos)
{
   DomUI *dom_ui = item->domUi();

   QMainWindow *mw = dynamic_cast<QMainWindow *>(mainContainer());
   if (! mw) {
      return false;
   }

   QDesignerResource resource(this);
   const FormBuilderClipboard clipboard = resource.paste(dom_ui, mw);
   if (clipboard.m_widgets.size() != 1) { // multiple-paste from DomUI not supported yet
      return false;
   }

   QWidget *centralWidget = mw->centralWidget();
   QPoint localPos = centralWidget->mapFromGlobal(global_mouse_pos);
   const QRect centralWidgetAreaRect = centralWidget->rect();
   Qt::DockWidgetArea area = detectDropArea(mw, centralWidgetAreaRect, localPos);

   beginCommand(tr("Drop widget"));

   clearSelection(false);
   highlightWidget(mw, QPoint(0, 0), FormWindow::Restore);

   QWidget *widget = clipboard.m_widgets.first();

   insertWidget(widget, QRect(0, 0, 1, 1), mw);

   selectWidget(widget, true);
   mw->setFocus(Qt::MouseFocusReason); // in case focus was in e.g. object inspector

   core()->formWindowManager()->setActiveFormWindow(this);
   mainContainer()->activateWindow();

   QDesignerPropertySheetExtension *propertySheet =
      dynamic_cast<QDesignerPropertySheetExtension *>(m_core->extensionManager()->extension(widget,
            CS_TYPEID(QDesignerPropertySheetExtension)));

   if (propertySheet) {
      const QString dockWidgetAreaName = "dockWidgetArea";

      QVariant tmp = propertySheet->property(propertySheet->indexOf(dockWidgetAreaName));
      PropertySheetEnumValue e = tmp.value<PropertySheetEnumValue>();
      e.value = area;

      QVariant v;
      v.setValue(e);
      SetPropertyCommand *cmd = new SetPropertyCommand(this);
      cmd->init(widget, dockWidgetAreaName, v);
      m_undoStack.push(cmd);
   }

   endCommand();

   return true;
}

bool FormWindow::dropWidgets(const QList<QDesignerDnDItemInterface *> &item_list,
         QWidget *target, const QPoint &global_mouse_pos)
{
   QWidget *parent = target;

   if (parent == nullptr) {
      parent = mainContainer();
   }
   // You can only drop stuff onto the central widget of a QMainWindow
   // ### generalize to use container extension
   if (QMainWindow *main_win = dynamic_cast<QMainWindow *>(target)) {

      if (! main_win->centralWidget()) {
         csWarning(tr("QMainWindow based form does not contain a central widget."));
         return false;
      }

      const QPoint main_win_pos = main_win->mapFromGlobal(global_mouse_pos);
      const QRect central_wgt_geo = main_win->centralWidget()->geometry();

      if (!central_wgt_geo.contains(main_win_pos)) {
         return false;
      }
   }

   QWidget *container = findContainer(parent, false);
   if (container == nullptr) {
      return false;
   }

   beginCommand(tr("Drop widget"));

   clearSelection(false);
   highlightWidget(target, target->mapFromGlobal(global_mouse_pos), FormWindow::Restore);

   QPoint offset;
   QDesignerDnDItemInterface *current = nullptr;
   QDesignerFormWindowCursorInterface *c = cursor();

   for (QDesignerDnDItemInterface *item : item_list) {
      QWidget *w = item->widget();

      if (!current) {
         current = item;
      }

      if (c->current() == w) {
         current = item;
         break;
      }
   }

   if (current) {
      QRect geom = current->decoration()->geometry();
      QPoint topLeft = container->mapFromGlobal(geom.topLeft());
      offset = designerGrid().snapPoint(topLeft) - topLeft;
   }

   for (QDesignerDnDItemInterface *item : item_list) {
      DomUI *dom_ui  = item->domUi();
      QRect geometry = item->decoration()->geometry();
      Q_ASSERT(dom_ui != nullptr);

      geometry.moveTopLeft(container->mapFromGlobal(geometry.topLeft()) + offset);

      if (item->type() == QDesignerDnDItemInterface::CopyDrop) {
         // from widget box or CTRL + mouse move
         QWidget *widget = createWidget(dom_ui, geometry, parent);

         if (!widget) {
            endCommand();
            return false;
         }
         selectWidget(widget, true);
         mainContainer()->setFocus(Qt::MouseFocusReason); // in case focus was in e.g. object inspector

      } else { // same form move
         QWidget *widget = item->widget();
         Q_ASSERT(widget != nullptr);

         QDesignerFormWindowInterface *dest = findFormWindow(widget);

         if (dest == this) {
            dragWidgetWithinForm(widget, geometry, container);

         } else {
            // from other form
            FormWindow *source = dynamic_cast<FormWindow *>(item->source());
            Q_ASSERT(source != nullptr);

            source->deleteWidgetList(QWidgetList() << widget);
            QWidget *new_widget = createWidget(dom_ui, geometry, parent);

            selectWidget(new_widget, true);
         }
      }
   }

   core()->formWindowManager()->setActiveFormWindow(this);
   mainContainer()->activateWindow();
   endCommand();
   return true;
}

QDir FormWindow::absoluteDir() const
{
   if (fileName().isEmpty()) {
      return QDir::current();
   }

   return QFileInfo(fileName()).absoluteDir();
}

void FormWindow::layoutDefault(int *margin, int *spacing)
{
   *margin = m_defaultMargin;
   *spacing = m_defaultSpacing;
}

void FormWindow::setLayoutDefault(int margin, int spacing)
{
   m_defaultMargin = margin;
   m_defaultSpacing = spacing;
}

void FormWindow::layoutFunction(QString *margin, QString *spacing)
{
   *margin = m_marginFunction;
   *spacing = m_spacingFunction;
}

void FormWindow::setLayoutFunction(const QString &margin, const QString &spacing)
{
   m_marginFunction = margin;
   m_spacingFunction = spacing;
}

QString FormWindow::pixmapFunction() const
{
   return m_pixmapFunction;
}

void FormWindow::setPixmapFunction(const QString &pixmapFunction)
{
   m_pixmapFunction = pixmapFunction;
}

QStringList FormWindow::includeHints() const
{
   return m_includeHints;
}

void FormWindow::setIncludeHints(const QStringList &includeHints)
{
   m_includeHints = includeHints;
}

QString FormWindow::exportMacro() const
{
   return m_exportMacro;
}

void FormWindow::setExportMacro(const QString &exportMacro)
{
   m_exportMacro = exportMacro;
}

QEditorFormBuilder *FormWindow::createFormBuilder()
{
   return new QDesignerResource(this);
}

QWidget *FormWindow::formContainer() const
{
   return m_widgetStack->formContainer();
}

QUndoStack *FormWindow::commandHistory() const
{
   return const_cast<QDesignerUndoStack &>(m_undoStack).qundoStack();
}

}   // end namespace - qdesigner_internal
