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

#include <abstract_dialoggui.h>
#include <abstract_formeditor.h>
#include <abstract_formwindowtool.h>
#include <abstract_introspection.h>
#include <abstract_language.h>
#include <container.h>
#include <designer_dockwidget.h>
#include <designer_membersheet.h>
#include <designer_menu.h>
#include <designer_menubar.h>
#include <designer_propertysheet.h>
#include <designer_resource.h>
#include <designer_stackedbox.h>
#include <designer_tabwidget.h>
#include <designer_toolbar.h>
#include <designer_toolbox.h>
#include <designer_utils.h>
#include <designer_widgetitem.h>
#include <dynamicpropertysheet.h>
#include <extension.h>
#include <extrainfo.h>
#include <formbuilderextra.h>
#include <formwindow.h>
#include <layout.h>
#include <layout_info.h>
#include <layout_propertysheet.h>
#include <layout_widget.h>
#include <mdi_area_container.h>
#include <plugin_manager.h>
#include <wizard_container.h>

#include <resource_model.h>
#include <resourcebuilder.h>
#include <textbuilder.h>
#include <ui4.h>
#include <widgetfactory.h>

#include <metadatabase_p.h>
#include <spacer_widget_p.h>
#include <widgetdatabase_p.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QBuffer>
#include <QButtonGroup>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QLayout>
#include <QMainWindow>
#include <QMdiArea>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaProperty>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabBar>
#include <QTabWidget>
#include <QToolBar>
#include <QToolBox>
#include <QWizardPage>
#include <QXmlStreamWriter>

// following two functions were lifted from gui/layout/qlayoutengine_p.h

Q_DECL_IMPORT QSize qSmartMinSize(const QSize &sizeHint, const QSize &minSizeHint,
   const QSize &minSize, const QSize &maxSize, const QSizePolicy &sizePolicy);

Q_DECL_IMPORT QSize qSmartMinSize(const QWidget *w);

CS_DECLARE_METATYPE(QWidgetList)

static const QString currentUiVersion    = "4.0";
static const QString clipboardObjectName = "__qt_fake_top_level";

namespace qdesigner_internal {

class QDesignerResourceBuilder : public QResourceBuilder
{
 public:
   QDesignerResourceBuilder(QDesignerFormEditorInterface *core, DesignerPixmapCache *pixmapCache, DesignerIconCache *iconCache);

   void setPixmapCache(DesignerPixmapCache *pixmapCache) {
      m_pixmapCache = pixmapCache;
   }
   void setIconCache(DesignerIconCache *iconCache)       {
      m_iconCache = iconCache;
   }
   bool isSaveRelative() const                           {
      return m_saveRelative;
   }
   void setSaveRelative(bool relative)                   {
      m_saveRelative = relative;
   }

   QStringList usedQrcFiles() const                      {
      return m_usedQrcFiles.keys();
   }

   QVariant loadResource(const QDir &workingDirectory, const DomProperty *icon) const override;

   QVariant toNativeValue(const QVariant &value) const override;

   DomProperty *saveResource(const QDir &workingDirectory, const QVariant &value) const override;

   bool isResourceType(const QVariant &value) const override;

 private:
   QDesignerFormEditorInterface *m_core;
   DesignerPixmapCache *m_pixmapCache;
   DesignerIconCache   *m_iconCache;

   const QDesignerLanguageExtension *m_lang;

   bool m_saveRelative;
   mutable QMap<QString, bool> m_usedQrcFiles;
   mutable QMap<QString, bool> m_loadedQrcFiles;
};

QDesignerResourceBuilder::QDesignerResourceBuilder(QDesignerFormEditorInterface *core,
   DesignerPixmapCache *pixmapCache, DesignerIconCache *iconCache)
   : m_core(core), m_pixmapCache(pixmapCache), m_iconCache(iconCache),
     m_lang(qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core)), m_saveRelative(true)
{
}

static inline void setIconPixmap(QIcon::Mode m, QIcon::State s, const QDir &workingDirectory,
   QString path, PropertySheetIconValue &icon, const QDesignerLanguageExtension *lang = nullptr)
{
   if (lang == nullptr || !lang->isLanguageResource(path)) {
      path = QFileInfo(workingDirectory, path).absoluteFilePath();
   }

   icon.setPixmap(m, s, PropertySheetPixmapValue(path));
}

QVariant QDesignerResourceBuilder::loadResource(const QDir &workingDirectory, const DomProperty *property) const
{
   switch (property->kind()) {
      case DomProperty::Pixmap: {
         PropertySheetPixmapValue pixmap;
         DomResourcePixmap *dp = property->elementPixmap();

         if (! dp->text().isEmpty()) {
            if (m_lang != nullptr && m_lang->isLanguageResource(dp->text())) {
               pixmap.setPath(dp->text());
            } else {
               pixmap.setPath(QFileInfo(workingDirectory, dp->text()).absoluteFilePath());
            }
         }
         return QVariant::fromValue(pixmap);
      }

      case DomProperty::IconSet: {
         PropertySheetIconValue icon;
         DomResourceIcon *di = property->elementIconSet();
         icon.setTheme(di->attributeTheme());

         if (const int flags = iconStateFlags(di)) {

            if (flags & NormalOff) {
               setIconPixmap(QIcon::Normal, QIcon::Off, workingDirectory, di->elementNormalOff()->text(), icon, m_lang);
            }
            if (flags & NormalOn) {
               setIconPixmap(QIcon::Normal, QIcon::On, workingDirectory, di->elementNormalOn()->text(), icon, m_lang);
            }
            if (flags & DisabledOff) {
               setIconPixmap(QIcon::Disabled, QIcon::Off, workingDirectory, di->elementDisabledOff()->text(), icon, m_lang);
            }
            if (flags & DisabledOn) {
               setIconPixmap(QIcon::Disabled, QIcon::On, workingDirectory, di->elementDisabledOn()->text(), icon, m_lang);
            }
            if (flags & ActiveOff) {
               setIconPixmap(QIcon::Active, QIcon::Off, workingDirectory, di->elementActiveOff()->text(), icon, m_lang);
            }
            if (flags & ActiveOn) {
               setIconPixmap(QIcon::Active, QIcon::On, workingDirectory, di->elementActiveOn()->text(), icon, m_lang);
            }
            if (flags & SelectedOff) {
               setIconPixmap(QIcon::Selected, QIcon::Off, workingDirectory, di->elementSelectedOff()->text(), icon, m_lang);
            }
            if (flags & SelectedOn) {
               setIconPixmap(QIcon::Selected, QIcon::On, workingDirectory, di->elementSelectedOn()->text(), icon, m_lang);
            }
         }

         return QVariant::fromValue(icon);
      }

      default:
         break;
   }

   return QVariant();
}

QVariant QDesignerResourceBuilder::toNativeValue(const QVariant &value) const
{
   if (value.canConvert<PropertySheetPixmapValue>()) {
      if (m_pixmapCache) {
         return m_pixmapCache->pixmap(value.value<PropertySheetPixmapValue>());
      }
   } else if (value.canConvert<PropertySheetIconValue>()) {
      if (m_iconCache) {
         return m_iconCache->icon(value.value<PropertySheetIconValue>());
      }
   }

   return value;
}

DomProperty *QDesignerResourceBuilder::saveResource(const QDir &workingDirectory, const QVariant &value) const
{
   DomProperty *p = new DomProperty;

   if (value.canConvert<PropertySheetPixmapValue>()) {
      const PropertySheetPixmapValue pix = value.value<PropertySheetPixmapValue>();

      DomResourcePixmap *rp = new DomResourcePixmap;
      const QString pixPath = pix.path();

      switch (pix.pixmapSource(m_core)) {

         case PropertySheetPixmapValue::LanguageResourcePixmap:
            rp->setText(pixPath);
            break;

         case PropertySheetPixmapValue::ResourcePixmap: {
            rp->setText(pixPath);
            const QString qrcFile = m_core->resourceModel()->qrcPath(pixPath);

            if (!qrcFile.isEmpty()) {
               m_usedQrcFiles.insert(qrcFile, false);
            }
         }
         break;

         case PropertySheetPixmapValue::FilePixmap:
            rp->setText(m_saveRelative ? workingDirectory.relativeFilePath(pixPath) : pixPath);
            break;
      }

      p->setElementPixmap(rp);
      return p;

   } else if (value.canConvert<PropertySheetIconValue>()) {

      const PropertySheetIconValue icon = value.value<PropertySheetIconValue>();
      const QMap<QPair<QIcon::Mode, QIcon::State>, PropertySheetPixmapValue> pixmaps = icon.paths();
      const QString theme = icon.theme();

      if (!pixmaps.isEmpty() || !theme.isEmpty()) {
         DomResourceIcon *ri = new DomResourceIcon;
         if (!theme.isEmpty()) {
            ri->setAttributeTheme(theme);
         }

         QMapIterator<QPair<QIcon::Mode, QIcon::State>, PropertySheetPixmapValue> itPix(pixmaps);
         while (itPix.hasNext()) {
            const QIcon::Mode mode = itPix.next().key().first;
            const QIcon::State state = itPix.key().second;

            DomResourcePixmap *rp = new DomResourcePixmap;

            const PropertySheetPixmapValue pix = itPix.value();
            const PropertySheetPixmapValue::PixmapSource ps = pix.pixmapSource(m_core);
            const QString pixPath = pix.path();

            rp->setText(ps == PropertySheetPixmapValue::FilePixmap && m_saveRelative ? workingDirectory.relativeFilePath(pixPath) : pixPath);

            if (state == QIcon::Off) {
               switch (mode) {
                  case QIcon::Normal:
                     ri->setElementNormalOff(rp);

                     if (ps == PropertySheetPixmapValue::ResourcePixmap) {
                        // Be sure that ri->text() file comes from active resourceSet (i.e. make appropriate
                        // resourceSet active before calling this method)

                        const QString qrcFile = m_core->resourceModel()->qrcPath(ri->text());
                        if (qrcFile.isEmpty()) {
                           m_usedQrcFiles.insert(qrcFile, false);
                        }
                     }
                     break;

                  case QIcon::Disabled:
                     ri->setElementDisabledOff(rp);
                     break;

                  case QIcon::Active:
                     ri->setElementActiveOff(rp);
                     break;

                  case QIcon::Selected:
                     ri->setElementSelectedOff(rp);
                     break;

               }
            } else {
               switch (mode) {
                  case QIcon::Normal:
                     ri->setElementNormalOn(rp);
                     break;

                  case QIcon::Disabled:
                     ri->setElementDisabledOn(rp);
                     break;

                  case QIcon::Active:
                     ri->setElementActiveOn(rp);
                     break;

                  case QIcon::Selected:
                     ri->setElementSelectedOn(rp);
                     break;
               }
            }
         }
         p->setElementIconSet(ri);
         return p;
      }
   }
   delete p;
   return nullptr;
}

