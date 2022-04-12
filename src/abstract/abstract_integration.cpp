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

#include <abstract_integration.h>
#include <abstract_formwindow.h>
#include <abstract_formwindowmanager.h>
#include <abstract_formwindowcursor.h>
#include <abstract_formeditor.h>
#include <abstract_actioneditor.h>
#include <abstract_widgetbox.h>
#include <abstract_resourcebrowser.h>
#include <propertysheet.h>
#include <abstract_propertyeditor.h>
#include <extension_manager.h>
#include <gradient_manager.h>
#include <gradient_utils.h>
#include <designer_widgetbox.h>
#include <resource_model.h>
#include <designer_property_command.h>
#include <designer_property_editor.h>
#include <designer_object_inspector.h>
#include <widgetfactory.h>

#include <widgetdatabase_p.h>

#include <QVariant>
#include <QFile>
#include <QDir>
#include <QDebug>

class QDesignerIntegrationInterfacePrivate
{
 public:
   QDesignerIntegrationInterfacePrivate(QDesignerFormEditorInterface *core)
      : m_core(core) {
   }

   QDesignerFormEditorInterface *m_core;
};

QDesignerIntegrationInterface::QDesignerIntegrationInterface(QDesignerFormEditorInterface *core, QObject *parent)
   : QObject(parent), d(new QDesignerIntegrationInterfacePrivate(core))
{
   core->setIntegration(this);
}

QDesignerIntegrationInterface::~QDesignerIntegrationInterface()
{
}

QDesignerFormEditorInterface *QDesignerIntegrationInterface::core() const
{
   return d->m_core;
}

bool QDesignerIntegrationInterface::hasFeature(Feature f) const
{
   return (features() & f) != 0;
}

void QDesignerIntegrationInterface::emitObjectNameChanged(QDesignerFormWindowInterface *formWindow, QObject *object,
   const QString &newName, const QString &oldName)
{
   emit objectNameChanged(formWindow, object, newName, oldName);
}

void QDesignerIntegrationInterface::emitNavigateToSlot(const QString &objectName,
   const QString &signalSignature,
   const QStringList &parameterNames)
{
   emit navigateToSlot(objectName, signalSignature, parameterNames);
}

void QDesignerIntegrationInterface::emitNavigateToSlot(const QString &slotSignature)
{
   emit navigateToSlot(slotSignature);
}


namespace qdesigner_internal {

class QDesignerIntegrationPrivate
{
 public:
   explicit QDesignerIntegrationPrivate(QDesignerIntegration *qq);

   QWidget *containerWindow(QWidget *widget) const;

   // Load plugins into widget database and factory

   // emerald - temporary hold, plugins
   // static void initializePlugins(QDesignerFormEditorInterface *formEditor);

   QString contextHelpId() const;

   void updateProperty(const QString &name, const QVariant &value, bool enableSubPropertyHandling);
   void resetProperty(const QString &name);
   void addDynamicProperty(const QString &name, const QVariant &value);
   void removeDynamicProperty(const QString &name);

   void setupFormWindow(QDesignerFormWindowInterface *formWindow);
   void updateSelection();

   // emerald - temporary hold, custom widget
   // void updateCustomWidgetPlugins();

   void initialize();
   void getSelection(qdesigner_internal::Selection &s);
   QObject *propertyEditorObject();

