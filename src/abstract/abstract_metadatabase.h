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

#ifndef ABSTRACTMETADATABASE_H
#define ABSTRACTMETADATABASE_H

#include <QHash>
#include <QList>
#include <QObject>

class QDesignerFormEditorInterface;

class QCursor;
class QWidget;

class QDesignerMetaDataBaseItemInterface
{
 public:
   virtual ~QDesignerMetaDataBaseItemInterface() {}

   virtual QString name() const = 0;
   virtual void setName(const QString &name) = 0;

   virtual QList<QWidget *> tabOrder() const = 0;
   virtual void setTabOrder(const QList<QWidget *> &tabOrder) = 0;

   virtual bool enabled() const = 0;
   virtual void setEnabled(bool b) = 0;
};


class QDesignerMetaDataBaseInterface: public QObject
{
   CS_OBJECT(QDesignerMetaDataBaseInterface)
 public:
   explicit QDesignerMetaDataBaseInterface(QObject *parent = nullptr);
   virtual ~QDesignerMetaDataBaseInterface();

   virtual QDesignerMetaDataBaseItemInterface *item(QObject *object) const = 0;
   virtual void add(QObject *object) = 0;
   virtual void remove(QObject *object) = 0;

   virtual QList<QObject *> objects() const = 0;

   virtual QDesignerFormEditorInterface *core() const = 0;

 public:
   CS_SIGNAL_1(Public, void changed())
   CS_SIGNAL_2(changed)
};

#endif