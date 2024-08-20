/***********************************************************************
*
* Copyright (c) 2021-2024 Barbara Geller
* Copyright (c) 2021-2024 Ansel Sermersheim
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

#include <abstract_formeditor.h>
#include <abstract_language.h>
#include <customwidget.h>
#include <designer_utils.h>
#include <designer_widgetbox.h>
#include <extension.h>
#include <extension_manager.h>
#include <plugin_manager.h>
#include <propertysheet.h>
#include <table_classes.h>
#include <ui4.h>
#include <widgetfactory.h>
#include <utils.h>

#include <spacer_widget_p.h>
#include <widgetdatabase_p.h>

#include <QCoreApplication>
#include <QMetaProperty>
#include <QRegularExpression>
#include <QScopedPointer>
#include <QTextStream>
#include <QXmlStreamWriter>

namespace qdesigner_internal {

WidgetDataBaseItem::WidgetDataBaseItem(const QString &name, const QString &group)
   : m_name(name), m_group(group), m_compat(0), m_container(0), m_custom(0), m_promoted(0)
{
}

QString WidgetDataBaseItem::name() const
{
   return m_name;
}

void WidgetDataBaseItem::setName(const QString &name)
{
   m_name = name;
}

QString WidgetDataBaseItem::group() const
{
   return m_group;
}

void WidgetDataBaseItem::setGroup(const QString &group)
{
   m_group = group;
}

QString WidgetDataBaseItem::toolTip() const
{
   return m_toolTip;
}

void WidgetDataBaseItem::setToolTip(const QString &toolTip)
{
   m_toolTip = toolTip;
}

QString WidgetDataBaseItem::whatsThis() const
{
   return m_whatsThis;
}

void WidgetDataBaseItem::setWhatsThis(const QString &whatsThis)
{
   m_whatsThis = whatsThis;
}

QString WidgetDataBaseItem::includeFile() const
{
   return m_includeFile;
}

void WidgetDataBaseItem::setIncludeFile(const QString &includeFile)
{
   m_includeFile = includeFile;
}

QIcon WidgetDataBaseItem::icon() const
{
   return m_icon;
}

void WidgetDataBaseItem::setIcon(const QIcon &icon)
{
   m_icon = icon;
}

bool WidgetDataBaseItem::isCompat() const
{
   return m_compat;
}

void WidgetDataBaseItem::setCompat(bool b)
{
   m_compat = b;
}

bool WidgetDataBaseItem::isContainer() const
{
   return m_container;
}

void WidgetDataBaseItem::setContainer(bool b)
{
   m_container = b;
}

bool WidgetDataBaseItem::isCustom() const
{
   return m_custom;
}

void WidgetDataBaseItem::setCustom(bool b)
{
   m_custom = b;
}

QString WidgetDataBaseItem::pluginPath() const
{
   return m_pluginPath;
}

void WidgetDataBaseItem::setPluginPath(const QString &path)
{
   m_pluginPath = path;
}

bool WidgetDataBaseItem::isPromoted() const
{
   return m_promoted;
}

void WidgetDataBaseItem::setPromoted(bool b)
{
   m_promoted = b;
}

QString WidgetDataBaseItem::extends() const
{
   return m_extends;
}

void WidgetDataBaseItem::setExtends(const QString &s)
{
   m_extends = s;
}

void WidgetDataBaseItem::setDefaultPropertyValues(const QList<QVariant> &list)
{
   m_defaultPropertyValues = list;
}

QList<QVariant> WidgetDataBaseItem::defaultPropertyValues() const
{
   return m_defaultPropertyValues;
}

QStringList WidgetDataBaseItem::fakeSlots() const
{
   return m_fakeSlots;
}

void WidgetDataBaseItem::setFakeSlots(const QStringList &fs)
{
   m_fakeSlots = fs;
}

QStringList WidgetDataBaseItem::fakeSignals() const
{
   return m_fakeSignals;
}

void WidgetDataBaseItem::setFakeSignals(const QStringList &fs)
{
   m_fakeSignals = fs;
}

QString WidgetDataBaseItem::addPageMethod() const
{
   return m_addPageMethod;
}

void WidgetDataBaseItem::setAddPageMethod(const QString &m)
{
   m_addPageMethod = m;
}

WidgetDataBaseItem *WidgetDataBaseItem::clone(const QDesignerWidgetDataBaseItemInterface *item)
{
   WidgetDataBaseItem *rc = new WidgetDataBaseItem(item->name(), item->group());

   rc->setToolTip(item->toolTip());
   rc->setWhatsThis(item->whatsThis());
   rc->setIncludeFile(item->includeFile());
   rc->setIcon(item->icon());
   rc->setCompat(item->isCompat());
   rc->setContainer(item->isContainer());
   rc->setCustom(item->isCustom() );
   rc->setPluginPath(item->pluginPath());
   rc->setPromoted(item->isPromoted());
   rc->setExtends(item->extends());
   rc->setDefaultPropertyValues(item->defaultPropertyValues());
   // container page method, fake slots and signals ignored here.y
   return rc;
}

// ----------------------------------------------------------
WidgetDataBase::WidgetDataBase(QDesignerFormEditorInterface *core, QObject *parent)
   : QDesignerWidgetDataBaseInterface(parent),
     m_core(core)
{
#define DECLARE_LAYOUT(L, C)
#define DECLARE_COMPAT_WIDGET(W, C) DECLARE_WIDGET(W, C)
#define DECLARE_WIDGET(W, C)        append(new WidgetDataBaseItem(#W));

#include <widgets.table>

#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_LAYOUT
#undef DECLARE_WIDGET
#undef DECLARE_WIDGET_1

   append(new WidgetDataBaseItem(QString::fromUtf8("Line")));
   append(new WidgetDataBaseItem(QString::fromUtf8("Spacer")));
   append(new WidgetDataBaseItem(QString::fromUtf8("QSplitter")));
   append(new WidgetDataBaseItem(QString::fromUtf8("QLayoutWidget")));

   // QDesignerWidget is used as central widget and as container for tab widgets, etc.
   WidgetDataBaseItem *designerWidgetItem = new WidgetDataBaseItem(QString::fromUtf8("QDesignerWidget"));
   designerWidgetItem->setContainer(true);
   append(designerWidgetItem);

   append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerDialog")));
   append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerMenu")));
   append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerMenuBar")));
   append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerDockWidget")));
   append(new WidgetDataBaseItem(QString::fromUtf8("QAction")));
   append(new WidgetDataBaseItem(QString::fromUtf8("QButtonGroup")));

   // ### remove me
   // ### check the casts

#if 0 // ### enable me after 4.1
   item(indexOfClassName(QString("QToolBar")))->setContainer(true);
#endif

   item(indexOfClassName(QString("QTabWidget")))->setContainer(true);
   item(indexOfClassName(QString("QGroupBox")))->setContainer(true);
   item(indexOfClassName(QString("QScrollArea")))->setContainer(true);
   item(indexOfClassName(QString("QStackedWidget")))->setContainer(true);
   item(indexOfClassName(QString("QToolBox")))->setContainer(true);
   item(indexOfClassName(QString("QFrame")))->setContainer(true);
   item(indexOfClassName(QString("QLayoutWidget")))->setContainer(true);
   item(indexOfClassName(QString("QDesignerWidget")))->setContainer(true);
   item(indexOfClassName(QString("QDesignerDialog")))->setContainer(true);
   item(indexOfClassName(QString("QSplitter")))->setContainer(true);
   item(indexOfClassName(QString("QMainWindow")))->setContainer(true);
   item(indexOfClassName(QString("QDockWidget")))->setContainer(true);
   item(indexOfClassName(QString("QDesignerDockWidget")))->setContainer(true);
   item(indexOfClassName(QString("QMdiArea")))->setContainer(true);
   item(indexOfClassName(QString("QWizard")))->setContainer(true);
   item(indexOfClassName(QString("QWizardPage")))->setContainer(true);

   item(indexOfClassName(QString("QWidget")))->setContainer(true);
   item(indexOfClassName(QString("QDialog")))->setContainer(true);
}

WidgetDataBase::~WidgetDataBase()
{
}

QDesignerFormEditorInterface *WidgetDataBase::core() const
{
   return m_core;
}

int WidgetDataBase::indexOfObject(QObject *object, bool /*resolveName*/) const
{
   QExtensionManager *mgr = m_core->extensionManager();
   QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension *> (mgr, m_core);

   QString id;

   if (lang) {
      id = lang->classNameOf(object);
   }

   if (id.isEmpty()) {
      id = WidgetFactory::classNameOf(m_core, object);
   }

   return QDesignerWidgetDataBaseInterface::indexOfClassName(id);
}

