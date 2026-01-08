/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#ifndef ABSTRACTFORMBUILDER_H
#define ABSTRACTFORMBUILDER_H

#include <QDir>
#include <QHash>
#include <QList>
#include <QPalette>
#include <QScopedPointer>
#include <QSizePolicy>

class DomAction;
class DomActionGroup;
class DomActionRef;
class DomBrush;
class DomButtonGroup;
class DomButtonGroups;
class DomColorGroup;
class DomConnections;
class DomCustomWidgets;
class DomLayout;
class DomLayoutItem;
class DomProperty;
class DomResourcePixmap;
class DomResources;
class DomSpacer;
class DomString;
class DomTabStops;
class DomUI;
class DomWidget;
class QFormBuilderExtra;
class QResourceBuilder;
class QTextBuilder;

class QAbstractButton;
class QAbstractItemView;
class QAction;
class QActionGroup;
class QButtonGroup;
class QComboBox;
class QIODevice;
class QIcon;
class QLayout;
class QLayoutItem;
class QListWidget;
class QObject;
class QSpacerItem;
class QTableWidget;
class QTreeWidget;
class QVariant;
class QWidget;

class QAbstractFormBuilder
{
 public:
   QAbstractFormBuilder();
   virtual ~QAbstractFormBuilder();

   QDir workingDirectory() const;
   void setWorkingDirectory(const QDir &directory);

   virtual QWidget *load(QIODevice *dev, QWidget *parentWidget = nullptr);
   virtual void save(QIODevice *dev, QWidget *widget);

   QString errorString() const;

 protected:
   virtual void loadExtraInfo(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget);

