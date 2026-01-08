/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#ifndef MAINWINDOW_CONTAINER_H
#define MAINWINDOW_CONTAINER_H

#include <container.h>

#include <extensionfactory_p.h>

#include <QMainWindow>

namespace qdesigner_internal {

class QMainWindowContainer: public QObject, public QDesignerContainerExtension
{
   CS_OBJECT(QMainWindowContainer)

   CS_INTERFACES(QDesignerContainerExtension)

 public:
   explicit QMainWindowContainer(QMainWindow *widget, QObject *parent = nullptr);

   bool canAddWidget() const override {
      return true;
   }

   bool canRemove(int) const override {
      return true;
   }

   int count() const override;
   QWidget *widget(int index) const override;
   int currentIndex() const override;
   void setCurrentIndex(int index) override;
   void addWidget(QWidget *widget) override;
   void insertWidget(int index, QWidget *widget) override;
   void remove(int index) override;

 private:
   QMainWindow *m_mainWindow;
   QList<QWidget *> m_widgets;
};

typedef ExtensionFactory<QDesignerContainerExtension, QMainWindow, QMainWindowContainer> QMainWindowContainerFactory;

}  // namespace qdesigner_internal

#endif
