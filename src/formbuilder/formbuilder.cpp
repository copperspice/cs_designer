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

#include <customwidget.h>
#include <formbuilder.h>
#include <formbuilderextra.h>
#include <table_classes.h>
#include <ui4.h>

QFormBuilder::QFormBuilder()
{
}

QFormBuilder::~QFormBuilder()
{
}

QWidget *QFormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
   if (! d->parentWidgetIsSet()) {
      d->setParentWidget(QPointer<QWidget>(parentWidget));
   }

   // Is this a QLayoutWidget with a margin of 0: Not a known page-based
   // container and no method for adding pages registered

   d->setProcessingLayoutWidget(false);

   if (ui_widget->attributeClass() == QFormBuilderStrings::instance().qWidgetClass && !ui_widget->hasAttributeNative()
      && parentWidget
      && ! dynamic_cast<QMainWindow *>(parentWidget)
      && ! dynamic_cast<QToolBox *>(parentWidget)
      && ! dynamic_cast<QStackedWidget *>(parentWidget)
      && ! dynamic_cast<QTabWidget *>(parentWidget)
      && ! dynamic_cast<QScrollArea *>(parentWidget)
      && ! dynamic_cast<QMdiArea *>(parentWidget)
      && ! dynamic_cast<QDockWidget *>(parentWidget)) {

      const QString parentClassName = parentWidget->metaObject()->className();

      if (! d->isCustomWidgetContainer(parentClassName)) {
         d->setProcessingLayoutWidget(true);
      }
   }

   return QAbstractFormBuilder::create(ui_widget, parentWidget);
}

QWidget *QFormBuilder::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
   if (widgetName.isEmpty()) {
      // empty class name passed to widget factory method
      qWarning("QFormBuilder::createWidget() Widget class name was empty for object: %s", csPrintable(name));

      return nullptr;
   }

   QWidget *formWidget = nullptr;

   if (dynamic_cast<QTabWidget *>(parentWidget)) {
      parentWidget = nullptr;
   }

   if (dynamic_cast<QStackedWidget *>(parentWidget)) {
      parentWidget = nullptr;
   }

   if (dynamic_cast<QToolBox *>(parentWidget)) {
      parentWidget = nullptr;
   }

   // special-casing for Line (QFrame)
   do {
      if (widgetName == QFormBuilderStrings::instance().lineClass) {
         formWidget = new QFrame(parentWidget);
         static_cast<QFrame *>(formWidget)->setFrameStyle(cs_enum_cast(QFrame::HLine) | cs_enum_cast(QFrame::Sunken));
         break;
      }

      if (formWidget != nullptr) {
         // symmetry for macro
      }

#define DECLARE_LAYOUT(L, C)
#define DECLARE_COMPAT_WIDGET(W, C)
#define DECLARE_WIDGET(W, C)   else if (widgetName == #W) { Q_ASSERT(formWidget == nullptr); formWidget = new W(parentWidget); }
#define DECLARE_WIDGET_1(W, C) else if (widgetName == #W) { Q_ASSERT(formWidget == nullptr); formWidget = new W(nullptr, parentWidget); }

#include "widgets.table"

#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_LAYOUT
#undef DECLARE_WIDGET
#undef DECLARE_WIDGET_1

      if (formWidget != nullptr) {
         break;
      }

      // try with a registered custom widget
      QDesignerCustomWidgetInterface *factory = d->m_customWidgets.value(widgetName);

      if (factory != nullptr) {
         formWidget = factory->createWidget(parentWidget);
      }

   } while (false);

   if (formWidget == nullptr) {
      // Attempt to instantiate base class of promoted/custom widgets
      const QString baseClassName = d->customWidgetBaseClass(widgetName);

      if (! baseClassName.isEmpty()) {
         qWarning("QFormBuilder::createWidget() Unable to create a custom widget of the class %s, defaulting to base class",
               csPrintable(baseClassName));

         return createWidget(baseClassName, parentWidget, name);
      }
   }

   if (formWidget == nullptr) {
      // nothing to do
      qWarning("QFormBuilder::createWidget() Unable to create a widget of class %s", csPrintable(widgetName));

      return nullptr;
   }

   formWidget ->setObjectName(name);

   if (dynamic_cast<QDialog *>(formWidget)) {
      formWidget ->setParent(parentWidget);
   }

   return formWidget;
}