bool QDesignerResourceBuilder::isResourceType(const QVariant &value) const
{
   if (value.canConvert<PropertySheetPixmapValue>() || value.canConvert<PropertySheetIconValue>()) {
      return true;
   }
   return false;
}

template <class DomElement> // for DomString, potentially DomStringList
inline void translationParametersToDom(const PropertySheetTranslatableData &data, DomElement *e)
{
   const QString propertyComment = data.disambiguation();
   if (!propertyComment.isEmpty()) {
      e->setAttributeComment(propertyComment);
   }
   const QString propertyExtracomment = data.comment();
   if (!propertyExtracomment.isEmpty()) {
      e->setAttributeExtraComment(propertyExtracomment);
   }
   if (!data.translatable()) {
      e->setAttributeNotr(QString("true"));
   }
}

template <class DomElement> // for DomString, potentially DomStringList
inline void translationParametersFromDom(const DomElement *e, PropertySheetTranslatableData *data)
{
   if (e->hasAttributeComment()) {
      data->setDisambiguation(e->attributeComment());
   }
   if (e->hasAttributeExtraComment()) {
      data->setComment(e->attributeExtraComment());
   }
   if (e->hasAttributeNotr()) {
      const QString notr = e->attributeNotr();
      const bool translatable = !(notr == QString("true") || notr == QString("yes"));
      data->setTranslatable(translatable);
   }
}

class QDesignerTextBuilder : public QTextBuilder
{
 public:
   QDesignerTextBuilder() {}

   QVariant loadText(const DomProperty *icon) const override;

   QVariant toNativeValue(const QVariant &value) const override;

   DomProperty *saveText(const QVariant &value) const override;
};

QVariant QDesignerTextBuilder::loadText(const DomProperty *text) const
{
   if (const DomString *domString = text->elementString()) {
      PropertySheetStringValue stringValue(domString->text());
      translationParametersFromDom(domString, &stringValue);
      return QVariant::fromValue(stringValue);
   }
   return QVariant(QString());
}

QVariant QDesignerTextBuilder::toNativeValue(const QVariant &value) const
{
   if (value.canConvert<PropertySheetStringValue>()) {
      return QVariant::fromValue(value.value<PropertySheetStringValue>().value());
   }

   return value;
}

static inline DomProperty *stringToDomProperty(const QString &value)
{
   DomString *domString = new DomString();
   domString->setText(value);
   DomProperty *property = new DomProperty();
   property->setElementString(domString);
   return property;
}

static inline DomProperty *stringToDomProperty(const QString &value,
   const PropertySheetTranslatableData &translatableData)
{
   DomString *domString = new DomString();
   domString->setText(value);
   translationParametersToDom(translatableData, domString);
   DomProperty *property = new DomProperty();
   property->setElementString(domString);
   return property;
}

DomProperty *QDesignerTextBuilder::saveText(const QVariant &value) const
{
   if (value.canConvert<PropertySheetStringValue>()) {
      const PropertySheetStringValue str = value.value<PropertySheetStringValue>();
      return stringToDomProperty(str.value(), str);
   }

   if (value.canConvert<QString>()) {
      return stringToDomProperty(value.toString());
   }

   return nullptr;
}

QDesignerResource::QDesignerResource(FormWindow *formWindow)
   : QEditorFormBuilder(formWindow->core()), m_formWindow(formWindow), m_isMainWidget(false),
     m_copyWidget(false), m_selected(nullptr),
     m_resourceBuilder(new QDesignerResourceBuilder(m_formWindow->core(), m_formWindow->pixmapCache(),
     m_formWindow->iconCache()))
{
   // Check language unless extension present (Jambi)
   QDesignerFormEditorInterface *core = m_formWindow->core();

   if (const QDesignerLanguageExtension *le = qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core)) {
      d->m_language = le->name();
   }

   setWorkingDirectory(formWindow->absoluteDir());
   setResourceBuilder(m_resourceBuilder);
   setTextBuilder(new QDesignerTextBuilder());

   // ### generalise
   const QString designerWidget = "QDesignerWidget";
   const QString layoutWidget   = "QLayoutWidget";
   const QString widget         = "QWidget";

   m_internal_to_qt.insert(layoutWidget, widget);
   m_internal_to_qt.insert(designerWidget, widget);
   m_internal_to_qt.insert(QString("QDesignerDialog"),     QString("QDialog"));
   m_internal_to_qt.insert(QString("QDesignerMenuBar"),    QString("QMenuBar"));
   m_internal_to_qt.insert(QString("QDesignerMenu"),       QString("QMenu"));
   m_internal_to_qt.insert(QString("QDesignerDockWidget"), QString("QDockWidget"));

   // invert
   auto cend = m_internal_to_qt.constEnd();

   for (auto it = m_internal_to_qt.constBegin(); it != cend; ++it )  {
      if (it.value() != designerWidget  && it.value() != layoutWidget) {
         m_qt_to_internal.insert(it.value(), it.key());
      }
   }
}

QDesignerResource::~QDesignerResource()
{
}

static inline QString messageBoxTitle()
{
   return QApplication::translate("Designer", "CS Designer");
}

void QDesignerResource::save(QIODevice *dev, QWidget *widget)
{
   QAbstractFormBuilder::save(dev, widget);
}

void QDesignerResource::saveDom(DomUI *ui, QWidget *widget)
{
   QAbstractFormBuilder::saveDom(ui, widget);

   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), widget);
   Q_ASSERT(sheet != nullptr);

   const QVariant classVar = sheet->property(sheet->indexOf(QString("objectName")));
   QString classStr;

   if (classVar.canConvert(QVariant::String)) {
      classStr = classVar.toString();
   } else {
      classStr = classVar.value<PropertySheetStringValue>().value();
   }

   ui->setElementClass(classStr);

   for (int index = 0; index < m_formWindow->toolCount(); ++index) {
      QDesignerFormWindowToolInterface *tool = m_formWindow->tool(index);
      Q_ASSERT(tool != nullptr);

      tool->saveToDom(ui, widget);
   }

   const QString author = m_formWindow->author();
   if (!author.isEmpty()) {
      ui->setElementAuthor(author);
   }

   const QString comment = m_formWindow->comment();
   if (!comment.isEmpty()) {
      ui->setElementComment(comment);
   }

   const QString exportMacro = m_formWindow->exportMacro();
   if (!exportMacro.isEmpty()) {
      ui->setElementExportMacro(exportMacro);
   }

   const QVariantMap designerFormData = m_formWindow->formData();
   if (! designerFormData.empty()) {
      QList<DomProperty *> domPropertyList;

      auto cend = designerFormData.constEnd();

      for (auto it = designerFormData.constBegin(); it != cend; ++it) {
         if (DomProperty *prop = variantToDomProperty(this, *(widget->metaObject()), it.key(), it.value())) {
            domPropertyList += prop;
         }
      }

      if (!domPropertyList.empty()) {
         DomDesignerData *domDesignerFormData = new DomDesignerData;
         domDesignerFormData->setElementProperty(domPropertyList);
         ui->setElementDesignerdata(domDesignerFormData);
      }
   }

   if (!m_formWindow->includeHints().isEmpty()) {
      const QString local = QString("local");
      const QString global = QString("global");
      QList<DomInclude *> ui_includes;

      for (QString includeHint : m_formWindow->includeHints()) {
         if (includeHint.isEmpty()) {
            continue;
         }

         DomInclude *incl = new DomInclude;
         const QString location = includeHint.at(0) == QLatin1Char('<') ? global : local;
         includeHint.remove(QLatin1Char('"'));
         includeHint.remove(QLatin1Char('<'));
         includeHint.remove(QLatin1Char('>'));

         incl->setAttributeLocation(location);
         incl->setText(includeHint);
         ui_includes.append(incl);
      }

      DomIncludes *includes = new DomIncludes;
      includes->setElementInclude(ui_includes);
      ui->setElementIncludes(includes);
   }

   int defaultMargin = INT_MIN, defaultSpacing = INT_MIN;
   m_formWindow->layoutDefault(&defaultMargin, &defaultSpacing);

   if (defaultMargin != INT_MIN || defaultSpacing != INT_MIN) {
      DomLayoutDefault *def = new DomLayoutDefault;
      if (defaultMargin != INT_MIN) {
         def->setAttributeMargin(defaultMargin);
      }
      if (defaultSpacing != INT_MIN) {
         def->setAttributeSpacing(defaultSpacing);
      }
      ui->setElementLayoutDefault(def);
   }

   QString marginFunction, spacingFunction;
   m_formWindow->layoutFunction(&marginFunction, &spacingFunction);
   if (!marginFunction.isEmpty() || !spacingFunction.isEmpty()) {
      DomLayoutFunction *def = new DomLayoutFunction;

      if (!marginFunction.isEmpty()) {
         def->setAttributeMargin(marginFunction);
      }
      if (!spacingFunction.isEmpty()) {
         def->setAttributeSpacing(spacingFunction);
      }
      ui->setElementLayoutFunction(def);
   }

   QString pixFunction = m_formWindow->pixmapFunction();
   if (!pixFunction.isEmpty()) {
      ui->setElementPixmapFunction(pixFunction);
   }

   if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension *>(core()->extensionManager(), core())) {
      extra->saveUiExtraInfo(ui);
   }

   if (MetaDataBase *metaDataBase = dynamic_cast<MetaDataBase *>(core()->metaDataBase())) {
      const MetaDataBaseItem *item = metaDataBase->metaDataBaseItem(m_formWindow->mainContainer());
      const QStringList fakeSlots   = item->fakeSlots();
      const QStringList fakeSignals = item->fakeSignals();

      if (!fakeSlots.empty() || !fakeSignals.empty()) {
         DomSlots *domSlots = new DomSlots();
         domSlots->setElementSlot(fakeSlots);
         domSlots->setElementSignal(fakeSignals);
         ui->setElementSlots(domSlots);
      }
   }
}