   virtual QWidget *create(DomUI *ui, QWidget *parentWidget);
   virtual QWidget *create(DomWidget *ui_widget, QWidget *parentWidget);
   virtual QLayout *create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget);
   virtual QLayoutItem *create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget);

   virtual QAction *create(DomAction *ui_action, QObject *parent);
   virtual QActionGroup *create(DomActionGroup *ui_action_group, QObject *parent);
   virtual void addMenuAction(QAction *action);

   virtual void applyProperties(QObject *o, const QList<DomProperty *> &properties);
   bool applyPropertyInternally(QObject *o, const QString &propertyName, const QVariant &value);

   virtual void applyTabStops(QWidget *widget, DomTabStops *tabStops);

   virtual QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name);
   virtual QLayout *createLayout(const QString &layoutName, QObject *parent, const QString &name);
   virtual QAction *createAction(QObject *parent, const QString &name);
   virtual QActionGroup *createActionGroup(QObject *parent, const QString &name);

   virtual void createCustomWidgets(DomCustomWidgets *) {}
   virtual void createConnections(DomConnections *, QWidget *) {}
   virtual void createResources(DomResources *) {}

   virtual bool addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout);
   virtual bool addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget);

   virtual void saveExtraInfo(QWidget *widget, DomWidget *ui_widget, DomWidget *ui_parentWidget);

   virtual void saveDom(DomUI *ui, QWidget *widget);

   virtual DomActionRef *createActionRefDom(QAction *action);

   virtual DomWidget *createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive = true);
   virtual DomLayout *createDom(QLayout *layout, DomLayout *ui_layout, DomWidget *ui_parentWidget);
   virtual DomLayoutItem *createDom(QLayoutItem *item, DomLayout *ui_parentLayout, DomWidget *ui_parentWidget);
   virtual DomSpacer *createDom(QSpacerItem *spacer, DomLayout *ui_parentLayout, DomWidget *ui_parentWidget);

   virtual DomAction *createDom(QAction *action);
   virtual DomActionGroup *createDom(QActionGroup *actionGroup);
   DomButtonGroup *createDom(QButtonGroup *buttonGroup);

   virtual DomConnections *saveConnections();
   virtual DomCustomWidgets *saveCustomWidgets();
   virtual DomTabStops *saveTabStops();
   virtual DomResources *saveResources();
   DomButtonGroups *saveButtonGroups(const QWidget *mainContainer);
   virtual QList<DomProperty *> computeProperties(QObject *obj);
   virtual bool checkProperty(QObject *obj, const QString &prop) const;
   virtual DomProperty *createProperty(QObject *object, const QString &propertyName, const QVariant &value);

   virtual void layoutInfo(DomLayout *layout, QObject *parent, int *margin, int *spacing);

   void loadListWidgetExtraInfo(DomWidget *ui_widget, QListWidget *listWidget, QWidget *parentWidget);
   void loadTreeWidgetExtraInfo(DomWidget *ui_widget, QTreeWidget *treeWidget, QWidget *parentWidget);
   void loadTableWidgetExtraInfo(DomWidget *ui_widget, QTableWidget *tableWidget, QWidget *parentWidget);
   void loadComboBoxExtraInfo(DomWidget *ui_widget, QComboBox *comboBox, QWidget *parentWidget);
   void loadButtonExtraInfo(const DomWidget *ui_widget, QAbstractButton *button, QWidget *parentWidget);
   void loadItemViewExtraInfo(DomWidget *ui_widget, QAbstractItemView *itemView, QWidget *parentWidget);

   void saveListWidgetExtraInfo(QListWidget *widget, DomWidget *ui_widget, DomWidget *ui_parentWidget);
   void saveTreeWidgetExtraInfo(QTreeWidget *treeWidget, DomWidget *ui_widget, DomWidget *ui_parentWidget);
   void saveTableWidgetExtraInfo(QTableWidget *tablWidget, DomWidget *ui_widget, DomWidget *ui_parentWidget);
   void saveComboBoxExtraInfo(QComboBox *widget, DomWidget *ui_widget, DomWidget *ui_parentWidget);
   void saveButtonExtraInfo(const QAbstractButton *widget, DomWidget *ui_widget, DomWidget *ui_parentWidget);
   void saveItemViewExtraInfo(const QAbstractItemView *itemView, DomWidget *ui_widget, DomWidget *ui_parentWidget);

   void setResourceBuilder(QResourceBuilder *builder);
   QResourceBuilder *resourceBuilder() const;
   DomProperty *saveResource(const QVariant &v) const;

   void setTextBuilder(QTextBuilder *builder);
   QTextBuilder *textBuilder() const;
   DomProperty *saveText(const QString &attributeName, const QVariant &v) const;

   QVariant toVariant(const QMetaObject &meta, DomProperty *property);
   static QString toString(const DomString *str);

   typedef QHash<QString, DomProperty *> DomPropertyHash;
   static DomPropertyHash propertyMap(const QList<DomProperty *> &properties);

   void setupColorGroup(QPalette &palette, QPalette::ColorGroup colorGroup, DomColorGroup *group);
   DomColorGroup *saveColorGroup(const QPalette &palette);
   QBrush setupBrush(DomBrush *brush);
   DomBrush *saveBrush(const QBrush &brush);

   void reset();
   void initialize(const DomUI *ui);

   static QMetaEnum toolBarAreaMetaEnum();

   // A Pair of icon path/qrc path.
   typedef QPair<QString, QString> IconPaths;

   IconPaths iconPaths(const QIcon &) const;

   void setIconProperty(DomProperty &, const IconPaths &) const;
   void setPixmapProperty(DomProperty &, const IconPaths &) const;
   DomProperty *iconToDomProperty(const QIcon &) const;

   static const DomResourcePixmap *domPixmap(const DomProperty *p);
   QIcon domPropertyToIcon(const DomResourcePixmap *);
   QIcon domPropertyToIcon(const DomProperty *p);

 private:
   static Qt::ToolBarArea toolbarAreaFromDOMAttributes(const DomPropertyHash &attributeMap);

   QAbstractFormBuilder(const QAbstractFormBuilder &other);
   void operator = (const QAbstractFormBuilder &other);

   friend DomProperty *variantToDomProperty(QAbstractFormBuilder *abstractFormBuilder,
      const QMetaObject &meta, const QString &propertyName, const QVariant &value);

   friend QVariant domPropertyToVariant(QAbstractFormBuilder *abstractFormBuilder,
      const QMetaObject &meta, const DomProperty *property);

 protected:
   QScopedPointer<QFormBuilderExtra> d;
};

#endif
