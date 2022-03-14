/***********************************************************************
*
* Copyright (c) 2021-2022 Barbara Geller
* Copyright (c) 2021-2022 Ansel Sermersheim
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

#ifndef DEFAULT_CONTAINER_H
#define DEFAULT_CONTAINER_H

#include <container.h>
#include <extension.h>
#include <extensionfactory_p.h>

#include <QStackedWidget>
#include <QTabWidget>
#include <QToolBox>
#include <QScrollArea>
#include <QDockWidget>

namespace qdesigner_internal {

class QStackedWidgetContainer: public QObject, public QDesignerContainerExtension
{
   CS_OBJECT(QStackedWidgetContainer)
   CS_INTERFACES(QDesignerContainerExtension)

 public:
   explicit QStackedWidgetContainer(QStackedWidget *widget, QObject *parent = nullptr);

   int count() const override {
      return m_widget->count();
   }

   QWidget *widget(int index) const override {
      return m_widget->widget(index);
   }

   int currentIndex() const override {
      return m_widget->currentIndex();
   }
    void setCurrentIndex(int index) override;

   bool canAddWidget() const override {
      return true;
   }

   bool canRemove(int) const override {
      return true;
   }


   void addWidget(QWidget *widget) override;
   void insertWidget(int index, QWidget *widget) override;
   void remove(int index) override;

 private:
   QStackedWidget *m_widget;
};

class QTabWidgetContainer: public QObject, public QDesignerContainerExtension
{
   CS_OBJECT(QTabWidgetContainer)
   CS_INTERFACES(QDesignerContainerExtension)

 public:
   explicit QTabWidgetContainer(QTabWidget *widget, QObject *parent = nullptr);

   int count() const override {
      return m_widget->count();
   }

   QWidget *widget(int index) const override {
      return m_widget->widget(index);
   }

   int currentIndex() const override {
      return m_widget->currentIndex();
   }
   void setCurrentIndex(int index) override;

   bool canAddWidget() const override {
      return true;
   }

   bool canRemove(int) const override {
      return true;
   }

   void addWidget(QWidget *widget) override;
   void insertWidget(int index, QWidget *widget) override;
   void remove(int index) override;

 private:
   QTabWidget *m_widget;
};

class QToolBoxContainer: public QObject, public QDesignerContainerExtension
{
   CS_OBJECT(QToolBoxContainer)
   CS_INTERFACES(QDesignerContainerExtension)

 public:
   explicit QToolBoxContainer(QToolBox *widget, QObject *parent = nullptr);

   int count() const override {
      return m_widget->count();
   }
   QWidget *widget(int index) const override {
      return m_widget->widget(index);
   }

   int currentIndex() const override {
      return m_widget->currentIndex();
   }
   void setCurrentIndex(int index) override;

   bool canAddWidget() const override {
      return true;
   }

   bool canRemove(int) const override {
      return true;
   }

   void addWidget(QWidget *widget) override;
   void insertWidget(int index, QWidget *widget) override;
   void remove(int index) override;

 private:
   QToolBox *m_widget;
};

// ------------ SingleChildContainer:
//  Template for containers that have a single child widget using widget()/setWidget().

template <class Container>
class SingleChildContainer: public QDesignerContainerExtension
{
 protected:
   explicit SingleChildContainer(Container *widget, bool active = true);

 public:
   int count() const override;
   QWidget *widget(int index) const override;
   int currentIndex() const override;
   void setCurrentIndex(int /*index*/) override {}
   void addWidget(QWidget *widget) override;
   void insertWidget(int index, QWidget *widget) override;
   void remove(int /*index*/) override {}

   bool canAddWidget() const override {
      return false;
   }

   bool canRemove(int) const override {
      return false;
   }

 private:
   const bool m_active;
   Container *m_container;
};

template <class Container> SingleChildContainer<Container>::SingleChildContainer(Container *widget, bool active) :
   m_active(active),
   m_container(widget)
{
}

template <class Container>
int SingleChildContainer<Container>::count() const
{
   return  m_active && m_container->widget() ? 1 : 0;
}

template <class Container>
QWidget *SingleChildContainer<Container>::widget(int /* index */) const
{
   return m_container->widget();
}

template <class Container>
int SingleChildContainer<Container>::currentIndex() const
{
   return m_active && m_container->widget() ? 0 : -1;
}

template <class Container>
void SingleChildContainer<Container>::addWidget(QWidget *widget)
{
   Q_ASSERT(m_container->widget() == 0);
   widget->setParent(m_container);
   m_container->setWidget(widget);
}

template <class Container>
void SingleChildContainer<Container>::insertWidget(int /* index */, QWidget *widget)
{
   addWidget(widget);
}

// ------------  QScrollAreaContainer
class QScrollAreaContainer: public QObject, public SingleChildContainer<QScrollArea>
{
   CS_OBJECT(QScrollAreaContainer)
   CS_INTERFACES(QDesignerContainerExtension)

 public:
   explicit QScrollAreaContainer(QScrollArea *widget, QObject *parent = nullptr);
};

// --------------- QDockWidgetContainer
class QDockWidgetContainer: public QObject, public SingleChildContainer<QDockWidget>
{
   CS_OBJECT(QDockWidgetContainer)
   CS_INTERFACES(QDesignerContainerExtension)

 public:
   explicit QDockWidgetContainer(QDockWidget *widget, QObject *parent = nullptr);
};

typedef ExtensionFactory<QDesignerContainerExtension, QStackedWidget, QStackedWidgetContainer> QDesignerStackedWidgetContainerFactory;
typedef ExtensionFactory<QDesignerContainerExtension, QTabWidget, QTabWidgetContainer> QDesignerTabWidgetContainerFactory;
typedef ExtensionFactory<QDesignerContainerExtension, QToolBox, QToolBoxContainer> QDesignerToolBoxContainerFactory;
typedef ExtensionFactory<QDesignerContainerExtension, QScrollArea, QScrollAreaContainer> QScrollAreaContainerFactory;
typedef ExtensionFactory<QDesignerContainerExtension,  QDockWidget, QDockWidgetContainer> QDockWidgetContainerFactory;

}   // end namespace - qdesigner_internal

#endif