QWidget *QDesignerResource::load(QIODevice *dev, QWidget *parentWidget)
{
   QWidget *w = QEditorFormBuilder::load(dev, parentWidget);

   if (w) {
      // Store the class name as 'reset' value for the main container's object name.
      w->setProperty("_q_classname", w->objectName());
   }
   return w;
}

bool QDesignerResource::saveRelative() const
{
   return m_resourceBuilder->isSaveRelative();
}

void QDesignerResource::setSaveRelative(bool relative)
{
   m_resourceBuilder->setSaveRelative(relative);
}

QWidget *QDesignerResource::create(DomUI *ui, QWidget *parentWidget)
{
   // Load extra info extension. This is used by Jambi for preventing
   // UI files from being loaded

   if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension *>(core()->extensionManager(), core())) {
      if (!extra->loadUiExtraInfo(ui)) {
         const QString errorMessage = QApplication::translate("Designer",
               "This file cannot be read because the extra info extension failed to load.");
         core()->dialogGui()->message(parentWidget->window(), QDesignerDialogGuiInterface::FormLoadFailureMessage,
            QMessageBox::Warning, messageBoxTitle(), errorMessage, QMessageBox::Ok);

         return nullptr;
      }
   }

   qdesigner_internal::WidgetFactory *factory = dynamic_cast<qdesigner_internal::WidgetFactory *>(core()->widgetFactory());
   Q_ASSERT(factory != nullptr);

   QDesignerFormWindowInterface *previousFormWindow = factory->currentFormWindow(m_formWindow);

   m_isMainWidget = true;
   QDesignerWidgetItemInstaller wii; // Make sure we use QDesignerWidgetItem.
   QWidget *mainWidget = QAbstractFormBuilder::create(ui, parentWidget);

   if (mainWidget && m_formWindow) {
      m_formWindow->setAuthor(ui->elementAuthor());
      m_formWindow->setComment(ui->elementComment());
      m_formWindow->setExportMacro(ui->elementExportMacro());

      // Designer data
      QVariantMap designerFormData;
      if (ui->hasElementDesignerdata()) {
         const QList<DomProperty *> domPropertyList = ui->elementDesignerdata()->elementProperty();
         auto cend = domPropertyList.constEnd();

         for (auto it = domPropertyList.constBegin(); it != cend; ++it) {
            const QVariant vprop = domPropertyToVariant(this, *(mainWidget->metaObject()), *it);

            if (vprop.type() != QVariant::Invalid) {
               designerFormData.insert((*it)->attributeName(), vprop);
            }
         }
      }
      m_formWindow->setFormData(designerFormData);

      m_formWindow->setPixmapFunction(ui->elementPixmapFunction());

      if (DomLayoutDefault *def = ui->elementLayoutDefault()) {
         m_formWindow->setLayoutDefault(def->attributeMargin(), def->attributeSpacing());
      }

      if (DomLayoutFunction *fun = ui->elementLayoutFunction()) {
         m_formWindow->setLayoutFunction(fun->attributeMargin(), fun->attributeSpacing());
      }

      if (DomIncludes *includes = ui->elementIncludes()) {
         const QString global = QString("global");
         QStringList includeHints;
         for (DomInclude *incl : includes->elementInclude()) {
            QString text = incl->text();

            if (text.isEmpty()) {
               continue;
            }

            if (incl->hasAttributeLocation() && incl->attributeLocation() == global ) {
               text = text.prepend(QLatin1Char('<')).append(QLatin1Char('>'));
            } else {
               text = text.prepend(QLatin1Char('"')).append(QLatin1Char('"'));
            }

            includeHints.append(text);
         }

         m_formWindow->setIncludeHints(includeHints);
      }

      // Register all button groups the form builder adds as children of the main container for them to be found
      // in the signal slot editor
      const QObjectList mchildren = mainWidget->children();

      if (!mchildren.empty()) {
         QDesignerMetaDataBaseInterface *mdb = core()->metaDataBase();
         const QObjectList::const_iterator cend = mchildren.constEnd();

         for (QObjectList::const_iterator it = mchildren.constBegin(); it != cend; ++it)
            if (QButtonGroup *bg = dynamic_cast<QButtonGroup *>(*it)) {
               mdb->add(bg);
            }
      }

      // Load tools
      for (int index = 0; index < m_formWindow->toolCount(); ++index) {
         QDesignerFormWindowToolInterface *tool = m_formWindow->tool(index);
         Q_ASSERT(tool != nullptr);
         tool->loadFromDom(ui, mainWidget);
      }
   }

   factory->currentFormWindow(previousFormWindow);

   if (const DomSlots *domSlots = ui->elementSlots()) {
      if (MetaDataBase *metaDataBase = dynamic_cast<MetaDataBase *>(core()->metaDataBase())) {
         QStringList fakeSlots;
         QStringList fakeSignals;

         if (addFakeMethods(domSlots, fakeSlots, fakeSignals)) {
            MetaDataBaseItem *item = metaDataBase->metaDataBaseItem(mainWidget);
            item->setFakeSlots(fakeSlots);
            item->setFakeSignals(fakeSignals);
         }
      }
   }
   if (mainWidget) {
      // Initialize the mainwindow geometry. Has it been  explicitly specified?
      bool hasExplicitGeometry = false;
      const QList<DomProperty *> properties = ui->elementWidget()->elementProperty();

      if (! properties.empty()) {
         const QString geometry = "geometry";

         for (const DomProperty *p : properties)
            if (p->attributeName() == geometry) {
               hasExplicitGeometry = true;
               break;
            }
      }
      if (hasExplicitGeometry) {
         // Geometry was specified explicitly: Verify that smartMinSize is respected
         // (changed fonts, label wrapping policies, etc). This does not happen automatically in docked mode.

         const QSize size    = mainWidget->size();
         const QSize minSize = size.expandedTo(qSmartMinSize(mainWidget));

         if (minSize != size) {
            mainWidget->resize(minSize);
         }

      } else {
         // No explicit Geometry: perform an adjustSize() to resize the form correctly before embedding it into a container
         // (which might otherwise squeeze the form)
         mainWidget->adjustSize();
      }

      // Some integration wizards create forms with main containers
      // based on derived classes of QWidget and load them into Designer
      // without the plugin existing. This will trigger the auto-promotion
      // mechanism of Designer, which will set container=false for
      // QWidgets. For the main container, force container=true and warn.

      const QDesignerWidgetDataBaseInterface *wdb = core()->widgetDataBase();
      const int wdbIndex = wdb->indexOfObject(mainWidget);

      if (wdbIndex != -1) {
         QDesignerWidgetDataBaseItemInterface *item = wdb->item(wdbIndex);
         // Promoted main container that is not of container type
         if (item->isPromoted() && !item->isContainer()) {
            item->setContainer(true);

            qWarning("Main container is an unknown custom widget %s."
               " Defaulting to a promoted instance of %s",
               csPrintable(item->name()), csPrintable(item->extends()));
         }
      }
   }
   return mainWidget;
}

QWidget *QDesignerResource::create(DomWidget *ui_widget, QWidget *parentWidget)
{
   const QString className = ui_widget->attributeClass();

   if (! m_isMainWidget && className == "QWidget" && ui_widget->elementLayout().size() &&
         ! ui_widget->hasAttributeNative()) {

      // ### check if elementLayout.size() == 1

      QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), parentWidget);

      if (container == nullptr) {
         // generate a QLayoutWidget iff the parent is not an QDesignerContainerExtension.
         ui_widget->setAttributeClass("QLayoutWidget");
      }
   }

   // save the actions
   const QList<DomActionRef *> actionRefs = ui_widget->elementAddAction();
   ui_widget->setElementAddAction(QList<DomActionRef *>());

   QWidget *w = QAbstractFormBuilder::create(ui_widget, parentWidget);

   // restore the actions
   ui_widget->setElementAddAction(actionRefs);

   if (w == nullptr) {
      return nullptr;
   }

   // generalize using the extension manager
   QDesignerMenu *menu       = dynamic_cast<QDesignerMenu *>(w);
   QDesignerMenuBar *menuBar = dynamic_cast<QDesignerMenuBar *>(w);

   if (menu) {
      menu->interactive(false);
      menu->hide();
   } else if (menuBar) {
      menuBar->interactive(false);
   }

   for (DomActionRef *ui_action_ref : actionRefs) {
      const QString name = ui_action_ref->attributeName();

      if (name == QString("separator")) {
         QAction *sep = new QAction(w);
         sep->setSeparator(true);
         w->addAction(sep);
         addMenuAction(sep);

      } else if (QAction *a = d->m_actions.value(name)) {
         w->addAction(a);

      } else if (QActionGroup *g = d->m_actionGroups.value(name)) {
         w->addActions(g->actions());

      } else if (QMenu *childMenu = w->findChild<QMenu *>(name)) {
         w->addAction(childMenu->menuAction());
         addMenuAction(childMenu->menuAction());
      }
   }

   if (menu) {
      menu->interactive(true);
      menu->adjustSpecialActions();

   } else if (menuBar) {
      menuBar->interactive(true);
      menuBar->adjustSpecialActions();
   }

   ui_widget->setAttributeClass(className); // fix the class name
   applyExtensionDataFromDOM(this, core(), ui_widget, w);

   return w;
}

QLayout *QDesignerResource::create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget)
{
   QLayout *newLayout = QAbstractFormBuilder::create(ui_layout, layout, parentWidget);

   if (QGridLayout *gridLayout = dynamic_cast<QGridLayout *>(newLayout)) {
      QLayoutSupport::createEmptyCells(gridLayout);

   } else {
      if (QFormLayout *formLayout = dynamic_cast<QFormLayout *>(newLayout)) {
         QLayoutSupport::createEmptyCells(formLayout);
      }
   }

   // While the actual values are applied by the form builder, we still need
   // to mark them as 'changed'.
   LayoutPropertySheet::markChangedStretchProperties(core(), newLayout, ui_layout);

   return newLayout;
}