static WidgetDataBaseItem *createCustomWidgetItem(const QDesignerCustomWidgetInterface *c,
   const QDesignerCustomWidgetData &data)
{
   WidgetDataBaseItem *item = new WidgetDataBaseItem(c->name(), c->group());
   item->setContainer(c->isContainer());
   item->setCustom(true);
   item->setIcon(c->icon());
   item->setIncludeFile(c->includeFile());
   item->setToolTip(c->toolTip());
   item->setWhatsThis(c->whatsThis());
   item->setPluginPath(data.pluginPath());
   item->setAddPageMethod(data.xmlAddPageMethod());
   item->setExtends(data.xmlExtends());
   return item;
}

void WidgetDataBase::loadPlugins()
{
   typedef QMap<QString, int> NameIndexMap;
   typedef QList<QDesignerWidgetDataBaseItemInterface *> ItemList;
   typedef QSet<QString> NameSet;

   // 1) create a map of existing custom classes
   NameIndexMap existingCustomClasses;
   NameSet nonCustomClasses;
   const int count = m_items.size();
   for (int i = 0; i < count; i++)    {
      const QDesignerWidgetDataBaseItemInterface *item =  m_items[i];
      if (item->isCustom() && !item->isPromoted()) {
         existingCustomClasses.insert(item->name(), i);
      } else {
         nonCustomClasses.insert(item->name());
      }
   }

   // 2) create a list plugins
   ItemList pluginList;
   const QDesignerPluginManager *pm = m_core->pluginManager();
   for (QDesignerCustomWidgetInterface *c : pm->registeredCustomWidgets()) {
      pluginList += createCustomWidgetItem(c, pm->customWidgetData(c));
   }

   // 3) replace custom classes or add new ones, remove them from existingCustomClasses,
   // leaving behind deleted items
   unsigned replacedPlugins = 0;
   unsigned addedPlugins = 0;
   unsigned removedPlugins = 0;

   if (!pluginList.empty()) {
      ItemList::const_iterator cend = pluginList.constEnd();
      for (ItemList::const_iterator it = pluginList.constBegin(); it != cend; ++it )  {
         QDesignerWidgetDataBaseItemInterface *pluginItem = *it;
         const QString pluginName = pluginItem->name();
         NameIndexMap::iterator existingIt = existingCustomClasses.find(pluginName);

         if (existingIt == existingCustomClasses.end()) {
            // Add new class.
            if (nonCustomClasses.contains(pluginName)) {
               csWarning(tr("A custom widget plugin whose class name (%1) matches that of an existing class has been found.")
                     .formatArg(pluginName));
            } else {
               append(pluginItem);
               ++addedPlugins;
            }

         } else {
            // replace existing info
            const int existingIndex = existingIt.value();
            delete m_items[existingIndex];
            m_items[existingIndex] = pluginItem;
            existingCustomClasses.erase(existingIt);

            ++replacedPlugins;
         }
      }
   }

   // 4) remove classes that have not been matched. The stored indexes become invalid while deleting.
   if (!existingCustomClasses.empty()) {
      NameIndexMap::const_iterator cend = existingCustomClasses.constEnd();
      for (NameIndexMap::const_iterator it = existingCustomClasses.constBegin(); it != cend; ++it )  {
         const int index = indexOfClassName(it.key());
         if (index != -1) {
            remove(index);
            ++removedPlugins;
         }
      }
   }
}

