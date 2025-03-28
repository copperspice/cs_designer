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

#ifndef TEXTBUILDER_H
#define TEXTBUILDER_H

#include <QList>
#include <QString>

class DomProperty;
class DomString;

class QDir;
class QVariant;

class QTextBuilder
{
 public:
   QTextBuilder();
   virtual ~QTextBuilder();

   virtual QVariant loadText(const DomProperty *property) const;

   virtual QVariant toNativeValue(const QVariant &value) const;

   virtual DomProperty *saveText(const QVariant &value) const;
};

#endif