QLayoutItem *QDesignerResource::create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget)
{
   if (ui_layoutItem->kind() == DomLayoutItem::Spacer) {
      const DomSpacer *domSpacer = ui_layoutItem->elementSpacer();

      Spacer *spacer = static_cast<Spacer *>(core()->widgetFactory()->createWidget("Spacer", parentWidget));

      if (domSpacer->hasAttributeName()) {
         changeObjectName(spacer, domSpacer->attributeName());
      }

      core()->metaDataBase()->add(spacer);

      spacer->setInteractiveMode(false);
      applyProperties(spacer, ui_layoutItem->elementSpacer()->elementProperty());
      spacer->setInteractiveMode(true);

      if (m_formWindow) {
         m_formWindow->manageWidget(spacer);

         if (QDesignerPropertySheetExtension *sheet =
                  qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), spacer)) {

            sheet->setChanged(sheet->indexOf("orientation"), true);
         }
      }

      return new QWidgetItem(spacer);

   } else if (ui_layoutItem->kind() == DomLayoutItem::Layout && parentWidget) {
      DomLayout *ui_layout = ui_layoutItem->elementLayout();
      QLayoutWidget *layoutWidget = new QLayoutWidget(m_formWindow, parentWidget);
      core()->metaDataBase()->add(layoutWidget);

      if (m_formWindow) {
         m_formWindow->manageWidget(layoutWidget);
      }

      (void) create(ui_layout, nullptr, layoutWidget);
      return new QWidgetItem(layoutWidget);
   }

   return QAbstractFormBuilder::create(ui_layoutItem, layout, parentWidget);
}

void QDesignerResource::changeObjectName(QObject *obj, QString objName)
{
   m_formWindow->unify(obj, objName, true);
   obj->setObjectName(objName);
}

/* If the property is a enum or flag value, retrieve
 * the existing enum/flag via property sheet and use it to convert */

static bool readDomEnumerationValue(const DomProperty *p, const QDesignerPropertySheetExtension *sheet,
         int index, QVariant &v)
{
   switch (p->kind()) {
      case DomProperty::Set: {
         const QVariant sheetValue = sheet->property(index);

         if (sheetValue.canConvert<PropertySheetFlagValue>()) {
            const PropertySheetFlagValue f = sheetValue.value<PropertySheetFlagValue>();

            bool ok = false;
            v = f.metaFlags.parseFlags(p->elementSet(), &ok);

            if (!ok) {
               f.metaFlags.messageParseFailed(p->elementSet());
            }

            return true;
         }
      }
      break;

      case DomProperty::Enum: {
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

void QDesignerResource::applyProperties(QObject *o, const QList<DomProperty *> &properties)
{
   if (properties.empty()) {
      return;
   }

   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), o);
   if (! sheet) {
      return;
   }

   QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension *>
      (core()->extensionManager(), o);

   const bool dynamicPropertiesAllowed = dynamicSheet && dynamicSheet->dynamicPropertiesAllowed();

   const QString objectNameProperty = "objectName";
   auto cend = properties.constEnd();

   for (auto it = properties.constBegin(); it != cend; ++it) {
      const DomProperty *p = *it;
      QString propertyName  = p->attributeName();

      if (propertyName == "numDigits" && o->inherits("QLCDNumber"))  {
         // update old property name
         propertyName = "digitCount";
      }

      const int index = sheet->indexOf(propertyName);
      QVariant v;

      if (! readDomEnumerationValue(p, sheet, index, v)) {
         v = toVariant(*(o->metaObject()), *it);
      }

      switch (p->kind()) {

         case DomProperty::String:
            if (index != -1 && sheet->property(index).userType() == QVariant::typeToTypeId<PropertySheetKeySequenceValue>()) {
               const DomString *key = p->elementString();
               PropertySheetKeySequenceValue keyVal(QKeySequence(key->text()));
               translationParametersFromDom(key, &keyVal);
               v = QVariant::fromValue(keyVal);

            } else {
               const DomString *str = p->elementString();
               PropertySheetStringValue strVal(v.toString());
               translationParametersFromDom(str, &strVal);
               v = QVariant::fromValue(strVal);
            }
            break;

         case DomProperty::StringList: {
            const DomStringList *list = p->elementStringList();
            PropertySheetStringListValue listValue(list->elementString());
            translationParametersFromDom(list, &listValue);
            v = QVariant::fromValue(listValue);
         }
         break;
         default:
            break;
      }

      d->applyPropertyInternally(o, propertyName, v);

      if (index != -1) {
         sheet->setProperty(index, v);
         sheet->setChanged(index, true);

      } else if (dynamicPropertiesAllowed) {
         QVariant defaultValue = QVariant(v.type());
         bool isDefault = (v == defaultValue);

         if (v.canConvert<PropertySheetIconValue>()) {
            defaultValue = QVariant(QVariant::Icon);
            isDefault = (v.value<PropertySheetIconValue>() == PropertySheetIconValue());

         } else if (v.canConvert<PropertySheetPixmapValue>()) {
            defaultValue = QVariant(QVariant::Pixmap);
            isDefault = (v.value<PropertySheetPixmapValue>() == PropertySheetPixmapValue());

         } else if (v.canConvert<PropertySheetStringValue>()) {
            defaultValue = QVariant(QVariant::String);
            isDefault = (v.value<PropertySheetStringValue>() == PropertySheetStringValue());

         } else if (v.canConvert<PropertySheetStringListValue>()) {
            defaultValue = QVariant(QVariant::StringList);
            isDefault = (v.value<PropertySheetStringListValue>() == PropertySheetStringListValue());

         } else if (v.canConvert<PropertySheetKeySequenceValue>()) {
            defaultValue = QVariant(QVariant::KeySequence);
            isDefault = (v.value<PropertySheetKeySequenceValue>() == PropertySheetKeySequenceValue());
         }

         if (defaultValue.type() != QVariant::UserType) {
            const int idx = dynamicSheet->addDynamicProperty(p->attributeName(), defaultValue);

            if (idx != -1) {
               sheet->setProperty(idx, v);
               sheet->setChanged(idx, !isDefault);
            }
         }
      }

      if (propertyName == objectNameProperty) {
         changeObjectName(o, o->objectName());
      }
   }
}

QWidget *QDesignerResource::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &_name)
{
   QString name = _name;
   QString className = widgetName;

   if (m_isMainWidget) {
      m_isMainWidget = false;
   }

   QWidget *w = core()->widgetFactory()->createWidget(className, parentWidget);
   if (! w) {
      return nullptr;
   }

   if (name.isEmpty()) {
      QDesignerWidgetDataBaseInterface *db = core()->widgetDataBase();

      if (QDesignerWidgetDataBaseItemInterface *item = db->item(db->indexOfObject(w))) {
         name = refactorClassName(item->name());
      }
   }

   changeObjectName(w, name);

   QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), parentWidget);
   if (! dynamic_cast<QMenu *>(w) && (!parentWidget || !container)) {
      m_formWindow->manageWidget(w);

      if (parentWidget) {
         QVariant tmp = parentWidget->property("_q_widgetOrder");
         QList<QWidget *> list = tmp.value<QWidgetList>();

         list.append(w);
         parentWidget->setProperty("_q_widgetOrder", QVariant::fromValue(list));

         tmp = parentWidget->property("_q_zOrder");
         QList<QWidget *> zOrder = tmp.value<QWidgetList>();

         zOrder.append(w);
         parentWidget->setProperty("_q_zOrder", QVariant::fromValue(zOrder));
      }

   } else {
      core()->metaDataBase()->add(w);
   }

   w->setWindowFlags(w->windowFlags() & ~Qt::Window);

   // Make sure it is non-modal (for example, KDialog calls setModal(true) in the constructor).
   w->setWindowModality(Qt::NonModal);

   return w;
}

QLayout *QDesignerResource::createLayout(const QString &layoutName, QObject *parent, const QString &name)
{
   QWidget *layoutBase = nullptr;
   QLayout *layout = dynamic_cast<QLayout *>(parent);

   if (parent->isWidgetType()) {
      layoutBase = static_cast<QWidget *>(parent);
   } else {
      Q_ASSERT( layout != nullptr );
      layoutBase = layout->parentWidget();
   }

   LayoutInfo::Type layoutType = LayoutInfo::layoutType(layoutName);

   if (layoutType == LayoutInfo::NoLayout) {
      csWarning(QCoreApplication::translate("QDesignerResource",
            "The layout type '%1' is not supported, defaulting to grid.").formatArg(layoutName));

      layoutType = LayoutInfo::Grid;
   }

   QLayout *lay = core()->widgetFactory()->createLayout(layoutBase, layout, layoutType);

   if (lay != nullptr) {
      changeObjectName(lay, name);
   }

   return lay;
}

// save
DomWidget * QDesignerResource::createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive)
{
   QDesignerMetaDataBaseItemInterface *item = core()->metaDataBase()->item(widget);
   if (! item) {
      return nullptr;
   }

   if (dynamic_cast<Spacer *>(widget) && m_copyWidget == false) {
      return nullptr;
   }

   const QDesignerWidgetDataBaseInterface *wdb = core()->widgetDataBase();
   QDesignerWidgetDataBaseItemInterface *widgetInfo =  nullptr;
   const int widgetInfoIndex = wdb->indexOfObject(widget, false);

   if (widgetInfoIndex != -1) {
      widgetInfo = wdb->item(widgetInfoIndex);

      // Recursively add all dependent custom widgets
      QDesignerWidgetDataBaseItemInterface *customInfo = widgetInfo;

      while (customInfo && customInfo->isCustom()) {
         m_usedCustomWidgets.insert(customInfo, true);
         const QString extends = customInfo->extends();

         if (extends == customInfo->name()) {
            // There are faulty files around that have name==extends
            break;

         } else {
            const int extendsIndex = wdb->indexOfClassName(customInfo->extends());

            customInfo = extendsIndex != -1 ?  wdb->item(extendsIndex) : nullptr;
         }
      }
   }

   DomWidget *w = nullptr;

   if (QTabWidget *tabWidget = dynamic_cast<QTabWidget *>(widget)) {
      w = saveWidget(tabWidget, ui_parentWidget);

   } else if (QStackedWidget *stackedWidget = dynamic_cast<QStackedWidget *>(widget)) {
      w = saveWidget(stackedWidget, ui_parentWidget);

   } else if (QToolBox *toolBox = dynamic_cast<QToolBox *>(widget)) {
      w = saveWidget(toolBox, ui_parentWidget);

   } else if (QToolBar *toolBar = dynamic_cast<QToolBar *>(widget)) {
      w = saveWidget(toolBar, ui_parentWidget);

   } else if (QDesignerDockWidget *dockWidget = dynamic_cast<QDesignerDockWidget *>(widget)) {
      w = saveWidget(dockWidget, ui_parentWidget);

   } else if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), widget)) {
      w = saveWidget(widget, container, ui_parentWidget);

   } else if (QWizardPage *wizardPage = dynamic_cast<QWizardPage *>(widget)) {
      w = saveWidget(wizardPage, ui_parentWidget);

   } else {
      w = QAbstractFormBuilder::createDom(widget, ui_parentWidget, recursive);
   }

   Q_ASSERT(w != nullptr);

   if (! dynamic_cast<QLayoutWidget *>(widget) && w->attributeClass() == QString("QWidget")) {
      w->setAttributeNative(true);
   }

   const QString className = w->attributeClass();
   if (m_internal_to_qt.contains(className)) {
      w->setAttributeClass(m_internal_to_qt.value(className));
   }

   w->setAttributeName(widget->objectName());

   if (isPromoted( core(), widget)) { // is promoted?
      Q_ASSERT(widgetInfo != nullptr);

      w->setAttributeName(widget->objectName());
      w->setAttributeClass(widgetInfo->name());

      QList<DomProperty *> prop_list = w->elementProperty();

      for (DomProperty *prop : prop_list) {
         if (prop->attributeName() == QString("geometry")) {
            if (DomRect *rect = prop->elementRect()) {
               rect->setElementX(widget->x());
               rect->setElementY(widget->y());
            }
            break;
         }
      }

   } else if (widgetInfo != nullptr && m_usedCustomWidgets.contains(widgetInfo)) {
      if (widgetInfo->name() != w->attributeClass()) {
         w->setAttributeClass(widgetInfo->name());
      }
   }
   addExtensionDataToDOM(this, core(), w, widget);
   return w;
}

