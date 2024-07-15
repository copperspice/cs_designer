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

#include <abstract_formwindow.h>
#include <designer_widget.h>

#include <formwindowbase_p.h>
#include <grid_p.h>

#include <QEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

/* QDesignerDialog / QDesignerWidget are used to paint a grid on QDialog and QWidget main containers
 * and container extension pages.
 * The paint routines work as follows:
 * We need to clean the background here (to make the parent grid disappear in case we are a container page
 * and to make palette background settings take effect),
 * which would normally break style sheets with background settings.
 * So, we manually make the style paint after cleaning. On top comes the grid
 * In addition, this code works around
 * the QStyleSheetStyle setting Qt::WA_StyledBackground to false for subclasses of QWidget.
 */
QDesignerDialog::QDesignerDialog(QDesignerFormWindowInterface *fw, QWidget *parent) :
   QDialog(parent),
   m_formWindow(dynamic_cast<qdesigner_internal::FormWindowBase*>(fw))
{
}

void QDesignerDialog::paintEvent(QPaintEvent *e)
{
   QPainter p(this);

   QStyleOption opt;
   opt.initFrom(this);

   p.fillRect(e->rect(), palette().brush(backgroundRole()));
   style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

   if (m_formWindow && m_formWindow->gridVisible()) {
      m_formWindow->designerGrid().paint(p, this, e);
   }
}

QDesignerWidget::QDesignerWidget(QDesignerFormWindowInterface *formWindow, QWidget *parent)  :
   QWidget(parent),
   m_formWindow(dynamic_cast<qdesigner_internal::FormWindowBase*>(formWindow))
{
}

QDesignerWidget::~QDesignerWidget()
{
}

QDesignerFormWindowInterface *QDesignerWidget::formWindow() const
{
   return m_formWindow;
}

void QDesignerWidget::paintEvent(QPaintEvent *e)
{
   QPainter p(this);

   QStyleOption opt;
   opt.initFrom(this);

   p.fillRect(e->rect(), palette().brush(backgroundRole()));
   style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

   if (m_formWindow && m_formWindow->gridVisible()) {
      m_formWindow->designerGrid().paint(p, this, e);
   }
}