QLayout *QFormBuilder::createLayout(const QString &layoutName, QObject *parent, const QString &name)
{
   QLayout *retval = nullptr;

   QWidget *parentWidget = dynamic_cast<QWidget *>(parent);
   QLayout *parentLayout = dynamic_cast<QLayout *>(parent);

   Q_ASSERT(parentWidget || parentLayout);

#define DECLARE_COMPAT_WIDGET(W, C)
#define DECLARE_WIDGET(W, C)

#define DECLARE_LAYOUT(L, C) \
    if (layoutName == #L) { \
        Q_ASSERT(retval == nullptr); \
        retval = parentLayout ? new L() : new L(parentWidget); \
    }

#include "widgets.table"

#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_WIDGET
#undef DECLARE_LAYOUT

   if (retval == nullptr) {
      qWarning("QFormBuilder::createLayout() Layout type %s is not supported", csPrintable(layoutName));
   } else {
     retval->setObjectName(name);
   }

   return retval;
}

bool QFormBuilder::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
   return QAbstractFormBuilder::addItem(ui_item, item, layout);
}

bool QFormBuilder::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
   return QAbstractFormBuilder::addItem(ui_widget, widget, parentWidget);
}

QWidget *QFormBuilder::widgetByName(QWidget *topLevel, const QString &name)
{
   Q_ASSERT(topLevel);

   if (topLevel->objectName() == name) {
      return topLevel;
   }

   return topLevel->findChild<QWidget *>(name);
}

static QObject *objectByName(QWidget *topLevel, const QString &name)
{
   Q_ASSERT(topLevel);

   if (topLevel->objectName() == name) {
      return topLevel;
   }

   return topLevel->findChild<QObject *>(name);
}

void QFormBuilder::createConnections(DomConnections *ui_connections, QWidget *widget)
{
   Q_ASSERT(widget != nullptr);

   if (ui_connections == nullptr) {
      return;
   }

   for (const auto item : ui_connections->elementConnection()) {

      QObject *sender   = objectByName(widget, item->elementSender());
      QObject *receiver = objectByName(widget, item->elementReceiver());

      if (! sender || ! receiver) {
         continue;
      }

      QString signalName  = item->elementSignal();
      QString slotName    = item->elementSlot();

      QObject::connect(sender, signalName, receiver, slotName);
   }
}

QWidget *QFormBuilder::create(DomUI *ui, QWidget *parentWidget)
{
   return QAbstractFormBuilder::create(ui, parentWidget);
}

QLayout *QFormBuilder::create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget)
{
   // Is this a temporary layout widget used to represent QLayout hierarchies in Designer?
   // Set its margins to 0.

   bool layoutWidget = d->processingLayoutWidget();
   QLayout *retval = QAbstractFormBuilder::create(ui_layout, layout, parentWidget);

   if (layoutWidget) {
      const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

      int left   = 0;
      int top    = 0;
      int right  = 0;
      int bottom = 0;;

      const DomPropertyHash properties = propertyMap(ui_layout->elementProperty());

      if (DomProperty *prop = properties.value(strings.leftMarginProperty)) {
         left = prop->elementNumber();
      }

      if (DomProperty *prop = properties.value(strings.topMarginProperty)) {
         top = prop->elementNumber();
      }

      if (DomProperty *prop = properties.value(strings.rightMarginProperty)) {
         right = prop->elementNumber();
      }

      if (DomProperty *prop = properties.value(strings.bottomMarginProperty)) {
         bottom = prop->elementNumber();
      }

      retval->setContentsMargins(left, top, right, bottom);
      d->setProcessingLayoutWidget(false);
   }

   return retval;
}

