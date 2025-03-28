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

#include <abstract_formeditor.h>
#include <designer_command.h>
#include <designer_property_command.h>
#include <extension.h>
#include <extension_manager.h>
#include <formwindow.h>
#include <formwindowmanager.h>
#include <layout.h>
#include <layout_info.h>
#include <widgetselection.h>

#include <formwindowbase_p.h>
#include <grid_p.h>

#include <QApplication>
#include <QFormLayout>
#include <QGridLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QVariant>
#include <QWidget>

#include <algorithm>

namespace qdesigner_internal {

// Return the layout the widget is in
template <class Layout>
static inline Layout *managedLayoutOf(const QDesignerFormEditorInterface *core, QWidget *w, const Layout * = nullptr)
{
   if (QWidget *p = w->parentWidget()) {
      if (QLayout *newLayout = LayoutInfo::managedLayout(core, p)) {
         return dynamic_cast<Layout *>(newLayout);
      }
   }

   return nullptr;
}

WidgetHandle::WidgetHandle(FormWindow *parent, WidgetHandle::Type t, WidgetSelection *s)
   : InvisibleWidget(parent->formContainer()), m_widget(nullptr),
     m_type(t), m_formWindow( parent), m_sel(s), m_active(true)
{
   setMouseTracking(false);
   setAutoFillBackground(true);

   setBackgroundRole(m_active ? QPalette::Text : QPalette::Dark);
   setFixedSize(6, 6);

   updateCursor();
}

void WidgetHandle::updateCursor()
{
   if (! m_active) {
      setCursor(Qt::ArrowCursor);
      return;
   }

   switch (m_type) {
      case LeftTop:
         setCursor(Qt::SizeFDiagCursor);
         break;

      case Top:
         setCursor(Qt::SizeVerCursor);
         break;

      case RightTop:
         setCursor(Qt::SizeBDiagCursor);
         break;

      case Right:
         setCursor(Qt::SizeHorCursor);
         break;

      case RightBottom:
         setCursor(Qt::SizeFDiagCursor);
         break;

      case Bottom:
         setCursor(Qt::SizeVerCursor);
         break;

      case LeftBottom:
         setCursor(Qt::SizeBDiagCursor);
         break;

      case Left:
         setCursor(Qt::SizeHorCursor);
         break;

      default:
         Q_ASSERT(false);
   }
}

QDesignerFormEditorInterface *WidgetHandle::core() const
{
   if (m_formWindow) {
      return m_formWindow->core();
   }

   return nullptr;
}

void WidgetHandle::setActive(bool a)
{
   m_active = a;
   setBackgroundRole(m_active ? QPalette::Text : QPalette::Dark);
   updateCursor();
}

void WidgetHandle::setWidget(QWidget *w)
{
   m_widget = w;
}

void WidgetHandle::paintEvent(QPaintEvent *)
{
   QDesignerFormWindowManagerInterface *m = m_formWindow->core()->formWindowManager();

   QStylePainter p(this);
   if (m_formWindow->currentWidget() == m_widget) {
      p.setPen(m->activeFormWindow() == m_formWindow ? Qt::blue : Qt::red);
      p.drawRect(0, 0, width() - 1, height() - 1);
   }
}

void WidgetHandle::mousePressEvent(QMouseEvent *e)
{
   e->accept();

   if (!m_formWindow->hasFeature(FormWindow::EditFeature)) {
      return;
   }

   if (!(m_widget && e->button() == Qt::LeftButton)) {
      return;
   }

   if (!(m_active)) {
      return;
   }

   QWidget *container = m_widget->parentWidget();

   m_origPressPos = container->mapFromGlobal(e->globalPos());
   m_geom = m_origGeom = m_widget->geometry();
}

void WidgetHandle::mouseMoveEvent(QMouseEvent *e)
{
   if (!(m_widget && m_active && e->buttons() & Qt::LeftButton)) {
      return;
   }

   e->accept();

   QWidget *container = m_widget->parentWidget();

   const QPoint rp = container->mapFromGlobal(e->globalPos());
   const QPoint d = rp - m_origPressPos;

   const QRect pr = container->rect();

   qdesigner_internal::Grid grid;
   if (const qdesigner_internal::FormWindowBase *fwb = dynamic_cast<const qdesigner_internal::FormWindowBase *>(m_formWindow)) {
      grid = fwb->designerGrid();
   }

   switch (m_type) {

      case LeftTop: {
         if (rp.x() > pr.width() - 2 * width() || rp.y() > pr.height() - 2 * height()) {
            return;
         }

         int w = m_origGeom.width() - d.x();
         m_geom.setWidth(w);
         w = grid.widgetHandleAdjustX(w);

         int h = m_origGeom.height() - d.y();
         m_geom.setHeight(h);
         h = grid.widgetHandleAdjustY(h);

         const int dx = m_widget->width() - w;
         const int dy = m_widget->height() - h;

         trySetGeometry(m_widget, m_widget->x() + dx, m_widget->y() + dy, w, h);
      }
      break;

      case Top: {
         if (rp.y() > pr.height() - 2 * height()) {
            return;
         }

         int h = m_origGeom.height() - d.y();
         m_geom.setHeight(h);
         h = grid.widgetHandleAdjustY(h);

         const int dy = m_widget->height() - h;
         trySetGeometry(m_widget, m_widget->x(), m_widget->y() + dy, m_widget->width(), h);
      }
      break;

      case RightTop: {
         if (rp.x() < 2 * width() || rp.y() > pr.height() - 2 * height()) {
            return;
         }

         int h = m_origGeom.height() - d.y();
         m_geom.setHeight(h);
         h = grid.widgetHandleAdjustY(h);

         const int dy = m_widget->height() - h;

         int w = m_origGeom.width() + d.x();
         m_geom.setWidth(w);
         w = grid.widgetHandleAdjustX(w);

         trySetGeometry(m_widget, m_widget->x(), m_widget->y() + dy, w, h);
      }
      break;

      case Right: {
         if (rp.x() < 2 * width()) {
            return;
         }

         int w = m_origGeom.width() + d.x();
         m_geom.setWidth(w);
         w = grid.widgetHandleAdjustX(w);

         tryResize(m_widget, w, m_widget->height());
      }
      break;

      case RightBottom: {
         if (rp.x() < 2 * width() || rp.y() < 2 * height()) {
            return;
         }

         int w = m_origGeom.width() + d.x();
         m_geom.setWidth(w);
         w = grid.widgetHandleAdjustX(w);

         int h = m_origGeom.height() + d.y();
         m_geom.setHeight(h);
         h = grid.widgetHandleAdjustY(h);

         tryResize(m_widget, w, h);
      }
      break;

      case Bottom: {
         if (rp.y() < 2 * height()) {
            return;
         }

         int h = m_origGeom.height() + d.y();
         m_geom.setHeight(h);
         h = grid.widgetHandleAdjustY(h);

         tryResize(m_widget, m_widget->width(), h);
      }
      break;

      case LeftBottom: {
         if (rp.x() > pr.width() - 2 * width() || rp.y() < 2 * height()) {
            return;
         }

         int w = m_origGeom.width() - d.x();
         m_geom.setWidth(w);
         w = grid.widgetHandleAdjustX(w);

         int h = m_origGeom.height() + d.y();
         m_geom.setHeight(h);
         h = grid.widgetHandleAdjustY(h);

         int dx = m_widget->width() - w;

         trySetGeometry(m_widget, m_widget->x() + dx, m_widget->y(), w, h);
      }
      break;

      case Left: {
         if (rp.x() > pr.width() - 2 * width()) {
            return;
         }

         int w = m_origGeom.width() - d.x();
         m_geom.setWidth(w);
         w = grid.widgetHandleAdjustX(w);

         const int dx = m_widget->width() - w;

         trySetGeometry(m_widget, m_widget->x() + dx, m_widget->y(), w, m_widget->height());
      }
      break;

      default:
         break;

   } // end switch

   m_sel->updateGeometry();

   if (LayoutInfo::layoutType(m_formWindow->core(), m_widget) != LayoutInfo::NoLayout) {
      m_formWindow->updateChildSelections(m_widget);
   }
}

void WidgetHandle::mouseReleaseEvent(QMouseEvent *e)
{
   if (e->button() != Qt::LeftButton || !m_active) {
      return;
   }

   e->accept();

   if (!m_formWindow->hasFeature(FormWindow::EditFeature)) {
      return;
   }

   switch (WidgetSelection::widgetState(m_formWindow->core(), m_widget)) {
      case WidgetSelection::UnlaidOut:
         if (m_geom != m_widget->geometry()) {
            SetPropertyCommand *cmd = new SetPropertyCommand(m_formWindow);
            cmd->init(m_widget, QString("geometry"), m_widget->geometry());
            cmd->setOldValue(m_origGeom);
            m_formWindow->commandHistory()->push(cmd);
            m_formWindow->emitSelectionChanged();
         }
         break;

      case WidgetSelection::LaidOut:
         break;

      case WidgetSelection::ManagedGridLayout:
         changeGridLayoutItemSpan();
         break;

      case WidgetSelection::ManagedFormLayout:
         changeFormLayoutItemSpan();
         break;
   }
}

// Match the left/right widget handle mouse movements to form layout span-changing operations
static inline int formLayoutLeftHandleOperation(int dx, unsigned possibleOperations)
{
   if (dx < 0) {
      if (possibleOperations & ChangeFormLayoutItemRoleCommand::FieldToSpanning) {
         return ChangeFormLayoutItemRoleCommand::FieldToSpanning;
      }

      return 0;
   }

   if (possibleOperations & ChangeFormLayoutItemRoleCommand::SpanningToField) {
      return ChangeFormLayoutItemRoleCommand::SpanningToField;
   }

   return 0;
}

static inline int formLayoutRightHandleOperation(int dx, unsigned possibleOperations)
{
   if (dx < 0) {
      if (possibleOperations & ChangeFormLayoutItemRoleCommand::SpanningToLabel) {
         return ChangeFormLayoutItemRoleCommand::SpanningToLabel;
      }
      return 0;
   }

   if (possibleOperations & ChangeFormLayoutItemRoleCommand::LabelToSpanning) {
      return ChangeFormLayoutItemRoleCommand::LabelToSpanning;
   }

   return 0;
}

// Change form layout item horizontal span
void WidgetHandle::changeFormLayoutItemSpan()
{
   QUndoCommand *cmd = nullptr;
   // Figure out command according to the movement
   const int dx = m_widget->geometry().center().x() - m_origGeom.center().x();

   if (qAbs(dx) >= QApplication::startDragDistance()) {
      int operation = 0;

      if (const unsigned possibleOperations = ChangeFormLayoutItemRoleCommand::possibleOperations(m_formWindow->core(), m_widget)) {
         switch (m_type) {
            case WidgetHandle::Left:
               operation = formLayoutLeftHandleOperation(dx, possibleOperations);
               break;

            case WidgetHandle::Right:
               operation = formLayoutRightHandleOperation(dx, possibleOperations);
               break;

            default:
               break;
         }

         if (operation) {
            ChangeFormLayoutItemRoleCommand *fcmd = new ChangeFormLayoutItemRoleCommand(m_formWindow);
            fcmd->init(m_widget, static_cast<ChangeFormLayoutItemRoleCommand::Operation>(operation));
            cmd = fcmd;
         }
      }
   }

   if (cmd) {
      m_formWindow->commandHistory()->push(cmd);

   } else {
      // Cancelled/Invalid. Restore the size of the widget.
      if (QFormLayout *form = managedLayoutOf<QFormLayout>(m_formWindow->core(), m_widget)) {
         form->invalidate();
         form->activate();
         m_formWindow->clearSelection(false);
         m_formWindow->selectWidget(m_widget);
      }
   }
}

void WidgetHandle::changeGridLayoutItemSpan()
{
   QDesignerLayoutDecorationExtension *deco = qt_extension<QDesignerLayoutDecorationExtension *>(core()->extensionManager(),
         m_widget->parentWidget());

   if (! deco) {
      return;
   }

   QGridLayout *grid = managedLayoutOf<QGridLayout>(m_formWindow->core(), m_widget);
   if (! grid) {
      return;
   }

   const int index  = deco->indexOf(m_widget);
   const QRect info = deco->itemInfo(index);
   const int top    = deco->findItemAt(info.top() - 1, info.left());
   const int left   = deco->findItemAt(info.top(), info.left() - 1);
   const int bottom = deco->findItemAt(info.bottom() + 1, info.left());
   const int right  = deco->findItemAt(info.top(), info.right() + 1);

   const QPoint pt  = m_origGeom.center() - m_widget->geometry().center();

   ChangeLayoutItemGeometry *cmd = nullptr;

   switch (m_type) {
      default:
         break;

      case WidgetHandle::Top: {
         if (pt.y() < 0 && info.height() > 1) {
            cmd = new ChangeLayoutItemGeometry(m_formWindow);
            cmd->init(m_widget, info.y() + 1, info.x(), info.height() - 1, info.width());
         } else if (pt.y() > 0 && top != -1 && grid->itemAt(top)->spacerItem()) {
            cmd = new ChangeLayoutItemGeometry(m_formWindow);
            cmd->init(m_widget, info.y() - 1, info.x(), info.height() + 1, info.width());
         }
      }
      break;

      case WidgetHandle::Left: {
         if (pt.x() < 0 && info.width() > 1) {
            cmd = new ChangeLayoutItemGeometry(m_formWindow);
            cmd->init(m_widget, info.y(), info.x() + 1, info.height(), info.width() - 1);
         } else if (pt.x() > 0 && left != -1 && grid->itemAt(left)->spacerItem()) {
            cmd = new ChangeLayoutItemGeometry(m_formWindow);
            cmd->init(m_widget, info.y(), info.x() - 1, info.height(), info.width() + 1);
         }
      }
      break;

      case WidgetHandle::Right: {
         if (pt.x() > 0 && info.width() > 1) {
            cmd = new ChangeLayoutItemGeometry(m_formWindow);
            cmd->init(m_widget, info.y(), info.x(), info.height(), info.width() - 1);
         } else if (pt.x() < 0 && right != -1 && grid->itemAt(right)->spacerItem()) {
            cmd = new ChangeLayoutItemGeometry(m_formWindow);
            cmd->init(m_widget, info.y(), info.x(), info.height(), info.width() + 1);
         }
      }
      break;

      case WidgetHandle::Bottom: {
         if (pt.y() > 0 && info.width() > 1) {
            cmd = new ChangeLayoutItemGeometry(m_formWindow);
            cmd->init(m_widget, info.y(), info.x(), info.height() - 1, info.width());
         } else if (pt.y() < 0 && bottom != -1 && grid->itemAt(bottom)->spacerItem()) {
            cmd = new ChangeLayoutItemGeometry(m_formWindow);
            cmd->init(m_widget, info.y(), info.x(), info.height() + 1, info.width());
         }
      }
      break;
   }

   if (cmd != nullptr) {
      m_formWindow->commandHistory()->push(cmd);
   } else {
      grid->invalidate();
      grid->activate();
      m_formWindow->clearSelection(false);
      m_formWindow->selectWidget(m_widget);
   }
}

void WidgetHandle::trySetGeometry(QWidget *w, int x, int y, int width, int height)
{
   if (! m_formWindow->hasFeature(FormWindow::EditFeature)) {
      return;
   }

   int minw = w->minimumSize().width();
   minw = qMax(minw, 2 * m_formWindow->grid().x());

   int minh = w->minimumSize().height();
   minh = qMax(minh, 2 * m_formWindow->grid().y());

   if (qMax(minw, width) > w->maximumWidth() ||
      qMax(minh, height) > w->maximumHeight()) {
      return;
   }

   if (width < minw && x != w->x()) {
      x -= minw - width;
   }

   if (height < minh && y != w->y()) {
      y -= minh - height;
   }

   w->setGeometry(x, y, qMax(minw, width), qMax(minh, height));
}

void WidgetHandle::tryResize(QWidget *w, int width, int height)
{
   int minw = w->minimumSize().width();
   minw = qMax(minw, 16);

   int minh = w->minimumSize().height();
   minh = qMax(minh, 16);

   w->resize(qMax(minw, width), qMax(minh, height));
}

WidgetSelection::WidgetState WidgetSelection::widgetState(const QDesignerFormEditorInterface *core, QWidget *w)
{
   bool isManaged;
   const LayoutInfo::Type  lt =  LayoutInfo::laidoutWidgetType(core, w, &isManaged);

 if (lt == LayoutInfo::NoLayout) {
      return UnlaidOut;
   }

   if (! isManaged) {
      return LaidOut;
   }

   switch (lt) {
      case LayoutInfo::Grid:
         return ManagedGridLayout;

      case  LayoutInfo::Form:
         return ManagedFormLayout;

      default:
         break;
   }

   return LaidOut;
}

WidgetSelection::WidgetSelection(FormWindow *parent)
   :  m_widget(nullptr), m_formWindow(parent)
{
   for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
      m_handles[i] = new WidgetHandle(m_formWindow, static_cast<WidgetHandle::Type>(i), this);
   }

