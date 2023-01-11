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

#ifndef FORMBUILDER_H
#define FORMBUILDER_H

#include <abstract_formbuilder.h>

class QDesignerCustomWidgetInterface;

class QFormBuilder: public QAbstractFormBuilder
{
 public:
   QFormBuilder();
   virtual ~QFormBuilder();

   QStringList pluginPaths() const;

   void clearPluginPaths();
   void addPluginPath(const QString &pluginPath);
   void setPluginPath(const QStringList &pluginPaths);

   QList<QDesignerCustomWidgetInterface *> customWidgets() const;

 protected:
   QWidget *create(DomUI *ui, QWidget *parentWidget) override;
   QWidget *create(DomWidget *ui_widget, QWidget *parentWidget) override;
   QLayout *create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget) override;
   QLayoutItem *create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget) override;
   QAction *create(DomAction *ui_action, QObject *parent) override;
   QActionGroup *create(DomActionGroup *ui_action_group, QObject *parent) override;

   QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name) override;
   QLayout *createLayout(const QString &layoutName, QObject *parent, const QString &name) override;

   void createConnections(DomConnections *connections, QWidget *widget) override;

   bool addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout) override;
   bool addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget) override;

   virtual void updateCustomWidgets();
   void applyProperties(QObject *o, const QList<DomProperty *> &properties) override;

   static QWidget *widgetByName(QWidget *topLevel, const QString &name);
};

#endif // FORMBUILDER_H
