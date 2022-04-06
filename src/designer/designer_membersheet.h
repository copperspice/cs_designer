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

#ifndef QDESIGNER_MEMBERSHEET_H
#define QDESIGNER_MEMBERSHEET_H

#include <default_extensionfactory.h>
#include <membersheet.h>

#include <QStringList>

class QDesignerMemberSheetPrivate;

class QDesignerMemberSheet: public QObject, public QDesignerMemberSheetExtension
{
   CS_OBJECT(QDesignerMemberSheet)
   CS_INTERFACES(QDesignerMemberSheetExtension)

 public:
   explicit QDesignerMemberSheet(QObject *object, QObject *parent = nullptr);
   virtual ~QDesignerMemberSheet();

   int indexOf(const QString &name) const override;

   int count() const override;
   QString memberName(int index) const override;

   QString memberGroup(int index) const override;
   void setMemberGroup(int index, const QString &group) override;

   bool isVisible(int index) const override;
   void setVisible(int index, bool b) override;

   bool isSignal(int index) const override;
   bool isSlot(int index) const override;

   bool inheritedFromWidget(int index) const override;

   static bool signalMatchesSlot(const QString &signal, const QString &slot);

   QString declaredInClass(int index) const override;

   QString signature(int index) const override;
   QList<QByteArray> parameterTypes(int index) const override;
   QList<QByteArray> parameterNames(int index) const override;

 private:
   QDesignerMemberSheetPrivate *d;
};

class QDesignerMemberSheetFactory: public QExtensionFactory
{
   CS_OBJECT(QDesignerMemberSheetFactory)
   CS_INTERFACES(QAbstractExtensionFactory)
 public:
   QDesignerMemberSheetFactory(QExtensionManager *parent = nullptr);

 protected:
   QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const override;
};



#endif // QDESIGNER_MEMBERSHEET_H
