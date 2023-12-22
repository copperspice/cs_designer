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

#include <customwidget.h>
#include <formbuilder.h>
#include <table_classes.h>
#include <formbuilderextra.h>
#include <textbuilder.h>
#include <ui_loader.h>
#include <ui4.h>

#include <ui_loader_p.h>

#include <QDebug>
#include <QDataStream>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDir>
#include <QFontComboBox>
#include <QLibraryInfo>
#include <QLayout>
#include <QWidget>
#include <QMap>

typedef QMap<QString, bool> widget_map;
Q_GLOBAL_STATIC(widget_map, g_widgets)

class QUiLoader;
class QUiLoaderPrivate;

QDataStream &operator<<(QDataStream &out, const QUiTranslatableStringValue &s)
{
   out << s.comment() << s.value();
   return out;
}

QDataStream &operator>>(QDataStream &in, QUiTranslatableStringValue &s)
{
   in >> s.m_comment >> s.m_value;
   return in;
}

class TranslatingTextBuilder : public QTextBuilder
{
 public:
   TranslatingTextBuilder(bool trEnabled, const QString &className)
      : m_trEnabled(trEnabled), m_className(className)
   {
   }

   QVariant loadText(const DomProperty *icon) const override;

   QVariant toNativeValue(const QVariant &value) const override;

 private:
   bool m_trEnabled;
   QString m_className;
};

QVariant TranslatingTextBuilder::loadText(const DomProperty *text) const
{
   const DomString *str = text->elementString();

   if (! str) {
      return QVariant();
   }

   if (str->hasAttributeNotr()) {
      const QString notr = str->attributeNotr();
      if (notr == "true" || notr == "yes") {
         return QVariant::fromValue(str->text());
      }
   }

   QUiTranslatableStringValue strVal;

   strVal.setValue(str->text().toUtf8());
   if (str->hasAttributeComment()) {
      strVal.setComment(str->attributeComment().toUtf8());
   }

   return QVariant::fromValue(strVal);
}

QVariant TranslatingTextBuilder::toNativeValue(const QVariant &value) const
{
   if (value.canConvert<QUiTranslatableStringValue>()) {
      QUiTranslatableStringValue tsv = value.value<QUiTranslatableStringValue>();

      if (! m_trEnabled) {
         return QString::fromUtf8(tsv.value().data());
      }

      return QVariant::fromValue(QCoreApplication::translate(m_className, tsv.value(), tsv.comment()));
   }

   if (value.canConvert<QString>()) {
      return QVariant::fromValue(value.value<QString>());
   }

   return value;
}

// "exported" to linguist

const QUiItemRolePair qUiItemRoles[] = {
   { Qt::DisplayRole,   Qt::DisplayPropertyRole },
   { Qt::ToolTipRole,   Qt::ToolTipPropertyRole },
   { Qt::StatusTipRole, Qt::StatusTipPropertyRole },
   { Qt::WhatsThisRole, Qt::WhatsThisPropertyRole },
   { -1, -1 }
};

static void recursiveReTranslate(QTreeWidgetItem *item, const QString &class_name)
{
   const QUiItemRolePair *irs = qUiItemRoles;

   int cnt = item->columnCount();

   for (int i = 0; i < cnt; ++i) {
      for (unsigned j = 0; irs[j].shadowRole >= 0; ++j) {
         QVariant v = item->data(i, irs[j].shadowRole);

         if (v.isValid()) {
            QUiTranslatableStringValue tsv = v.value<QUiTranslatableStringValue>();
            const QString text = QCoreApplication::translate(class_name, tsv.value(), tsv.comment());
            item->setData(i, irs[j].realRole, text);
         }
      }
   }

   cnt = item->childCount();
   for (int i = 0; i < cnt; ++i) {
      recursiveReTranslate(item->child(i), class_name);
   }
}

template<typename T>
static void reTranslateWidgetItem(T *item, const QString &class_name)
{
   const QUiItemRolePair *irs = qUiItemRoles;

   for (unsigned j = 0; irs[j].shadowRole >= 0; j++) {
      QVariant v = item->data(irs[j].shadowRole);

      if (v.isValid()) {
         QUiTranslatableStringValue tsv = v.value<QUiTranslatableStringValue>();
         const QString text = QCoreApplication::translate(class_name, tsv.value(), tsv.comment());
         item->setData(irs[j].realRole, text);
      }
   }
}