   hide();
}

void WidgetSelection::setWidget(QWidget *w)
{
   if (m_widget != nullptr) {
      m_widget->removeEventFilter(this);
   }

   if (w == nullptr) {
      hide();
      m_widget = nullptr;
      return;
   }

   m_widget = w;

   m_widget->installEventFilter(this);

   updateActive();

   updateGeometry();
   show();
}

void WidgetSelection::updateActive()
{
   const WidgetState ws = widgetState(m_formWindow->core(), m_widget);
   bool active[WidgetHandle::TypeCount];
   std::fill(active, active + WidgetHandle::TypeCount, false);

   // Determine active handles
   switch (ws) {
      case UnlaidOut:
         std::fill(active, active + WidgetHandle::TypeCount, true);
         break;

      case ManagedGridLayout: // Grid: Allow changing span
         active[WidgetHandle::Left] = active[WidgetHandle::Top] = active[WidgetHandle::Right] = active[WidgetHandle::Bottom] = true;
         break;

      case ManagedFormLayout:  // Form: Allow changing column span
         if (const unsigned operation = ChangeFormLayoutItemRoleCommand::possibleOperations(m_formWindow->core(), m_widget)) {
            active[WidgetHandle::Left]  = operation & (ChangeFormLayoutItemRoleCommand::SpanningToField |
                  ChangeFormLayoutItemRoleCommand::FieldToSpanning);
            active[WidgetHandle::Right] = operation & (ChangeFormLayoutItemRoleCommand::SpanningToLabel |
                  ChangeFormLayoutItemRoleCommand::LabelToSpanning);
         }
         break;

      default:
         break;
   }

   for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i)
      if (WidgetHandle *h = m_handles[i]) {
         h->setWidget(m_widget);
         h->setActive(active[i]);
      }
}