void WidgetDataBase::remove(int index)
{
   Q_ASSERT(index < m_items.size());
   delete m_items.takeAt(index);
}

QList<QVariant> WidgetDataBase::defaultPropertyValues(const QString &name)
{
   WidgetFactory *factory = dynamic_cast<WidgetFactory *>(m_core->widgetFactory());
   Q_ASSERT(factory);
   // Create non-widgets, widgets in order
   QObject *object = factory->createObject(name, nullptr);

   if (! object) {
      object = factory->createWidget(name, nullptr);
   }

   if (!object) {
      return QList<QVariant>();
   }

   // Get properties from sheet.
   QList<QVariant> result;

   if (const QDesignerPropertySheetExtension *sheet =
         qt_extension<QDesignerPropertySheetExtension *>(m_core->extensionManager(), object)) {
      const int propertyCount = sheet->count();

      for (int i = 0; i < propertyCount; ++i) {
         result.append(sheet->property(i));
      }
   }

   delete object;

   return result;
}

void WidgetDataBase::grabDefaultPropertyValues()
{
   const int itemCount = count();

   for (int i = 0; i < itemCount; ++i) {
      QDesignerWidgetDataBaseItemInterface *dbItem = item(i);
      const QList<QVariant> default_prop_values = defaultPropertyValues(dbItem->name());
      dbItem->setDefaultPropertyValues(default_prop_values);
   }
}