static void reTranslateTableItem(QTableWidgetItem *item, const QString &class_name)
{
   if (item) {
      reTranslateWidgetItem(item, class_name);
   }
}

class TranslationWatcher: public QObject
{
   CS_OBJECT(TranslationWatcher)

 public:
   TranslationWatcher(QObject *parent, const QString &className)
      : QObject(parent), m_className(className) {
   }

   bool eventFilter(QObject *o, QEvent *event) override {
      if (event->type() == QEvent::LanguageChange) {
         for (const QString &prop : o->dynamicPropertyNames()) {

            if (prop.startsWith(PROP_GENERIC_PREFIX)) {
               const QString propName = prop.mid(sizeof(PROP_GENERIC_PREFIX) - 1);

               QVariant data = o->property(prop);
               const QUiTranslatableStringValue tsv = data.value<QUiTranslatableStringValue>();

               const QString text = QCoreApplication::translate(m_className, tsv.value(), tsv.comment());
               o->setProperty(propName, text);
            }
         }

         if (QTabWidget *tabw = dynamic_cast<QTabWidget *>(o)) {
            const int cnt = tabw->count();

            for (int i = 0; i < cnt; ++i) {
               QVariant v = tabw->widget(i)->property(PROP_TABPAGETEXT);

               if (v.isValid()) {
                  QUiTranslatableStringValue tsv = v.value<QUiTranslatableStringValue>();
                  const QString text = QCoreApplication::translate(m_className, tsv.value(), tsv.comment());
                  tabw->setTabText(i, text);
               }

               //
               v = tabw->widget(i)->property(PROP_TABPAGETOOLTIP);

               if (v.isValid()) {
                  QUiTranslatableStringValue tsv = v.value<QUiTranslatableStringValue>();
                  const QString text = QCoreApplication::translate(m_className, tsv.value(), tsv.comment());
                  tabw->setTabToolTip(i, text);
               }

               //
               v = tabw->widget(i)->property(PROP_TABPAGEWHATSTHIS);

               if (v.isValid()) {
                  QUiTranslatableStringValue tsv = v.value<QUiTranslatableStringValue>();
                  const QString text = QCoreApplication::translate(m_className, tsv.value(), tsv.comment());
                  tabw->setTabWhatsThis(i, text);
               }
            }

         } else if (QListWidget *listw = dynamic_cast<QListWidget *>(o)) {
            const int cnt = listw->count();
            for (int i = 0; i < cnt; ++i) {
               reTranslateWidgetItem(listw->item(i), m_className);
            }


         } else if (QTreeWidget *treew = dynamic_cast<QTreeWidget *>(o)) {
            if (QTreeWidgetItem *item = treew->headerItem()) {
               recursiveReTranslate(item, m_className);
            }

            const int cnt = treew->topLevelItemCount();
            for (int i = 0; i < cnt; ++i) {
               QTreeWidgetItem *item = treew->topLevelItem(i);
               recursiveReTranslate(item, m_className);
            }


         } else if (QTableWidget *tablew = dynamic_cast<QTableWidget *>(o)) {
            const int row_cnt = tablew->rowCount();
            const int col_cnt = tablew->columnCount();

            for (int j = 0; j < col_cnt; ++j) {
               reTranslateTableItem(tablew->horizontalHeaderItem(j), m_className);
            }

            for (int i = 0; i < row_cnt; ++i) {
               reTranslateTableItem(tablew->verticalHeaderItem(i), m_className);
               for (int j = 0; j < col_cnt; ++j) {
                  reTranslateTableItem(tablew->item(i, j), m_className);
               }
            }

         } else if (QComboBox *combow = dynamic_cast<QComboBox *>(o)) {
            if (! dynamic_cast<QFontComboBox *>(o)) {
               const int cnt = combow->count();

               for (int i = 0; i < cnt; ++i) {
                  const QVariant v = combow->itemData(i, Qt::DisplayPropertyRole);

                  if (v.isValid()) {
                     QUiTranslatableStringValue tsv = v.value<QUiTranslatableStringValue>();
                     const QString text = QCoreApplication::translate(m_className, tsv.value(), tsv.comment());
                     combow->setItemText(i, text);
                  }
               }
            }

         } else if (QToolBox *toolw = dynamic_cast<QToolBox *>(o)) {
            const int cnt = toolw->count();

            for (int i = 0; i < cnt; ++i) {
               //
               QVariant v = toolw->widget(i)->property(PROP_TOOLITEMTEXT);

               if (v.isValid()) {
                  QUiTranslatableStringValue tsv = v.value<QUiTranslatableStringValue>();
                  const QString text = QCoreApplication::translate(m_className, tsv.value(), tsv.comment());
                  toolw->setItemText(i, text);
               }

               //
               v = toolw->widget(i)->property(PROP_TOOLITEMTOOLTIP);

               if (v.isValid()) {
                  QUiTranslatableStringValue tsv = v.value<QUiTranslatableStringValue>();
                  const QString text = QCoreApplication::translate(m_className, tsv.value(), tsv.comment());
                  toolw->setItemToolTip(i, text);
               }
            }
         }
      }

      return false;
   }

