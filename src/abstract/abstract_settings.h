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

#ifndef ABSTRACTSETTINGS_P_H
#define ABSTRACTSETTINGS_P_H

#include <QString>
#include <QVariant>

class QDesignerSettingsInterface
{
 public:
   virtual ~QDesignerSettingsInterface() {}

   virtual void beginGroup(const QString &prefix) = 0;
   virtual void endGroup() = 0;

   virtual bool contains(const QString &key) const = 0;
   virtual void setValue(const QString &key, const QVariant &value) = 0;
   virtual QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const = 0;
   virtual void remove(const QString &key) = 0;
};

#endif