void WidgetDataBase::grabStandardWidgetBoxIcons()
{
   // At this point, grab the default icons for the non-custom widgets from
   // the widget box. They will show up in the object inspector.
   if (const QDesignerWidgetBox *wb = dynamic_cast<const QDesignerWidgetBox *>(m_core->widgetBox())) {
      const QString qWidgetClass = QString("QWidget");
      const int itemCount = count();
      for (int i = 0; i < itemCount; ++i) {
         QDesignerWidgetDataBaseItemInterface *dbItem = item(i);
         if (!dbItem->isCustom() && dbItem->icon().isNull()) {
            // Careful not to catch the layout icons when looking for
            // QWidget
            const QString name = dbItem->name();
            if (name == qWidgetClass) {
               dbItem->setIcon(wb->iconForWidget(name, QString("Containers")));
            } else {
               dbItem->setIcon(wb->iconForWidget(name));
            }
         }
      }
   }
}

// --------------------- Functions relevant generation of new forms based on widgets (apart from the standard templates)

enum { NewFormWidth = 400, NewFormHeight = 300 };

// Check if class is suitable to generate a form from
static inline bool isExistingTemplate(const QString &className)
{
   return className == QString("QWidget") || className == QString("QDialog") || className == QString("QMainWindow");
}

// Check if class is suitable to generate a form from
static inline bool suitableForNewForm(const QString &className)
{
   if (className.isEmpty()) { // Missing custom widget information
      return false;
   }
   if (className == QString("QSplitter")) {
      return false;
   }
   if (className.startsWith(QString("QDesigner")) ||  className.startsWith(QString("QLayout"))) {
      return false;
   }
   return true;
}

// Return a list of widget classes from which new forms can be generated.
// Suitable for 'New form' wizards in integrations.
QStringList WidgetDataBase::formWidgetClasses(const QDesignerFormEditorInterface *core)
{
   static QStringList rc;
   if (rc.empty()) {
      const QDesignerWidgetDataBaseInterface *wdb = core->widgetDataBase();
      const int widgetCount = wdb->count();
      for (int i = 0; i < widgetCount; i++) {
         const QDesignerWidgetDataBaseItemInterface *item = wdb->item(i);
         if (item->isContainer() && !item->isCustom() && !item->isPromoted()) {
            const QString name = item->name(); // Standard Widgets: no existing templates
            if (!isExistingTemplate(name) && suitableForNewForm(name)) {
               rc += name;
            }
         }
      }
   }
   return rc;
}

// Return a list of custom widget classes from which new forms can be generated.
// Suitable for 'New form' wizards in integrations.
QStringList WidgetDataBase::customFormWidgetClasses(const QDesignerFormEditorInterface *core)
{
   QStringList rc;
   const QDesignerWidgetDataBaseInterface *wdb = core->widgetDataBase();
   const int widgetCount = wdb->count();
   for (int i = 0; i < widgetCount; i++) { // Custom widgets: check name and base class.
      const QDesignerWidgetDataBaseItemInterface *item = wdb->item(i);
      if (item->isContainer() && item->isCustom() && !item->isPromoted()) {
         if (suitableForNewForm(item->name()) && suitableForNewForm(item->extends())) {
            rc += item->name();
         }
      }
   }
   return rc;
}

