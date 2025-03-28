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

#include <designer_toolbar.h>
#include <formwindow.h>
#include <mainwindow_container.h>

#include <QDockWidget>
#include <QLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>

using namespace qdesigner_internal;

QMainWindowContainer::QMainWindowContainer(QMainWindow *widget, QObject *parent)
   : QObject(parent), m_mainWindow(widget)
{
}

int QMainWindowContainer::count() const
{
   return m_widgets.count();
}

QWidget *QMainWindowContainer::widget(int index) const
{
   if (index == -1) {
      return nullptr;
   }

   return m_widgets.at(index);
}

int QMainWindowContainer::currentIndex() const
{
   return m_mainWindow->centralWidget() ? 0 : -1;
}

void QMainWindowContainer::setCurrentIndex(int index)
{
   (void) index;
}

namespace {

using ToolBarData = QPair<Qt::ToolBarArea, bool>;

ToolBarData toolBarData(QToolBar *toolBar)
{
   const QMainWindow *mainWindow = dynamic_cast<const QMainWindow *>(toolBar->parentWidget());

   if (! mainWindow || ! mainWindow->layout() || mainWindow->layout()->indexOf(toolBar) == -1) {
      return ToolBarData(Qt::TopToolBarArea, false);
   }

   return ToolBarData(mainWindow->toolBarArea(toolBar), mainWindow->toolBarBreak(toolBar));
}

Qt::DockWidgetArea dockWidgetArea(QDockWidget *dockWidget)
{
   if (const QMainWindow *mainWindow = dynamic_cast<const QMainWindow *>(dockWidget->parentWidget())) {
      // Make sure the variable dockWidget is managed by mainWindow, otherwise
      // QMainWindow::dockWidgetArea() will not work

      QList<QLayout *> candidates;

      if (mainWindow->layout()) {
         candidates.append(mainWindow->layout());
         candidates += mainWindow->layout()->findChildren<QLayout *>();
      }

      for (QLayout *layout : candidates) {
         if (layout->indexOf(dockWidget) != -1) {
            return mainWindow->dockWidgetArea(dockWidget);
         }
      }
   }

   return Qt::LeftDockWidgetArea;
}

}   // end namespace

void QMainWindowContainer::addWidget(QWidget *widget)
{
   // remove all the occurrences of widget
   m_widgets.removeAll(widget);

   if (QToolBar *toolBar = dynamic_cast<QToolBar *>(widget)) {
      m_widgets.append(widget);
      const ToolBarData data = toolBarData(toolBar);
      m_mainWindow->addToolBar(data.first, toolBar);

      if (data.second) {
         m_mainWindow->insertToolBarBreak(toolBar);
      }

      toolBar->show();
   }

   else if (QMenuBar *menuBar = dynamic_cast<QMenuBar *>(widget)) {
      if (menuBar != m_mainWindow->menuBar()) {
         m_mainWindow->setMenuBar(menuBar);
      }

      m_widgets.append(widget);
      menuBar->show();
   }

   else if (QStatusBar *statusBar = dynamic_cast<QStatusBar *>(widget)) {
      if (statusBar != m_mainWindow->statusBar()) {
         m_mainWindow->setStatusBar(statusBar);
      }

      m_widgets.append(widget);
      statusBar->show();
   }

   else if (QDockWidget *dockWidget = dynamic_cast<QDockWidget *>(widget)) {
      m_widgets.append(widget);
      m_mainWindow->addDockWidget(dockWidgetArea(dockWidget), dockWidget);
      dockWidget->show();

      if (FormWindow *fw = FormWindow::findFormWindow(m_mainWindow)) {
         fw->manageWidget(widget);
      }
   }

   else if (widget) {
      m_widgets.prepend(widget);

      if (widget != m_mainWindow->centralWidget()) {
         // note that qmainwindow will delete the current central widget if you
         // call setCentralWidget(), we end up with dangeling pointers in m_widgets list
         m_widgets.removeAll(m_mainWindow->centralWidget());

         widget->setParent(m_mainWindow);
         m_mainWindow->setCentralWidget(widget);
      }
   }
}

void QMainWindowContainer::insertWidget(int index, QWidget *widget)
{
   (void) index;

   addWidget(widget);
}

void QMainWindowContainer::remove(int index)
{
   QWidget *widget = m_widgets.at(index);

   if (QToolBar *toolBar = dynamic_cast<QToolBar *>(widget)) {
      m_mainWindow->removeToolBar(toolBar);

   } else if (QMenuBar *menuBar = dynamic_cast<QMenuBar *>(widget)) {
      menuBar->hide();
      menuBar->setParent(nullptr);
      m_mainWindow->setMenuBar(nullptr);

   } else if (QStatusBar *statusBar = dynamic_cast<QStatusBar *>(widget)) {
      statusBar->hide();
      statusBar->setParent(nullptr);
      m_mainWindow->setStatusBar(nullptr);

   } else if (QDockWidget *dockWidget = dynamic_cast<QDockWidget *>(widget)) {
      m_mainWindow->removeDockWidget(dockWidget);
   }

   m_widgets.removeAt(index);
}