bool WidgetSelection::isUsed() const
{
   return m_widget != nullptr;
}

void WidgetSelection::updateGeometry()
{
   if (!m_widget || !m_widget->parentWidget()) {
      return;
   }

   QPoint p = m_widget->parentWidget()->mapToGlobal(m_widget->pos());
   p = m_formWindow->formContainer()->mapFromGlobal(p);
   const QRect r(p, m_widget->size());

   const int w = 6;
   const int h = 6;

   for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
      WidgetHandle *hndl = m_handles[ i ];
      if (!hndl) {
         continue;
      }
      switch (i) {
         case WidgetHandle::LeftTop:
            hndl->move(r.x() - w / 2, r.y() - h / 2);
            break;
         case WidgetHandle::Top:
            hndl->move(r.x() + r.width() / 2 - w / 2, r.y() - h / 2);
            break;
         case WidgetHandle::RightTop:
            hndl->move(r.x() + r.width() - w / 2, r.y() - h / 2);
            break;
         case WidgetHandle::Right:
            hndl->move(r.x() + r.width() - w / 2, r.y() + r.height() / 2 - h / 2);
            break;
         case WidgetHandle::RightBottom:
            hndl->move(r.x() + r.width() - w / 2, r.y() + r.height() - h / 2);
            break;
         case WidgetHandle::Bottom:
            hndl->move(r.x() + r.width() / 2 - w / 2, r.y() + r.height() - h / 2);
            break;
         case WidgetHandle::LeftBottom:
            hndl->move(r.x() - w / 2, r.y() + r.height() - h / 2);
            break;
         case WidgetHandle::Left:
            hndl->move(r.x() - w / 2, r.y() + r.height() / 2 - h / 2);
            break;
         default:
            break;
      }
   }
}

void WidgetSelection::hide()
{
   for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
      WidgetHandle *h = m_handles[ i ];
      if (h) {
         h->hide();
      }
   }
}

void WidgetSelection::show()
{
   for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
      WidgetHandle *h = m_handles[ i ];
      if (h) {
         h->show();
         h->raise();
      }
   }
}

void WidgetSelection::update()
{
   for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
      WidgetHandle *h = m_handles[ i ];
      if (h) {
         h->update();
      }
   }
}

QWidget *WidgetSelection::widget() const
{
   return m_widget;
}

QDesignerFormEditorInterface *WidgetSelection::core() const
{
   if (m_formWindow) {
      return m_formWindow->core();
   }

   return nullptr;
}

bool WidgetSelection::eventFilter(QObject *object, QEvent *event)
{
   if (object != widget()) {
      return false;
   }

   switch (event->type()) {
      default:
         break;

      case QEvent::Move:
      case QEvent::Resize:
         updateGeometry();
         break;
      case QEvent::ZOrderChange:
         show();
         break;
   } // end switch

   return false;
}

}