// Get XML for a new form from the widget box. Change objectName/geometry
// properties to be suitable for new forms
static QString xmlFromWidgetBox(const QDesignerFormEditorInterface *core, const QString &className, const QString &objectName)
{
   typedef QList<DomProperty *> PropertyList;

   QDesignerWidgetBoxInterface::Widget widget;
   const bool found = QDesignerWidgetBox::findWidget(core->widgetBox(), className, QString(), &widget);
   if (!found) {
      return QString();
   }
   QScopedPointer<DomUI> domUI(QDesignerWidgetBox::xmlToUi(className, widget.domXml(), false));
   if (domUI.isNull()) {
      return QString();
   }
   domUI->setAttributeVersion(QString("4.0"));
   DomWidget *domWidget = domUI->elementWidget();
   if (!domWidget) {
      return QString();
   }
   // Properties: Remove the "objectName" property in favour of the name attribute and check geometry.
   domWidget->setAttributeName(objectName);
   const QString geometryProperty = QString("geometry");
   const QString objectNameProperty  = QString("objectName");
   PropertyList properties = domWidget->elementProperty();
   for (PropertyList::iterator it = properties.begin(); it != properties.end(); ) {
      DomProperty *property = *it;
      if (property->attributeName() == objectNameProperty) { // remove  "objectName"
         it = properties.erase(it);
         delete property;
      } else {
         if (property->attributeName() == geometryProperty) { // Make sure form is at least 400, 300
            if (DomRect *geom = property->elementRect()) {
               if (geom->elementWidth() < NewFormWidth) {
                  geom->setElementWidth(NewFormWidth);
               }
               if (geom->elementHeight() < NewFormHeight) {
                  geom->setElementHeight(NewFormHeight);
               }
            }
         }
         ++it;
      }
   }
   // Add a window title property
   DomString *windowTitleString = new DomString;
   windowTitleString->setText(objectName);
   DomProperty *windowTitleProperty = new DomProperty;
   windowTitleProperty->setAttributeName(QString("windowTitle"));
   windowTitleProperty->setElementString(windowTitleString);
   properties.push_back(windowTitleProperty);
   // ------
   domWidget->setElementProperty(properties);
   // Embed in in DomUI and get string. Omit the version number.
   domUI->setElementClass(objectName);

   QString rc;
   {
      // Serialize domUI
      QXmlStreamWriter writer(&rc);
      writer.setAutoFormatting(true);
      writer.setAutoFormattingIndent(1);
      writer.writeStartDocument();
      domUI->write(writer);
      writer.writeEndDocument();
   }
   return rc;
}

// Generate default standard ui new form xml based on the class passed on as similarClassName.
static QString generateNewFormXML(const QString &className, const QString &similarClassName, const QString &name)
{
   QString rc;
   {
      QTextStream str(&rc);
      str << QString("<ui version=\"4.0\" >\n<class>") << name << QString("</class>\n")
         <<  QString("<widget class=\"") << className << QString("\" name=\"") << name << QString("\" >\n")
         <<  QString("<property name=\"geometry\" >\n<rect><x>0</x><y>0</y><width>")
         << NewFormWidth << QString("</width><height>") << NewFormHeight << QString("</height></rect>\n</property>\n");
      str << QString("<property name=\"windowTitle\" >\n<string>") << name << QString("</string>\n</property>\n");

      if (similarClassName == QString("QMainWindow")) {
         str << QString("<widget class=\"QWidget\" name=\"centralwidget\" />\n");
      } else {
         if (similarClassName == QString("QWizard")) {
            str << QString("<widget class=\"QWizardPage\" name=\"wizardPage1\" /><widget class=\"QWizardPage\" name=\"wizardPage2\" />\n");
         }
      }
      str << QString("</widget>\n</ui>\n");
   }
   return rc;
}

