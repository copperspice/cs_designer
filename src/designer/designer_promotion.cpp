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
#include <abstract_formwindow.h>
#include <abstract_widgetbox.h>
#include <abstract_widgetdatabase.h>
#include <designer_object_inspector.h>
#include <designer_promotion.h>

#include <formwindowbase_p.h>
#include <metadatabase_p.h>
#include <widgetdatabase_p.h>

#include <QCoreApplication>
#include <QDebug>
#include <QMap>

namespace {
// Return a set of on-promotable classes
const QSet<QString> &nonPromotableClasses()
{
   static QSet<QString> rc;
   if (rc.empty()) {
      rc.insert(QString("Line"));
      rc.insert(QString("QAction"));
      rc.insert(QString("Spacer"));
      rc.insert(QString("QMainWindow"));
      rc.insert(QString("QDialog"));
      rc.insert(QString("QMdiArea"));
      rc.insert(QString("QMdiSubWindow"));
   }
   return rc;
}

// Return widget database index of a promoted class or -1 with error message
int promotedWidgetDataBaseIndex(const QDesignerWidgetDataBaseInterface *widgetDataBase,
   const QString &className,
   QString *errorMessage)
{
   const int index = widgetDataBase->indexOfClassName(className);
   if (index == -1 || !widgetDataBase->item(index)->isPromoted()) {
      *errorMessage = QCoreApplication::tr("%1 is not a promoted class.").formatArg(className);
      return -1;
   }
   return index;
}

// Return widget database item of a promoted class or 0 with error message
QDesignerWidgetDataBaseItemInterface *promotedWidgetDataBaseItem(const QDesignerWidgetDataBaseInterface *widgetDataBase,
   const QString &className,
   QString *errorMessage)
{

   const int index =  promotedWidgetDataBaseIndex(widgetDataBase, className, errorMessage);
   if (index == -1) {
      return nullptr;
   }
   return widgetDataBase->item(index);
}

// extract class name from xml  "<widget class="QWidget" ...>"
QString classNameFromXml(QString xml)
{
   static const QString tag = QString("class=\"");
   const int pos = xml.indexOf(tag);
   if (pos == -1) {
      return QString();
   }
   xml.remove(0, pos + tag.size());
   const int closingPos = xml.indexOf(QLatin1Char('"'));
   if (closingPos == -1) {
      return QString();
   }
   xml.remove(closingPos, xml.size() - closingPos);
   return xml;
}

// return a list of class names in the scratch pad
QStringList getScratchPadClasses(const QDesignerWidgetBoxInterface *wb)
{
   QStringList rc;
   const int catCount =  wb->categoryCount();

   for (int c = 0; c <  catCount; c++) {
      const QDesignerWidgetBoxInterface::Category category = wb->category(c);

      if (category.type() == QDesignerWidgetBoxInterface::Category::Scratchpad) {
         const int widgetCount = category.widgetCount();

         for (int w = 0; w < widgetCount; w++) {
            const QString className = classNameFromXml( category.widget(w).domXml());

            if (! className.isEmpty()) {
               rc += className;
            }
         }
      }
   }

   return rc;
}
}

