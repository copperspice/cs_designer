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

#include <abstract_formwindow.h>
#include <inplace_widget_helper.h>

#include <QResizeEvent>
#include <QPushButton>
#include <QToolButton>
#include <QShortcut>

namespace qdesigner_internal {
InPlaceWidgetHelper::InPlaceWidgetHelper(QWidget *editorWidget, QWidget *parentWidget, QDesignerFormWindowInterface *fw)
   : QObject(nullptr), m_editorWidget(editorWidget), m_parentWidget(parentWidget),
     m_noChildEvent(m_parentWidget->testAttribute(Qt::WA_NoChildEventsForParent))
{
   typedef void (QWidget::*QWidgetVoidSlot)();

   m_editorWidget->setAttribute(Qt::WA_DeleteOnClose);
   m_editorWidget->setParent(m_parentWidget->window());
   m_parentWidget->installEventFilter(this);
   m_editorWidget->installEventFilter(this);
   connect(m_editorWidget, &QObject::destroyed,
      fw->mainContainer(), static_cast<QWidgetVoidSlot>(&QWidget::setFocus));
}

InPlaceWidgetHelper::~InPlaceWidgetHelper()
{
   if (m_parentWidget) {
      m_parentWidget->setAttribute(Qt::WA_NoChildEventsForParent, m_noChildEvent);
   }
}

Qt::Alignment InPlaceWidgetHelper::alignment() const
{
   if (m_parentWidget->metaObject()->indexOfProperty("alignment") != -1) {
      return Qt::Alignment(m_parentWidget->property("alignment").toInt());
   }

   if (dynamic_cast<const QPushButton *>(m_parentWidget.data())
         || dynamic_cast<const QToolButton *>(m_parentWidget.data()))  {

      /* tool needs to be more complex */
      return Qt::AlignHCenter;
   }

   return Qt::AlignJustify;
}


bool InPlaceWidgetHelper::eventFilter(QObject *object, QEvent *e)
{
   if (object == m_parentWidget) {
      if (e->type() == QEvent::Resize) {
         const QResizeEvent *event = static_cast<const QResizeEvent *>(e);
         const QPoint localPos = m_parentWidget->geometry().topLeft();
         const QPoint globalPos = m_parentWidget->parentWidget() ? m_parentWidget->parentWidget()->mapToGlobal(localPos) : localPos;
         const QPoint newPos = (m_editorWidget->parentWidget() ? m_editorWidget->parentWidget()->mapFromGlobal(globalPos) : globalPos)
            + m_posOffset;

         const QSize newSize = event->size() + m_sizeOffset;
         m_editorWidget->setGeometry(QRect(newPos, newSize));
      }

   } else if (object == m_editorWidget) {
      if (e->type() == QEvent::ShortcutOverride) {
         if (static_cast<QKeyEvent *>(e)->key() == Qt::Key_Escape) {
            e->accept();
            return false;
         }

      } else if (e->type() == QEvent::KeyPress) {
         if (static_cast<QKeyEvent *>(e)->key() == Qt::Key_Escape) {
            e->accept();
            m_editorWidget->close();
            return true;
         }

      } else if (e->type() == QEvent::Show) {
         const QPoint localPos = m_parentWidget->geometry().topLeft();
         const QPoint globalPos = m_parentWidget->parentWidget() ? m_parentWidget->parentWidget()->mapToGlobal(localPos) : localPos;
         const QPoint newPos = m_editorWidget->parentWidget() ? m_editorWidget->parentWidget()->mapFromGlobal(globalPos) : globalPos;
         m_posOffset = m_editorWidget->geometry().topLeft() - newPos;
         m_sizeOffset = m_editorWidget->size() - m_parentWidget->size();
      }
   }

   return QObject::eventFilter(object, e);
}

}   // end namespace