// Generate a form template using a class name obtained from formWidgetClasses(), customFormWidgetClasses().
QString WidgetDataBase::formTemplate(const QDesignerFormEditorInterface *core, const QString &className, const QString &objectName)
{
   // How to find suitable XML for a class:
   // 1) Look in widget box (as all the required centralwidgets, tab widget pages, etc. should be there).
   const QString widgetBoxXml = xmlFromWidgetBox(core, className, objectName);
   if (! widgetBoxXml.isEmpty()) {
      return widgetBoxXml;
   }

   // 2) If that fails, only custom main windows, custom dialogs and unsupported Widgets should
   //    be left over. Generate something that is similar to the default templates. Find a similar class.
   const QDesignerWidgetDataBaseInterface *wdb = core->widgetDataBase();

   QString similarClass = QString("QWidget");
   const int index = wdb->indexOfClassName(className);

   if (index != -1) {
      const QDesignerWidgetDataBaseItemInterface *item = wdb->item(index);
      similarClass = item->isCustom() ? item->extends() : item->name();
   }

   // Generate standard ui based on the class passed on as baseClassName.
   const QString rc = generateNewFormXML(className, similarClass, objectName);

   return rc;
}

// Set a fixed size on a XML template
QString WidgetDataBase::scaleFormTemplate(const QString &xml, const QSize &size, bool fixed)
{
   typedef QList<DomProperty *> PropertyList;
   DomUI *domUI = QDesignerWidgetBox::xmlToUi(QString("Form"), xml, false);

   if (!domUI) {
      return QString();
   }

   DomWidget *domWidget = domUI->elementWidget();
   if (!domWidget) {
      return QString();
   }

   // Properties: Find/Ensure the geometry, minimum and maximum sizes properties
   const QString geometryPropertyName    = QString("geometry");
   const QString minimumSizePropertyName = QString("minimumSize");
   const QString maximumSizePropertyName = QString("maximumSize");

   DomProperty *geomProperty        = nullptr;
   DomProperty *minimumSizeProperty = nullptr;
   DomProperty *maximumSizeProperty = nullptr;

   PropertyList properties = domWidget->elementProperty();
   const PropertyList::const_iterator cend = properties.constEnd();

   for (PropertyList::const_iterator it = properties.constBegin(); it != cend; ++it) {
      const QString name = (*it)->attributeName();

      if (name == geometryPropertyName) {
         geomProperty = *it;
      } else {
         if (name == minimumSizePropertyName) {
            minimumSizeProperty = *it;
         } else {
            if (name == maximumSizePropertyName) {
               maximumSizeProperty = *it;
            }
         }
      }
   }

   if (!geomProperty) {
      geomProperty = new DomProperty;
      geomProperty->setAttributeName(geometryPropertyName);
      geomProperty->setElementRect(new DomRect);
      properties.push_front(geomProperty);
   }

   if (fixed) {
      if (!minimumSizeProperty) {
         minimumSizeProperty = new DomProperty;
         minimumSizeProperty->setAttributeName(minimumSizePropertyName);
         minimumSizeProperty->setElementSize(new DomSize);
         properties.push_back(minimumSizeProperty);
      }

      if (!maximumSizeProperty) {
         maximumSizeProperty = new DomProperty;
         maximumSizeProperty->setAttributeName(maximumSizePropertyName);
         maximumSizeProperty->setElementSize(new DomSize);
         properties.push_back(maximumSizeProperty);
      }
   }

   // Set values of geometry, minimum and maximum sizes properties
   const int width = size.width();
   const int height = size.height();
   if (DomRect *geom = geomProperty->elementRect()) {
      geom->setElementWidth(width);
      geom->setElementHeight(height);
   }
   if (fixed) {
      if (DomSize *s = minimumSizeProperty->elementSize()) {
         s->setElementWidth(width);
         s->setElementHeight(height);
      }
      if (DomSize *s = maximumSizeProperty->elementSize()) {
         s->setElementWidth(width);
         s->setElementHeight(height);
      }
   }
   // write back
   domWidget->setElementProperty(properties);

   QString rc;
   {
      // serialize domUI
      QXmlStreamWriter writer(&rc);
      writer.setAutoFormatting(true);
      writer.setAutoFormattingIndent(1);
      writer.writeStartDocument();
      domUI->write(writer);
      writer.writeEndDocument();
   }

   delete domUI;
   return rc;
}