 private:
   QString m_className;
};

class FormBuilderPrivate: public QFormBuilder
{
   friend class QT_PREPEND_NAMESPACE(QUiLoader);
   friend class QT_PREPEND_NAMESPACE(QUiLoaderPrivate);
   typedef QFormBuilder ParentClass;

 public:
   QUiLoader *loader;

   bool dynamicTr;
   bool trEnabled;

   FormBuilderPrivate()
      : loader(nullptr), dynamicTr(false), trEnabled(true), m_trwatch(nullptr)
   {
   }

   QWidget *defaultCreateWidget(const QString &className, QWidget *parent, const QString &name) {
      return ParentClass::createWidget(className, parent, name);
   }

   QLayout *defaultCreateLayout(const QString &className, QObject *parent, const QString &name) {
      return ParentClass::createLayout(className, parent, name);
   }

   QAction *defaultCreateAction(QObject *parent, const QString &name) {
      return ParentClass::createAction(parent, name);
   }

   QActionGroup *defaultCreateActionGroup(QObject *parent, const QString &name) {
      return ParentClass::createActionGroup(parent, name);
   }

   QWidget *createWidget(const QString &className, QWidget *parent, const QString &name) override {
      if (QWidget *widget = loader->createWidget(className, parent, name)) {
         widget->setObjectName(name);
         return widget;
      }

      return nullptr;
   }

   QLayout *createLayout(const QString &className, QObject *parent, const QString &name) override {
      if (QLayout *layout = loader->createLayout(className, parent, name)) {
         layout->setObjectName(name);
         return layout;
      }

      return nullptr;
   }

   QActionGroup *createActionGroup(QObject *parent, const QString &name) override {
      if (QActionGroup *actionGroup = loader->createActionGroup(parent, name)) {
         actionGroup->setObjectName(name);
         return actionGroup;
      }

      return nullptr;
   }

   QAction *createAction(QObject *parent, const QString &name)  override {
      if (QAction *action = loader->createAction(parent, name)) {
         action->setObjectName(name);
         return action;
      }

      return nullptr;
   }

   void applyProperties(QObject *o, const QList<DomProperty *> &properties) override;
   QWidget *create(DomUI *ui, QWidget *parentWidget) override;
   QWidget *create(DomWidget *ui_widget, QWidget *parentWidget) override;
   bool addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget) override;

 private:
   QString m_class;
   TranslationWatcher *m_trwatch;
};

static QString convertTranslatable(const DomProperty *p, const QString &className,
   QUiTranslatableStringValue *strVal)
{
   if (p->kind() != DomProperty::String) {
      return QString();
   }

   const DomString *dom_str = p->elementString();

   if (!dom_str) {
      return QString();
   }

   if (dom_str->hasAttributeNotr()) {
      const QString notr = dom_str->attributeNotr();
      if (notr == QString("yes") || notr == QString("true")) {
         return QString();
      }
   }

   strVal->setValue(dom_str->text().toUtf8());
   strVal->setComment(dom_str->attributeComment().toUtf8());

   if (strVal->value().isEmpty() && strVal->comment().isEmpty()) {
      return QString();
   }

   return QCoreApplication::translate(className, strVal->value(), strVal->comment());
}

