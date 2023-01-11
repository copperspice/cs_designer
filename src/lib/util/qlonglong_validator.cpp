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

#include <qlonglong_validator.h>

using namespace qdesigner_internal;

//
QLongLongValidator::QLongLongValidator(QObject *parent)
   : QValidator(parent), b(Q_UINT64_C(0x8000000000000000)), t(Q_UINT64_C(0x7FFFFFFFFFFFFFFF))
{
}

QLongLongValidator::QLongLongValidator(qint64 minimum, qint64 maximum, QObject *parent)
   : QValidator(parent), b(minimum), t(maximum)
{
}

QLongLongValidator::~QLongLongValidator()
{
   // nothing
}

QValidator::State QLongLongValidator::validate(QString &input, int &) const
{
   if (input.contains(' ')) {
      return State::Invalid;
   }

   if (input.isEmpty() || (b < 0 && input == "-")) {
      return State::Intermediate;
   }

   bool ok;
   qint64 entered = input.toInteger<qint64>(&ok);

   if (!ok || (entered < 0 && b >= 0)) {
      return State::Invalid;

   } else if (entered >= b && entered <= t) {
      return State::Acceptable;

   } else {
      if (entered >= 0) {
         return (entered > t) ? State::Invalid : State::Intermediate;
      } else {
         return (entered < b) ? State::Invalid : State::Intermediate;
      }
   }
}

void QLongLongValidator::setRange(qint64 bottom, qint64 top)
{
   b = bottom;
   t = top;
}

void QLongLongValidator::setBottom(qint64 bottom)
{
   setRange(bottom, top());
}

void QLongLongValidator::setTop(qint64 top)
{
   setRange(bottom(), top);
}

//
QULongLongValidator::QULongLongValidator(QObject *parent)
   : QValidator(parent), b(0), t(Q_UINT64_C(0xFFFFFFFFFFFFFFFF))
{
}

QULongLongValidator::QULongLongValidator(quint64 minimum, quint64 maximum, QObject *parent)
   : QValidator(parent), b(minimum), t(maximum)
{
}

QULongLongValidator::~QULongLongValidator()
{
   // nothing
}

QValidator::State QULongLongValidator::validate(QString &input, int &) const
{
   if (input.isEmpty()) {
      return State::Intermediate;
   }

   bool ok;
   quint64 entered = input.toInteger<quint64>(&ok);

   if (input.contains(' ') || input.contains('-') || ! ok) {
      return State::Invalid;
   }

   if (entered >= b && entered <= t) {
      return State::Acceptable;
   }

   return State::Invalid;
}

void QULongLongValidator::setRange(quint64 bottom, quint64 top)
{
   b = bottom;
   t = top;
}

void QULongLongValidator::setBottom(quint64 bottom)
{
   setRange(bottom, top());
}

void QULongLongValidator::setTop(quint64 top)
{
   setRange(bottom(), top);
}