DomLayout *QDesignerResource::createDom(QLayout *layout, DomLayout *ui_parentLayout, DomWidget *ui_parentWidget)
{
   QDesignerMetaDataBaseItemInterface *item = core()->metaDataBase()->item(layout);

   if (item == nullptr) {
      layout = layout->findChild<QLayout *>();
      // refresh the meta database item
      item = core()->metaDataBase()->item(layout);
   }

   if (item == nullptr) {
      // nothing to do
      return nullptr;
   }

   if (dynamic_cast<QSplitter *>(layout->parentWidget()) != nullptr) {
      // nothing to do.
      return nullptr;
   }

   m_chain.push(layout);

   DomLayout *domLayout = QAbstractFormBuilder::createDom(layout, ui_parentLayout, ui_parentWidget);
   Q_ASSERT(domLayout != nullptr);

   LayoutPropertySheet::stretchAttributesToDom(core(), layout, domLayout);

   m_chain.pop();

   return domLayout;
}

DomLayoutItem *QDesignerResource::createDom(QLayoutItem *item, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
   DomLayoutItem *ui_item = nullptr;

   if (Spacer *s = dynamic_cast<Spacer *>(item->widget())) {
      if (!core()->metaDataBase()->item(s)) {
         return nullptr;
      }

      DomSpacer *spacer = new DomSpacer();
      const QString objectName = s->objectName();

      if (!objectName.isEmpty()) {
         spacer->setAttributeName(objectName);
      }

      const QList<DomProperty *> properties = computeProperties(item->widget());

      // ### filter the properties
      spacer->setElementProperty(properties);

      ui_item = new DomLayoutItem();
      ui_item->setElementSpacer(spacer);
      d->m_laidout.insert(item->widget(), true);

   } else if (QLayoutWidget *layoutWidget = dynamic_cast<QLayoutWidget *>(item->widget())) {
      // Do not save a QLayoutWidget if it is within a layout (else it is saved as "QWidget"
      Q_ASSERT(layoutWidget->layout());

      DomLayout *domLayout = createDom(layoutWidget->layout(), ui_layout, ui_parentWidget);
      ui_item = new DomLayoutItem();
      ui_item->setElementLayout(domLayout);
      d->m_laidout.insert(item->widget(), true);

   } else if (! item->spacerItem()) {
      // we use spacer as fake item in the Designer
      ui_item = QAbstractFormBuilder::createDom(item, ui_layout, ui_parentWidget);

   } else {
      return nullptr;
   }

   return ui_item;
}

void QDesignerResource::createCustomWidgets(DomCustomWidgets *dom_custom_widgets)
{
   QSimpleResource::handleDomCustomWidgets(core(), dom_custom_widgets);
}

DomTabStops *QDesignerResource::saveTabStops()
{
   QDesignerMetaDataBaseItemInterface *item = core()->metaDataBase()->item(m_formWindow);
   Q_ASSERT(item);

   QStringList tabStops;
   for (QWidget *widget : item->tabOrder()) {
      if (m_formWindow->mainContainer()->isAncestorOf(widget)) {
         tabStops.append(widget->objectName());
      }
   }

   if (tabStops.count()) {
      DomTabStops *dom = new DomTabStops;
      dom->setElementTabStop(tabStops);
      return dom;
   }

   return nullptr;
}

void QDesignerResource::applyTabStops(QWidget *widget, DomTabStops *tabStops)
{
   if (!tabStops) {
      return;
   }

   QList<QWidget *> tabOrder;
   for (const QString &widgetName : tabStops->elementTabStop()) {
      if (QWidget *w = widget->findChild<QWidget *>(widgetName)) {
         tabOrder.append(w);
      }
   }

   QDesignerMetaDataBaseItemInterface *item = core()->metaDataBase()->item(m_formWindow);
   Q_ASSERT(item);
   item->setTabOrder(tabOrder);
}

/* Unmanaged container pages occur when someone adds a page in a custom widget
 * constructor. They don't have a meta DB entry which causes createDom to return 0. */
inline QString msgUnmanagedPage(QDesignerFormEditorInterface *core,
   QWidget *container, int index, QWidget *page)
{
   return QCoreApplication::translate("QDesignerResource",
         "The container extension of the widget '%1' (%2) returned a widget not managed by Designer '%3' (%4) when queried for page #%5.\n"
         "Container pages should only be added by specifying them in XML returned by the domXml() method of the custom widget.")
      .formatArgs(container->objectName(), WidgetFactory::classNameOf(core, container), page->objectName(), WidgetFactory::classNameOf(core,
            page))
      .formatArg(index);
}

DomWidget *QDesignerResource::saveWidget(QWidget *widget, QDesignerContainerExtension *container, DomWidget *ui_parentWidget)
{
   DomWidget *ui_widget = QAbstractFormBuilder::createDom(widget, ui_parentWidget, false);
   QList<DomWidget *> ui_widget_list;

   for (int i = 0; i < container->count(); ++i) {
      QWidget *page = container->widget(i);
      Q_ASSERT(page);

      if (DomWidget *ui_page = createDom(page, ui_widget)) {
         ui_widget_list.append(ui_page);
      } else {
         csWarning(msgUnmanagedPage(core(), widget, i, page));
      }
   }

   ui_widget->setElementWidget(ui_widget_list);

   return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QStackedWidget *widget, DomWidget *ui_parentWidget)
{
   DomWidget *ui_widget = QAbstractFormBuilder::createDom(widget, ui_parentWidget, false);
   QList<DomWidget *> ui_widget_list;

   if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), widget)) {
      for (int i = 0; i < container->count(); ++i) {
         QWidget *page = container->widget(i);
         Q_ASSERT(page);

         if (DomWidget *ui_page = createDom(page, ui_widget)) {
            ui_widget_list.append(ui_page);
         } else {
            csWarning(msgUnmanagedPage(core(), widget, i, page));
         }
      }
   }

   ui_widget->setElementWidget(ui_widget_list);

   return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QToolBar *toolBar, DomWidget *ui_parentWidget)
{
   DomWidget *ui_widget = QAbstractFormBuilder::createDom(toolBar, ui_parentWidget, false);

   if (const QMainWindow *mainWindow = dynamic_cast<QMainWindow *>(toolBar->parentWidget())) {
      const bool toolBarBreak    = mainWindow->toolBarBreak(toolBar);
      const Qt::ToolBarArea area = mainWindow->toolBarArea(toolBar);

      QList<DomProperty *> attributes = ui_widget->elementAttribute();

      DomProperty *attr = new DomProperty();
      attr->setAttributeName("toolBarArea");

      attr->setElementEnum(QString(toolBarAreaMetaEnum().valueToKey(area)));
      attributes << attr;

      attr = new DomProperty();
      attr->setAttributeName("toolBarBreak");
      attr->setElementBool(toolBarBreak ? QString("true") : QString("false"));
      attributes << attr;
      ui_widget->setElementAttribute(attributes);
   }

   return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QDesignerDockWidget *dockWidget, DomWidget *ui_parentWidget)
{
   DomWidget *ui_widget = QAbstractFormBuilder::createDom(dockWidget, ui_parentWidget, true);

   if (QMainWindow *mainWindow = dynamic_cast<QMainWindow *>(dockWidget->parentWidget())) {
      const Qt::DockWidgetArea area = mainWindow->dockWidgetArea(dockWidget);
      DomProperty *attr = new DomProperty();
      attr->setAttributeName("dockWidgetArea");
      attr->setElementNumber(int(area));
      ui_widget->setElementAttribute(ui_widget->elementAttribute() << attr);
   }

   return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QTabWidget *widget, DomWidget *ui_parentWidget)
{
   DomWidget *ui_widget = QAbstractFormBuilder::createDom(widget, ui_parentWidget, false);
   QList<DomWidget *> ui_widget_list;

   if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), widget)) {
      const int current = widget->currentIndex();
      for (int i = 0; i < container->count(); ++i) {
         QWidget *page = container->widget(i);
         Q_ASSERT(page);

         DomWidget *ui_page = createDom(page, ui_widget);
         if (!ui_page) {
            csWarning(msgUnmanagedPage(core(), widget, i, page));
            continue;
         }
         QList<DomProperty *> ui_attribute_list;

         const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

         // attribute `icon'
         widget->setCurrentIndex(i);
         QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), widget);

         QVariant tmp = sheet->property(sheet->indexOf("currentTabIcon"));
         PropertySheetIconValue icon = tmp.value<PropertySheetIconValue>();

         DomProperty *p = resourceBuilder()->saveResource(workingDirectory(), QVariant::fromValue(icon));

         if (p) {
            p->setAttributeName(strings.iconAttribute);
            ui_attribute_list.append(p);
         }

         // attribute `title'
         p = textBuilder()->saveText(sheet->property(sheet->indexOf("currentTabText")));
         if (p) {

            p->setAttributeName(strings.titleAttribute);
            ui_attribute_list.append(p);
         }

         // attribute `toolTip'
         QVariant v = sheet->property(sheet->indexOf(QString("currentTabToolTip")));
         if (! v.value<PropertySheetStringValue>().value().isEmpty()) {
            p = textBuilder()->saveText(v);

            if (p) {
               p->setAttributeName(strings.toolTipAttribute);
               ui_attribute_list.append(p);
            }
         }

         // attribute `whatsThis'
         v = sheet->property(sheet->indexOf(QString("currentTabWhatsThis")));
         if (! v.value<PropertySheetStringValue>().value().isEmpty()) {
            p = textBuilder()->saveText(v);

            if (p) {
               p->setAttributeName(strings.whatsThisAttribute);

               ui_attribute_list.append(p);
            }
         }

         ui_page->setElementAttribute(ui_attribute_list);

         ui_widget_list.append(ui_page);
      }
      widget->setCurrentIndex(current);
   }

   ui_widget->setElementWidget(ui_widget_list);

   return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QToolBox *widget, DomWidget *ui_parentWidget)
{
   DomWidget *ui_widget = QAbstractFormBuilder::createDom(widget, ui_parentWidget, false);
   QList<DomWidget *> ui_widget_list;

   if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), widget)) {
      const int current = widget->currentIndex();
      for (int i = 0; i < container->count(); ++i) {
         QWidget *page = container->widget(i);
         Q_ASSERT(page);

         DomWidget *ui_page = createDom(page, ui_widget);
         if (!ui_page) {
            csWarning(msgUnmanagedPage(core(), widget, i, page));
            continue;
         }

         // attribute `label'
         QList<DomProperty *> ui_attribute_list;

         const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

         // attribute `icon'
         widget->setCurrentIndex(i);
         QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), widget);

         QVariant tmp = sheet->property(sheet->indexOf("currentItemIcon"));
         PropertySheetIconValue icon = tmp.value<PropertySheetIconValue>();

         DomProperty *p = resourceBuilder()->saveResource(workingDirectory(), QVariant::fromValue(icon));

         if (p) {
            p->setAttributeName(strings.iconAttribute);
            ui_attribute_list.append(p);
         }

         p = textBuilder()->saveText(sheet->property(sheet->indexOf(QString("currentItemText"))));

         if (p) {
            p->setAttributeName(strings.labelAttribute);
            ui_attribute_list.append(p);
         }

         // attribute `toolTip'
         QVariant v = sheet->property(sheet->indexOf(QString("currentItemToolTip")));
         if (! v.value<PropertySheetStringValue>().value().isEmpty()) {
            p = textBuilder()->saveText(v);
            if (p) {
               p->setAttributeName(strings.toolTipAttribute);
               ui_attribute_list.append(p);
            }
         }

         ui_page->setElementAttribute(ui_attribute_list);

         ui_widget_list.append(ui_page);
      }
      widget->setCurrentIndex(current);
   }

   ui_widget->setElementWidget(ui_widget_list);

   return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QWizardPage *wizardPage, DomWidget *ui_parentWidget)
{
   DomWidget *ui_widget = QAbstractFormBuilder::createDom(wizardPage, ui_parentWidget, true);
   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), wizardPage);

   // Save the page id (string) attribute, append to existing attributes
   const QString pageIdPropertyName = QWizardPagePropertySheet::pageIdProperty;
   const int pageIdIndex = sheet->indexOf(pageIdPropertyName);

   if (pageIdIndex != -1 && sheet->isChanged(pageIdIndex)) {
      DomProperty *property = variantToDomProperty(this, *(wizardPage->metaObject()), pageIdPropertyName, sheet->property(pageIdIndex));
      Q_ASSERT(property);

      property->elementString()->setAttributeNotr("true");

      QList<DomProperty *> attributes = ui_widget->elementAttribute();
      attributes.push_back(property);
      ui_widget->setElementAttribute(attributes);
   }

   return ui_widget;
}

