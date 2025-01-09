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

#include <abstract_dialoggui.h>
#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <abstract_widgetfactory.h>
#include <container.h>
#include <customwidget.h>
#include <designer_formbuilder.h>
#include <designer_introspection.h>
#include <designer_propertysheet.h>
#include <designer_utils.h>
#include <dynamicpropertysheet.h>
#include <extension.h>
#include <formbuilderextra.h>
#include <propertysheet.h>
#include <resource_model.h>
#include <simple_resource.h>
#include <ui4.h>
#include <widgetfactory.h>

#include <formwindowbase_p.h>

#include <QAbstractScrollArea>
#include <QApplication>
#include <QBuffer>
#include <QCoreApplication>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QStyle>
#include <QStyleFactory>
#include <QToolBar>
#include <QWidget>

namespace qdesigner_internal {

QDesignerFormBuilder::QDesignerFormBuilder(QDesignerFormEditorInterface *core, const DeviceProfile &deviceProfile)
   : m_core(core), m_deviceProfile(deviceProfile), m_pixmapCache(nullptr), m_iconCache(nullptr),
     m_ignoreCreateResources(false), m_tempResourceSet(nullptr), m_mainWidget(true)
{
   Q_ASSERT(m_core);
}

QString QDesignerFormBuilder::systemStyle() const
{
   if (m_deviceProfile.isEmpty()) {
      return QApplication::style()->metaObject()->className();

   } else {
      return m_deviceProfile.style();
   }
}

QWidget *QDesignerFormBuilder::create(DomUI *ui, QWidget *parentWidget)
{
   m_mainWidget = true;
   QtResourceSet *resourceSet = core()->resourceModel()->currentResourceSet();

   // reload resource properties;
   createResources(ui->elementResources());
   core()->resourceModel()->setCurrentResourceSet(m_tempResourceSet);

   m_ignoreCreateResources = true;
   DesignerPixmapCache pixmapCache;
   DesignerIconCache iconCache(&pixmapCache);
   m_pixmapCache = &pixmapCache;
   m_iconCache   = &iconCache;

   QWidget *widget = QFormBuilder::create(ui, parentWidget);

   core()->resourceModel()->setCurrentResourceSet(resourceSet);
   core()->resourceModel()->removeResourceSet(m_tempResourceSet);
   m_tempResourceSet = nullptr;
   m_ignoreCreateResources = false;
   m_pixmapCache = nullptr;
   m_iconCache   = nullptr;

   m_customWidgetsWithScript.clear();

   return widget;
}

QWidget *QDesignerFormBuilder::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
   QWidget *widget = nullptr;

   if (widgetName == "QToolBar") {
      widget = new QToolBar(parentWidget);

   } else if (widgetName == "QMenu") {
      widget = new QMenu(parentWidget);

   } else if (widgetName == "QMenuBar") {
      widget = new QMenuBar(parentWidget);

   } else {
      widget = core()->widgetFactory()->createWidget(widgetName, parentWidget);
   }

   if (widget) {
      widget->setObjectName(name);

      if (QSimpleResource::hasCustomWidgetScript(m_core, widget)) {
         m_customWidgetsWithScript.insert(widget);
      }
   }

   if (m_mainWidget) {
      // apply the DPI here to take effect on size hints, etc.
      m_deviceProfile.apply(m_core, widget, DeviceProfile::ApplyPreview);
      m_mainWidget = false;
   }

   return widget;
}

bool QDesignerFormBuilder::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
   // Use container extension or rely on scripts unless main window.
   if (QFormBuilder::addItem(ui_widget, widget, parentWidget)) {
      return true;
   }

   if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension *>(m_core->extensionManager(), parentWidget)) {
      container->addWidget(widget);
      return true;
   }
   return false;
}

bool QDesignerFormBuilder::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
   return QFormBuilder::addItem(ui_item, item, layout);
}

