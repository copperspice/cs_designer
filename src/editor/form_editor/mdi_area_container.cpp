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
#include <extension.h>
#include <mdi_area_container.h>

#include <QApplication>
#include <QHash>
#include <QMdiArea>
#include <QMdiSubWindow>

namespace qdesigner_internal {

QMdiAreaContainer::QMdiAreaContainer(QMdiArea *widget, QObject *parent)
   : QObject(parent), m_mdiArea(widget)
{
}

int QMdiAreaContainer::count() const
{
   return m_mdiArea->subWindowList(QMdiArea::CreationOrder).count();
}

QWidget *QMdiAreaContainer::widget(int index) const
{
   if (index < 0) {
      return nullptr;
   }

   return m_mdiArea->subWindowList(QMdiArea::CreationOrder).at(index)->widget();
}

int QMdiAreaContainer::currentIndex() const
{
   if (QMdiSubWindow *sub = m_mdiArea->activeSubWindow()) {
      return m_mdiArea->subWindowList(QMdiArea::CreationOrder).indexOf(sub);
   }

   return -1;
}

void QMdiAreaContainer::setCurrentIndex(int index)
{
   if (index < 0) {
      return;
   }

   QMdiSubWindow *frame = m_mdiArea->subWindowList(QMdiArea::CreationOrder).at(index);
   m_mdiArea->setActiveSubWindow(frame);
}

void QMdiAreaContainer::addWidget(QWidget *widget)
{
   QMdiSubWindow *frame = m_mdiArea->addSubWindow(widget, Qt::Window);
   frame->show();
   m_mdiArea->cascadeSubWindows();
   positionNewMdiChild(m_mdiArea, frame);
}

// Semi-smart positioning of new windows: Make child fill the whole MDI window below
// cascaded other windows
void QMdiAreaContainer::positionNewMdiChild(const QWidget *area, QWidget *mdiChild)
{
   constexpr const int MIN_SIZE = 20;

   const QPoint pos     = mdiChild->pos();
   const QSize areaSize = area->size();

   switch (QApplication::layoutDirection()) {
      case Qt::LayoutDirectionAuto:
      case Qt::LeftToRight: {
         const QSize fullSize = QSize(areaSize.width() - pos.x(), areaSize.height() - pos.y());
         if (fullSize.width() > MIN_SIZE && fullSize.height() > MIN_SIZE) {
            mdiChild->resize(fullSize);
         }
      }
      break;

      case Qt::RightToLeft: {
         const QSize fullSize = QSize(pos.x() + mdiChild->width(), areaSize.height() - pos.y());
         if (fullSize.width() > MIN_SIZE && fullSize.height() > MIN_SIZE) {
            mdiChild->move(0, pos.y());
            mdiChild->resize(fullSize);
         }
      }
      break;
   }
}

void QMdiAreaContainer::insertWidget(int, QWidget *widget)
{
   addWidget(widget);
}

void QMdiAreaContainer::remove(int index)
{
   QList<QMdiSubWindow *> subWins = m_mdiArea->subWindowList(QMdiArea::CreationOrder);

   if (index >= 0 && index < subWins.size()) {
      QMdiSubWindow *f = subWins.at(index);
      m_mdiArea->removeSubWindow(f->widget());
      delete f;
   }
}

// ---------- MdiAreaPropertySheet, creates fake properties:
// 1) window name (object name of child)
// 2) title (windowTitle of child).

static const QString subWindowTitleC = "activeSubWindowTitle";
static const QString subWindowNameC  = "activeSubWindowName";

QMdiAreaPropertySheet::QMdiAreaPropertySheet(QWidget *mdiArea, QObject *parent)
   : QDesignerPropertySheet(mdiArea, parent), m_windowTitleProperty("windowTitle")
{
   createFakeProperty(subWindowNameC,  QString());
   createFakeProperty(subWindowTitleC, QString());
}

QMdiAreaPropertySheet::MdiAreaProperty QMdiAreaPropertySheet::mdiAreaProperty(const QString &name)
{
   static QHash<QString, MdiAreaProperty> mdiHash;

   if (mdiHash.empty()) {
      mdiHash.insert(subWindowNameC, MdiAreaSubWindowName);
      mdiHash.insert(subWindowTitleC, MdiAreaSubWindowTitle);
   }

   return mdiHash.value(name, MdiAreaNone);
}

void QMdiAreaPropertySheet::setProperty(int index, const QVariant &value)
{
   switch (mdiAreaProperty(propertyName(index))) {
      case MdiAreaSubWindowName:
         if (QWidget *w = currentWindow()) {
            w->setObjectName(value.toString());
         }
         break;

      case MdiAreaSubWindowTitle:
         // Forward to window title of subwindow

         if (QDesignerPropertySheetExtension *cws = currentWindowSheet()) {
            const int titleIndex = cws->indexOf(m_windowTitleProperty);
            cws->setProperty(titleIndex, value);
            cws->setChanged(titleIndex, true);
         }
         break;

      default:
         QDesignerPropertySheet::setProperty(index, value);
         break;
   }
}

bool QMdiAreaPropertySheet::reset(int index)
{
   bool rc = true;

   switch (mdiAreaProperty(propertyName(index))) {
      case MdiAreaSubWindowName:
         setProperty(index, QVariant(QString()));
         setChanged(index, false);
         break;

      case MdiAreaSubWindowTitle:
         // Forward to window title of subwindow

         if (QDesignerPropertySheetExtension *cws = currentWindowSheet()) {
            const int titleIndex = cws->indexOf(m_windowTitleProperty);
            rc = cws->reset(titleIndex);
         }
         break;

      default:
         rc = QDesignerPropertySheet::reset(index);
         break;
   }

   return rc;
}

QVariant QMdiAreaPropertySheet::property(int index) const
{
   switch (mdiAreaProperty(propertyName(index))) {
      case MdiAreaSubWindowName:
         if (QWidget *w = currentWindow()) {
            return w->objectName();
         }
         return QVariant(QString());

      case MdiAreaSubWindowTitle:
         if (QWidget *w = currentWindow()) {
            return w->windowTitle();
         }
         return QVariant(QString());

      case MdiAreaNone:
         break;
   }

   return QDesignerPropertySheet::property(index);
}

bool QMdiAreaPropertySheet::isEnabled(int index) const
{
   switch (mdiAreaProperty(propertyName(index))) {
      case MdiAreaSubWindowName:
      case MdiAreaSubWindowTitle:
         return currentWindow() != nullptr;

      case MdiAreaNone:
         break;
   }

   return QDesignerPropertySheet::isEnabled(index);
}

bool QMdiAreaPropertySheet::isChanged(int index) const
{
   bool rc = false;
   switch (mdiAreaProperty(propertyName(index))) {
      case MdiAreaSubWindowName:
         rc = currentWindow() != nullptr;
         break;

      case MdiAreaSubWindowTitle:
         if (QDesignerPropertySheetExtension *cws = currentWindowSheet()) {
            const int titleIndex = cws->indexOf(m_windowTitleProperty);
            rc = cws->isChanged(titleIndex);
         }
         break;

      default:
         rc = QDesignerPropertySheet::isChanged(index);
         break;
   }

   return rc;
}

QWidget *QMdiAreaPropertySheet::currentWindow() const
{
   if (const QDesignerContainerExtension *c = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), object())) {
      const int ci = c->currentIndex();

      if (ci < 0) {
         return nullptr;
      }

      return c->widget(ci);
   }

   return nullptr;
}

QDesignerPropertySheetExtension *QMdiAreaPropertySheet::currentWindowSheet() const
{
   QWidget *cw = currentWindow();

   if (cw == nullptr) {
      return nullptr;
   }

   return qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), cw);
}

bool QMdiAreaPropertySheet::checkProperty(const QString &propertyName)
{
   return mdiAreaProperty(propertyName) == MdiAreaNone;
}

}

