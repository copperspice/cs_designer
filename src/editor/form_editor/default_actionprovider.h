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

#ifndef DEFAULT_ACTIONPROVIDER_H
#define DEFAULT_ACTIONPROVIDER_H

#include <action_provider.h>

#include <extensionfactory_p.h>

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>

namespace qdesigner_internal {

class FormWindow;

class ActionProviderBase: public QDesignerActionProviderExtension
{
 public:
   void adjustIndicator(const QPoint &pos) override;
   virtual Qt::Orientation orientation() const = 0;

 protected:
   explicit ActionProviderBase(QWidget *widget);

 protected:
   virtual QRect indicatorGeometry(const QPoint &pos, Qt::LayoutDirection layoutDirection) const;

 private:
   QWidget *m_indicator;
};

class QToolBarActionProvider: public QObject, public ActionProviderBase
{
   CS_OBJECT(QToolBarActionProvider)
   CS_INTERFACES(QDesignerActionProviderExtension)

 public:
   explicit QToolBarActionProvider(QToolBar *widget, QObject *parent = nullptr);

   QRect actionGeometry(QAction *action) const override;
   QAction *actionAt(const QPoint &pos) const override;
   Qt::Orientation orientation() const override;

 protected:
   QRect indicatorGeometry(const QPoint &pos, Qt::LayoutDirection layoutDirection) const override;

 private:
   QToolBar *m_widget;
};

class QMenuBarActionProvider: public QObject, public ActionProviderBase
{
   CS_OBJECT(QMenuBarActionProvider)
   CS_INTERFACES(QDesignerActionProviderExtension)

 public:
   explicit QMenuBarActionProvider(QMenuBar *widget, QObject *parent = nullptr);

   QRect actionGeometry(QAction *action) const override;
   QAction *actionAt(const QPoint &pos) const override;
   Qt::Orientation orientation() const override;

 private:
   QMenuBar *m_widget;
};

class QMenuActionProvider: public QObject, public ActionProviderBase
{
   CS_OBJECT(QMenuActionProvider)
   CS_INTERFACES(QDesignerActionProviderExtension)

 public:
   explicit QMenuActionProvider(QMenu *widget, QObject *parent = nullptr);

   QRect actionGeometry(QAction *action) const override;
   QAction *actionAt(const QPoint &pos) const override;
   Qt::Orientation orientation() const override;

 private:
   QMenu *m_widget;
};

typedef ExtensionFactory<QDesignerActionProviderExtension, QToolBar, QToolBarActionProvider> QToolBarActionProviderFactory;
typedef ExtensionFactory<QDesignerActionProviderExtension, QMenuBar, QMenuBarActionProvider> QMenuBarActionProviderFactory;
typedef ExtensionFactory<QDesignerActionProviderExtension, QMenu, QMenuActionProvider> QMenuActionProviderFactory;

} // namespace qdesigner_internal



#endif // DEFAULT_ACTIONPROVIDER_H