static bool readDomEnumerationValue(const DomProperty *p,
      const QDesignerPropertySheetExtension *sheet, QVariant &v)
{
   // If the property is a enum or flag value retrieve the existing enum/flag type
   // via the property sheet and use it to convert

   switch (p->kind()) {
      case DomProperty::Set: {
         const int index = sheet->indexOf(p->attributeName());

         if (index == -1) {
            return false;
         }

         const QVariant sheetValue = sheet->property(index);

         if (sheetValue.canConvert<PropertySheetFlagValue>()) {
            const PropertySheetFlagValue f = sheetValue.value<PropertySheetFlagValue>();

            bool ok = false;
            v = f.metaFlags.parseFlags(p->elementSet(), &ok);

            if (! ok) {
               f.metaFlags.messageParseFailed(p->elementSet());
            }

            return true;
         }
      }
      break;

      case DomProperty::Enum: {
         const int index = sheet->indexOf(p->attributeName());

         if (index == -1) {
            return false;
         }

         const QVariant sheetValue = sheet->property(index);

         if (sheetValue.canConvert<PropertySheetEnumValue>()) {
            const PropertySheetEnumValue e = sheetValue.value<PropertySheetEnumValue>();

            bool ok = false;
            v = e.metaEnum.parseEnum(p->elementEnum(), &ok);

            if (! ok) {
               e.metaEnum.messageParseFailed(p->elementEnum());
            }

            return true;
         }
      }
      break;

      default:
         break;
   }

   return false;
}

void QDesignerFormBuilder::applyProperties(QObject *o, const QList<DomProperty *> &properties)
{
   if (properties.empty()) {
      return;
   }

   const QDesignerPropertySheetExtension *sheet =
      qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), o);

   const QDesignerDynamicPropertySheetExtension *dynamicSheet =
      qt_extension<QDesignerDynamicPropertySheetExtension *>(core()->extensionManager(), o);

   const bool changingMetaObject = WidgetFactory::classNameOf(core(), o) == "QAxWidget";

   const QDesignerMetaObjectInterface *meta = core()->introspection()->metaObject(o);
   const bool dynamicPropertiesAllowed = dynamicSheet && dynamicSheet->dynamicPropertiesAllowed();

   QDesignerPropertySheet *designerPropertySheet = dynamic_cast<QDesignerPropertySheet *>(
         core()->extensionManager()->extension(o, CS_TYPEID(QDesignerPropertySheetExtension)));

   if (designerPropertySheet) {
      if (designerPropertySheet->pixmapCache()) {
         designerPropertySheet->setPixmapCache(m_pixmapCache);
      }

      if (designerPropertySheet->iconCache()) {
         designerPropertySheet->setIconCache(m_iconCache);
      }
   }

   auto cend = properties.constEnd();

   for (auto it = properties.constBegin(); it != cend; ++it) {
      DomProperty *p = *it;

      QVariant v;

      if (! readDomEnumerationValue(p, sheet, v)) {
         v = toVariant(*(o->metaObject()), p);
      }

      if (! v.isValid()) {
         continue;
      }

      const QString attributeName = p->attributeName();
      if (d->applyPropertyInternally(o, attributeName, v)) {
         continue;
      }

      // refuse fake properties like current tab name (weak test)
      if (! dynamicPropertiesAllowed) {

         if (changingMetaObject) {
            // Changes after setting control of QAxWidget
            meta = core()->introspection()->metaObject(o);
         }

         if (meta->indexOfProperty(attributeName) == -1) {
            continue;
         }
      }

      QObject *obj = o;
      QAbstractScrollArea *scroll = dynamic_cast<QAbstractScrollArea *>(o);

      if (scroll && attributeName == "cursor" && scroll->viewport()) {
         obj = scroll->viewport();
      }

      // a real property
      obj->setProperty(attributeName.toUtf8(), v);
   }
}

DomWidget *QDesignerFormBuilder::createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive)
{
   DomWidget *ui_widget = QFormBuilder::createDom(widget, ui_parentWidget, recursive);
   QSimpleResource::addExtensionDataToDOM(this, m_core, ui_widget, widget);
   return ui_widget;
}

QWidget *QDesignerFormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
   QWidget *widget = QFormBuilder::create(ui_widget, parentWidget);

   // Do not apply state if scripts are to be run in preview mode
   QSimpleResource::applyExtensionDataFromDOM(this, m_core, ui_widget, widget);
   return widget;
}

void QDesignerFormBuilder::createResources(DomResources *resources)
{
   if (m_ignoreCreateResources) {
      return;
   }

   QStringList paths;

   if (resources != nullptr) {
      const QList<DomResource *> dom_include = resources->elementInclude();
      for (DomResource *res : dom_include) {
         QString path = QDir::cleanPath(workingDirectory().absoluteFilePath(res->attributeLocation()));
         paths << path;
      }
   }

   m_tempResourceSet = core()->resourceModel()->addResourceSet(paths);
}