   QDesignerIntegration *q;
   QString headerSuffix;
   bool headerLowercase;
   QDesignerIntegrationInterface::Feature m_features;
   QDesignerIntegrationInterface::ResourceFileWatcherBehaviour m_resourceFileWatcherBehaviour;
   QString m_gradientsPath;
   QtGradientManager *m_gradientManager;
};

QDesignerIntegrationPrivate::QDesignerIntegrationPrivate(QDesignerIntegration *qq)
   : q(qq), headerSuffix(QString(".h")), headerLowercase(true),
     m_features(QDesignerIntegrationInterface::DefaultFeature),
     m_resourceFileWatcherBehaviour(QDesignerIntegrationInterface::PromptToReloadResourceFile),
     m_gradientManager(nullptr)
{
}

void QDesignerIntegrationPrivate::initialize()
{
   typedef void (QDesignerIntegration::*QDesignerIntegrationUpdatePropertySlot3)(const QString &, const QVariant &, bool);

   // integrate the `Form Editor component'

   // Extensions
   QDesignerFormEditorInterface *core = q->core();

   if (QDesignerPropertyEditor *designerPropertyEditor = dynamic_cast<QDesignerPropertyEditor *>(core->propertyEditor())) {
      QObject::connect(designerPropertyEditor, &QDesignerPropertyEditor::propertyValueChanged,
         q, static_cast<QDesignerIntegrationUpdatePropertySlot3>(&QDesignerIntegration::updateProperty));

      QObject::connect(designerPropertyEditor, &QDesignerPropertyEditor::resetProperty,
         q, &QDesignerIntegration::resetProperty);

      QObject::connect(designerPropertyEditor, &QDesignerPropertyEditor::addDynamicProperty,
         q, &QDesignerIntegration::addDynamicProperty);

      QObject::connect(designerPropertyEditor, &QDesignerPropertyEditor::removeDynamicProperty,
         q, &QDesignerIntegration::removeDynamicProperty);
   }

   QObject::connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowAdded,
      q, &QDesignerIntegrationInterface::setupFormWindow);

   QObject::connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::activeFormWindowChanged,
      q, &QDesignerIntegrationInterface::updateActiveFormWindow);

   m_gradientManager = new QtGradientManager(q);
   core->setGradientManager(m_gradientManager);

   // check if the user has their own file
   m_gradientsPath = QDir::homePath() + QDir::separator() + ".designer" + QDir::separator() + "gradients.xml";

   QFile f(m_gradientsPath);

   if (f.open(QIODevice::ReadOnly)) {
      QtGradientUtils::restoreState(m_gradientManager, QString::fromLatin1(f.readAll()));
      f.close();

   } else {
      QFile defaultGradients(":/resources/defaultgradients.xml");

      if (defaultGradients.open(QIODevice::ReadOnly)) {
         QtGradientUtils::restoreState(m_gradientManager, QString::fromLatin1(defaultGradients.readAll()));
         defaultGradients.close();
      }
   }

   if (WidgetDataBase *widgetDataBase = qobject_cast<WidgetDataBase *>(core->widgetDataBase())) {
      widgetDataBase->grabStandardWidgetBoxIcons();
   }
}

void QDesignerIntegrationPrivate::updateProperty(const QString &name, const QVariant &value, bool enableSubPropertyHandling)
{
   QDesignerFormWindowInterface *formWindow = q->core()->formWindowManager()->activeFormWindow();
   if (!formWindow) {
      return;
   }

   Selection selection;
   getSelection(selection);
   if (selection.empty()) {
      return;
   }

   SetPropertyCommand *cmd = new SetPropertyCommand(formWindow);
   // find a reference object to compare to and to find the right group
   if (cmd->init(selection.selection(), name, value, propertyEditorObject(), enableSubPropertyHandling)) {
      formWindow->commandHistory()->push(cmd);
   } else {
      delete cmd;
      qDebug() << "Unable to set  property " << name << '.';
   }
}

void QDesignerIntegrationPrivate::resetProperty(const QString &name)
{
   QDesignerFormWindowInterface *formWindow = q->core()->formWindowManager()->activeFormWindow();
   if (!formWindow) {
      return;
   }

   Selection selection;
   getSelection(selection);
   if (selection.empty()) {
      return;
   }

   ResetPropertyCommand *cmd = new ResetPropertyCommand(formWindow);
   // find a reference object to find the right group
   if (cmd->init(selection.selection(), name, propertyEditorObject())) {
      formWindow->commandHistory()->push(cmd);
   } else {
      delete cmd;
      qDebug() << "** WARNING Unable to reset property " << name << '.';
   }
}

