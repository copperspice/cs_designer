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

#include <textbuilder.h>
#include <ui4.h>

#include <QVariant>

QTextBuilder::QTextBuilder()
{

}

QTextBuilder::~QTextBuilder()
{

}

QVariant QTextBuilder::loadText(const DomProperty *property) const
{
   if (property->kind() == DomProperty::String) {
      return property->elementString()->text();
   }
   return QVariant();
}

QVariant QTextBuilder::toNativeValue(const QVariant &value) const
{
   return value;
}

DomProperty *QTextBuilder::saveText(const QVariant &value) const
{
   Q_UNUSED(value)
   return nullptr;
}