QLayout *QDesignerFormBuilder::create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget)
{
   return QFormBuilder::create(ui_layout, layout, parentWidget);
}

void QDesignerFormBuilder::loadExtraInfo(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
   QFormBuilder::loadExtraInfo(ui_widget, widget, parentWidget);
}

QWidget *QDesignerFormBuilder::createPreview(const QDesignerFormWindowInterface *fw, const QString &styleName,
      const QString &appStyleSheet, const DeviceProfile &deviceProfile, QString *errorMessage)
{
   // load
   QDesignerFormBuilder builder(fw->core(), deviceProfile);
   builder.setWorkingDirectory(fw->absoluteDir());

   QByteArray bytes = fw->contents().toUtf8();

   QBuffer buffer(&bytes);
   buffer.open(QIODevice::ReadOnly);

   QWidget *widget = builder.load(&buffer, nullptr);

   if (! widget) {
      // should not happen
      *errorMessage = QCoreApplication::translate("QDesignerFormBuilder", "The preview failed to build.");
      return  nullptr;
   }

   // Make sure palette is applied
   const QString styleToUse = styleName.isEmpty() ? builder.deviceProfile().style() : styleName;

   if (! styleToUse.isEmpty()) {
      if (WidgetFactory *wf = dynamic_cast<qdesigner_internal::WidgetFactory *>(fw->core()->widgetFactory())) {
         if (styleToUse != wf->styleName()) {
            WidgetFactory::applyStyleToTopLevel(wf->getStyle(styleToUse), widget);
         }
      }
   }

   // Fake application style sheet by prepending. (If this does not work, fake by nesting into parent widget).
   if (! appStyleSheet.isEmpty()) {
      QString styleSheet = appStyleSheet;
      styleSheet += '\n';
      styleSheet += widget->styleSheet();

      widget->setStyleSheet(styleSheet);
   }

   return widget;
}

QWidget *QDesignerFormBuilder::createPreview(const QDesignerFormWindowInterface *fw, const QString &styleName)
{
   return createPreview(fw, styleName, QString());
}

QWidget *QDesignerFormBuilder::createPreview(const QDesignerFormWindowInterface *fw,
   const QString &styleName, const QString &appStyleSheet, QString *errorMessage)
{
   return  createPreview(fw, styleName, appStyleSheet, DeviceProfile(), errorMessage);
}

QWidget *QDesignerFormBuilder::createPreview(const QDesignerFormWindowInterface *fw, const QString &styleName,
   const QString &appStyleSheet)
{
   QString errorMessage;
   QWidget *widget = createPreview(fw, styleName, appStyleSheet, DeviceProfile(), &errorMessage);
   if (!widget && !errorMessage.isEmpty()) {
      // Display Script errors or message box
      QWidget *dialogParent = fw->core()->topLevel();
      fw->core()->dialogGui()->message(dialogParent, QDesignerDialogGuiInterface::PreviewFailureMessage,
         QMessageBox::Warning, QCoreApplication::translate("QDesignerFormBuilder", "Designer"),
         errorMessage, QMessageBox::Ok);
      return nullptr;
   }
   return widget;
}

QPixmap QDesignerFormBuilder::createPreviewPixmap(const QDesignerFormWindowInterface *fw, const QString &styleName,
   const QString &appStyleSheet)
{
   QWidget *widget = createPreview(fw, styleName, appStyleSheet);
   if (!widget) {
      return QPixmap();
   }

   const QPixmap rc = widget->grab(QRect(0, 0, -1, -1));
   widget->deleteLater();
   return rc;
}

// ---------- NewFormWidgetFormBuilder

NewFormWidgetFormBuilder::NewFormWidgetFormBuilder(QDesignerFormEditorInterface *core,
   const DeviceProfile &deviceProfile) :
   QDesignerFormBuilder(core, deviceProfile)
{
}

void NewFormWidgetFormBuilder::createCustomWidgets(DomCustomWidgets *dc)
{
   QSimpleResource::handleDomCustomWidgets(core(), dc);
}

} // namespace qdesigner_internal

