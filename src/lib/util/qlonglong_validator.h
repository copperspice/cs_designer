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

#ifndef QLONGLONG_VALIDATOR_H
#define QLONGLONG_VALIDATOR_H

#include <QValidator>

namespace qdesigner_internal {

class QLongLongValidator : public QValidator
{
   CS_OBJECT(QLongLongValidator)

   CS_PROPERTY_READ(bottom, bottom)
   CS_PROPERTY_WRITE(bottom, setBottom)

   CS_PROPERTY_READ(top, top)
   CS_PROPERTY_WRITE(top, setTop)

 public:
   explicit QLongLongValidator(QObject *parent);
   QLongLongValidator(qint64 bottom, qint64 top, QObject *parent);
   ~QLongLongValidator();

   QValidator::State validate(QString &, int &) const override;

   void setBottom(qint64);
   void setTop(qint64);
   void setRange(qint64 bottom, qint64 top);

   qint64 bottom() const {
      return b;
   }

   qint64 top() const {
      return t;
   }

 private:
   Q_DISABLE_COPY(QLongLongValidator)

   qint64 b;
   qint64 t;
};

class QULongLongValidator : public QValidator
{
   CS_OBJECT(QULongLongValidator)

   CS_PROPERTY_READ(bottom, bottom)
   CS_PROPERTY_WRITE(bottom, setBottom)

   CS_PROPERTY_READ(top, top)
   CS_PROPERTY_WRITE(top, setTop)

 public:
   explicit QULongLongValidator(QObject *parent);
   QULongLongValidator(quint64 bottom, quint64 top, QObject *parent);
   ~QULongLongValidator();

   QValidator::State validate(QString &, int &) const override;

   void setBottom(quint64);
   void setTop(quint64);
   void setRange(quint64 bottom, quint64 top);

   quint64 bottom() const {
      return b;
   }
   quint64 top() const {
      return t;
   }

 private:
   Q_DISABLE_COPY(QULongLongValidator)

   quint64 b;
   quint64 t;
};

}  // end namespace qdesigner_internal

#endif