// Do not save the 'currentTabName' properties of containers
static inline bool checkContainerProperty(const QWidget *w, const QString &propertyName)
{
   if (dynamic_cast<const QToolBox *>(w)) {
      return QToolBoxWidgetPropertySheet::checkProperty(propertyName);
   }

   if (dynamic_cast<const QTabWidget *>(w)) {
      return QTabWidgetPropertySheet::checkProperty(propertyName);
   }

   if (dynamic_cast<const QStackedWidget *>(w)) {
      return QStackedWidgetPropertySheet::checkProperty(propertyName);
   }

   if (dynamic_cast<const QMdiArea *>(w)) {
      return QMdiAreaPropertySheet::checkProperty(propertyName);
   }

   return true;
}

bool QDesignerResource::checkProperty(QObject *obj, const QString &prop) const
{
   const QDesignerMetaObjectInterface *meta = core()->introspection()->metaObject(obj);

   int pindex = meta->indexOfProperty(prop);

   if (pindex != -1 && !(meta->property(pindex)->attributes(obj) & QDesignerMetaPropertyInterface::StoredAttribute)) {
      return false;
   }

   if (prop == QString("objectName") || prop == QString("spacerName")) {
      // do not store the property objectName
      return false;
   }

   QWidget *check_widget = nullptr;

   if (obj->isWidgetType()) {
      check_widget = static_cast<QWidget *>(obj);
   }

   if (check_widget && prop == QString("geometry")) {
      if (check_widget == m_formWindow->mainContainer()) {
         return true;   // Save although maincontainer is technically laid-out by embedding container
      }

      if (m_selected && m_selected == check_widget) {
         return true;
      }

      return !LayoutInfo::isWidgetLaidout(core(), check_widget);
   }

   if (check_widget && !checkContainerProperty(check_widget, prop)) {
      return false;
   }

   if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), obj)) {

      QDesignerDynamicPropertySheetExtension *dynamicSheet =
            qt_extension<QDesignerDynamicPropertySheetExtension *>(core()->extensionManager(), obj);

      pindex = sheet->indexOf(prop);

      if (sheet->isAttribute(pindex)) {
         return false;
      }

      if (! dynamicSheet || ! dynamicSheet->isDynamicProperty(pindex)) {
         return sheet->isChanged(pindex);
      }

      if (! sheet->isVisible(pindex)) {
         return false;
      }

      return true;
   }

   return false;
}

bool QDesignerResource::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
   if (item->widget() == nullptr) {
      return false;
   }

   QGridLayout *grid = dynamic_cast<QGridLayout *>(layout);
   QBoxLayout *box    = dynamic_cast<QBoxLayout *>(layout);

   if (grid != nullptr) {
      const int rowSpan = ui_item->hasAttributeRowSpan() ? ui_item->attributeRowSpan() : 1;
      const int colSpan = ui_item->hasAttributeColSpan() ? ui_item->attributeColSpan() : 1;
      grid->addWidget(item->widget(), ui_item->attributeRow(), ui_item->attributeColumn(), rowSpan, colSpan, item->alignment());

      return true;

   } else if (box != nullptr) {
      box->addItem(item);

      return true;
   }

   return QAbstractFormBuilder::addItem(ui_item, item, layout);
}

bool QDesignerResource::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
   core()->metaDataBase()->add(widget); // ensure the widget is in the meta database

   if (! QAbstractFormBuilder::addItem(ui_widget, widget, parentWidget) || dynamic_cast<QMainWindow *> (parentWidget)) {
      if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), parentWidget)) {
         container->addWidget(widget);
      }
   }

   if (QTabWidget *tabWidget = dynamic_cast<QTabWidget *>(parentWidget)) {
      const int tabIndex = tabWidget->count() - 1;
      const int current = tabWidget->currentIndex();

      tabWidget->setCurrentIndex(tabIndex);

      const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

      const DomPropertyHash attributes = propertyMap(ui_widget->elementAttribute());
      QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), parentWidget);
      if (DomProperty *picon = attributes.value(strings.iconAttribute)) {
         QVariant v = resourceBuilder()->loadResource(workingDirectory(), picon);
         sheet->setProperty(sheet->indexOf(QString("currentTabIcon")), v);
      }
      if (DomProperty *ptext = attributes.value(strings.titleAttribute)) {
         QVariant v = textBuilder()->loadText(ptext);
         sheet->setProperty(sheet->indexOf(QString("currentTabText")), v);
      }
      if (DomProperty *ptext = attributes.value(strings.toolTipAttribute)) {
         QVariant v = textBuilder()->loadText(ptext);
         sheet->setProperty(sheet->indexOf(QString("currentTabToolTip")), v);
      }
      if (DomProperty *ptext = attributes.value(strings.whatsThisAttribute)) {
         QVariant v = textBuilder()->loadText(ptext);
         sheet->setProperty(sheet->indexOf(QString("currentTabWhatsThis")), v);
      }
      tabWidget->setCurrentIndex(current);
   } else if (QToolBox *toolBox = dynamic_cast<QToolBox *>(parentWidget)) {
      const int itemIndex = toolBox->count() - 1;
      const int current = toolBox->currentIndex();

      toolBox->setCurrentIndex(itemIndex);

      const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

      const DomPropertyHash attributes = propertyMap(ui_widget->elementAttribute());
      QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), parentWidget);
      if (DomProperty *picon = attributes.value(strings.iconAttribute)) {
         QVariant v = resourceBuilder()->loadResource(workingDirectory(), picon);
         sheet->setProperty(sheet->indexOf(QString("currentItemIcon")), v);
      }
      if (DomProperty *ptext = attributes.value(strings.labelAttribute)) {
         QVariant v = textBuilder()->loadText(ptext);
         sheet->setProperty(sheet->indexOf(QString("currentItemText")), v);
      }
      if (DomProperty *ptext = attributes.value(strings.toolTipAttribute)) {
         QVariant v = textBuilder()->loadText(ptext);
         sheet->setProperty(sheet->indexOf(QString("currentItemToolTip")), v);
      }
      toolBox->setCurrentIndex(current);
   }

   return true;
}

bool QDesignerResource::copy(QIODevice *dev, const FormBuilderClipboard &selection)
{
   m_copyWidget = true;

   DomUI *ui = copy(selection);

   d->m_laidout.clear();
   m_copyWidget = false;

   if (!ui) {
      return false;
   }

   QXmlStreamWriter writer(dev);
   writer.setAutoFormatting(true);
   writer.setAutoFormattingIndent(1);
   writer.writeStartDocument();
   ui->write(writer);
   writer.writeEndDocument();
   delete ui;
   return true;
}

