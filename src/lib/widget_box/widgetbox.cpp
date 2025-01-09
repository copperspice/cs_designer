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
#include <designer_utils.h>
#include <widgetbox.h>
#include <widgetbox_dnditem.h>
#include <widgetbox_treewidget.h>

#include <formwindowbase_p.h>
#include <iconloader_p.h>

#include <QApplication>
#include <QDropEvent>
#include <QIcon>
#include <QLineEdit>
#include <QToolBar>
#include <QVBoxLayout>

namespace qdesigner_internal {

/* WidgetBoxFilterLineEdit: This widget should never have initial focus
 * (ie, be the first widget of a dialog, else, the hint cannot be displayed.
 * As it is the only focusable control in the widget box, it clears the focus
 * policy and focusses explicitly on click only (note that setting Qt::ClickFocus
 * is not sufficient for that as an ActivationFocus will occur). */

class WidgetBoxFilterLineEdit : public QLineEdit
{
 public:
   explicit WidgetBoxFilterLineEdit(QWidget *parent = nullptr) : QLineEdit(parent), m_defaultFocusPolicy(focusPolicy()) {
      setFocusPolicy(Qt::NoFocus);
   }

 protected:
   void mousePressEvent(QMouseEvent *event) override;
   void focusInEvent(QFocusEvent *e) override;

 private:
   const Qt::FocusPolicy m_defaultFocusPolicy;
};

void WidgetBoxFilterLineEdit::mousePressEvent(QMouseEvent *e)
{
   if (!hasFocus()) { // Explicitly focus on click.
      setFocus(Qt::OtherFocusReason);
   }
   QLineEdit::mousePressEvent(e);
}

void WidgetBoxFilterLineEdit::focusInEvent(QFocusEvent *e)
{
   // Refuse the focus if the mouse it outside. In addition to the mouse
   // press logic, this prevents a re-focussing which occurs once
   // we actually had focus
   const Qt::FocusReason reason = e->reason();
   if (reason == Qt::ActiveWindowFocusReason || reason == Qt::PopupFocusReason) {
      const QPoint mousePos = mapFromGlobal(QCursor::pos());
      const bool refuse = !geometry().contains(mousePos);
      if (refuse) {
         e->ignore();
         return;
      }
   }
   QLineEdit::focusInEvent(e);
}

WidgetBox::WidgetBox(QDesignerFormEditorInterface *core, QWidget *parent, Qt::WindowFlags flags)
   : QDesignerWidgetBox(parent, flags), m_core(core), m_view(new WidgetBoxTreeWidget(m_core))
{
   QVBoxLayout *list = new QVBoxLayout(this);

   list->setMargin(0);
   list->setSpacing(0);

   // Prevent the filter from grabbing focus since our view has Qt::NoFocus
   QToolBar *toolBar = new QToolBar(this);

   QLineEdit *filterWidget = new WidgetBoxFilterLineEdit(toolBar);
   filterWidget->setPlaceholderText(tr("Filter"));
   filterWidget->setClearButtonEnabled(true);

   connect(filterWidget, &QLineEdit::textChanged, m_view, &WidgetBoxTreeWidget::filter);
   toolBar->addWidget(filterWidget);
   list->addWidget(toolBar);

   // View
   connect(m_view, &WidgetBoxTreeWidget::pressed, this, &WidgetBox::handleMousePress);
   list->addWidget(m_view);

   setAcceptDrops (true);
}

WidgetBox::~WidgetBox()
{
}

QDesignerFormEditorInterface *WidgetBox::core() const
{
   return m_core;
}

void WidgetBox::handleMousePress(const QString &name, const QString &xml, const QPoint &global_mouse_pos)
{
   if (QApplication::mouseButtons() != Qt::LeftButton) {
      return;
   }

   DomUI *ui = xmlToUi(name, xml, true);

   if (ui == nullptr) {
      return;
   }

   QList<QDesignerDnDItemInterface *> item_list;

   item_list.append(new WidgetBoxDnDItem(core(), ui, global_mouse_pos));
   m_core->formWindowManager()->dragItems(item_list);
}

int WidgetBox::categoryCount() const
{
   return m_view->categoryCount();
}

QDesignerWidgetBoxInterface::Category WidgetBox::category(int cat_idx) const
{
   return m_view->category(cat_idx);
}

void WidgetBox::addCategory(const Category &cat)
{
   m_view->addCategory(cat);
}

void WidgetBox::removeCategory(int cat_idx)
{
   m_view->removeCategory(cat_idx);
}

int WidgetBox::widgetCount(int cat_idx) const
{
   return m_view->widgetCount(cat_idx);
}

QDesignerWidgetBoxInterface::Widget WidgetBox::widget(int cat_idx, int wgt_idx) const
{
   return m_view->widget(cat_idx, wgt_idx);
}

void WidgetBox::addWidget(int cat_idx, const Widget &wgt)
{
   m_view->addWidget(cat_idx, wgt);
}

void WidgetBox::removeWidget(int cat_idx, int wgt_idx)
{
   m_view->removeWidget(cat_idx, wgt_idx);
}

void WidgetBox::dropWidgets(const QList<QDesignerDnDItemInterface *> &item_list, const QPoint &)
{
   m_view->dropWidgets(item_list);
}

void WidgetBox::setFileName(const QString &file_name)
{
   m_view->setFileName(file_name);
}

QString WidgetBox::fileName() const
{
   return m_view->fileName();
}

bool WidgetBox::load()
{
   return m_view->load(loadMode());
}

bool WidgetBox::loadContents(const QString &contents)
{
   return m_view->loadContents(contents);
}

bool WidgetBox::save()
{
   return m_view->save();
}

static const QDesignerMimeData *checkDragEvent(QDropEvent *event,
   bool acceptEventsFromWidgetBox)
{
   const QDesignerMimeData *mimeData = dynamic_cast<const QDesignerMimeData *>(event->mimeData());
   if (!mimeData) {
      event->ignore();
      return nullptr;
   }
   // If desired, ignore a widget box drag and drop, where widget==0.
   if (!acceptEventsFromWidgetBox) {
      const bool fromWidgetBox = !mimeData->items().first()->widget();
      if (fromWidgetBox) {
         event->ignore();
         return nullptr;
      }
   }

   mimeData->acceptEvent(event);
   return mimeData;
}

void WidgetBox::dragEnterEvent (QDragEnterEvent *event)
{
   // We accept event originating from the widget box also here,
   // because otherwise Windows will not show the DnD pixmap.
   checkDragEvent(event, true);
}

void WidgetBox::dragMoveEvent(QDragMoveEvent *event)
{
   checkDragEvent(event, true);
}

void WidgetBox::dropEvent(QDropEvent *event)
{
   const QDesignerMimeData *mimeData = checkDragEvent(event, false);
   if (!mimeData) {
      return;
   }

   dropWidgets(mimeData->items(), event->pos());
   QDesignerMimeData::removeMovedWidgetsFromSourceForm(mimeData->items());
}

QIcon WidgetBox::iconForWidget(const QString &className, const QString &category) const
{
   Widget widgetData;
   if (!findWidget(this, className, category, &widgetData)) {
      return QIcon();
   }
   return m_view->iconForWidget(widgetData.iconName());
}

}   // end namespace qdesigner_internal


