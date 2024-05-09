/***********************************************************************
*
* Copyright (c) 2021-2024 Barbara Geller
* Copyright (c) 2021-2024 Ansel Sermersheim
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

#ifndef ABSTRACTWIDGETDATABASE_H
#define ABSTRACTWIDGETDATABASE_H

class QDesignerFormEditorInterface;

#include <QList>
#include <QObject>
#include <QString>

class QDebug;
class QIcon;

class QDesignerWidgetDataBaseItemInterface
{
 public:
   virtual ~QDesignerWidgetDataBaseItemInterface() {}

   virtual QString name() const = 0;
   virtual void setName(const QString &name) = 0;

   virtual QString group() const = 0;
   virtual void setGroup(const QString &group) = 0;

   virtual QString toolTip() const = 0;
   virtual void setToolTip(const QString &toolTip) = 0;

   virtual QString whatsThis() const = 0;
   virtual void setWhatsThis(const QString &whatsThis) = 0;

   virtual QString includeFile() const = 0;
   virtual void setIncludeFile(const QString &includeFile) = 0;

   virtual QIcon icon() const = 0;
   virtual void setIcon(const QIcon &icon) = 0;

   virtual bool isCompat() const = 0;
   virtual void setCompat(bool compat) = 0;

   virtual bool isContainer() const = 0;
   virtual void setContainer(bool container) = 0;

   virtual bool isCustom() const = 0;
   virtual void setCustom(bool custom) = 0;

   virtual QString pluginPath() const = 0;
   virtual void setPluginPath(const QString &path) = 0;

   virtual bool isPromoted() const = 0;
   virtual void setPromoted(bool b) = 0;

   virtual QString extends() const = 0;
   virtual void setExtends(const QString &s) = 0;

   virtual void setDefaultPropertyValues(const QList<QVariant> &list) = 0;
   virtual QList<QVariant> defaultPropertyValues() const = 0;
};

class QDesignerWidgetDataBaseInterface: public QObject
{
   CS_OBJECT(QDesignerWidgetDataBaseInterface)
 public:
   explicit QDesignerWidgetDataBaseInterface(QObject *parent = nullptr);
   virtual ~QDesignerWidgetDataBaseInterface();

   virtual int count() const;
   virtual QDesignerWidgetDataBaseItemInterface *item(int index) const;

   virtual int indexOf(QDesignerWidgetDataBaseItemInterface *item) const;
   virtual void insert(int index, QDesignerWidgetDataBaseItemInterface *item);
   virtual void append(QDesignerWidgetDataBaseItemInterface *item);

   virtual int indexOfObject(QObject *object, bool resolveName = true) const;
   virtual int indexOfClassName(const QString &className, bool resolveName = true) const;

   virtual QDesignerFormEditorInterface *core() const;

   bool isContainer(QObject *object, bool resolveName = true) const;
   bool isCustom(QObject *object, bool resolveName = true) const;

 public:
   CS_SIGNAL_1(Public, void changed())
   CS_SIGNAL_2(changed)

 protected:
   QList<QDesignerWidgetDataBaseItemInterface *> m_items;
};

#endif