void QDesignerIntegrationPrivate::addDynamicProperty(const QString &name, const QVariant &value)
{
   QDesignerFormWindowInterface *formWindow = q->core()->formWindowManager()->activeFormWindow();
   if (!formWindow) {
      return;
   }

   Selection selection;
   getSelection(selection);
   if (selection.empty()) {
      return;
   }

   AddDynamicPropertyCommand *cmd = new AddDynamicPropertyCommand(formWindow);
   if (cmd->init(selection.selection(), propertyEditorObject(), name, value)) {
      formWindow->commandHistory()->push(cmd);
   } else {
      delete cmd;
      qDebug() <<  "** WARNING Unable to add dynamic property " << name << '.';
   }
}

void QDesignerIntegrationPrivate::removeDynamicProperty(const QString &name)
{
   QDesignerFormWindowInterface *formWindow = q->core()->formWindowManager()->activeFormWindow();
   if (!formWindow) {
      return;
   }

   Selection selection;
   getSelection(selection);
   if (selection.empty()) {
      return;
   }

   RemoveDynamicPropertyCommand *cmd = new RemoveDynamicPropertyCommand(formWindow);
   if (cmd->init(selection.selection(), propertyEditorObject(), name)) {
      formWindow->commandHistory()->push(cmd);
   } else {
      delete cmd;
      qDebug() << "** WARNING Unable to remove dynamic property " << name << '.';
   }

}

void QDesignerIntegrationPrivate::setupFormWindow(QDesignerFormWindowInterface *formWindow)
{
   QObject::connect(formWindow, &QDesignerFormWindowInterface::selectionChanged,
      q, &QDesignerIntegrationInterface::updateSelection);
}

void QDesignerIntegrationPrivate::updateSelection()
{
   QDesignerFormEditorInterface *core       = q->core();
   QDesignerFormWindowInterface *formWindow = core->formWindowManager()->activeFormWindow();
   QWidget *selection = nullptr;

   if (formWindow) {
      selection = formWindow->cursor()->current();
   }

   if (QDesignerActionEditorInterface *actionEditor = core->actionEditor()) {
      actionEditor->setFormWindow(formWindow);
   }

   if (QDesignerPropertyEditorInterface *propertyEditor = core->propertyEditor()) {
      propertyEditor->setObject(selection);
   }

   if (QDesignerObjectInspectorInterface *objectInspector = core->objectInspector()) {
      objectInspector->setFormWindow(formWindow);
   }

}

QWidget *QDesignerIntegrationPrivate::containerWindow(QWidget *widget) const
{
   // Find the parent window to apply a geometry to.
   while (widget) {
      if (widget->isWindow()) {
         break;
      }

      if (widget->metaObject()->className() == "QMdiSubWindow") {
         break;
      }

      widget = widget->parentWidget();
   }

   return widget;
}

void QDesignerIntegrationPrivate::getSelection(Selection &s)
{
   QDesignerFormEditorInterface *core = q->core();
   // Get multiselection from object inspector
   if (QDesignerObjectInspector *designerObjectInspector = qobject_cast<QDesignerObjectInspector *>(core->objectInspector())) {
      designerObjectInspector->getSelection(s);
      // Action editor puts actions that are not on the form yet
      // into the property editor only.
      if (s.empty())
         if (QObject *object = core->propertyEditor()->object()) {
            s.objects.push_back(object);
         }

   } else {
      // Just in case someone plugs in an old-style object inspector: Emulate selection
      s.clear();
      QDesignerFormWindowInterface *formWindow = core->formWindowManager()->activeFormWindow();
      if (!formWindow) {
         return;
      }

      QObject *object = core->propertyEditor()->object();

      if (object->isWidgetType()) {
         QWidget *widget = static_cast<QWidget *>(object);
         QDesignerFormWindowCursorInterface *cursor = formWindow->cursor();

         if (cursor->isWidgetSelected(widget)) {
            s.managed.push_back(widget);
         } else {
            s.unmanaged.push_back(widget);
         }

      } else {
         s.objects.push_back(object);
      }
   }
}

