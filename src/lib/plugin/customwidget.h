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

#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <QIcon>
#include <QObject>
#include <QString>

class QDesignerFormEditorInterface;

class QWidget;

class QDesignerCustomWidgetInterface
{
 public:
   virtual ~QDesignerCustomWidgetInterface() {}

   virtual QString name() const = 0;
   virtual QString group() const = 0;
   virtual QString toolTip() const = 0;
   virtual QString whatsThis() const = 0;
   virtual QString includeFile() const = 0;
   virtual QIcon icon() const = 0;

   virtual bool isContainer() const = 0;

   virtual QWidget *createWidget(QWidget *parent) = 0;

   virtual bool isInitialized() const {
      return false;
   }

   virtual void initialize(QDesignerFormEditorInterface *core) {
      (void) core;
   }

   virtual QString domXml() const {
      return QString::fromUtf8("<widget class=\"%1\" name=\"%2\"/>")
         .formatArg(name()).formatArg(name().toLower());
   }

   virtual QString codeTemplate() const {
      return QString();
   }
};

#define QDesignerCustomWidgetInterface_iid "copperspice.com.CS.QDesignerCustomWidgetInterface"
CS_DECLARE_INTERFACE(QDesignerCustomWidgetInterface, QDesignerCustomWidgetInterface_iid)

class QDesignerCustomWidgetCollectionInterface
{
 public:
   virtual ~QDesignerCustomWidgetCollectionInterface() {}

   virtual QList<QDesignerCustomWidgetInterface *> customWidgets() const = 0;
};

#define QDesignerCustomWidgetCollectionInterface_iid "copperspice.com.CS.QDesignerCustomWidgetCollectionInterface"
CS_DECLARE_INTERFACE(QDesignerCustomWidgetCollectionInterface, QDesignerCustomWidgetCollectionInterface_iid)

#endif // CUSTOMWIDGET_H
