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

#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <designer_introspection.h>
#include <designer_propertysheet.h>
#include <designer_utils.h>
#include <formbuilderextra.h>
#include <layout_info.h>
#include <layout_widget.h>

#include <formwindowbase_p.h>
#include <widgetdatabase_p.h>

#include <QApplication>
#include <QDialog>
#include <QDockWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QMainWindow>
#include <QMenuBar>
#include <QStyle>
#include <QToolBar>

#define USE_LAYOUT_SIZE_CONSTRAINT

static const QDesignerMetaObjectInterface *propertyBelongsTo(const QDesignerMetaObjectInterface *meta, const QString &name)
{
   const QDesignerMetaObjectInterface *retval = nullptr;

   auto parent = meta->superClass();

   while (parent != nullptr) {

      if (parent->indexOfProperty(name) == -1) {
         break;
      }

      retval = parent;
      parent = retval->superClass();
   }

   return retval;
}

// Layout fake properties (prefixed by 'layout' to distinguish them from other 'margins'
// that might be around. These are forwarded to the layout sheet (after name transformation).

// 'layoutObjectName' is the name of the actual layout

static const QString layoutBottomMarginC      = "layoutBottomMargin";
static const QString layoutHorizontalSpacingC = "layoutHorizontalSpacing";
static const QString layoutLeftMarginC        = "layoutLeftMargin";
static const QString layoutObjectNameC        = "layoutName";
static const QString layoutRightMarginC       = "layoutRightMargin";
static const QString layoutSizeConstraintC    = "layoutSizeConstraint";
static const QString layoutSpacingC           = "layoutSpacing";
static const QString layoutTopMarginC         = "layoutTopMargin";
static const QString layoutVerticalSpacingC   = "layoutVerticalSpacing";

// form layout
static const QString layoutFieldGrowthPolicyC = "layoutFieldGrowthPolicy";
static const QString layoutFormAlignmentC     = "layoutFormAlignment";
static const QString layoutLabelAlignmentC    = "layoutLabelAlignment";
static const QString layoutRowWrapPolicyC     = "layoutRowWrapPolicy";

// stretches
static const QString layoutGridColumnMinimumWidthC    = "layoutColumnMinimumWidth";
static const QString layoutGridColumnStretchPropertyC = "layoutColumnStretch";
static const QString layoutGridRowMinimumHeightC      = "layoutRowMinimumHeight";
static const QString layoutGridRowStretchPropertyC    = "layoutRowStretch";
static const QString layoutboxStretchPropertyC        = "layoutStretch";

// Find the form editor in the hierarchy. We know the parent of the sheet is the
// extension manager whose parent is the core.

static QDesignerFormEditorInterface *formEditorForObject(QObject *obj)
{
   do {
      if (QDesignerFormEditorInterface *core = dynamic_cast<QDesignerFormEditorInterface *>(obj)) {
         return core;
      }

      obj = obj->parent();

   } while (obj);

   Q_ASSERT(obj);

   return nullptr;
}

static bool hasLayoutAttributes(QDesignerFormEditorInterface *core, QObject *object)
{
   if (! object->isWidgetType()) {
      return false;
   }

   QWidget *w =  dynamic_cast<QWidget *>(object);
   if (const QDesignerWidgetDataBaseInterface *db = core->widgetDataBase()) {
      if (db->isContainer(w)) {
         return true;
      }
   }
   return false;
}

// Cache DesignerMetaEnum by scope/name of a QMetaEnum
static const qdesigner_internal::DesignerMetaEnum &designerMetaEnumFor(const QDesignerMetaEnumInterface *me)
{
   typedef QPair<QString, QString> ScopeNameKey;
   typedef QMap<ScopeNameKey, qdesigner_internal::DesignerMetaEnum> DesignerMetaEnumCache;

   static DesignerMetaEnumCache cache;

   const QString name  = me->name();
   const QString scope = me->scope();

   const ScopeNameKey key = ScopeNameKey(scope, name);
   auto iter = cache.find(key);

   if (iter == cache.end()) {
      // add new value

      qdesigner_internal::DesignerMetaEnum dme = qdesigner_internal::DesignerMetaEnum(name, scope, me->separator());
      const int keyCount = me->keyCount();

      for (int index = 0; index < keyCount; ++index) {
         // add the values for the enum
         dme.addKey(me->value(index), me->key(index));
      }

      iter = cache.insert(key, dme);
   }

   return iter.value();
}

// Cache DesignerMetaFlags by scope/name of a QMetaEnum
static const qdesigner_internal::DesignerMetaFlags &designerMetaFlagsFor(const QDesignerMetaEnumInterface *me)
{
   typedef QPair<QString, QString> ScopeNameKey;
   typedef QMap<ScopeNameKey, qdesigner_internal::DesignerMetaFlags> DesignerMetaFlagsCache;

   static DesignerMetaFlagsCache cache;

   const QString name  = me->name();
   const QString scope = me->scope();

   const ScopeNameKey key = ScopeNameKey(scope, name);
   auto iter = cache.find(key);

   if (iter == cache.end()) {
      qdesigner_internal::DesignerMetaFlags dme = qdesigner_internal::DesignerMetaFlags(name, scope, me->separator());
      const int keyCount = me->keyCount();

      for (int index = 0; index < keyCount; ++index) {
         dme.addKey(me->value(index), me->key(index));
      }

      iter = cache.insert(key, dme);
   }

   return iter.value();
}

class QDesignerPropertySheetPrivate
{
 public:
   using PropertyType = QDesignerPropertySheet::PropertyType;
   using ObjectType   = QDesignerPropertySheet::ObjectType;

   explicit QDesignerPropertySheetPrivate(QDesignerPropertySheet *sheetPublic, QObject *object, QObject *sheetParent);

   bool invalidIndex(const QString &functionName, int index) const;

   int count() const {
      return m_meta->propertyCount() + m_addProperties.count();
   }

   PropertyType propertyType(int index) const;
   QString transformLayoutPropertyName(int index) const;
   QLayout *layout(QDesignerPropertySheetExtension **layoutPropertySheet = nullptr) const;
   static ObjectType objectType(const QObject *o);

   bool isReloadableProperty(int index) const;
   bool isResourceProperty(int index) const;
   void addResourceProperty(int index, QVariant::Type type);
   QVariant resourceProperty(int index) const;
   void setResourceProperty(int index, const QVariant &value);

   QVariant emptyResourceProperty(int index) const;    // type PropertySheetPixmapValue / PropertySheetIconValue
   QVariant defaultResourceProperty(int index) const;  // type QPixmap / QIcon (maybe it can be generalized for all types, not resource only)

   bool isStringProperty(int index) const;
   void addStringProperty(int index);
   qdesigner_internal::PropertySheetStringValue stringProperty(int index) const;
   void setStringProperty(int index, const qdesigner_internal::PropertySheetStringValue &value);
   bool isStringListProperty(int index) const;
   void addStringListProperty(int index);
   qdesigner_internal::PropertySheetStringListValue stringListProperty(int index) const;
   void setStringListProperty(int index, const qdesigner_internal::PropertySheetStringListValue &value);

   bool isKeySequenceProperty(int index) const;
   void addKeySequenceProperty(int index);
   qdesigner_internal::PropertySheetKeySequenceValue keySequenceProperty(int index) const;
   void setKeySequenceProperty(int index, const qdesigner_internal::PropertySheetKeySequenceValue &value);

   enum PropertyKind {
      NormalProperty,
      FakeProperty,
      DynamicProperty,
      DefaultDynamicProperty
   };

   class Info
   {
     public:
       Info();

       QString group;
       QVariant defaultValue;
       bool changed;
       bool visible;
       bool attribute;
       bool reset;
       PropertyType propertyType;
       PropertyKind kind;
   };

   Info &ensureInfo(int index);

   QDesignerPropertySheet *q;
   QDesignerFormEditorInterface *m_core;
   const QDesignerMetaObjectInterface *m_meta;
   const ObjectType m_objectType;

   QHash<int, Info> m_info;
   QHash<int, QVariant> m_fakeProperties;
   QHash<int, QVariant> m_addProperties;
   QHash<QString, int> m_addIndex;
   QHash<int, QVariant> m_resourceProperties; // only PropertySheetPixmapValue snd PropertySheetIconValue here
   QHash<int, qdesigner_internal::PropertySheetStringValue> m_stringProperties; // only PropertySheetStringValue
   QHash<int, qdesigner_internal::PropertySheetStringListValue> m_stringListProperties; // only PropertySheetStringListValue
   QHash<int, qdesigner_internal::PropertySheetKeySequenceValue> m_keySequenceProperties; // only PropertySheetKeySequenceValue

