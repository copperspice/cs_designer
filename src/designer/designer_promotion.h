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

#ifndef QDESIGNERPROMOTION_H
#define QDESIGNERPROMOTION_H

#include <abstract_promotioninterface.h>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class   QDesignerPromotion : public QDesignerPromotionInterface
{
 public:
   explicit QDesignerPromotion(QDesignerFormEditorInterface *core);

   PromotedClasses promotedClasses() const override;

   QSet<QString> referencedPromotedClassNames() const override;

   virtual bool addPromotedClass(const QString &baseClass,
      const QString &className,
      const QString &includeFile,
      QString *errorMessage) override;

   bool removePromotedClass(const QString &className, QString *errorMessage) override;

   bool changePromotedClassName(const QString &oldclassName, const QString &newClassName, QString *errorMessage) override;

   bool setPromotedClassIncludeFile(const QString &className, const QString &includeFile, QString *errorMessage) override;

   QList<QDesignerWidgetDataBaseItemInterface *> promotionBaseClasses() const override;

 private:
   bool canBePromoted(const QDesignerWidgetDataBaseItemInterface *) const;
   void refreshObjectInspector();

   QDesignerFormEditorInterface *m_core;
};
}



#endif // QDESIGNERPROMOTION_H