DomUI *QDesignerResource::copy(const FormBuilderClipboard &selection)
{
   if (selection.empty()) {
      return nullptr;
   }

   m_copyWidget = true;

   DomWidget *ui_widget = new DomWidget();
   ui_widget->setAttributeName(clipboardObjectName);
   bool hasItems = false;

   // Widgets
   if (! selection.m_widgets.empty()) {
      QList<DomWidget *> ui_widget_list;
      const int size = selection.m_widgets.size();

      for (int i = 0; i < size; ++i) {
         QWidget *w = selection.m_widgets.at(i);
         m_selected = w;

         DomWidget *ui_child = createDom(w, ui_widget);
         m_selected = nullptr;

         if (ui_child) {
            ui_widget_list.append(ui_child);
         }
      }

      if (! ui_widget_list.empty()) {
         ui_widget->setElementWidget(ui_widget_list);
         hasItems = true;
      }
   }

   // actions
   if (! selection.m_actions.empty()) {
      QList<DomAction *> domActions;

      for (QAction *action :  selection.m_actions)
         if (DomAction *domAction = createDom(action)) {
            domActions += domAction;
         }
      if (! domActions.empty()) {
         ui_widget-> setElementAction(domActions);
         hasItems = true;
      }
   }

   d->m_laidout.clear();
   m_copyWidget = false;

   if (!hasItems) {
      delete ui_widget;
      return nullptr;
   }

   // UI
   DomUI *ui = new DomUI();
   ui->setAttributeVersion(currentUiVersion);
   ui->setElementWidget(ui_widget);
   ui->setElementResources(saveResources(m_resourceBuilder->usedQrcFiles()));

   if (DomCustomWidgets *cws = saveCustomWidgets()) {
      ui->setElementCustomWidgets(cws);
   }

   return ui;
}

FormBuilderClipboard QDesignerResource::paste(DomUI *ui, QWidget *widgetParent, QObject *actionParent)
{
   QDesignerWidgetItemInstaller wii;

   const bool old_isMainWidget = m_isMainWidget;
   m_isMainWidget  = false;

   FormBuilderClipboard rc;

   // Widgets
   const DomWidget *topLevel = ui->elementWidget();
   initialize(ui);

   const QList<DomWidget *> domWidgets = topLevel->elementWidget();

   if (! domWidgets.empty()) {
      const QPoint offset = m_formWindow->grid();

      for (DomWidget *domWidget : domWidgets) {
         if (QWidget *w = create(domWidget, widgetParent)) {
            w->move(w->pos() + offset);

            // ### change the init properties of w
            rc.m_widgets.append(w);
         }
      }
   }
   const QList<DomAction *> domActions = topLevel->elementAction();

   if (! domActions.empty()) {
      for (DomAction *domAction : domActions) {
         if (QAction *a = create(domAction, actionParent)) {
            rc.m_actions .append(a);
         }
      }
   }

   // restore value
   m_isMainWidget = old_isMainWidget;

   if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension *>(core()->extensionManager(), core())) {
      extra->loadUiExtraInfo(ui);
   }

   createResources(ui->elementResources());

   return rc;
}

FormBuilderClipboard QDesignerResource::paste(QIODevice *dev, QWidget *widgetParent, QObject *actionParent)
{
   DomUI ui;
   QXmlStreamReader reader(dev);
   bool uiInitialized = false;

   const QString uiElement = "ui";

   while (! reader.atEnd()) {
      if (reader.readNext() == QXmlStreamReader::StartElement) {

         if (reader.name().toLower() == uiElement) {
            ui.read(reader);
            uiInitialized = true;

         } else {
            // Parsing clipboard contents
            reader.raiseError(QCoreApplication::translate("QDesignerResource", "Unexpected element <%1>").formatArg(reader.name()));
         }
      }
   }
   if (reader.hasError()) {
      // Parsing clipboard contents
      csWarning(QCoreApplication::translate("QDesignerResource", "Error while pasting clipboard contents at line %1, column %2: %3")
         .formatArg(reader.lineNumber()).formatArg(reader.columnNumber()).formatArg(reader.errorString()));
      uiInitialized = false;

   } else if (uiInitialized == false) {
      // Parsing clipboard contents
      csWarning(QCoreApplication::translate("QDesignerResource",
            "Error while pasting clipboard contents: The root element <ui> is missing."));
   }

   if (!uiInitialized) {
      return FormBuilderClipboard();
   }

   FormBuilderClipboard clipBoard = paste(&ui, widgetParent, actionParent);

   return clipBoard;
}

void QDesignerResource::layoutInfo(DomLayout *layout, QObject *parent, int *margin, int *spacing)
{
   QAbstractFormBuilder::layoutInfo(layout, parent, margin, spacing);
}

DomCustomWidgets *QDesignerResource::saveCustomWidgets()
{
   if (m_usedCustomWidgets.isEmpty()) {
      return nullptr;
   }

   // We would like the list to be in order of the widget database indexes
   // to ensure that base classes come first (nice optics)
   QDesignerFormEditorInterface *core = m_formWindow->core();
   QDesignerWidgetDataBaseInterface *db = core->widgetDataBase();
   const bool isInternalWidgetDataBase = dynamic_cast<const WidgetDataBase *>(db);
   typedef QMap<int, DomCustomWidget *>  OrderedDBIndexDomCustomWidgetMap;
   OrderedDBIndexDomCustomWidgetMap orderedMap;

   const QString global = QString("global");
   for (QDesignerWidgetDataBaseItemInterface *item : m_usedCustomWidgets.keys()) {
      const QString name = item->name();
      DomCustomWidget *custom_widget = new DomCustomWidget;

      custom_widget->setElementClass(name);
      if (item->isContainer()) {
         custom_widget->setElementContainer(item->isContainer());
      }

      if (!item->includeFile().isEmpty()) {
         DomHeader *header = new DomHeader;
         const  IncludeSpecification spec = includeSpecification(item->includeFile());
         header->setText(spec.first);
         if (spec.second == IncludeGlobal) {
            header->setAttributeLocation(global);
         }
         custom_widget->setElementHeader(header);
         custom_widget->setElementExtends(item->extends());
      }

      if (isInternalWidgetDataBase) {
         WidgetDataBaseItem *internalItem = static_cast<WidgetDataBaseItem *>(item);
         const QStringList fakeSlots = internalItem->fakeSlots();
         const QStringList fakeSignals = internalItem->fakeSignals();
         if (!fakeSlots.empty() || !fakeSignals.empty()) {
            DomSlots *domSlots = new DomSlots();
            domSlots->setElementSlot(fakeSlots);
            domSlots->setElementSignal(fakeSignals);
            custom_widget->setElementSlots(domSlots);
         }
         const QString addPageMethod = internalItem->addPageMethod();
         if (!addPageMethod.isEmpty()) {
            custom_widget->setElementAddPageMethod(addPageMethod);
         }
      }

      orderedMap.insert(db->indexOfClassName(name), custom_widget);
   }

   DomCustomWidgets *customWidgets = new DomCustomWidgets;
   customWidgets->setElementCustomWidget(orderedMap.values());
   return customWidgets;
}

bool QDesignerResource::canCompressSpacings(QObject *object) const
{
   if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), object)) {
      if (dynamic_cast<QGridLayout *>(object)) {
         const int h = sheet->property(sheet->indexOf(QString("horizontalSpacing"))).toInt();
         const int v = sheet->property(sheet->indexOf(QString("verticalSpacing"))).toInt();
         if (h == v) {
            return true;
         }
      }
   }
   return false;
}

QList<DomProperty *> QDesignerResource::computeProperties(QObject *object)
{
   QList<DomProperty *> properties;

   if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), object)) {
      QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension *>
         (core()->extensionManager(), object);

      const int count = sheet->count();
      QList<DomProperty *> spacingProperties;
      const bool compressSpacings = canCompressSpacings(object);

      for (int index = 0; index < count; ++index) {
         if (!sheet->isChanged(index) && (!dynamicSheet || !dynamicSheet->isDynamicProperty(index))) {
            continue;
         }

         const QString propertyName = sheet->propertyName(index);
         // Suppress windowModality in legacy forms that have it set on child widgets
         if (propertyName == QString("windowModality") && !sheet->isVisible(index)) {
            continue;
         }

         const QVariant value = sheet->property(index);
         if (DomProperty *p = createProperty(object, propertyName, value)) {
            if (compressSpacings && (propertyName == QString("horizontalSpacing")
                  || propertyName == QString("verticalSpacing"))) {
               spacingProperties.append(p);
            } else {
               properties.append(p);
            }
         }
      }
      if (compressSpacings) {
         if (spacingProperties.count() == 2) {
            DomProperty *spacingProperty = spacingProperties.at(0);
            spacingProperty->setAttributeName(QString("spacing"));
            properties.append(spacingProperty);
            delete spacingProperties.at(1);
         } else {
            properties += spacingProperties;
         }
      }
   }
   return properties;
}

DomProperty *QDesignerResource::applyProperStdSetAttribute(QObject *object, const QString &propertyName, DomProperty *property)
{
   if (!property) {
      return nullptr;
   }

   QExtensionManager *mgr = core()->extensionManager();
   if (const QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(mgr, object)) {
      const QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension *>(mgr, object);
      const QDesignerPropertySheet *designerSheet = dynamic_cast<QDesignerPropertySheet *>(core()->extensionManager()->extension(object,
               CS_TYPEID(QDesignerPropertySheetExtension)));

      const int index = sheet->indexOf(propertyName);
      if ((dynamicSheet && dynamicSheet->isDynamicProperty(index)) || (designerSheet && designerSheet->isDefaultDynamicProperty(index))) {
         property->setAttributeStdset(0);
      }
   }
   return property;
}

// Optimistic check for a standard setter function
static inline bool hasSetter(QDesignerFormEditorInterface *core, QObject *object, const QString &propertyName)
{
   const QDesignerMetaObjectInterface *meta = core->introspection()->metaObject(object);
   const int pindex = meta->indexOfProperty(propertyName);

   if (pindex == -1) {
      return true;
   }

   return  meta->property(pindex)->hasSetter();
}

