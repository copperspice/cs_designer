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

#ifndef METADATABASE_H
#define METADATABASE_H

#include <abstract_metadatabase.h>

#include <QCursor>
#include <QHash>
#include <QStringList>

namespace qdesigner_internal {

class MetaDataBaseItem: public QDesignerMetaDataBaseItemInterface
{
 public:
   explicit MetaDataBaseItem(QObject *object);
   virtual ~MetaDataBaseItem();

   QString name() const override;
   void setName(const QString &name) override;

   typedef QList<QWidget *> TabOrder;
   TabOrder tabOrder() const override;
   void setTabOrder(const TabOrder &tabOrder) override;

   bool enabled() const override;
   void setEnabled(bool b) override;

   QString customClassName() const;
   void setCustomClassName(const QString &customClassName);

   QStringList fakeSlots() const;
   void setFakeSlots(const QStringList &);

   QStringList fakeSignals() const;
   void setFakeSignals(const QStringList &);

 private:
   QObject *m_object;
   TabOrder m_tabOrder;
   bool m_enabled;
   QString m_customClassName;
   QStringList m_fakeSlots;
   QStringList m_fakeSignals;
};

class MetaDataBase: public QDesignerMetaDataBaseInterface
{
   CS_OBJECT(MetaDataBase)

 public:
   explicit MetaDataBase(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
   virtual ~MetaDataBase();

   QDesignerFormEditorInterface *core() const override;

   QDesignerMetaDataBaseItemInterface *item(QObject *object) const override {
      return metaDataBaseItem(object);
   }

   virtual MetaDataBaseItem *metaDataBaseItem(QObject *object) const;
   void add(QObject *object) override;
   void remove(QObject *object) override;

   QList<QObject *> objects() const override;

 private:
   CS_SLOT_1(Private, void slotDestroyed(QObject *object))
   CS_SLOT_2(slotDestroyed)

   QDesignerFormEditorInterface *m_core;
   QHash<QObject *, MetaDataBaseItem *> m_items;
};

// promotion convenience
bool promoteWidget(QDesignerFormEditorInterface *core, QWidget *widget, const QString &customClassName);
void demoteWidget(QDesignerFormEditorInterface *core, QWidget *widget);
bool isPromoted(QDesignerFormEditorInterface *core, QWidget *w);
QString promotedCustomClassName(QDesignerFormEditorInterface *core, QWidget *w);
QString promotedExtends(QDesignerFormEditorInterface *core, QWidget *w);

} // end namespace qdesigner_internal

#endif