QLayoutItem *QFormBuilder::create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget)
{
   return QAbstractFormBuilder::create(ui_layoutItem, layout, parentWidget);
}

QAction *QFormBuilder::create(DomAction *ui_action, QObject *parent)
{
   return QAbstractFormBuilder::create(ui_action, parent);
}

QActionGroup *QFormBuilder::create(DomActionGroup *ui_action_group, QObject *parent)
{
   return QAbstractFormBuilder::create(ui_action_group, parent);
}

QStringList QFormBuilder::pluginPaths() const
{
   return d->m_pluginPaths;
}

void QFormBuilder::clearPluginPaths()
{
   d->m_pluginPaths.clear();
   updateCustomWidgets();
}

void QFormBuilder::addPluginPath(const QString &pluginPath)
{
   d->m_pluginPaths.append(pluginPath);
   updateCustomWidgets();
}

void QFormBuilder::setPluginPath(const QStringList &pluginPaths)
{
   d->m_pluginPaths = pluginPaths;
   updateCustomWidgets();
}

/* emerald - hold custom plugins

static void insertPlugins(QObject *o, QMap<QString, QDesignerCustomWidgetInterface *> *customWidgets)
{
   // step 1) try with a normal plugin
   if (QDesignerCustomWidgetInterface *iface = dynamic_cast<QDesignerCustomWidgetInterface *>(o)) {
      customWidgets->insert(iface->name(), iface);
      return;
   }

   // step 2) try with a collection of plugins
   if (QDesignerCustomWidgetCollectionInterface *c = dynamic_cast<QDesignerCustomWidgetCollectionInterface *>(o)) {
      for (QDesignerCustomWidgetInterface *iface : c->customWidgets()) {
         customWidgets->insert(iface->name(), iface);
      }
   }
}

*/

void QFormBuilder::updateCustomWidgets()
{
   /* emerald - hold custom plugins

       d->m_customWidgets.clear();

       for (const QString &path : d->m_pluginPaths) {
           const QDir dir(path);
           const QStringList candidates = dir.entryList(QDir::Files);

           for (const QString &plugin : candidates) {
               if (!QLibrary::isLibrary(plugin))
                   continue;

               QString loaderPath = path;
               loaderPath += '/';
               loaderPath += plugin;

               QPluginLoader loader(loaderPath);
               if (loader.load())
                   insertPlugins(loader.instance(), &d->m_customWidgets);
           }
       }

       // Check statically linked plugins
       const QObjectList staticPlugins = QPluginLoader::staticInstances();

       if (! staticPlugins.empty())
           for (QObject *o : staticPlugins)
               insertPlugins(o, &d->m_customWidgets);
   */

}

QList<QDesignerCustomWidgetInterface *> QFormBuilder::customWidgets() const
{
   return d->m_customWidgets.values();
}

void QFormBuilder::applyProperties(QObject *o, const QList<DomProperty *> &properties)
{
   if (properties.empty()) {
      return;
   }

   const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

   auto cend = properties.constEnd();

   for (auto it = properties.constBegin(); it != cend; ++it) {
      const QVariant v = toVariant(*(o->metaObject()), *it);

      if (! v.isValid()) {
         continue;
      }

      const QString attributeName = (*it)->attributeName();
      const bool isWidget = o->isWidgetType();

      if (isWidget && o->parent() == d->parentWidget() && attributeName == strings.geometryProperty) {
         // apply only the size part of a geometry for the root widget
         static_cast<QWidget *>(o)->resize(v.value<QRect>().size());

      } else if (d->applyPropertyInternally(o, attributeName, v)) {

      } else if (isWidget && "QFrame" == o->metaObject()->className() && attributeName == strings.orientationProperty) {
         // special casing for Line (QFrame) -- try to fix
         o->setProperty("frameShape", v);          // v is of QFrame::Shape enum

      } else {
         o->setProperty(attributeName.toUtf8(), v);
      }
   }
}