DomProperty *QDesignerResource::createProperty(QObject *object, const QString &propertyName, const QVariant &value)
{
   if (! checkProperty(object, propertyName)) {
      return nullptr;
   }

   if (value.canConvert<PropertySheetFlagValue>()) {
      const PropertySheetFlagValue f = value.value<PropertySheetFlagValue>();
      const QString flagString = f.metaFlags.toString(f.value, DesignerMetaFlags::FullyQualified);

      if (flagString.isEmpty()) {
         return nullptr;
      }

      DomProperty *p = new DomProperty;

      // check if we have a standard cpp set function
      if (! hasSetter(core(), object, propertyName)) {
         p->setAttributeStdset(0);
      }

      p->setAttributeName(propertyName);
      p->setElementSet(flagString);

      return applyProperStdSetAttribute(object, propertyName, p);

   } else if (value.canConvert<PropertySheetEnumValue>()) {
      const PropertySheetEnumValue e = value.value<PropertySheetEnumValue>();

      bool ok;
      const QString id = e.metaEnum.toString(e.value, DesignerMetaEnum::FullyQualified, &ok);

      if (! ok) {
         e.metaEnum.messageToStringFailed(e.value);
      }

      if (id.isEmpty()) {
         return nullptr;
      }

      DomProperty *p = new DomProperty;
      // check if we have a standard cpp set function
      if (!hasSetter(core(), object, propertyName)) {
         p->setAttributeStdset(0);
      }
      p->setAttributeName(propertyName);
      p->setElementEnum(id);

      return applyProperStdSetAttribute(object, propertyName, p);

   } else if (value.canConvert<PropertySheetStringValue>()) {
      const PropertySheetStringValue strVal = value.value<PropertySheetStringValue>();
      DomProperty *p = stringToDomProperty(strVal.value(), strVal);
      if (!hasSetter(core(), object, propertyName)) {
         p->setAttributeStdset(0);
      }

      p->setAttributeName(propertyName);

      return applyProperStdSetAttribute(object, propertyName, p);

   } else if (value.canConvert<PropertySheetStringListValue>()) {
      const PropertySheetStringListValue listValue = value.value<PropertySheetStringListValue>();

      DomProperty *p = new DomProperty;
      if (!hasSetter(core(), object, propertyName)) {
         p->setAttributeStdset(0);
      }

      p->setAttributeName(propertyName);

      DomStringList *domStringList = new DomStringList();
      domStringList->setElementString(listValue.value());
      translationParametersToDom(listValue, domStringList);
      p->setElementStringList(domStringList);

      return applyProperStdSetAttribute(object, propertyName, p);

   } else if (value.canConvert<PropertySheetKeySequenceValue>()) {
      const PropertySheetKeySequenceValue keyVal = value.value<PropertySheetKeySequenceValue>();
      DomProperty *p = stringToDomProperty(keyVal.value().toString(), keyVal);

      if (! hasSetter(core(), object, propertyName)) {
         p->setAttributeStdset(0);
      }

      p->setAttributeName(propertyName);

      return applyProperStdSetAttribute(object, propertyName, p);
   }

   return applyProperStdSetAttribute(object, propertyName, QAbstractFormBuilder::createProperty(object, propertyName, value));
}

QStringList QDesignerResource::mergeWithLoadedPaths(const QStringList &paths) const
{
   QStringList newPaths = paths;

   return newPaths;
}

void QDesignerResource::createResources(DomResources *resources)
{
   QStringList paths;

   if (resources != nullptr) {
      const QList<DomResource *> dom_include = resources->elementInclude();

      for (DomResource *res : dom_include) {
         QString path = QDir::cleanPath(m_formWindow->absoluteDir().absoluteFilePath(res->attributeLocation()));

         while (!QFile::exists(path)) {
            QWidget *dialogParent = m_formWindow->core()->topLevel();
            const QString promptTitle = QCoreApplication::translate("qdesigner_internal::QDesignerResource", "Loading qrc file");

            const QString prompt = QCoreApplication::translate("qdesigner_internal::QDesignerResource",
                  "The specified qrc file <p><b>%1</b></p><p>could not be found. Do you want to update the file location?</p>")
                  .formatArg(path);

            const QMessageBox::StandardButton answer = core()->dialogGui()->message(dialogParent,
                  QDesignerDialogGuiInterface::ResourceLoadFailureMessage,
                  QMessageBox::Warning, promptTitle,  prompt, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

            if (answer == QMessageBox::Yes) {
               const QFileInfo fi(path);
               const QString fileDialogTitle = QCoreApplication::translate("qdesigner_internal::QDesignerResource",
                     "New location for %1").formatArg(fi.fileName());

               const QString fileDialogPattern = QCoreApplication::translate("qdesigner_internal::QDesignerResource", "Resource files (*.qrc)");
               path = core()->dialogGui()->getOpenFileName(dialogParent, fileDialogTitle, fi.absolutePath(), fileDialogPattern);

               if (path.isEmpty()) {
                  break;
               }

               m_formWindow->setProperty("_q_resourcepathchanged", QVariant(true));

            } else {
               break;
            }
         }
         if (!path.isEmpty()) {
            paths << path;
            m_formWindow->addResourceFile(path);
         }
      }
   }

   QtResourceSet *resourceSet = m_formWindow->resourceSet();

   if (resourceSet) {
      QStringList oldPaths = resourceSet->activeResourceFilePaths();
      QStringList newPaths = oldPaths;
      QStringListIterator it(paths);
      while (it.hasNext()) {
         const QString path = it.next();
         if (!newPaths.contains(path)) {
            newPaths << path;
         }
      }
      resourceSet->activateResourceFilePaths(newPaths);
   } else {
      resourceSet = m_formWindow->core()->resourceModel()->addResourceSet(paths);
      m_formWindow->setResourceSet(resourceSet);
      QObject::connect(m_formWindow->core()->resourceModel(), &QtResourceModel::resourceSetActivated,
         m_formWindow, &FormWindowBase::resourceSetActivated);
   }
}

DomResources *QDesignerResource::saveResources()
{
   QStringList paths;
   switch (m_formWindow->resourceFileSaveMode()) {
      case QDesignerFormWindowInterface::SaveAllResourceFiles:
         paths = m_formWindow->activeResourceFilePaths();
         break;
      case QDesignerFormWindowInterface::SaveOnlyUsedResourceFiles:
         paths = m_resourceBuilder->usedQrcFiles();
         break;
      case QDesignerFormWindowInterface::DontSaveResourceFiles:
         break;
   }
   return saveResources(paths);
}

DomResources *QDesignerResource::saveResources(const QStringList &qrcPaths)
{
   QtResourceSet *resourceSet = m_formWindow->resourceSet();
   QList<DomResource *> dom_include;
   if (resourceSet) {
      const QStringList activePaths = resourceSet->activeResourceFilePaths();
      for (const QString &path : activePaths) {
         if (qrcPaths.contains(path)) {
            DomResource *dom_res = new DomResource;
            QString conv_path = path;
            if (m_resourceBuilder->isSaveRelative()) {
               conv_path = m_formWindow->absoluteDir().relativeFilePath(path);
            }
            dom_res->setAttributeLocation(conv_path.replace(QDir::separator(), QLatin1Char('/')));
            dom_include.append(dom_res);
         }
      }
   }

   DomResources *dom_resources = new DomResources;
   dom_resources->setElementInclude(dom_include);

   return dom_resources;
}

DomAction *QDesignerResource::createDom(QAction *action)
{
   if (!core()->metaDataBase()->item(action) || action->menu()) {
      return nullptr;
   }

   return QAbstractFormBuilder::createDom(action);
}

DomActionGroup *QDesignerResource::createDom(QActionGroup *actionGroup)
{
   if (core()->metaDataBase()->item(actionGroup) != nullptr) {
      return QAbstractFormBuilder::createDom(actionGroup);
   }

   return nullptr;
}

QAction *QDesignerResource::create(DomAction *ui_action, QObject *parent)
{
   if (QAction *action = QAbstractFormBuilder::create(ui_action, parent)) {
      core()->metaDataBase()->add(action);
      return action;
   }

   return nullptr;
}

QActionGroup *QDesignerResource::create(DomActionGroup *ui_action_group, QObject *parent)
{
   if (QActionGroup *actionGroup = QAbstractFormBuilder::create(ui_action_group, parent)) {
      core()->metaDataBase()->add(actionGroup);
      return actionGroup;
   }

   return nullptr;
}

DomActionRef *QDesignerResource::createActionRefDom(QAction *action)
{
   if (!core()->metaDataBase()->item(action)
      || (!action->isSeparator() && !action->menu() && action->objectName().isEmpty())) {
      return nullptr;
   }

   return QAbstractFormBuilder::createActionRefDom(action);
}

void QDesignerResource::addMenuAction(QAction *action)
{
   core()->metaDataBase()->add(action);
}

QAction *QDesignerResource::createAction(QObject *parent, const QString &name)
{
   if (QAction *action = QAbstractFormBuilder::createAction(parent, name)) {
      core()->metaDataBase()->add(action);
      return action;
   }

   return nullptr;
}

QActionGroup *QDesignerResource::createActionGroup(QObject *parent, const QString &name)
{
   if (QActionGroup *actionGroup = QAbstractFormBuilder::createActionGroup(parent, name)) {
      core()->metaDataBase()->add(actionGroup);
      return actionGroup;
   }

   return nullptr;
}

/* Apply the attributes to a widget via property sheet where appropriate,
 * that is, the sheet handles attributive fake properties */
void QDesignerResource::applyAttributesToPropertySheet(const DomWidget *ui_widget, QWidget *widget)
{
   const QList<DomProperty *> attributes = ui_widget->elementAttribute();

   if (attributes.empty()) {
      return;
   }

   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(m_formWindow->core()->extensionManager(),
         widget);
   auto acend = attributes.constEnd();

   for (auto it = attributes.constBegin(); it != acend; ++it) {
      const QString name = (*it)->attributeName();
      const int index = sheet->indexOf(name);

      if (index == -1) {
         const QString msg = QString("Unable to apply attributive property '%1' to '%2'. It does not exist.")
            .formatArgs(name, widget->objectName());

         csWarning(msg);

      } else {
         sheet->setProperty(index, domPropertyToVariant(this, *(widget->metaObject()), *it));
         sheet->setChanged(index, true);
      }
   }
}

void QDesignerResource::loadExtraInfo(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
   QAbstractFormBuilder::loadExtraInfo(ui_widget, widget, parentWidget);
   // Apply the page id attribute of a QWizardPage (which is an  attributive fake property)
   if (dynamic_cast<const QWizardPage *>(widget)) {
      applyAttributesToPropertySheet(ui_widget, widget);
   }
}

}