QObject *QDesignerIntegrationPrivate::propertyEditorObject()
{
   if (QDesignerPropertyEditorInterface *propertyEditor = q->core()->propertyEditor()) {
      return propertyEditor->object();
   }

   return nullptr;
}

/* emerald - temporary hold, plugins

// Load plugins into widget database and factory.
void QDesignerIntegrationPrivate::initializePlugins(QDesignerFormEditorInterface *formEditor)
{
    // load the plugins
    qdesigner_internal::WidgetDataBase *widgetDataBase = qobject_cast<qdesigner_internal::WidgetDataBase*>(formEditor->widgetDataBase());
    if (widgetDataBase) {
        widgetDataBase->loadPlugins();
    }

    if (qdesigner_internal::WidgetFactory *widgetFactory = qobject_cast<qdesigner_internal::WidgetFactory*>(formEditor->widgetFactory())) {
        widgetFactory->loadPlugins();
    }

    if (widgetDataBase) {
        widgetDataBase->grabDefaultPropertyValues();
    }
}

void QDesignerIntegrationPrivate::updateCustomWidgetPlugins()
{
    QDesignerFormEditorInterface *formEditor = q->core();
    if (QDesignerPluginManager *pm = formEditor->pluginManager())
        pm->registerNewPlugins();

    initializePlugins(formEditor);

    // Do not just reload the last file as the WidgetBox merges the compiled-in resources
    // and $HOME/.designer/widgetbox.xml. This would also double the scratchpad.
    if (QDesignerWidgetBox *wb = qobject_cast<QDesignerWidgetBox*>(formEditor->widgetBox())) {
        const QDesignerWidgetBox::LoadMode oldLoadMode = wb->loadMode();
        wb->setLoadMode(QDesignerWidgetBox::LoadCustomWidgetsOnly);
        wb->load();
        wb->setLoadMode(oldLoadMode);
    }
}

*/

static QString fixHelpClassName(const QString &className)
{
   // ### generalize using the Widget Data Base
   if (className == QString("Line")) {
      return QString("QFrame");
   }
   if (className == QString("Spacer")) {
      return QString("QSpacerItem");
   }
   if (className == QString("QLayoutWidget")) {
      return QString("QLayout");
   }
   return className;
}

// Return class in which the property is defined
static QString classForProperty(QDesignerFormEditorInterface *core, QObject *object, const QString &property)
{
   if (const QDesignerPropertySheetExtension *ps = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), object)) {
      const int index = ps->indexOf(property);
      if (index >= 0) {
         return ps->propertyGroup(index);
      }
   }
   return QString();
}

QString QDesignerIntegrationPrivate::contextHelpId() const
{
   QDesignerFormEditorInterface *core = q->core();
   QObject *currentObject = core->propertyEditor()->object();
   if (!currentObject) {
      return QString();
   }
   // Return a help index id consisting of "class::property"
   QString className;
   QString currentPropertyName = core->propertyEditor()->currentPropertyName();
   if (!currentPropertyName.isEmpty()) {
      className = classForProperty(core, currentObject, currentPropertyName);
   }
   if (className.isEmpty()) {
      currentPropertyName.clear(); // We hit on some fake property.
      className = qdesigner_internal::WidgetFactory::classNameOf(core, currentObject);
   }
   QString helpId = fixHelpClassName(className);
   if (!currentPropertyName.isEmpty()) {
      helpId += QString("::");
      helpId += currentPropertyName;
   }
   return helpId;
}

}   // end namespace - qdesigner_internal