// ---- free functions
IncludeSpecification  includeSpecification(QString includeFile)
{
   const bool global = !includeFile.isEmpty() &&
      includeFile[0] == '<' && includeFile[includeFile.size() - 1] ==  '>';

   if (global) {
      includeFile.remove(includeFile.size() - 1, 1);
      includeFile.remove(0, 1);
   }

   return IncludeSpecification(includeFile, global ? IncludeGlobal : IncludeLocal);
}

QString buildIncludeFile(QString includeFile, IncludeType includeType)
{
   if (includeType == IncludeGlobal && !includeFile.isEmpty()) {
      includeFile.append('>');
      includeFile.insert(0, '<');
   }

   return includeFile;
}

/* Appends a derived class to the database inheriting the data of the base class. Used
   for custom and promoted widgets.

   Depending on whether an entry exists, the existing or a newly created entry is
   returned. A return value of 0 indicates that the base class could not be found.
*/
QDesignerWidgetDataBaseItemInterface *appendDerived(QDesignerWidgetDataBaseInterface *db, const QString &className,
      const QString &group, const QString &baseClassName, const QString &includeFile, bool promoted, bool custom)
{
   if (className.isEmpty() || baseClassName.isEmpty()) {
      csWarning("appendDerived() Called with an empty class name " + className + ", base class " + baseClassName);

      return nullptr;
   }

   // Check whether item already exists.
   QDesignerWidgetDataBaseItemInterface *derivedItem = nullptr;
   const int existingIndex = db->indexOfClassName(className);

   if ( existingIndex != -1) {
      derivedItem =  db->item(existingIndex);
   }

   if (derivedItem) {
      // Check the existing item for base class mismatch. This will likely
      // happen when loading a file written by an instance with missing plugins.
      // In that case, just warn and ignore the file properties.
      //
      // An empty base class indicates that it is not known (for example, for custom plugins).
      // In this case, the widget DB is later updated once the widget is created
      // by DOM (by querying the metaobject). Suppress the warning.
      const QString existingBaseClass = derivedItem->extends();
      if (existingBaseClass.isEmpty() || baseClassName ==  existingBaseClass) {
         return derivedItem;
      }

      // Warn about mismatches
      csWarning(QCoreApplication::translate("WidgetDataBase",
            "The file contains a custom widget '%1' whose base class (%2)"
            " differs from the current entry in the widget database (%3)."
            " The widget database is left unchanged.").formatArgs(className, baseClassName, existingBaseClass));
      return derivedItem;
   }

   // Create this item, inheriting its base properties
   const int baseIndex = db->indexOfClassName(baseClassName);

   if (baseIndex == -1) {
      return nullptr;
   }

   const QDesignerWidgetDataBaseItemInterface *baseItem = db->item(baseIndex);
   derivedItem = WidgetDataBaseItem::clone(baseItem);

   // If base class is QWidget, we most likely
   // do not want to inherit the container attribute.

   static const QString qWidgetName = QString("QWidget");
   if (baseItem->name() == qWidgetName) {
      derivedItem->setContainer(false);
   }

   // set new props
   derivedItem->setName(className);
   derivedItem->setGroup(group);
   derivedItem->setCustom(custom);
   derivedItem->setPromoted(promoted);
   derivedItem->setExtends(baseClassName);
   derivedItem->setIncludeFile(includeFile);
   db->append(derivedItem);

   return derivedItem;
}

/* Return a list of database items to which a class can be promoted to. */

WidgetDataBaseItemList promotionCandidates(const QDesignerWidgetDataBaseInterface *db,
   const QString &baseClassName)
{
   WidgetDataBaseItemList rc;

   // find existing promoted widgets deriving from base
   const int count = db->count();

   for (int i = 0; i < count; ++i) {
      QDesignerWidgetDataBaseItemInterface *item = db->item(i);
      if (item->isPromoted() && item->extends() == baseClassName) {
         rc.push_back(item);
      }
   }
   return rc;
}

} // namespace qdesigner_internal