void FormBuilderPrivate::applyProperties(QObject *o, const QList<DomProperty *> &properties)
{
   QFormBuilder::applyProperties(o, properties);

   if (! m_trwatch) {
      m_trwatch = new TranslationWatcher(o, m_class);
   }

   if (properties.empty()) {
      return;
   }

   // Unlike string item roles, string properties are not loaded via the textBuilder
   // (as they are "shadowed" by the property sheets in designer). So do the initial
   // translation here.
   bool anyTrs = false;

   for (const DomProperty *p : properties) {
      QUiTranslatableStringValue strVal;
      const QString text = convertTranslatable(p, m_class, &strVal);

      if (text.isEmpty()) {
         continue;
      }

      const QString name = p->attributeName();

      if (dynamicTr) {
         const QString dynname = QString(PROP_GENERIC_PREFIX + name);
         o->setProperty(dynname, QVariant::fromValue(strVal));
         anyTrs = trEnabled;
      }
      o->setProperty(name, text);
   }

   if (anyTrs) {
      o->installEventFilter(m_trwatch);
   }
}

QWidget *FormBuilderPrivate::create(DomUI *ui, QWidget *parentWidget)
{
   m_class  = ui->elementClass();
   m_trwatch = nullptr;

   setTextBuilder(new TranslatingTextBuilder(trEnabled, m_class));
   return QFormBuilder::create(ui, parentWidget);
}

QWidget *FormBuilderPrivate::create(DomWidget *ui_widget, QWidget *parentWidget)
{
   QWidget *w = QFormBuilder::create(ui_widget, parentWidget);

   if (w == nullptr) {
      return nullptr;
   }

   if (dynamic_cast<QTabWidget *>(w)) {
      // do nothing

   } else if (dynamic_cast<QListWidget *>(w)) {
      // do nothing

   } else if (dynamic_cast<QTreeWidget *>(w)) {
      // do nothing

   } else if (dynamic_cast<QTableWidget *>(w)) {
      // do nothing

   } else if (dynamic_cast<QComboBox *>(w)) {
      if (dynamic_cast<QFontComboBox *>(w)) {
         return w;
      }

   } else if (dynamic_cast<QToolBox *>(w)) {

   } else {
      return w;
   }

   if (dynamicTr && trEnabled) {
      w->installEventFilter(m_trwatch);
   }

   return w;
}

#define TRANSLATE_SUBWIDGET_PROP(mainWidget, attribute, setter, propName) \
    do { \
        if (const DomProperty *p##attribute = attributes.value(strings.attribute)) { \
            QUiTranslatableStringValue strVal; \
            const QString text = convertTranslatable(p##attribute, m_class, &strVal); \
            if (!text.isEmpty()) { \
                if (dynamicTr) \
                    mainWidget->widget(i)->setProperty(propName, QVariant::fromValue(strVal)); \
                mainWidget->setter(i, text); \
            } \
        } \
    } while (0)

bool FormBuilderPrivate::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
   if (parentWidget == nullptr) {
      return true;
   }

   if (! ParentClass::addItem(ui_widget, widget, parentWidget)) {
      return false;
   }

   // Check special cases. First: Custom container
   const QString className = parentWidget->metaObject()->className();
   if (! d->customWidgetAddPageMethod(className).isEmpty()) {
      return true;
   }

   const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

   if (QTabWidget *tabWidget = dynamic_cast<QTabWidget *>(parentWidget)) {
      const DomPropertyHash attributes = propertyMap(ui_widget->elementAttribute());
      const int i = tabWidget->count() - 1;

      TRANSLATE_SUBWIDGET_PROP(tabWidget, titleAttribute, setTabText, PROP_TABPAGETEXT);
      TRANSLATE_SUBWIDGET_PROP(tabWidget, toolTipAttribute, setTabToolTip, PROP_TABPAGETOOLTIP);
      TRANSLATE_SUBWIDGET_PROP(tabWidget, whatsThisAttribute, setTabWhatsThis, PROP_TABPAGEWHATSTHIS);

   } else if (QToolBox *toolBox = dynamic_cast<QToolBox *>(parentWidget)) {
      const DomPropertyHash attributes = propertyMap(ui_widget->elementAttribute());
      const int i = toolBox->count() - 1;

      TRANSLATE_SUBWIDGET_PROP(toolBox, labelAttribute, setItemText, PROP_TOOLITEMTEXT);
      TRANSLATE_SUBWIDGET_PROP(toolBox, toolTipAttribute, setItemToolTip, PROP_TOOLITEMTOOLTIP);
   }

   return true;
}

