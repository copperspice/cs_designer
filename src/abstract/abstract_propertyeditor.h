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

#ifndef ABSTRACTPROPERTYEDITOR_H
#define ABSTRACTPROPERTYEDITOR_H

class QDesignerFormEditorInterface;

#include <QString>
#include <QWidget>

class QVariant;

class QDesignerPropertyEditorInterface: public QWidget
{
   CS_OBJECT(QDesignerPropertyEditorInterface)

 public:
   explicit QDesignerPropertyEditorInterface(QWidget *parent, Qt::WindowFlags flags = Qt::WindowFlags());
   virtual ~QDesignerPropertyEditorInterface();

   virtual QDesignerFormEditorInterface *core() const;

   virtual bool isReadOnly() const = 0;
   virtual QObject *object() const = 0;

   virtual QString currentPropertyName() const = 0;
   virtual void setPropertyValue(const QString &name, const QVariant &value, bool changed = true) = 0;

   CS_SIGNAL_1(Public, void propertyChanged(const QString &name, const QVariant &value))
   CS_SIGNAL_2(propertyChanged, name, value)

   CS_SLOT_1(Public, virtual void setObject(QObject *object) = 0)
   CS_SLOT_2(setObject)

   CS_SLOT_1(Public, virtual void setReadOnly(bool readOnly) = 0)
   CS_SLOT_2(setReadOnly)
};

#endif
