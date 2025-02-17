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

#include <preview_frame.h>
#include <preview_widget.h>

#include <QCoreApplication>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QPaintEvent>
#include <QPainter>

namespace qdesigner_internal {

class PreviewMdiArea: public QMdiArea
{
 public:
   PreviewMdiArea(QWidget *parent = nullptr)
      : QMdiArea(parent)
   { }

 protected:
   bool viewportEvent(QEvent *event) override;
};

bool PreviewMdiArea::viewportEvent (QEvent *event)
{
   if (event->type() != QEvent::Paint) {
      return QMdiArea::viewportEvent (event);
   }

   QWidget *paintWidget = viewport();
   QPainter p(paintWidget);

   p.fillRect(rect(), paintWidget->palette().color(backgroundRole()).dark());
   p.setPen(QPen(Qt::white));

   // Palette editor background
   p.drawText(0, height() / 2,  width(), height(), Qt::AlignHCenter,
      QCoreApplication::translate("qdesigner_internal::PreviewMdiArea", "The moose in the noose\nate the goose who was loose."));
   return true;
}

PreviewFrame::PreviewFrame(QWidget *parent)
   : QFrame(parent), m_mdiArea(new PreviewMdiArea(this))
{
   m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
   m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
   setFrameStyle(cs_enum_cast(QFrame::StyledPanel) | cs_enum_cast(QFrame::Sunken));
   setLineWidth(1);

   QVBoxLayout *vbox = new QVBoxLayout(this);
   vbox->setMargin(0);
   vbox->addWidget(m_mdiArea);

   setMinimumSize(ensureMdiSubWindow()->minimumSizeHint());
}

void PreviewFrame::setPreviewPalette(const QPalette &pal)
{
   ensureMdiSubWindow()->widget()->setPalette(pal);
}

void PreviewFrame::setSubWindowActive(bool active)
{
   m_mdiArea->setActiveSubWindow (active ? ensureMdiSubWindow() : nullptr);
}

QMdiSubWindow *PreviewFrame::ensureMdiSubWindow()
{
   if (! m_mdiSubWindow) {
      PreviewWidget *previewWidget = new PreviewWidget(m_mdiArea);

      m_mdiSubWindow = m_mdiArea->addSubWindow(previewWidget,
            Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

      m_mdiSubWindow->move(10, 10);
      m_mdiSubWindow->showMaximized();
   }

   const Qt::WindowStates state = m_mdiSubWindow->windowState();

   if (state & Qt::WindowMinimized) {
      m_mdiSubWindow->setWindowState(state & ~Qt::WindowMinimized);
   }

   return m_mdiSubWindow;
}

}   // end namespace
