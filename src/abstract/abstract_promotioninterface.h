/***********************************************************************
*
* Copyright (c) 2021-2022 Barbara Geller
* Copyright (c) 2021-2022 Ansel Sermersheim
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

#ifndef ABSTRACTPROMOTIONINTERFACE_H
#define ABSTRACTPROMOTIONINTERFACE_H

#include <QPair>
#include <QList>
#include <QSet>

class QDesignerWidgetDataBaseItemInterface;

class QDesignerPromotionInterface
{
 public:
   virtual ~QDesignerPromotionInterface();

   struct PromotedClass {
      QDesignerWidgetDataBaseItemInterface *baseItem;
      QDesignerWidgetDataBaseItemInterface *promotedItem;
   };

   typedef QList<PromotedClass> PromotedClasses;

   virtual PromotedClasses promotedClasses() const = 0;

   virtual QSet<QString> referencedPromotedClassNames()  const = 0;

   virtual bool addPromotedClass(const QString &baseClass,
      const QString &className,
      const QString &includeFile,
      QString *errorMessage) = 0;

   virtual bool removePromotedClass(const QString &className, QString *errorMessage) = 0;

   virtual bool changePromotedClassName(const QString &oldClassName, const QString &newClassName, QString *errorMessage) = 0;

   virtual bool setPromotedClassIncludeFile(const QString &className, const QString &includeFile, QString *errorMessage) = 0;

   virtual QList<QDesignerWidgetDataBaseItemInterface *> promotionBaseClasses() const = 0;
};

#endif // ABSTRACTPROMOTIONINTERFACE_H
