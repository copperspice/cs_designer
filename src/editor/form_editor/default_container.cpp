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

#include <default_container.h>

#include <QDebug>

template <class Container>
static inline void setCurrentContainerIndex(int index, Container *container)
{
   const bool blocked = container->signalsBlocked();
   container->blockSignals(true);
   container->setCurrentIndex(index);
   container->blockSignals(blocked);
}

static inline void ensureNoParent(QWidget *widget)
{
   if (widget->parentWidget()) {
      widget->setParent(nullptr);
   }
}

static const QString PageLabel = "Page";

namespace qdesigner_internal {

QStackedWidgetContainer::QStackedWidgetContainer(QStackedWidget *widget, QObject *parent) :
   QObject(parent),
   m_widget(widget)
{
}

void QStackedWidgetContainer::setCurrentIndex(int index)
{
   setCurrentContainerIndex(index, m_widget);
}

void QStackedWidgetContainer::addWidget(QWidget *widget)
{
   ensureNoParent(widget);
   m_widget->addWidget(widget);
}

void QStackedWidgetContainer::insertWidget(int index, QWidget *widget)
{
   ensureNoParent(widget);
   m_widget->insertWidget(index, widget);
}

void QStackedWidgetContainer::remove(int index)
{
   m_widget->removeWidget(widget(index));
}

QTabWidgetContainer::QTabWidgetContainer(QTabWidget *widget, QObject *parent) :
   QObject(parent),
   m_widget(widget)
{
}

void QTabWidgetContainer::setCurrentIndex(int index)
{
   setCurrentContainerIndex(index, m_widget);
}

void QTabWidgetContainer::addWidget(QWidget *widget)
{
   ensureNoParent(widget);
   m_widget->addTab(widget, PageLabel);
}

void QTabWidgetContainer::insertWidget(int index, QWidget *widget)
{
   ensureNoParent(widget);
   m_widget->insertTab(index, widget, PageLabel);
}

void QTabWidgetContainer::remove(int index)
{
   m_widget->removeTab(index);
}

QToolBoxContainer::QToolBoxContainer(QToolBox *widget, QObject *parent)
   : QObject(parent), m_widget(widget)
{
}

void QToolBoxContainer::setCurrentIndex(int index)
{
   setCurrentContainerIndex(index, m_widget);
}

void QToolBoxContainer::addWidget(QWidget *widget)
{
   ensureNoParent(widget);
   m_widget->addItem(widget, PageLabel);
}

void QToolBoxContainer::insertWidget(int index, QWidget *widget)
{
   ensureNoParent(widget);
   m_widget->insertItem(index, widget, PageLabel);
}

void QToolBoxContainer::remove(int index)
{
   m_widget->removeItem(index);
}

// ------------------- QScrollAreaContainer
// We pass on active=true only if there are no children yet.
// If there are children, it is a legacy custom widget QScrollArea that has an internal,
// unmanaged child, in which case we deactivate the extension (otherwise we crash).
// The child will then not show up in the task menu

QScrollAreaContainer::QScrollAreaContainer(QScrollArea *widget, QObject *parent) :
   QObject(parent),
   SingleChildContainer<QScrollArea>(widget, widget->widget() == nullptr)
{
}

// ------------------- QDockWidgetContainer
QDockWidgetContainer::QDockWidgetContainer(QDockWidget *widget, QObject *parent)
   : QObject(parent), SingleChildContainer<QDockWidget>(widget)
{
}

}   // end namespace

