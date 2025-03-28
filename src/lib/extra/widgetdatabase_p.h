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

#ifndef WIDGETDATABASE_H
#define WIDGETDATABASE_H

#include <abstract_widgetdatabase.h>

#include <QIcon>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVariant>

class QDesignerCustomWidgetInterface;

class QObject;

namespace qdesigner_internal {

class WidgetDataBaseItem: public QDesignerWidgetDataBaseItemInterface
{
 public:
   explicit WidgetDataBaseItem(const QString &name = QString(), const QString &group = QString());

   QString name() const override;
   void setName(const QString &name) override;

   QString group() const override;
   void setGroup(const QString &group) override;

   QString toolTip() const  override;
   void setToolTip(const QString &toolTip) override;

   QString whatsThis() const override;
   void setWhatsThis(const QString &whatsThis) override;

   QString includeFile() const override;
   void setIncludeFile(const QString &includeFile) override;

   QIcon icon() const override;
   void setIcon(const QIcon &icon) override;

   bool isCompat() const override;
   void setCompat(bool compat) override;

   bool isContainer() const override;
   void setContainer(bool b) override;

   bool isCustom() const override;
   void setCustom(bool b) override;

   QString pluginPath() const override;
   void setPluginPath(const QString &path) override;

   bool isPromoted() const override;
   void setPromoted(bool b) override;

   QString extends() const override;
   void setExtends(const QString &s) override;

   void setDefaultPropertyValues(const QList<QVariant> &list) override;
   QList<QVariant> defaultPropertyValues() const override;

   static WidgetDataBaseItem *clone(const QDesignerWidgetDataBaseItemInterface *item);

   QStringList fakeSlots() const;
   void setFakeSlots(const QStringList &);

   QStringList fakeSignals() const;
   void setFakeSignals(const QStringList &);

   QString addPageMethod() const;
   void setAddPageMethod(const QString &m);

 private:
   QString m_name;
   QString m_group;
   QString m_toolTip;
   QString m_whatsThis;
   QString m_includeFile;
   QString m_pluginPath;
   QString m_extends;
   QString m_addPageMethod;
   QIcon m_icon;
   uint m_compat: 1;
   uint m_container: 1;
   uint m_custom: 1;
   uint m_promoted: 1;
   QList<QVariant> m_defaultPropertyValues;
   QStringList m_fakeSlots;
   QStringList m_fakeSignals;
};

enum IncludeType { IncludeLocal, IncludeGlobal  };

typedef  QPair<QString, IncludeType> IncludeSpecification;

IncludeSpecification  includeSpecification(QString includeFile);
QString buildIncludeFile(QString includeFile, IncludeType includeType);

class WidgetDataBase: public QDesignerWidgetDataBaseInterface
{
   CS_OBJECT(WidgetDataBase)

 public:
   explicit WidgetDataBase(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
   virtual ~WidgetDataBase();

   QDesignerFormEditorInterface *core() const override;

   int indexOfObject(QObject *o, bool resolveName = true) const override;

   void remove(int index);


   void grabDefaultPropertyValues();
   void grabStandardWidgetBoxIcons();

   // Helpers for 'New Form' wizards in integrations. Obtain a list of suitable classes and generate XML for them.
   static QStringList formWidgetClasses(const QDesignerFormEditorInterface *core);
   static QStringList customFormWidgetClasses(const QDesignerFormEditorInterface *core);
   static QString formTemplate(const QDesignerFormEditorInterface *core, const QString &className, const QString &objectName);

   // Helpers for 'New Form' wizards: Set a fixed size on a XML form template
   static QString scaleFormTemplate(const QString &xml, const QSize &size, bool fixed);

   CS_SLOT_1(Public, void loadPlugins())
   CS_SLOT_2(loadPlugins)

 private:
   QList<QVariant> defaultPropertyValues(const QString &name);
   QDesignerFormEditorInterface *m_core;
};

QDesignerWidgetDataBaseItemInterface *appendDerived(QDesignerWidgetDataBaseInterface *db,
      const QString &className, const QString &group, const QString &baseClassName,
      const QString &includeFile, bool promoted, bool custom);

QList<QDesignerWidgetDataBaseItemInterface *> promotionCandidates(const QDesignerWidgetDataBaseInterface *db,
      const QString &baseClassName);

}   // end namespace qdesigner_internal

#endif