   const bool m_canHaveLayoutAttributes;

   // Variables used for caching the layout, access via layout().
   QPointer<QObject> m_object;
   mutable QPointer<QLayout> m_lastLayout;
   mutable QDesignerPropertySheetExtension *m_lastLayoutPropertySheet;
   mutable bool m_LastLayoutByDesigner;

   qdesigner_internal::DesignerPixmapCache *m_pixmapCache;
   qdesigner_internal::DesignerIconCache *m_iconCache;
   QPointer<qdesigner_internal::FormWindowBase> m_fwb;

   // Enable Qt's internal properties starting with prefix "_q_"
   static bool m_internalDynamicPropertiesEnabled;
};

bool QDesignerPropertySheetPrivate::m_internalDynamicPropertiesEnabled = false;

bool QDesignerPropertySheetPrivate::isReloadableProperty(int index) const
{
   return isResourceProperty(index)
      || propertyType(index) == QDesignerPropertySheet::PropertyStyleSheet
      || propertyType(index) == QDesignerPropertySheet::PropertyText
      || q->property(index).type() == QVariant::Url;
}

/*
    Resource properties are those which:
        1) are reloadable
        2) their state is associated with a file which can be taken from resources
        3) we do not store them in the meta object system (because designer keeps different data structure for them)
*/

bool QDesignerPropertySheetPrivate::isResourceProperty(int index) const
{
   return m_resourceProperties.contains(index);
}

void QDesignerPropertySheetPrivate::addResourceProperty(int index, QVariant::Type type)
{
   if (type == QVariant::Pixmap) {
      m_resourceProperties.insert(index, QVariant::fromValue(qdesigner_internal::PropertySheetPixmapValue()));
   } else if (type == QVariant::Icon) {
      m_resourceProperties.insert(index, QVariant::fromValue(qdesigner_internal::PropertySheetIconValue()));
   }
}

QVariant QDesignerPropertySheetPrivate::emptyResourceProperty(int index) const
{
   QVariant v = m_resourceProperties.value(index);
   if (v.canConvert<qdesigner_internal::PropertySheetPixmapValue>()) {
      return QVariant::fromValue(qdesigner_internal::PropertySheetPixmapValue());
   }
   if (v.canConvert<qdesigner_internal::PropertySheetIconValue>()) {
      return QVariant::fromValue(qdesigner_internal::PropertySheetIconValue());
   }
   return v;
}

QVariant QDesignerPropertySheetPrivate::defaultResourceProperty(int index) const
{
   return m_info.value(index).defaultValue;
}

QVariant QDesignerPropertySheetPrivate::resourceProperty(int index) const
{
   return m_resourceProperties.value(index);
}

void QDesignerPropertySheetPrivate::setResourceProperty(int index, const QVariant &value)
{
   Q_ASSERT(isResourceProperty(index));

   QVariant &v = m_resourceProperties[index];

   if ((value.canConvert<qdesigner_internal::PropertySheetPixmapValue>() && v.canConvert<qdesigner_internal::PropertySheetPixmapValue>())
      || (value.canConvert<qdesigner_internal::PropertySheetIconValue>() && v.canConvert<qdesigner_internal::PropertySheetIconValue>())) {
      v = value;
   }
}

bool QDesignerPropertySheetPrivate::isStringProperty(int index) const
{
   return m_stringProperties.contains(index);
}

void QDesignerPropertySheetPrivate::addStringProperty(int index)
{
   m_stringProperties.insert(index, qdesigner_internal::PropertySheetStringValue());
}

qdesigner_internal::PropertySheetStringValue QDesignerPropertySheetPrivate::stringProperty(int index) const
{
   return m_stringProperties.value(index);
}

void QDesignerPropertySheetPrivate::setStringProperty(int index, const qdesigner_internal::PropertySheetStringValue &value)
{
   Q_ASSERT(isStringProperty(index));

   m_stringProperties[index] = value;
}

bool QDesignerPropertySheetPrivate::isStringListProperty(int index) const
{
   return m_stringListProperties.contains(index);
}

void QDesignerPropertySheetPrivate::addStringListProperty(int index)
{
   m_stringListProperties.insert(index, qdesigner_internal::PropertySheetStringListValue());
}

qdesigner_internal::PropertySheetStringListValue QDesignerPropertySheetPrivate::stringListProperty(int index) const
{
   return m_stringListProperties.value(index);
}

void QDesignerPropertySheetPrivate::setStringListProperty(int index, const qdesigner_internal::PropertySheetStringListValue &value)
{
   Q_ASSERT(isStringListProperty(index));

   m_stringListProperties[index] = value;
}

bool QDesignerPropertySheetPrivate::isKeySequenceProperty(int index) const
{
   return m_keySequenceProperties.contains(index);
}

void QDesignerPropertySheetPrivate::addKeySequenceProperty(int index)
{
   m_keySequenceProperties.insert(index, qdesigner_internal::PropertySheetKeySequenceValue());
}

qdesigner_internal::PropertySheetKeySequenceValue QDesignerPropertySheetPrivate::keySequenceProperty(int index) const
{
   return m_keySequenceProperties.value(index);
}

void QDesignerPropertySheetPrivate::setKeySequenceProperty(int index, const qdesigner_internal::PropertySheetKeySequenceValue &value)
{
   Q_ASSERT(isKeySequenceProperty(index));

   m_keySequenceProperties[index] = value;
}

QDesignerPropertySheetPrivate::Info::Info()
   : changed(false), visible(true), attribute(false), reset(true),
     propertyType(QDesignerPropertySheet::PropertyNone), kind(NormalProperty)
{
}

QDesignerPropertySheetPrivate::QDesignerPropertySheetPrivate(QDesignerPropertySheet *sheetPublic,
      QObject *object, QObject *sheetParent)
   : q(sheetPublic), m_core(formEditorForObject(sheetParent)), m_meta(m_core->introspection()->metaObject(object)),
     m_objectType(QDesignerPropertySheet::objectTypeFromObject(object)),
     m_canHaveLayoutAttributes(hasLayoutAttributes(m_core, object)),
     m_object(object), m_lastLayout(nullptr), m_lastLayoutPropertySheet(nullptr), m_LastLayoutByDesigner(false),
     m_pixmapCache(nullptr), m_iconCache(nullptr)
{
}

qdesigner_internal::FormWindowBase *QDesignerPropertySheet::formWindowBase() const
{
   return d->m_fwb;
}

bool QDesignerPropertySheetPrivate::invalidIndex(const QString &functionName, int index) const
{
   static const QString space(24, ' ');

   if (index < 0 || index >= count()) {
      QString msg = "Method %1\n" + space + "was passed an invalid index of %2 (%3)\n";
      msg = msg.formatArg(functionName).formatArg(index).formatArg(m_object->metaObject()->className());

      csWarning(msg);

      return true;
   }

   return false;
}

QLayout *QDesignerPropertySheetPrivate::layout(QDesignerPropertySheetExtension **layoutPropertySheet) const
{
   // Return the layout and its property sheet
   // only if it is managed by designer and not one created on a custom widget.
   // (attempt to cache the value as this requires some hoops).

   if (layoutPropertySheet) {
      *layoutPropertySheet = nullptr;
   }

   if (! m_object->isWidgetType() || !m_canHaveLayoutAttributes) {
      return nullptr;
   }

   QWidget *widget = dynamic_cast<QWidget *>(m_object.data());
   QLayout *widgetLayout = qdesigner_internal::LayoutInfo::internalLayout(widget);

   if (! widgetLayout) {
      m_lastLayout = nullptr;
      m_lastLayoutPropertySheet = nullptr;

      return nullptr;
   }

   // Smart logic to avoid retrieving the meta DB from the widget every time.
   if (widgetLayout != m_lastLayout) {
      m_lastLayout = widgetLayout;
      m_LastLayoutByDesigner = false;
      m_lastLayoutPropertySheet = nullptr;

      // Is this a layout managed by designer or some layout on a custom widget?
      if (qdesigner_internal::LayoutInfo::managedLayout(m_core, widgetLayout)) {
         m_LastLayoutByDesigner = true;
         m_lastLayoutPropertySheet = qt_extension<QDesignerPropertySheetExtension *>(m_core->extensionManager(), m_lastLayout);
      }
   }

   if (!m_LastLayoutByDesigner) {
      return nullptr;
   }

   if (layoutPropertySheet) {
      *layoutPropertySheet = m_lastLayoutPropertySheet;
   }

   return m_lastLayout;
}