class QUiLoaderPrivate
{
 public:
   FormBuilderPrivate builder;
   void setupWidgetMap() const;
};

void QUiLoaderPrivate::setupWidgetMap() const
{
   if (! g_widgets()->isEmpty()) {
      return;
   }

#define DECLARE_WIDGET(a, b) g_widgets()->insert(#a, true);
#define DECLARE_LAYOUT(a, b)

#include "widgets.table"

#undef DECLARE_WIDGET
#undef DECLARE_WIDGET_1
#undef DECLARE_LAYOUT
}

QUiLoader::QUiLoader(QObject *parent)
   : QObject(parent), d_ptr(new QUiLoaderPrivate)
{
   Q_D(QUiLoader);

   d->builder.loader = this;

   QStringList paths;

   for (const QString &path : QApplication::libraryPaths()) {
      QString libPath = path;

      libPath  += QDir::separator();
      libPath  += "designer";
      paths.append(libPath);
   }

   d->builder.setPluginPath(paths);
}

QUiLoader::~QUiLoader()
{
}

QWidget *QUiLoader::load(QIODevice *device, QWidget *parentWidget)
{
   Q_D(QUiLoader);

   // QXmlStreamReader will report errors on open failure.
   if (!device->isOpen()) {
      device->open(QIODevice::ReadOnly | QIODevice::Text);
   }

   return d->builder.load(device, parentWidget);
}

QStringList QUiLoader::pluginPaths() const
{
   Q_D(const QUiLoader);
   return d->builder.pluginPaths();
}

void QUiLoader::clearPluginPaths()
{
   Q_D(QUiLoader);
   d->builder.clearPluginPaths();
}

void QUiLoader::addPluginPath(const QString &path)
{
   Q_D(QUiLoader);
   d->builder.addPluginPath(path);
}

QWidget *QUiLoader::createWidget(const QString &className, QWidget *parent, const QString &name)
{
   Q_D(QUiLoader);
   return d->builder.defaultCreateWidget(className, parent, name);
}

QLayout *QUiLoader::createLayout(const QString &className, QObject *parent, const QString &name)
{
   Q_D(QUiLoader);
   return d->builder.defaultCreateLayout(className, parent, name);
}

QActionGroup *QUiLoader::createActionGroup(QObject *parent, const QString &name)
{
   Q_D(QUiLoader);
   return d->builder.defaultCreateActionGroup(parent, name);
}

QAction *QUiLoader::createAction(QObject *parent, const QString &name)
{
   Q_D(QUiLoader);
   return d->builder.defaultCreateAction(parent, name);
}

QStringList QUiLoader::availableWidgets() const
{
   Q_D(const QUiLoader);

   d->setupWidgetMap();
   widget_map available = *g_widgets();

   for (QDesignerCustomWidgetInterface *plugin : d->builder.customWidgets()) {
      available.insert(plugin->name(), true);
   }

   return available.keys();
}

QStringList QUiLoader::availableLayouts() const
{
   QStringList rc;

#define DECLARE_WIDGET(a, b)
#define DECLARE_LAYOUT(a, b) rc.push_back(#a);

#include "widgets.table"

#undef DECLARE_WIDGET
#undef DECLARE_LAYOUT
   return rc;
}

void QUiLoader::setWorkingDirectory(const QDir &dir)
{
   Q_D(QUiLoader);
   d->builder.setWorkingDirectory(dir);
}

QDir QUiLoader::workingDirectory() const
{
   Q_D(const QUiLoader);
   return d->builder.workingDirectory();
}

void QUiLoader::setLanguageChangeEnabled(bool enabled)
{
   Q_D(QUiLoader);
   d->builder.dynamicTr = enabled;
}

bool QUiLoader::isLanguageChangeEnabled() const
{
   Q_D(const QUiLoader);
   return d->builder.dynamicTr;
}

void QUiLoader::setTranslationEnabled(bool enabled)
{
   Q_D(QUiLoader);
   d->builder.trEnabled = enabled;
}

bool QUiLoader::isTranslationEnabled() const
{
   Q_D(const QUiLoader);
   return d->builder.trEnabled;
}

QString QUiLoader::errorString() const
{
   Q_D(const QUiLoader);
   return d->builder.errorString();
}