QDesignerIntegration::QDesignerIntegration(QDesignerFormEditorInterface *core, QObject *parent) :
   QDesignerIntegrationInterface(core, parent),
   d(new qdesigner_internal::QDesignerIntegrationPrivate(this))
{
   d->initialize();
}

QDesignerIntegration::~QDesignerIntegration()
{
   QFile f(d->m_gradientsPath);
   if (f.open(QIODevice::WriteOnly)) {
      f.write(QtGradientUtils::saveState(d->m_gradientManager).toUtf8());
      f.close();
   }
}

QString QDesignerIntegration::headerSuffix() const
{
   return d->headerSuffix;
}

void QDesignerIntegration::setHeaderSuffix(const QString &headerSuffix)
{
   d->headerSuffix = headerSuffix;
}

bool QDesignerIntegration::isHeaderLowercase() const
{
   return d->headerLowercase;
}

void QDesignerIntegration::setHeaderLowercase(bool headerLowercase)
{
   d->headerLowercase = headerLowercase;
}

QDesignerIntegrationInterface::Feature QDesignerIntegration::features() const
{
   return d->m_features;
}

void QDesignerIntegration::setFeatures(Feature f)
{
   d->m_features = f;
}

QDesignerIntegrationInterface::ResourceFileWatcherBehaviour QDesignerIntegration::resourceFileWatcherBehaviour() const
{
   return d->m_resourceFileWatcherBehaviour;
}

void QDesignerIntegration::setResourceFileWatcherBehaviour(ResourceFileWatcherBehaviour behaviour)
{
   if (d->m_resourceFileWatcherBehaviour != behaviour) {
      d->m_resourceFileWatcherBehaviour = behaviour;
      core()->resourceModel()->setWatcherEnabled(behaviour != QDesignerIntegrationInterface::NoResourceFileWatcher);
   }
}

void QDesignerIntegration::updateProperty(const QString &name, const QVariant &value, bool enableSubPropertyHandling)
{
   d->updateProperty(name, value, enableSubPropertyHandling);
   emit propertyChanged(core()->formWindowManager()->activeFormWindow(), name, value);
}

void QDesignerIntegration::updateProperty(const QString &name, const QVariant &value)
{
   updateProperty(name, value, true);
}

void QDesignerIntegration::resetProperty(const QString &name)
{
   d->resetProperty(name);
}

void QDesignerIntegration::addDynamicProperty(const QString &name, const QVariant &value)
{
   d->addDynamicProperty(name, value);
}

void QDesignerIntegration::removeDynamicProperty(const QString &name)
{
   d->removeDynamicProperty(name);
}

void QDesignerIntegration::updateActiveFormWindow(QDesignerFormWindowInterface *)
{
   d->updateSelection();
}

void QDesignerIntegration::setupFormWindow(QDesignerFormWindowInterface *formWindow)
{
   d->setupFormWindow(formWindow);
   connect(formWindow, &QDesignerFormWindowInterface::selectionChanged,
      this, &QDesignerIntegrationInterface::updateSelection);
}

void QDesignerIntegration::updateSelection()
{
   d->updateSelection();
}

QWidget *QDesignerIntegration::containerWindow(QWidget *widget) const
{
   return d->containerWindow(widget);
}


/* emerald - temporary hold, plugins

// Load plugins into widget database and factory
void QDesignerIntegration::initializePlugins(QDesignerFormEditorInterface *formEditor)
{
    qdesigner_internal::QDesignerIntegrationPrivate::initializePlugins(formEditor);
}

void QDesignerIntegration::updateCustomWidgetPlugins()
{
    d->updateCustomWidgetPlugins();
}

*/

QDesignerResourceBrowserInterface *QDesignerIntegration::createResourceBrowser(QWidget *)
{
   return nullptr;
}

QString QDesignerIntegration::contextHelpId() const
{
   return d->contextHelpId();
}