QDesignerPropertySheetPrivate::Info &QDesignerPropertySheetPrivate::ensureInfo(int index)
{
   auto iter = m_info.find(index);

   if (iter == m_info.end()) {
      iter = m_info.insert(index, Info());
   }

   return iter.value();
}

QDesignerPropertySheet::PropertyType QDesignerPropertySheetPrivate::propertyType(int index) const
{
   auto iter = m_info.constFind(index);

   if (iter == m_info.constEnd()) {
      return QDesignerPropertySheet::PropertyNone;
   }

   return iter.value().propertyType;
}

QString QDesignerPropertySheetPrivate::transformLayoutPropertyName(int index) const
{
   typedef QMap<QDesignerPropertySheet::PropertyType, QString> TypeNameMap;
   static TypeNameMap typeNameMap;

   if (typeNameMap.empty()) {
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutObjectName, QString("objectName"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutLeftMargin, QString("leftMargin"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutTopMargin, QString("topMargin"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutRightMargin, QString("rightMargin"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutBottomMargin, QString("bottomMargin"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutSpacing, QString("spacing"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutHorizontalSpacing, QString("horizontalSpacing"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutVerticalSpacing, QString("verticalSpacing"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutSizeConstraint, QString("sizeConstraint"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutFieldGrowthPolicy, QString("fieldGrowthPolicy"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutRowWrapPolicy, QString("rowWrapPolicy"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutLabelAlignment, QString("labelAlignment"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutFormAlignment, QString("formAlignment"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutBoxStretch, QString("stretch"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutGridRowStretch, QString("rowStretch"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutGridColumnStretch, QString("columnStretch"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutGridRowMinimumHeight, QString("rowMinimumHeight"));
      typeNameMap.insert(QDesignerPropertySheet::PropertyLayoutGridColumnMinimumWidth, QString("columnMinimumWidth"));
   }

   auto iter = typeNameMap.constFind(propertyType(index));

   if (iter != typeNameMap.constEnd()) {
      return iter.value();
   }

   return QString();
}

QDesignerPropertySheet::ObjectType QDesignerPropertySheet::objectTypeFromObject(const QObject *o)
{
   if (dynamic_cast<const QLayout *>(o)) {
      return ObjectLayout;
   }

   if (!o->isWidgetType()) {
      return ObjectNone;
   }

   if (dynamic_cast<const QLayoutWidget *>(o)) {
      return ObjectLayoutWidget;
   }

   if (dynamic_cast<const QLabel *>(o)) {
      return ObjectLabel;
   }

   return ObjectNone;
}

QDesignerPropertySheet::PropertyType QDesignerPropertySheet::propertyTypeFromName(const QString &name)
{
   static QHash<QString, PropertyType> propertyTypeHash;

   if (propertyTypeHash.empty()) {
      propertyTypeHash.insert(layoutObjectNameC,                 PropertyLayoutObjectName);
      propertyTypeHash.insert(layoutLeftMarginC,                 PropertyLayoutLeftMargin);
      propertyTypeHash.insert(layoutTopMarginC,                  PropertyLayoutTopMargin);
      propertyTypeHash.insert(layoutRightMarginC,                PropertyLayoutRightMargin);
      propertyTypeHash.insert(layoutBottomMarginC,               PropertyLayoutBottomMargin);
      propertyTypeHash.insert(layoutSpacingC,                    PropertyLayoutSpacing);
      propertyTypeHash.insert(layoutHorizontalSpacingC,          PropertyLayoutHorizontalSpacing);
      propertyTypeHash.insert(layoutVerticalSpacingC,            PropertyLayoutVerticalSpacing);
      propertyTypeHash.insert(layoutSizeConstraintC,             PropertyLayoutSizeConstraint);
      propertyTypeHash.insert(layoutFieldGrowthPolicyC,          PropertyLayoutFieldGrowthPolicy);
      propertyTypeHash.insert(layoutRowWrapPolicyC,              PropertyLayoutRowWrapPolicy);
      propertyTypeHash.insert(layoutLabelAlignmentC,             PropertyLayoutLabelAlignment);
      propertyTypeHash.insert(layoutFormAlignmentC,              PropertyLayoutFormAlignment);
      propertyTypeHash.insert(layoutboxStretchPropertyC,         PropertyLayoutBoxStretch);
      propertyTypeHash.insert(layoutGridRowStretchPropertyC,     PropertyLayoutGridRowStretch);
      propertyTypeHash.insert(layoutGridColumnStretchPropertyC,  PropertyLayoutGridColumnStretch);
      propertyTypeHash.insert(layoutGridRowMinimumHeightC,       PropertyLayoutGridRowMinimumHeight);
      propertyTypeHash.insert(layoutGridColumnMinimumWidthC,     PropertyLayoutGridColumnMinimumWidth);

      propertyTypeHash.insert("buddy",                           PropertyBuddy);
      propertyTypeHash.insert("geometry",                        PropertyGeometry);
      propertyTypeHash.insert("checkable",                       PropertyCheckable);
      propertyTypeHash.insert("accessibleName",                  PropertyAccessibility);
      propertyTypeHash.insert("accessibleDescription",           PropertyAccessibility);
      propertyTypeHash.insert("windowTitle",                     PropertyWindowTitle);
      propertyTypeHash.insert("windowIcon",                      PropertyWindowIcon);
      propertyTypeHash.insert("windowFilePath",                  PropertyWindowFilePath);
      propertyTypeHash.insert("windowOpacity",                   PropertyWindowOpacity);
      propertyTypeHash.insert("windowIconText",                  PropertyWindowIconText);
      propertyTypeHash.insert("windowModality",                  PropertyWindowModality);
      propertyTypeHash.insert("windowModified",                  PropertyWindowModified);
      propertyTypeHash.insert("styleSheet",                      PropertyStyleSheet);
      propertyTypeHash.insert("text",                            PropertyText);
   }

   return propertyTypeHash.value(name, PropertyNone);
}

QDesignerPropertySheet::QDesignerPropertySheet(QObject *object, QObject *parent)
   : QObject(parent), d(new QDesignerPropertySheetPrivate(this, object, parent))
{
   using Info = QDesignerPropertySheetPrivate::Info;

   const QDesignerMetaObjectInterface *baseMeta = d->m_meta;

   while (baseMeta && baseMeta->className().startsWith("QDesigner")) {
      baseMeta = baseMeta->superClass();
   }
   Q_ASSERT(baseMeta != nullptr);

   QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(d->m_object);
   d->m_fwb = dynamic_cast<qdesigner_internal::FormWindowBase *>(formWindow);

   if (d->m_fwb) {
      d->m_pixmapCache = d->m_fwb->pixmapCache();
      d->m_iconCache   = d->m_fwb->iconCache();
      d->m_fwb->addReloadablePropertySheet(this, object);
   }

   for (int index = 0; index < count(); ++index) {
      const QDesignerMetaPropertyInterface *p = d->m_meta->property(index);
      const QString name = p->name();

      if (p->type() == QVariant::KeySequence) {
         createFakeProperty(name);

      } else {
         // use the default for real properties
         setVisible(index, false);
      }

      QString pgroup = baseMeta->className();

      if (pgroup == "Spacer" && name == "sizeHint") {
         // keep this pgroup

      } else {
         const QDesignerMetaObjectInterface *correctedMeta = propertyBelongsTo(baseMeta, name);

         if (correctedMeta != nullptr) {
            pgroup = correctedMeta->className();
         }
      }

      Info &info = d->ensureInfo(index);
      info.group = pgroup;
      info.propertyType = propertyTypeFromName(name);

      const QVariant::Type type = p->type();

      switch (type) {
         case QVariant::Cursor:
         case QVariant::Icon:
         case QVariant::Pixmap:
            info.defaultValue = p->read(d->m_object);

            if (type == QVariant::Icon || type == QVariant::Pixmap) {
               d->addResourceProperty(index, type);
            }
            break;

         case QVariant::String:
            d->addStringProperty(index);
            break;

         case QVariant::StringList:
            d->addStringListProperty(index);
            break;

         case QVariant::KeySequence:
            d->addKeySequenceProperty(index);
            break;

         default:
            break;
      }
   }

   if (object->isWidgetType()) {
      createFakeProperty("focusPolicy");
      createFakeProperty("cursor");
      createFakeProperty("toolTip");
      createFakeProperty("whatsThis");
      createFakeProperty("acceptDrops");
      createFakeProperty("dragEnabled");

      // windowModality/Opacity is visible only for the main container
      setVisible(createFakeProperty("windowModality"), false);
      setVisible(createFakeProperty("windowOpacity", double(1.0)), false);

      if (dynamic_cast<const QToolBar *>(d->m_object.data())) {
         // prevent toolbars from being dragged off
         createFakeProperty("floatable", QVariant(true));

      } else {
         if (dynamic_cast<const QMenuBar *>(d->m_object.data())) {
            // Keep the menu bar editable in the form even if a native menu bar is used
            const bool nativeMenuBarDefault = ! qApp->testAttribute(Qt::AA_DontUseNativeMenuBar);
            createFakeProperty(QString("nativeMenuBar"), QVariant(nativeMenuBarDefault));
         }
      }

      if (d->m_canHaveLayoutAttributes) {
         static const QString layoutGroup = "Layout";

         const QStringList fakeLayoutProperties = {
            layoutObjectNameC,
            layoutLeftMarginC,
            layoutTopMarginC,
            layoutRightMarginC,
            layoutBottomMarginC,
            layoutSpacingC,
            layoutHorizontalSpacingC,
            layoutVerticalSpacingC,
            layoutFieldGrowthPolicyC,
            layoutRowWrapPolicyC,
            layoutLabelAlignmentC,
            layoutFormAlignmentC,
            layoutboxStretchPropertyC,
            layoutGridRowStretchPropertyC,
            layoutGridColumnStretchPropertyC,
            layoutGridRowMinimumHeightC,
            layoutGridColumnMinimumWidthC

#ifdef USE_LAYOUT_SIZE_CONSTRAINT
            , layoutSizeConstraintC
#endif
         };

         const int fakeLayoutPropertyCount = fakeLayoutProperties.size();
         const int size = count();

         for (int i = 0; i < fakeLayoutPropertyCount; i++) {
            createFakeProperty(fakeLayoutProperties[i], 0);
            setAttribute(size  + i, true);
            setPropertyGroup(size  + i, layoutGroup);
         }
      }

      if (d->m_objectType == ObjectLabel) {
         createFakeProperty("buddy", QVariant(QByteArray()));
      }

      /* need to create a fake property since the property does not work
       * for non-toplevel windows or on other systems than Mac and only if
       * it is above a certain Mac OS version. */

      if (dynamic_cast<const QMainWindow *>(d->m_object.data())) {
         createFakeProperty("unifiedTitleAndToolBarOnMac", false);
      }
   }

   if (dynamic_cast<const QDialog *>(object)) {
      createFakeProperty("modal");
   }

   if (dynamic_cast<const QDockWidget *>(object)) {
      createFakeProperty("floating");
   }

   const QList<QString> names = object->dynamicPropertyNames();

   if (! names.empty()) {
      for (const auto &item : names) {
         const int idx = addDynamicProperty(item, object->property(item));

         if (idx != -1) {
            d->ensureInfo(idx).kind = QDesignerPropertySheetPrivate::DefaultDynamicProperty;
         }
      }
   }
}

QDesignerPropertySheet::~QDesignerPropertySheet()
{
   if (d->m_fwb) {
      d->m_fwb->removeReloadablePropertySheet(this);
   }

   delete d;
}

QObject *QDesignerPropertySheet::object() const
{
   return d->m_object;
}

bool QDesignerPropertySheet::dynamicPropertiesAllowed() const
{
   return true;
}

bool QDesignerPropertySheet::canAddDynamicProperty(const QString &propName) const
{
   // used internally
   if (propName == "database" || propName == "buttonGroupId") {
      return false;
   }

   const int index = d->m_meta->indexOfProperty(propName);
   if (index != -1) {
      return false;   // property already exists and is not a dynamic one
   }
   if (d->m_addIndex.contains(propName)) {
      const int idx = d->m_addIndex.value(propName);

      if (isVisible(idx)) {
         return false;   // dynamic property already exists
      } else {
         return true;
      }
   }

   if (! QDesignerPropertySheet::internalDynamicPropertiesEnabled() && propName.startsWith("_q_")) {
      return false;
   }

   return true;
}

int QDesignerPropertySheet::addDynamicProperty(const QString &propName, const QVariant &value)
{
   using Info = QDesignerPropertySheetPrivate::Info;

   if (! value.isValid()) {
      return -1;   // property has invalid type
   }

   if (! canAddDynamicProperty(propName)) {
      return -1;
   }

   QVariant v = value;

   if (value.type() == QVariant::Icon) {
      v = QVariant::fromValue(qdesigner_internal::PropertySheetIconValue());

   } else if (value.type() == QVariant::Pixmap) {
      v = QVariant::fromValue(qdesigner_internal::PropertySheetPixmapValue());

   } else if (value.type() == QVariant::String) {
      v = QVariant::fromValue(qdesigner_internal::PropertySheetStringValue(value.toString()));

   } else if (value.type() == QVariant::StringList) {
      v = QVariant::fromValue(qdesigner_internal::PropertySheetStringListValue(value.toStringList()));

   } else if (value.type() == QVariant::KeySequence) {
      const QKeySequence keySequence = value.value<QKeySequence>();
      v = QVariant::fromValue(qdesigner_internal::PropertySheetKeySequenceValue(keySequence));
   }

   if (d->m_addIndex.contains(propName)) {
      const int idx = d->m_addIndex.value(propName);

      // currently invisible, checked in canAddDynamicProperty()
      setVisible(idx, true);

      d->m_addProperties.insert(idx, v);
      setChanged(idx, false);

      const int index = d->m_meta->indexOfProperty(propName);

      Info &info = d->ensureInfo(index);
      info.defaultValue = value;
      info.kind = QDesignerPropertySheetPrivate::DynamicProperty;

      if (value.type() == QVariant::Icon || value.type() == QVariant::Pixmap) {
         d->addResourceProperty(idx, value.type());

      } else if (value.type() == QVariant::String) {
         d->addStringProperty(idx);

      } else if (value.type() == QVariant::KeySequence) {
         d->addKeySequenceProperty(idx);

      }

      return idx;
   }

   const int index = count();
   d->m_addIndex.insert(propName, index);
   d->m_addProperties.insert(index, v);

   Info &info   = d->ensureInfo(index);
   info.visible = true;
   info.changed = false;
   info.defaultValue = value;
   info.kind    = QDesignerPropertySheetPrivate::DynamicProperty;

   setPropertyGroup(index, tr("Dynamic Properties"));

   switch (value.type()) {
      case QVariant::Icon:
      case QVariant::Pixmap:
         d->addResourceProperty(index, value.type());
         break;

      case QVariant::String:
         d->addStringProperty(index);
         break;

      case QVariant::StringList:
         d->addStringListProperty(index);
         break;

      case QVariant::KeySequence:
         d->addKeySequenceProperty(index);
         break;

      default:
         break;
   }

   return index;
}

bool QDesignerPropertySheet::removeDynamicProperty(int index)
{
   if (! d->m_addIndex.contains(propertyName(index))) {
      return false;
   }

   setVisible(index, false);
   return true;
}

bool QDesignerPropertySheet::isDynamic(int index) const
{
   if (! d->m_addProperties.contains(index)) {
      return false;
   }

   switch (propertyType(index)) {
      case PropertyBuddy:
         if (d->m_objectType == ObjectLabel) {
            return false;
         }
         break;

      case PropertyLayoutLeftMargin:
      case PropertyLayoutTopMargin:
      case PropertyLayoutRightMargin:
      case PropertyLayoutBottomMargin:
      case PropertyLayoutSpacing:
      case PropertyLayoutHorizontalSpacing:
      case PropertyLayoutVerticalSpacing:
      case PropertyLayoutObjectName:
      case PropertyLayoutSizeConstraint:
      case PropertyLayoutFieldGrowthPolicy:
      case PropertyLayoutRowWrapPolicy:
      case PropertyLayoutLabelAlignment:
      case PropertyLayoutFormAlignment:
      case PropertyLayoutBoxStretch:
      case PropertyLayoutGridRowStretch:
      case PropertyLayoutGridColumnStretch:
      case PropertyLayoutGridRowMinimumHeight:
      case PropertyLayoutGridColumnMinimumWidth:
         if (d->m_object->isWidgetType() && d->m_canHaveLayoutAttributes) {
            return false;
         }

      default:
         break;
   }
   return true;
}

bool QDesignerPropertySheet::isDynamicProperty(int index) const
{
   // an invalid index might be encountered, do not sure error msg
   // if someone implements a property sheet only, omitting the dynamic sheet.

   if (index < 0 || index >= count()) {
      return false;
   }

   return d->m_info.value(index).kind == QDesignerPropertySheetPrivate::DynamicProperty;
}

bool QDesignerPropertySheet::isDefaultDynamicProperty(int index) const
{
   if (d->invalidIndex("QDesignerPropertySheet::isDefaultDynamicProperty)", index)) {
      return false;
   }

   return d->m_info.value(index).kind == QDesignerPropertySheetPrivate::DefaultDynamicProperty;
}

bool QDesignerPropertySheet::isResourceProperty(int index) const
{
   return d->isResourceProperty(index);
}

QVariant QDesignerPropertySheet::defaultResourceProperty(int index) const
{
   return d->defaultResourceProperty(index);
}

qdesigner_internal::DesignerPixmapCache *QDesignerPropertySheet::pixmapCache() const
{
   return d->m_pixmapCache;
}

void QDesignerPropertySheet::setPixmapCache(qdesigner_internal::DesignerPixmapCache *cache)
{
   d->m_pixmapCache = cache;
}

qdesigner_internal::DesignerIconCache *QDesignerPropertySheet::iconCache() const
{
   return d->m_iconCache;
}

void QDesignerPropertySheet::setIconCache(qdesigner_internal::DesignerIconCache *cache)
{
   d->m_iconCache = cache;
}

int QDesignerPropertySheet::createFakeProperty(const QString &propertyName, const QVariant &value)
{
   using Info = QDesignerPropertySheetPrivate::Info;

   // fake properties
   const int index = d->m_meta->indexOfProperty(propertyName);

   if (index != -1) {

      if (! (d->m_meta->property(index)->attributes() & QDesignerMetaPropertyInterface::DesignableAttribute)) {
         return -1;
      }

      Info &info   = d->ensureInfo(index);
      info.visible = false;
      info.kind    = QDesignerPropertySheetPrivate::FakeProperty;

      QVariant v = value.isValid() ? value : metaProperty(index);

      if (v.type() == QVariant::String) {
         v = QVariant::fromValue(qdesigner_internal::PropertySheetStringValue());
      }

      if (v.type() == QVariant::StringList) {
         v = QVariant::fromValue(qdesigner_internal::PropertySheetStringListValue());
      }

      if (v.type() == QVariant::KeySequence) {
         v = QVariant::fromValue(qdesigner_internal::PropertySheetKeySequenceValue());
      }

      d->m_fakeProperties.insert(index, v);

      return index;

   } else {
      // property was not found

      if (! value.isValid()) {
         return -1;
      }

      const int newIndex = count();

      d->m_addIndex.insert(propertyName, newIndex);
      d->m_addProperties.insert(newIndex, value);

      Info &info = d->ensureInfo(newIndex);
      info.propertyType = propertyTypeFromName(propertyName);
      info.kind  = QDesignerPropertySheetPrivate::FakeProperty;

      return newIndex;
   }
}

bool QDesignerPropertySheet::isAdditionalProperty(int index) const
{
   if (d->invalidIndex("QDesignerPropertySheet::isAdditionalProperty()", index)) {
      return false;
   }

   return d->m_addProperties.contains(index);
}

bool QDesignerPropertySheet::isFakeProperty(int index) const
{
   if (d->invalidIndex("QDesignerPropertySheet::isFakeProperty()", index)) {
      return false;
   }

   // additional properties must be fake
   return (d->m_fakeProperties.contains(index) || isAdditionalProperty(index));
}

int QDesignerPropertySheet::count() const
{
   return d->count();
}

int QDesignerPropertySheet::indexOf(const QString &name) const
{
   int index = d->m_meta->indexOfProperty(name);

   if (index == -1) {
      index = d->m_addIndex.value(name, -1);
   }

   return index;
}

QDesignerPropertySheet::PropertyType QDesignerPropertySheet::propertyType(int index) const
{
   if (d->invalidIndex("QDesignerPropertySheet::propertyType()", index)) {
      return PropertyNone;
   }

   return d->propertyType(index);
}

QDesignerPropertySheet::ObjectType QDesignerPropertySheet::objectType() const
{
   return d->m_objectType;
}

QString QDesignerPropertySheet::propertyName(int index) const
{
   if (d->invalidIndex("QDesignerPropertySheet::propertyName()", index)) {
      return QString();
   }

   if (isAdditionalProperty(index)) {
      return d->m_addIndex.key(index);
   }

   return d->m_meta->property(index)->name();
}

QString QDesignerPropertySheet::propertyGroup(int index) const
{
   if (d->invalidIndex("QDesignerPropertySheet::propertyGroup()", index)) {
      return QString();
   }

   const QString retval = d->m_info.value(index).group;

   if (! retval.isEmpty()) {
      return retval;
   }

   if (propertyType(index) == PropertyAccessibility) {
      return QString("Accessibility");
   }

   if (isAdditionalProperty(index)) {
      return d->m_meta->className();
   }

   return retval;
}

void QDesignerPropertySheet::setPropertyGroup(int index, const QString &group)
{
   if (d->invalidIndex("QDesignerPropertySheet::setPropertyGroup()", index)) {
      return;
   }

   d->ensureInfo(index).group = group;
}

QVariant QDesignerPropertySheet::property(int index) const
{
   if (d->invalidIndex("QDesignerPropertySheet::property()", index)) {
      return QVariant();
   }

   if (isAdditionalProperty(index)) {
      if (isFakeLayoutProperty(index)) {
         QDesignerPropertySheetExtension *layoutPropertySheet;

         if (d->layout(&layoutPropertySheet) && layoutPropertySheet) {
            const QString newPropName = d->transformLayoutPropertyName(index);

            if (! newPropName.isEmpty()) {
               const int newIndex = layoutPropertySheet->indexOf(newPropName);

               if (newIndex != -1) {
                  return layoutPropertySheet->property(newIndex);
               }

               return QVariant();
            }
         }
      }

      return d->m_addProperties.value(index);
   }

   if (isFakeProperty(index)) {
      return d->m_fakeProperties.value(index);
   }

   if (d->isResourceProperty(index)) {
      return d->resourceProperty(index);
   }

   if (d->isStringProperty(index)) {
      QString strValue = metaProperty(index).toString();
      qdesigner_internal::PropertySheetStringValue value = d->stringProperty(index);

      if (strValue != value.value()) {
         value.setValue(strValue);
         d->setStringProperty(index, value); // cache it
      }

      return QVariant::fromValue(value);
   }

   if (d->isStringListProperty(index)) {
      const QStringList listValue = metaProperty(index).toStringList();
      qdesigner_internal::PropertySheetStringListValue value = d->stringListProperty(index);

      if (listValue != value.value()) {
         value.setValue(listValue);
         d->setStringListProperty(index, value); // cache it
      }

      return QVariant::fromValue(value);
   }

   if (d->isKeySequenceProperty(index)) {
      QVariant data = metaProperty(index);
      QKeySequence keyValue = data.value<QKeySequence>();

      qdesigner_internal::PropertySheetKeySequenceValue value = d->keySequenceProperty(index);

      if (keyValue != value.value()) {
         value.setValue(keyValue);
         d->setKeySequenceProperty(index, value); // cache it
      }

      return QVariant::fromValue(value);
   }

   return metaProperty(index);
}

QVariant QDesignerPropertySheet::metaProperty(int index) const
{
   Q_ASSERT(! isFakeProperty(index));

   const QDesignerMetaPropertyInterface *p = d->m_meta->property(index);
   QVariant v = p->read(d->m_object);

   switch (p->kind()) {
      case QDesignerMetaPropertyInterface::FlagKind: {
         qdesigner_internal::PropertySheetFlagValue psflags = qdesigner_internal::PropertySheetFlagValue(
               v.toInt(), designerMetaFlagsFor(p->enumerator()));

         v.setValue(psflags);
      }
      break;

      case QDesignerMetaPropertyInterface::EnumKind: {
         qdesigner_internal::PropertySheetEnumValue pse = qdesigner_internal::PropertySheetEnumValue(
               v.toInt(), designerMetaEnumFor(p->enumerator()));

         v.setValue(pse);
      }
      break;

      case QDesignerMetaPropertyInterface::OtherKind:
         break;
   }

   return v;
}

QVariant QDesignerPropertySheet::resolvePropertyValue(int index, const QVariant &value) const
{
   if (value.canConvert<qdesigner_internal::PropertySheetEnumValue>()) {
      return value.value<qdesigner_internal::PropertySheetEnumValue>().value;
   }

   if (value.canConvert<qdesigner_internal::PropertySheetFlagValue>()) {
      return value.value<qdesigner_internal::PropertySheetFlagValue>().value;
   }

   if (value.canConvert<qdesigner_internal::PropertySheetStringValue>()) {
      return value.value<qdesigner_internal::PropertySheetStringValue>().value();
   }

   if (value.canConvert<qdesigner_internal::PropertySheetStringListValue>()) {
      return value.value<qdesigner_internal::PropertySheetStringListValue>().value();
   }

   if (value.canConvert<qdesigner_internal::PropertySheetKeySequenceValue>()) {
      return QVariant::fromValue(value.value<qdesigner_internal::PropertySheetKeySequenceValue>().value());
   }

   if (value.canConvert<qdesigner_internal::PropertySheetPixmapValue>()) {
      const QString path = value.value<qdesigner_internal::PropertySheetPixmapValue>().path();
      if (path.isEmpty()) {
         return defaultResourceProperty(index);
      }
      if (d->m_pixmapCache) {
         return d->m_pixmapCache->pixmap(value.value<qdesigner_internal::PropertySheetPixmapValue>());
      }
   }

   if (value.canConvert<qdesigner_internal::PropertySheetIconValue>()) {
      const unsigned mask = value.value<qdesigner_internal::PropertySheetIconValue>().mask();
      if (mask == 0) {
         return defaultResourceProperty(index);
      }
      if (d->m_iconCache) {
         return d->m_iconCache->icon(value.value<qdesigner_internal::PropertySheetIconValue>());
      }
   }

   return value;
}

void QDesignerPropertySheet::setFakeProperty(int index, const QVariant &value)
{
   Q_ASSERT(isFakeProperty(index));

   QVariant &v = d->m_fakeProperties[index];

   // set resource properties also (if we are going to have fake resource properties)
   if (value.canConvert<qdesigner_internal::PropertySheetFlagValue>() || value.canConvert<qdesigner_internal::PropertySheetEnumValue>()) {
      v = value;

   } else if (v.canConvert<qdesigner_internal::PropertySheetFlagValue>()) {
      qdesigner_internal::PropertySheetFlagValue f = v.value<qdesigner_internal::PropertySheetFlagValue>();
      f.value = value.toInt();
      v.setValue(f);
      Q_ASSERT(value.type() == QVariant::Int);

   } else if (v.canConvert<qdesigner_internal::PropertySheetEnumValue>()) {
      qdesigner_internal::PropertySheetEnumValue e = v.value<qdesigner_internal::PropertySheetEnumValue>();
      e.value = value.toInt();
      v.setValue(e);
      Q_ASSERT(value.type() == QVariant::Int);

   } else {
      v = value;
   }
}

void QDesignerPropertySheet::clearFakeProperties()
{
   d->m_fakeProperties.clear();
}

// Buddy needs to be byte array, else uic won't work
static QVariant toByteArray(const QVariant &value)
{
   if (value.type() == QVariant::ByteArray) {
      return value;
   }
   const QByteArray ba = value.toString().toUtf8();
   return QVariant(ba);
}

void QDesignerPropertySheet::setProperty(int index, const QVariant &value)
{
   if (d->invalidIndex("QDesignerPropertySheet::setProperty)", index)) {
      return;
   }

   if (isAdditionalProperty(index)) {
      if (d->m_objectType == ObjectLabel && propertyType(index) == PropertyBuddy) {
         QFormBuilderExtra::applyBuddy(value.toString(), QFormBuilderExtra::BuddyApplyVisibleOnly,
               dynamic_cast<QLabel *>(d->m_object.data()));

         d->m_addProperties[index] = toByteArray(value);

         return;
      }

      if (isFakeLayoutProperty(index)) {
         QDesignerPropertySheetExtension *layoutPropertySheet;

         if (d->layout(&layoutPropertySheet) && layoutPropertySheet) {
            const QString newPropName = d->transformLayoutPropertyName(index);

            if (! newPropName.isEmpty()) {
               const int newIndex = layoutPropertySheet->indexOf(newPropName);

               if (newIndex != -1) {
                  layoutPropertySheet->setProperty(newIndex, value);
               }
            }
         }
      }

      if (isDynamicProperty(index) || isDefaultDynamicProperty(index)) {
         if (d->isResourceProperty(index)) {
            d->setResourceProperty(index, value);
         }

         if (d->isStringProperty(index)) {
            d->setStringProperty(index, value.value<qdesigner_internal::PropertySheetStringValue>());
         }

         if (d->isStringListProperty(index)) {
            d->setStringListProperty(index, value.value<qdesigner_internal::PropertySheetStringListValue>());
         }

         if (d->isKeySequenceProperty(index)) {
            d->setKeySequenceProperty(index, value.value<qdesigner_internal::PropertySheetKeySequenceValue>());
         }

         d->m_object->setProperty(propertyName(index), resolvePropertyValue(index, value));

         if (d->m_object->isWidgetType()) {
            // emerald, why set the current style sheet to the same thing ?

            // QWidget *w = dynamic_cast<QWidget *>(d->m_object.data());
            // w->setStyleSheet(w->styleSheet());
         }
      }

      d->m_addProperties[index] = value;

   } else if (isFakeProperty(index)) {
      setFakeProperty(index, value);

   } else {
      if (d->isResourceProperty(index)) {
         d->setResourceProperty(index, value);
      }

      if (d->isStringProperty(index)) {
         d->setStringProperty(index, value.value<qdesigner_internal::PropertySheetStringValue>());
      }

      if (d->isStringListProperty(index)) {
         d->setStringListProperty(index, value.value<qdesigner_internal::PropertySheetStringListValue>());
      }

      if (d->isKeySequenceProperty(index)) {
         d->setKeySequenceProperty(index, value.value<qdesigner_internal::PropertySheetKeySequenceValue>());
      }

      const QDesignerMetaPropertyInterface *p = d->m_meta->property(index);
      p->write(d->m_object, resolvePropertyValue(index, value));

      if (dynamic_cast<QGroupBox *>(d->m_object.data()) && propertyType(index) == PropertyCheckable) {
         const int idx = indexOf("focusPolicy");

         if (! isChanged(idx)) {
            QVariant data = property(idx);
            qdesigner_internal::PropertySheetEnumValue e = data.value<qdesigner_internal::PropertySheetEnumValue>();

            if (value.toBool()) {
               const QDesignerMetaPropertyInterface *propPtr = d->m_meta->property(idx);
               propPtr->write(d->m_object, Qt::NoFocus);
               e.value = Qt::StrongFocus;

               QVariant v;
               v.setValue(e);
               setFakeProperty(idx, v);

            } else {
               e.value = Qt::NoFocus;

               QVariant v;
               v.setValue(e);
               setFakeProperty(idx, v);
            }
         }
      }
   }
}

bool QDesignerPropertySheet::hasReset(int index) const
{
   if (d->invalidIndex("QDesignerPropertySheet::hasReset()", index)) {
      return false;
   }

   if (isAdditionalProperty(index)) {
      return d->m_info.value(index).reset;
   }

   return true;
}

bool QDesignerPropertySheet::reset(int index)
{
   if (d->invalidIndex("QDesignerPropertySheet::reset()", index)) {
      return false;
   }

   if (d->isStringProperty(index)) {
      qdesigner_internal::PropertySheetStringValue value;

      // main container, reset to the stored class name to avoid changing the file names generated by uic
      if (propertyName(index) == "objectName") {
         const QVariant classNameDefaultV = d->m_object->property("_q_classname");

         if (classNameDefaultV.isValid()) {
            value.setValue(classNameDefaultV.toString());
         }

      } else if (! isAdditionalProperty(index)) {
         const QDesignerMetaPropertyInterface *property = d->m_meta->property(index);

         if ((property->accessFlags() & QDesignerMetaPropertyInterface::ResetAccess) && property->reset(d->m_object)) {
            value.setValue(property->read(d->m_object).toString());
         } else {
            return false;
         }
      }

      setProperty(index, QVariant::fromValue(value));

      return true;
   }

   if (d->isStringListProperty(index)) {
      setProperty(index, QVariant::fromValue(qdesigner_internal::PropertySheetStringListValue()));
   }

   if (d->isKeySequenceProperty(index)) {
      setProperty(index, QVariant::fromValue(qdesigner_internal::PropertySheetKeySequenceValue()));
   }

   if (d->isResourceProperty(index)) {
      setProperty(index, d->emptyResourceProperty(index));
      return true;

   } else if (isDynamic(index)) {
      const QString propName  = propertyName(index);
      const QVariant oldValue = d->m_addProperties.value(index);
      const QVariant defaultValue = d->m_info.value(index).defaultValue;

      QVariant newValue = defaultValue;

      if (d->isStringProperty(index)) {
         newValue = QVariant::fromValue(qdesigner_internal::PropertySheetStringValue(newValue.toString()));

      } else if (d->isStringListProperty(index)) {
         newValue = QVariant::fromValue(qdesigner_internal::PropertySheetStringListValue(newValue.toStringList()));

      } else if (d->isKeySequenceProperty(index)) {
         const QKeySequence keySequence = newValue.value<QKeySequence>();
         newValue = QVariant::fromValue(qdesigner_internal::PropertySheetKeySequenceValue(keySequence));
      }

      if (oldValue == newValue) {
         return true;
      }

      d->m_object->setProperty(propName.toUtf8(), defaultValue);
      d->m_addProperties[index] = newValue;

      return true;

   } else if (d->m_info.value(index).defaultValue.isValid()) {
      setProperty(index, d->m_info.value(index).defaultValue);
      return true;
   }

   if (isAdditionalProperty(index)) {
      const PropertyType pType = propertyType(index);

      if (d->m_objectType == ObjectLabel && pType == PropertyBuddy) {
         setProperty(index, QVariant(QByteArray()));
         return true;
      }

      if (isFakeLayoutProperty(index)) {
         // special properties

         switch (pType) {
            case PropertyLayoutObjectName:
               setProperty(index, QString());
               return true;

            case PropertyLayoutSizeConstraint:
               setProperty(index, QVariant(QLayout::SetDefaultConstraint));
               return true;

            case PropertyLayoutBoxStretch:
            case PropertyLayoutGridRowStretch:
            case PropertyLayoutGridColumnStretch:
            case PropertyLayoutGridRowMinimumHeight:
            case PropertyLayoutGridColumnMinimumWidth:
            case PropertyLayoutFieldGrowthPolicy:
            case PropertyLayoutRowWrapPolicy:
            case PropertyLayoutLabelAlignment:
            case PropertyLayoutFormAlignment: {
               QDesignerPropertySheetExtension *layoutPropertySheet;

               if (d->layout(&layoutPropertySheet) && layoutPropertySheet) {
                  return layoutPropertySheet->reset(layoutPropertySheet->indexOf(d->transformLayoutPropertyName(index)));
               }
            }
            break;

            default:
               break;
         }

         // special margins
         int value = -1;

         switch (d->m_objectType) {
            case ObjectLayoutWidget:
               if (pType == PropertyLayoutLeftMargin || pType == PropertyLayoutTopMargin ||
                     pType == PropertyLayoutRightMargin || pType == PropertyLayoutBottomMargin) {
                  value = 0;
               }
               break;

            default:
               break;
         }

         setProperty(index, value);
         return true;
      }

      return false;

   } else if (isFakeProperty(index)) {
      const QDesignerMetaPropertyInterface *p = d->m_meta->property(index);
      const bool result = p->reset(d->m_object);
      d->m_fakeProperties[index] = p->read(d->m_object);

      return result;

   } else if (propertyType(index) == PropertyGeometry && d->m_object->isWidgetType()) {
      if (QWidget *w = dynamic_cast<QWidget *>(d->m_object.data())) {
         QWidget *widget = w;

         if (qdesigner_internal::Utils::isCentralWidget(d->m_fwb, widget) && d->m_fwb->parentWidget()) {
            widget = d->m_fwb->parentWidget();
         }

         if (widget != w && widget->parentWidget()) {
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            widget->parentWidget()->adjustSize();
         }

         QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
         widget->adjustSize();

         return true;
      }
   }

   const QDesignerMetaPropertyInterface *p = d->m_meta->property(index);

   // properties like sizePolicy will reach here, PropertyHelper::restoreDefaultValue() will take over

   return p->reset(d->m_object);
}

bool QDesignerPropertySheet::isChanged(int index) const
{
   if (d->invalidIndex("QDesignerPropertySheet::isChanged()", index)) {
      return false;
   }

   if (isAdditionalProperty(index)) {
      if (isFakeLayoutProperty(index)) {
         QDesignerPropertySheetExtension *layoutPropertySheet;

         if (d->layout(&layoutPropertySheet) && layoutPropertySheet) {
            const QString newPropName = d->transformLayoutPropertyName(index);

            if (! newPropName.isEmpty()) {
               const int newIndex = layoutPropertySheet->indexOf(newPropName);
               if (newIndex != -1) {
                  return layoutPropertySheet->isChanged(newIndex);
               }

               return false;
            }
         }
      }
   }

   return d->m_info.value(index).changed;
}

void QDesignerPropertySheet::setChanged(int index, bool changed)
{
   if (d->invalidIndex("QDesignerPropertySheet::setChanged()", index)) {
      return;
   }

   if (isAdditionalProperty(index)) {
      if (isFakeLayoutProperty(index)) {
         QDesignerPropertySheetExtension *layoutPropertySheet;

         if (d->layout(&layoutPropertySheet) && layoutPropertySheet) {
            const QString newPropName = d->transformLayoutPropertyName(index);

            if (! newPropName.isEmpty()) {
               const int newIndex = layoutPropertySheet->indexOf(newPropName);

               if (newIndex != -1) {
                  layoutPropertySheet->setChanged(newIndex, changed);
               }
            }
         }
      }
   }

   if (d->isReloadableProperty(index)) {
      if (d->m_fwb) {
         if (changed) {
            d->m_fwb->addReloadableProperty(this, index);
         } else {
            d->m_fwb->removeReloadableProperty(this, index);
         }
      }
   }
   d->ensureInfo(index).changed = changed;
}

bool QDesignerPropertySheet::isFakeLayoutProperty(int index) const
{
   if (!isAdditionalProperty(index)) {
      return false;
   }

   switch (propertyType(index)) {
      case PropertyLayoutObjectName:
      case PropertyLayoutSizeConstraint:
         return true;

      case PropertyLayoutLeftMargin:
      case PropertyLayoutTopMargin:
      case PropertyLayoutRightMargin:
      case PropertyLayoutBottomMargin:
      case PropertyLayoutSpacing:
      case PropertyLayoutHorizontalSpacing:
      case PropertyLayoutVerticalSpacing:
      case PropertyLayoutFieldGrowthPolicy:
      case PropertyLayoutRowWrapPolicy:
      case PropertyLayoutLabelAlignment:
      case PropertyLayoutFormAlignment:
      case PropertyLayoutBoxStretch:
      case PropertyLayoutGridRowStretch:
      case PropertyLayoutGridColumnStretch:
      case PropertyLayoutGridRowMinimumHeight:
      case PropertyLayoutGridColumnMinimumWidth:
         return d->m_canHaveLayoutAttributes;

      default:
         break;
   }
   return false;
}

// Determine the "designable" state of a property. Properties, which have a per-object
// boolean test function that returns false are shown in isabled state
// ("checked" depending on "checkable", etc.)
// Properties, which are generally not designable independent of the object are not shown

enum DesignableState {
   PropertyIsDesignable,

   // Object has a Designable test function that returns false
   PropertyOfObjectNotDesignable,
   PropertyNotDesignable
};

static DesignableState designableState(const QDesignerMetaPropertyInterface *p, const QObject *object)
{
   if (p->attributes(object) & QDesignerMetaPropertyInterface::DesignableAttribute) {
      return PropertyIsDesignable;
   }

   if (p->attributes() & QDesignerMetaPropertyInterface::DesignableAttribute) {
      return PropertyOfObjectNotDesignable;

   } else {
      return PropertyNotDesignable;
   }
}

bool QDesignerPropertySheet::isVisible(int index) const
{
   if (d->invalidIndex("QDesignerPropertySheet::isVisible()", index)) {
      return false;
   }

   const PropertyType type = propertyType(index);

   if (isAdditionalProperty(index)) {

      if (isFakeLayoutProperty(index) && d->m_object->isWidgetType()) {
         const QLayout *currentLayout = d->layout();

         if (currentLayout == nullptr) {
            return false;
         }

         const int visibleMask = qdesigner_internal::LayoutProperties::visibleProperties(currentLayout);

         switch (type) {
            case  PropertyLayoutSpacing:
               return visibleMask & qdesigner_internal::LayoutProperties::SpacingProperty;

            case PropertyLayoutHorizontalSpacing:
            case PropertyLayoutVerticalSpacing:
               return visibleMask & qdesigner_internal::LayoutProperties::HorizSpacingProperty;

            case PropertyLayoutFieldGrowthPolicy:
               return visibleMask & qdesigner_internal::LayoutProperties::FieldGrowthPolicyProperty;

            case PropertyLayoutRowWrapPolicy:
               return visibleMask & qdesigner_internal::LayoutProperties::RowWrapPolicyProperty;

            case PropertyLayoutLabelAlignment:
               return visibleMask & qdesigner_internal::LayoutProperties::LabelAlignmentProperty;

            case PropertyLayoutFormAlignment:
               return visibleMask & qdesigner_internal::LayoutProperties::FormAlignmentProperty;

            case PropertyLayoutBoxStretch:
               return visibleMask & qdesigner_internal::LayoutProperties::BoxStretchProperty;

            case PropertyLayoutGridRowStretch:
               return visibleMask & qdesigner_internal::LayoutProperties::GridRowStretchProperty;

            case PropertyLayoutGridColumnStretch:
               return visibleMask & qdesigner_internal::LayoutProperties::GridColumnStretchProperty;

            case PropertyLayoutGridRowMinimumHeight:
               return visibleMask & qdesigner_internal::LayoutProperties::GridRowMinimumHeightProperty;

            case PropertyLayoutGridColumnMinimumWidth:
               return visibleMask & qdesigner_internal::LayoutProperties::GridColumnMinimumWidthProperty;

            default:
               break;
         }

         return true;
      }

      return d->m_info.value(index).visible;
   }

   if (isFakeProperty(index)) {
      switch (type) {
         case PropertyWindowModality: // Hidden for child widgets
         case PropertyWindowOpacity:
            return d->m_info.value(index).visible;

         default:
            break;
      }

      return true;
   }

   const bool visible = d->m_info.value(index).visible;

   switch (type) {
      case PropertyWindowTitle:
      case PropertyWindowIcon:
      case PropertyWindowFilePath:
      case PropertyWindowOpacity:
      case PropertyWindowIconText:
      case PropertyWindowModified:
         return visible;

      default:
         if (visible) {
            return true;
         }

         break;
   }

   const QDesignerMetaPropertyInterface *p = d->m_meta->property(index);

   if  (! (p->accessFlags() & QDesignerMetaPropertyInterface::WriteAccess)) {
      return false;
   }

   // Enabled handling: Hide only statically not designable properties
   return designableState(p, d->m_object) != PropertyNotDesignable;
}

void QDesignerPropertySheet::setVisible(int index, bool visible)
{
   if (d->invalidIndex("QDesignerPropertySheet::setVisible()", index)) {
      return;
   }

   d->ensureInfo(index).visible = visible;
}

bool QDesignerPropertySheet::isEnabled(int index) const
{
   if (d->invalidIndex("QDesignerPropertySheet::isEnabled()", index)) {
      return false;
   }

   if (isAdditionalProperty(index)) {
      return true;
   }

   if (isFakeProperty(index)) {
      return true;
   }

   // Grey out geometry of laid-out widgets (including splitter)
   if (propertyType(index) == PropertyGeometry && d->m_object->isWidgetType()) {
      bool isManaged;

      const qdesigner_internal::LayoutInfo::Type lt = qdesigner_internal::LayoutInfo::laidoutWidgetType(d->m_core,
            dynamic_cast<QWidget *>(d->m_object.data()), &isManaged);

      return !isManaged || lt == qdesigner_internal::LayoutInfo::NoLayout;
   }

   if (d->m_info.value(index).visible == true) {
      return true;
   }

   // Enable setting of properties for statically non-designable properties
   // as this might be done via TaskMenu/Cursor::setProperty. Note that those
   // properties are not visible.

   const QDesignerMetaPropertyInterface *p = d->m_meta->property(index);

   return (p->accessFlags() & QDesignerMetaPropertyInterface::WriteAccess) &&
      designableState(p, d->m_object) != PropertyOfObjectNotDesignable;
}

bool QDesignerPropertySheet::isAttribute(int index) const
{
   if (d->invalidIndex("QDesignerPropertySheet::isAttribute()", index)) {
      return false;
   }

   if (isAdditionalProperty(index)) {
      return d->m_info.value(index).attribute;
   }

   if (isFakeProperty(index)) {
      return false;
   }

   return d->m_info.value(index).attribute;
}

void QDesignerPropertySheet::setAttribute(int index, bool attribute)
{
   if (d->invalidIndex("QDesignerPropertySheet::setAttribute)", index)) {
      return;
   }

   d->ensureInfo(index).attribute = attribute;
}

QDesignerFormEditorInterface *QDesignerPropertySheet::core() const
{
   return d->m_core;
}

bool QDesignerPropertySheet::internalDynamicPropertiesEnabled()
{
   return QDesignerPropertySheetPrivate::m_internalDynamicPropertiesEnabled;
}

void QDesignerPropertySheet::setInternalDynamicPropertiesEnabled(bool v)
{
   QDesignerPropertySheetPrivate::m_internalDynamicPropertiesEnabled = v;
}

struct QDesignerAbstractPropertySheetFactory::PropertySheetFactoryPrivate {
   PropertySheetFactoryPrivate();
   const QString m_propertySheetId;
   const QString m_dynamicPropertySheetId;

   QMap<QObject *, QObject *> m_extensions;
   QHash<QObject *, bool> m_extended;
};

QDesignerAbstractPropertySheetFactory::PropertySheetFactoryPrivate::PropertySheetFactoryPrivate() :
   m_propertySheetId(CS_TYPEID(QDesignerPropertySheetExtension)),
   m_dynamicPropertySheetId(CS_TYPEID(QDesignerDynamicPropertySheetExtension))
{
}

QDesignerAbstractPropertySheetFactory::QDesignerAbstractPropertySheetFactory(QExtensionManager *parent) :
   QExtensionFactory(parent),
   m_impl(new PropertySheetFactoryPrivate)
{
}

QDesignerAbstractPropertySheetFactory::~QDesignerAbstractPropertySheetFactory()
{
   delete m_impl;
}

QObject *QDesignerAbstractPropertySheetFactory::extension(QObject *object, const QString &iid) const
{
   if (! object) {
      return nullptr;
   }

   if (iid != m_impl->m_propertySheetId && iid != m_impl->m_dynamicPropertySheetId) {
      return nullptr;
   }

   auto it = m_impl->m_extensions.find(object);

   if (it == m_impl->m_extensions.end()) {
      if (QObject *ext = createPropertySheet(object, const_cast<QDesignerAbstractPropertySheetFactory *>(this))) {
         connect(ext, &QObject::destroyed, this, &QDesignerAbstractPropertySheetFactory::objectDestroyed);
         it = m_impl->m_extensions.insert(object, ext);
      }
   }

   if (! m_impl->m_extended.contains(object)) {
      connect(object, &QObject::destroyed, this, &QDesignerAbstractPropertySheetFactory::objectDestroyed);
      m_impl->m_extended.insert(object, true);
   }

   if (it == m_impl->m_extensions.end()) {
      return nullptr;
   }

   return it.value();
}

void QDesignerAbstractPropertySheetFactory::objectDestroyed(QObject *object)
{
   QMutableMapIterator<QObject *, QObject *> it(m_impl->m_extensions);

   while (it.hasNext()) {
      it.next();

      QObject *o = it.key();
      if (o == object || object == it.value()) {
         it.remove();
      }
   }

   m_impl->m_extended.remove(object);
}