namespace qdesigner_internal {

QDesignerPromotion::QDesignerPromotion(QDesignerFormEditorInterface *core)
   : m_core(core)
{
}

bool  QDesignerPromotion::addPromotedClass(const QString &baseClass,
      const QString &className, const QString &includeFile, QString *errorMessage)
{
   QDesignerWidgetDataBaseInterface *widgetDataBase = m_core->widgetDataBase();
   const int baseClassIndex = widgetDataBase->indexOfClassName(baseClass);

   if (baseClassIndex == -1) {
      *errorMessage = QCoreApplication::tr("The base class %1 is invalid.").formatArg(baseClass);
      return false;
   }

   const int existingClassIndex = widgetDataBase->indexOfClassName(className);

   if (existingClassIndex != -1) {
      *errorMessage = QCoreApplication::tr("The class %1 already exists.").formatArg(className);
      return false;
   }

   // Clone derived item

   QDesignerWidgetDataBaseItemInterface *promotedItem = WidgetDataBaseItem::clone(widgetDataBase->item(baseClassIndex));
   // Also inherit the container flag in case of QWidget-derived classes
   // as it is most likely intended for stacked pages, set new props
   promotedItem->setName(className);
   promotedItem->setGroup(QCoreApplication::tr("Promoted Widgets"));
   promotedItem->setCustom(true);
   promotedItem->setPromoted(true);
   promotedItem->setExtends(baseClass);
   promotedItem->setIncludeFile(includeFile);
   widgetDataBase->append(promotedItem);

   return true;
}

QList<QDesignerWidgetDataBaseItemInterface *> QDesignerPromotion::promotionBaseClasses() const
{
   typedef QMap<QString, QDesignerWidgetDataBaseItemInterface *> SortedDatabaseItemMap;
   SortedDatabaseItemMap sortedDatabaseItemMap;

   QDesignerWidgetDataBaseInterface *widgetDataBase = m_core->widgetDataBase();

   const int cnt = widgetDataBase->count();
   for (int i = 0; i <  cnt; i++) {
      QDesignerWidgetDataBaseItemInterface *dbItem = widgetDataBase->item(i);
      if (canBePromoted(dbItem)) {
         sortedDatabaseItemMap.insert(dbItem->name(), dbItem);
      }
   }

   return sortedDatabaseItemMap.values();
}


bool QDesignerPromotion::canBePromoted(const QDesignerWidgetDataBaseItemInterface *dbItem) const
{
   if (dbItem->isPromoted() ||  !dbItem->extends().isEmpty()) {
      return false;
   }

   const QString name = dbItem->name();

   if (nonPromotableClasses().contains(name)) {
      return false;
   }

   if (name.startsWith(QString("QDesigner")) ||
      name.startsWith(QString("QLayout"))) {
      return false;
   }

   return true;
}

QDesignerPromotion::PromotedClasses QDesignerPromotion::promotedClasses()  const
{
   typedef QMap<QString, QDesignerWidgetDataBaseItemInterface *> ClassNameItemMap;
   // A map containing base classes and their promoted classes.
   typedef QMap<QString, ClassNameItemMap> BaseClassPromotedMap;

   BaseClassPromotedMap baseClassPromotedMap;

   QDesignerWidgetDataBaseInterface *widgetDataBase = m_core->widgetDataBase();
   // Look for promoted classes and insert into map according to base class.
   const  int cnt = widgetDataBase->count();
   for (int i = 0; i < cnt; i++) {
      QDesignerWidgetDataBaseItemInterface *dbItem = widgetDataBase->item(i);
      if (dbItem->isPromoted()) {
         const QString baseClassName = dbItem->extends();
         BaseClassPromotedMap::iterator it = baseClassPromotedMap.find(baseClassName);
         if (it == baseClassPromotedMap.end()) {
            it = baseClassPromotedMap.insert(baseClassName, ClassNameItemMap());
         }
         it.value().insert(dbItem->name(), dbItem);
      }
   }
   // convert map into list.
   PromotedClasses rc;

   if (baseClassPromotedMap.empty()) {
      return rc;
   }

   const BaseClassPromotedMap::const_iterator bcend = baseClassPromotedMap.constEnd();
   for (BaseClassPromotedMap::const_iterator bit = baseClassPromotedMap.constBegin(); bit !=  bcend; ++bit) {
      const int baseIndex = widgetDataBase->indexOfClassName(bit.key());
      Q_ASSERT(baseIndex >= 0);
      QDesignerWidgetDataBaseItemInterface *baseItem = widgetDataBase->item(baseIndex);
      // promoted
      const ClassNameItemMap::const_iterator pcend = bit.value().constEnd();
      for (ClassNameItemMap::const_iterator pit = bit.value().constBegin(); pit != pcend; ++pit) {
         PromotedClass item;
         item.baseItem = baseItem;
         item.promotedItem = pit.value();
         rc.push_back(item);
      }
   }

   return rc;
}

QSet<QString> QDesignerPromotion::referencedPromotedClassNames()  const
{
   QSet<QString> rc;
   const MetaDataBase *metaDataBase = dynamic_cast<const MetaDataBase *>(m_core->metaDataBase());
   if (!metaDataBase) {
      return rc;
   }

   const QList<QObject *> objs = metaDataBase->objects();
   const QList<QObject *>::const_iterator cend = objs.constEnd();
   for ( QList<QObject *>::const_iterator it = objs.constBegin(); it != cend; ++it) {
      const QString customClass = metaDataBase->metaDataBaseItem(*it)->customClassName();
      if (!customClass.isEmpty()) {
         rc.insert(customClass);
      }

   }
   // check the scratchpad of the widget box
   if (QDesignerWidgetBoxInterface *widgetBox = m_core->widgetBox()) {
      const QStringList scratchPadClasses = getScratchPadClasses(widgetBox);
      if (!scratchPadClasses.empty()) {
         // Check whether these are actually promoted
         QDesignerWidgetDataBaseInterface *widgetDataBase = m_core->widgetDataBase();

         for (const QString &str : scratchPadClasses ) {
            const int index = widgetDataBase->indexOfClassName(str);

            if (index != -1 && widgetDataBase->item(index)->isPromoted()) {
               rc += str;
            }
         }
      }
   }
   return rc;
}

bool QDesignerPromotion::removePromotedClass(const QString &className, QString *errorMessage)
{
   // check if it exists and is promoted
   WidgetDataBase *widgetDataBase = dynamic_cast<WidgetDataBase *>(m_core->widgetDataBase());
   if (!widgetDataBase) {
      *errorMessage = QCoreApplication::tr("The class %1 cannot be removed").formatArg(className);
      return false;
   }

   const int index = promotedWidgetDataBaseIndex(widgetDataBase, className, errorMessage);
   if (index == -1) {
      return false;
   }

   if (referencedPromotedClassNames().contains(className)) {
      *errorMessage = QCoreApplication::tr("Cclass %1 can not be removed because it is still referenced.")
            .formatArg(className);

      return false;
   }
   // QTBUG-52963: Check for classes that specify the to-be-removed class as
   // base class of a promoted class. This should not happen in the normal case
   // as promoted classes cannot serve as base for further promotion. It is possible
   // though if a class provided by a plugin (say WebKit's QWebView) is used as
   // a base class for a promoted widget B and the plugin is removed in the next
   // launch. QWebView will then appear as promoted class itself and the promoted
   // class B will depend on it. When removing QWebView, the base class of B will
   // be changed to that of QWebView by the below code.

   const PromotedClasses promotedList = promotedClasses();

   for (PromotedClasses::const_iterator it = promotedList.constBegin(), end = promotedList.constEnd(); it != end; ++it) {
      if (it->baseItem->name() == className) {
         const QString extends = widgetDataBase->item(index)->extends();
         qWarning().nospace() << "Warning: Promoted class " << it->promotedItem->name()
            << " extends " << className << ", changing its base class to " <<  extends << '.';
         it->promotedItem->setExtends(extends);
      }
   }
   widgetDataBase->remove(index);
   return true;
}

bool QDesignerPromotion::changePromotedClassName(const QString &oldclassName, const QString &newClassName, QString *errorMessage)
{
   const MetaDataBase *metaDataBase = dynamic_cast<const MetaDataBase *>(m_core->metaDataBase());
   if (!metaDataBase) {
      *errorMessage = QCoreApplication::tr("The class %1 cannot be renamed").formatArg(oldclassName);
      return false;
   }
   QDesignerWidgetDataBaseInterface *widgetDataBase = m_core->widgetDataBase();

   // check the new name
   if (newClassName.isEmpty()) {
      *errorMessage = QCoreApplication::tr("The class %1 cannot be renamed to an empty name.").formatArg(oldclassName);
      return false;
   }
   const int existingIndex = widgetDataBase->indexOfClassName(newClassName);
   if (existingIndex != -1) {
      *errorMessage = QCoreApplication::tr("There is already a class named %1.").formatArg(newClassName);
      return false;
   }
   // Check old class
   QDesignerWidgetDataBaseItemInterface *dbItem = promotedWidgetDataBaseItem(widgetDataBase, oldclassName, errorMessage);
   if (!dbItem) {
      return false;
   }

   // Change the name in the data base and change all referencing objects in the meta database
   dbItem->setName(newClassName);
   bool foundReferences = false;
   for (QObject *object : metaDataBase->objects()) {
      MetaDataBaseItem *item =  metaDataBase->metaDataBaseItem(object);
      Q_ASSERT(item);
      if (item->customClassName() == oldclassName) {
         item->setCustomClassName(newClassName);
         foundReferences = true;
      }
   }
   // set state
   if (foundReferences) {
      refreshObjectInspector();
   }

   return true;
}

bool QDesignerPromotion::setPromotedClassIncludeFile(const QString &className, const QString &includeFile, QString *errorMessage)
{
   // check file
   if (includeFile.isEmpty()) {
      *errorMessage = QCoreApplication::tr("Cannot set an empty include file.");
      return false;
   }

   // check item
   QDesignerWidgetDataBaseInterface *widgetDataBase = m_core->widgetDataBase();
   QDesignerWidgetDataBaseItemInterface *dbItem = promotedWidgetDataBaseItem(widgetDataBase, className, errorMessage);

   if (!dbItem) {
      return false;
   }

   dbItem->setIncludeFile(includeFile);
   return true;
}

void QDesignerPromotion::refreshObjectInspector()
{
   if (QDesignerFormWindowManagerInterface *fwm = m_core->formWindowManager()) {
      if (QDesignerFormWindowInterface *fw = fwm->activeFormWindow())
         if ( QDesignerObjectInspectorInterface *oi = m_core->objectInspector()) {
            oi->setFormWindow(fw);
         }
   }
}

}   // end namespace


