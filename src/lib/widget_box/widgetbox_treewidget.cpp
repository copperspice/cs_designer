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
#include <abstract_settings.h>
#include <customwidget.h>
#include <designer_dnditem.h>
#include <designer_settings.h>
#include <designer_utils.h>
#include <plugin_manager.h>
#include <ui4.h>
#include <widgetbox_categorylistview.h>
#include <widgetbox_treewidget.h>

#include <iconloader_p.h>
#include <sheet_delegate_p.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDebug>
#include <QFile>
#include <QHeaderView>
#include <QMenu>
#include <QTimer>
static const QString widgetBoxRootElementC = "widgetbox";
static const QString widgetElementC        = "widget";
static const QString uiElementC            = "ui";
static const QString categoryElementC      = "category";
static const QString categoryEntryElementC = "categoryentry";
static const QString nameAttributeC        = "name";
static const QString typeAttributeC        = "type";
static const QString iconAttributeC        = "icon";
static const QString defaultTypeValueC     = "default";
static const QString customValueC          = "custom";
static const QString iconPrefixC           = "__qt_icon__";
static const QString scratchPadValueC      = "scratchpad";
static const QString invisibleNameC        = "[invisible]";

static const QString widgetBoxSettingsGroupC = "WidgetBox";
static const QString widgetBoxExpandedKeyC   = "Closed categories";
static const QString widgetBoxViewModeKeyC   = "View mode";

enum TopLevelRole  {
   NORMAL_ITEM,
   SCRATCHPAD_ITEM,
   CUSTOM_ITEM
};

static void setTopLevelRole(TopLevelRole tlr, QTreeWidgetItem *item)
{
   item->setData(0, Qt::UserRole, QVariant(tlr));
}

static TopLevelRole topLevelRole(const  QTreeWidgetItem *item)
{
   return static_cast<TopLevelRole>(item->data(0, Qt::UserRole).toInt());
}

namespace qdesigner_internal {

WidgetBoxTreeWidget::WidgetBoxTreeWidget(QDesignerFormEditorInterface *core, QWidget *parent)
   : QTreeWidget(parent), m_core(core), m_iconMode(false), m_scratchPadDeleteTimer(nullptr)
{
   setFocusPolicy(Qt::NoFocus);
   setIndentation(0);
   setRootIsDecorated(false);
   setColumnCount(1);
   header()->hide();
   header()->setSectionResizeMode(QHeaderView::Stretch);
   setTextElideMode(Qt::ElideMiddle);
   setVerticalScrollMode(ScrollPerPixel);

   setItemDelegate(new SheetDelegate(this, this));

   connect(this, &QTreeWidget::itemPressed,
         this, &WidgetBoxTreeWidget::handleMousePress);
}

QIcon WidgetBoxTreeWidget::iconForWidget(QString iconName) const
{
   if (iconName.isEmpty()) {
      QIcon iconImage = QIcon(":/resources/form_editor/images/scratchpad-32.png");
      return iconImage;
   }

   if (iconName.startsWith(iconPrefixC)) {
      const IconCache::const_iterator it = m_pluginIcons.constFind(iconName);

      if (it != m_pluginIcons.constEnd()) {
         return it.value();
      }
   }

   return createIconSet(iconName);
}

WidgetBoxCategoryListView *WidgetBoxTreeWidget::categoryViewAt(int idx) const
{
   WidgetBoxCategoryListView *rc = nullptr;

   if (QTreeWidgetItem *cat_item = topLevelItem(idx)) {
      if (QTreeWidgetItem *embedItem = cat_item->child(0)) {
         rc = dynamic_cast<WidgetBoxCategoryListView *>(itemWidget(embedItem, 0));
      }
   }

   Q_ASSERT(rc);

   return rc;
}

void WidgetBoxTreeWidget::saveExpandedState() const
{
   QStringList closedCategories;

   if (const int numCategories = categoryCount()) {
      for (int i = 0; i < numCategories; ++i) {
         const QTreeWidgetItem *cat_item = topLevelItem(i);
         if (!isItemExpanded(cat_item)) {
            closedCategories.append(cat_item->text(0));
         }
      }
   }

   QDesignerSettingsInterface *settings = m_core->settingsManager();
   settings->beginGroup(widgetBoxSettingsGroupC);
   settings->setValue(widgetBoxExpandedKeyC, closedCategories);
   settings->setValue(widgetBoxViewModeKeyC, m_iconMode);
   settings->endGroup();
}

void  WidgetBoxTreeWidget::restoreExpandedState()
{
   QDesignerSettingsInterface *settings = m_core->settingsManager();
   const QString groupKey = widgetBoxSettingsGroupC + '/';

   m_iconMode = settings->value(groupKey + widgetBoxViewModeKeyC).toBool();
   updateViewMode();

   const QSet<QString> closedCategories = settings->value(groupKey + widgetBoxExpandedKeyC, QStringList()).toStringList().toSet();
   expandAll();

   if (closedCategories.empty()) {
      return;
   }

   if (const int numCategories = categoryCount()) {
      for (int i = 0; i < numCategories; ++i) {
         QTreeWidgetItem *item = topLevelItem(i);

         if (closedCategories.contains(item->text(0))) {
            item->setExpanded(false);
         }
      }
   }
}

WidgetBoxTreeWidget::~WidgetBoxTreeWidget()
{
   saveExpandedState();
}

void WidgetBoxTreeWidget::setFileName(const QString &file_name)
{
   m_file_name = file_name;
}

QString WidgetBoxTreeWidget::fileName() const
{
   return m_file_name;
}

bool WidgetBoxTreeWidget::save()
{
   if (fileName().isEmpty()) {
      return false;
   }

   QFile file(fileName());
   if (!file.open(QIODevice::WriteOnly)) {
      return false;
   }

   CategoryList cat_list;
   const int count = categoryCount();
   for (int i = 0; i < count; ++i) {
      cat_list.append(category(i));
   }

   QXmlStreamWriter writer(&file);
   writer.setAutoFormatting(true);
   writer.setAutoFormattingIndent(1);
   writer.writeStartDocument();
   writeCategories(writer, cat_list);
   writer.writeEndDocument();

   return true;
}

void WidgetBoxTreeWidget::slotSave()
{
   save();
}

void WidgetBoxTreeWidget::handleMousePress(QTreeWidgetItem *item)
{
   if (item == nullptr) {
      return;
   }

   if (QApplication::mouseButtons() != Qt::LeftButton) {
      return;
   }

   if (item->parent() == nullptr) {
      setItemExpanded(item, !isItemExpanded(item));
      return;
   }
}

int WidgetBoxTreeWidget::ensureScratchpad()
{
   const int existingIndex = indexOfScratchpad();
   if (existingIndex != -1) {
      return existingIndex;
   }

   QTreeWidgetItem *scratch_item = new QTreeWidgetItem(this);
   scratch_item->setText(0, tr("Scratchpad"));
   setTopLevelRole(SCRATCHPAD_ITEM, scratch_item);
   addCategoryView(scratch_item, false); // Scratchpad in list mode.

   return categoryCount() - 1;
}

WidgetBoxCategoryListView *WidgetBoxTreeWidget::addCategoryView(QTreeWidgetItem *parent, bool iconMode)
{
   QTreeWidgetItem *embed_item = new QTreeWidgetItem(parent);
   embed_item->setFlags(Qt::ItemIsEnabled);
   WidgetBoxCategoryListView *categoryView = new WidgetBoxCategoryListView(m_core, this);
   categoryView->setViewMode(iconMode ? QListView::IconMode : QListView::ListMode);

   connect(categoryView, &WidgetBoxCategoryListView::scratchPadChanged,
      this, &WidgetBoxTreeWidget::slotSave);

   connect(categoryView, &WidgetBoxCategoryListView::pressed,
      this, &WidgetBoxTreeWidget::pressed);

   connect(categoryView, &WidgetBoxCategoryListView::itemRemoved,
      this, &WidgetBoxTreeWidget::slotScratchPadItemDeleted);

   connect(categoryView, &WidgetBoxCategoryListView::lastItemRemoved,
      this, &WidgetBoxTreeWidget::slotLastScratchPadItemDeleted);

   setItemWidget(embed_item, 0, categoryView);
   return categoryView;
}

int WidgetBoxTreeWidget::indexOfScratchpad() const
{
   if (const int numTopLevels =  topLevelItemCount()) {
      for (int i = numTopLevels - 1; i >= 0; --i) {
         if (topLevelRole(topLevelItem(i)) == SCRATCHPAD_ITEM) {
            return i;
         }
      }
   }
   return -1;
}

int WidgetBoxTreeWidget::indexOfCategory(const QString &name) const
{
   const int topLevelCount = topLevelItemCount();
   for (int i = 0; i < topLevelCount; ++i) {
      if (topLevelItem(i)->text(0) == name) {
         return i;
      }
   }
   return -1;
}

bool WidgetBoxTreeWidget::load(QDesignerWidgetBox::LoadMode loadMode)
{
   switch (loadMode) {
      case QDesignerWidgetBox::LoadReplace:
         clear();
         break;
      case QDesignerWidgetBox::LoadCustomWidgetsOnly:
         addCustomCategories(true);
         updateGeometries();
         return true;
      default:
         break;
   }

   const QString name = fileName();

   QFile f(name);
   if (!f.open(QIODevice::ReadOnly)) { // Might not exist at first startup
      return false;
   }

   const QString contents = QString::fromUtf8(f.readAll());
   return loadContents(contents);
}

bool WidgetBoxTreeWidget::loadContents(const QString &contents)
{
   QString errorMessage;
   CategoryList cat_list;
   if (!readCategories(m_file_name, contents, &cat_list, &errorMessage)) {
      csWarning(errorMessage);
      return false;
   }

   for (const Category &cat : cat_list) {
      addCategory(cat);
   }

   addCustomCategories(false);
   // Restore which items are expanded
   restoreExpandedState();
   return true;
}

void WidgetBoxTreeWidget::addCustomCategories(bool replace)
{
   if (replace) {
      // clear out all existing custom widgets
      if (const int numTopLevels =  topLevelItemCount()) {
         for (int t = 0; t < numTopLevels ; ++t) {
            categoryViewAt(t)->removeCustomWidgets();
         }
      }
   }
   // re-add
   const CategoryList customList = loadCustomCategoryList();
   const CategoryList::const_iterator cend = customList.constEnd();
   for (CategoryList::const_iterator it = customList.constBegin(); it != cend; ++it) {
      addCategory(*it);
   }
}

static inline QString msgXmlError(const QString &fileName, const QXmlStreamReader &r)
{
   return QDesignerWidgetBox::tr("An error has been encountered at line %1 of %2: %3")
      .formatArg(r.lineNumber()).formatArgs(fileName, r.errorString());
}

bool WidgetBoxTreeWidget::readCategories(const QString &fileName, const QString &contents,
   CategoryList *cats, QString *errorMessage)
{
   // Read widget box XML:
   //
   //<widgetbox version="4.5">
   // <category name="Layouts">
   //  <categoryentry name="Vertical Layout" icon="win/editvlayout.png" type="default">
   //   <widget class="QListWidget" ...>
   // ...

   QXmlStreamReader reader(contents);


   // Entries of category with name="invisible" should be ignored
   bool ignoreEntries = false;

   while (!reader.atEnd()) {
      switch (reader.readNext()) {
         case QXmlStreamReader::StartElement: {
            const QStringView tag = reader.name();

            if (tag == widgetBoxRootElementC) {
               //<widgetbox version="4.5">
               continue;
            }
            if (tag == categoryElementC) {
               // <category name="Layouts">
               const QXmlStreamAttributes attributes = reader.attributes();
               const QString categoryName = attributes.value(nameAttributeC).toString();

               if (categoryName == invisibleNameC) {
                  ignoreEntries = true;
               } else {
                  Category category(categoryName);
                  if (attributes.value(typeAttributeC) == scratchPadValueC) {
                     category.setType(Category::Scratchpad);
                  }
                  cats->push_back(category);
               }
               continue;
            }

            if (tag == categoryEntryElementC) {
               //  <categoryentry name="Vertical Layout" icon="win/editvlayout.png" type="default">
               if (!ignoreEntries) {
                  QXmlStreamAttributes attr = reader.attributes();

                  const QString widgetName = attr.value(nameAttributeC).toString();
                  const QString widgetIcon = attr.value(iconAttributeC).toString();

                  const WidgetBoxTreeWidget::Widget::Type widgetType =
                     attr.value(typeAttributeC).toString() == customValueC ?
                     WidgetBoxTreeWidget::Widget::Custom :
                     WidgetBoxTreeWidget::Widget::Default;

                  Widget w;
                  w.setName(widgetName);
                  w.setIconName(widgetIcon);
                  w.setType(widgetType);
                  if (!readWidget(&w, contents, reader)) {
                     continue;
                  }

                  cats->back().addWidget(w);
               } // ignoreEntries
               continue;
            }
            break;
         }
         case QXmlStreamReader::EndElement: {
            const QStringView tag = reader.name();
            if (tag == widgetBoxRootElementC) {
               continue;
            }
            if (tag == categoryElementC) {
               ignoreEntries = false;
               continue;
            }
            if (tag == categoryEntryElementC) {
               continue;
            }
            break;
         }
         default:
            break;
      }
   }

   if (reader.hasError()) {
      *errorMessage = msgXmlError(fileName, reader);
      return false;
   }

   return true;
}

/*!
 * Read out a widget within a category. This can either be
 * enclosed in a <ui> element or a (legacy) <widget> element which may
 * contain nested <widget> elements.
 *
 * Examples:
 *
 * <ui language="c++">
 *  <widget class="MultiPageWidget" name="multipagewidget"> ... </widget>
 *  <customwidgets>...</customwidgets>
 * <ui>
 *
 * or
 *
 * <widget>
 *   <widget> ... </widget>
 *   ...
 * <widget>
 *
 * Returns true on success, false if end was reached or an error has been encountered
 * in which case the reader has its error flag set. If successful, the current item
 * of the reader will be the closing element (</ui> or </widget>)
 */
bool WidgetBoxTreeWidget::readWidget(Widget *w, const QString &xml, QXmlStreamReader &r)
{
   qint64 startTagPosition = 0, endTagPosition = 0;

   int nesting = 0;
   bool endEncountered = false;
   bool parsedWidgetTag = false;
   QString outmostElement;
   while (!endEncountered) {
      const qint64 currentPosition = r.characterOffset();
      switch (r.readNext()) {
         case QXmlStreamReader::StartElement:
            if (nesting++ == 0) {
               // First element must be <ui> or (legacy) <widget>
               const QStringView name = r.name();
               if (name == uiElementC) {
                  startTagPosition = currentPosition;
               } else {
                  if (name == widgetElementC) {
                     startTagPosition = currentPosition;
                     parsedWidgetTag = true;
                  } else {
                     r.raiseError(QDesignerWidgetBox::tr("Unexpected element <%1> encountered when parsing for <widget> or <ui>").formatArg(
                           name.toString()));
                     return false;
                  }
               }
            } else {
               // We are within <ui> looking for the first <widget> tag
               if (!parsedWidgetTag && r.name() == widgetElementC) {
                  parsedWidgetTag = true;
               }
            }
            break;
         case QXmlStreamReader::EndElement:
            // Reached end of widget?
            if (--nesting == 0)  {
               endTagPosition = r.characterOffset();
               endEncountered = true;
            }
            break;
         case QXmlStreamReader::EndDocument:
            r.raiseError(QDesignerWidgetBox::tr("Unexpected end of file encountered when parsing widgets."));
            return false;
         case QXmlStreamReader::Invalid:
            return false;
         default:
            break;
      }
   }
   if (!parsedWidgetTag) {
      r.raiseError(QDesignerWidgetBox::tr("A widget element could not be found."));
      return false;
   }
   // Oddity: Startposition is 1 off
   QString widgetXml = xml.mid(startTagPosition, endTagPosition - startTagPosition);
   const QChar lessThan = QLatin1Char('<');
   if (!widgetXml.startsWith(lessThan)) {
      widgetXml.prepend(lessThan);
   }
   w->setDomXml(widgetXml);
   return true;
}

void WidgetBoxTreeWidget::writeCategories(QXmlStreamWriter &writer, const CategoryList &cat_list) const
{
   const QString widgetbox = widgetBoxRootElementC;
   const QString name = nameAttributeC;
   const QString type = typeAttributeC;
   const QString icon = iconAttributeC;
   const QString defaultType = defaultTypeValueC;
   const QString category = categoryElementC;
   const QString categoryEntry = categoryEntryElementC;
   const QString iconPrefix = iconPrefixC;
   const QString widgetTag = widgetElementC;

   //
   // <widgetbox>
   //   <category name="Layouts">
   //     <categoryEntry name="Vertical Layout" type="default" icon="win/editvlayout.png">
   //       <ui>
   //        ...
   //       </ui>
   //     </categoryEntry>
   //     ...
   //   </category>
   //   ...
   // </widgetbox>
   //

   writer.writeStartElement(widgetbox);

   for (const Category &cat : cat_list) {
      writer.writeStartElement(category);
      writer.writeAttribute(name, cat.name());
      if (cat.type() == Category::Scratchpad) {
         writer.writeAttribute(type, scratchPadValueC);
      }

      const int widgetCount = cat.widgetCount();
      for (int i = 0; i < widgetCount; ++i) {
         const  Widget wgt = cat.widget(i);
         if (wgt.type() == Widget::Custom) {
            continue;
         }

         writer.writeStartElement(categoryEntry);
         writer.writeAttribute(name, wgt.name());
         if (!wgt.iconName().startsWith(iconPrefix)) {
            writer.writeAttribute(icon, wgt.iconName());
         }
         writer.writeAttribute(type, defaultType);

         const DomUI *domUI = QDesignerWidgetBox::xmlToUi(wgt.name(), WidgetBoxCategoryListView::widgetDomXml(wgt), false);
         if (domUI) {
            domUI->write(writer);
            delete domUI;
         }

         writer.writeEndElement(); // categoryEntry
      }
      writer.writeEndElement(); // categoryEntry
   }

   writer.writeEndElement(); // widgetBox
}

static int findCategory(const QString &name, const WidgetBoxTreeWidget::CategoryList &list)
{
   int idx = 0;
   for (const WidgetBoxTreeWidget::Category &cat : list) {
      if (cat.name() == name) {
         return idx;
      }
      ++idx;
   }
   return -1;
}

static inline bool isValidIcon(const QIcon &icon)
{
   if (!icon.isNull()) {
      const QList<QSize> availableSizes = icon.availableSizes();
      if (!availableSizes.empty()) {
         return !availableSizes.front().isEmpty();
      }
   }
   return false;
}

WidgetBoxTreeWidget::CategoryList WidgetBoxTreeWidget::loadCustomCategoryList() const
{
   CategoryList result;

   const QDesignerPluginManager *pm = m_core->pluginManager();
   const QDesignerPluginManager::CustomWidgetList customWidgets = pm->registeredCustomWidgets();
   if (customWidgets.empty()) {
      return result;
   }

   static const QString customCatName = tr("Custom Widgets");

   const QString invisible = invisibleNameC;
   const QString iconPrefix = iconPrefixC;

   for (QDesignerCustomWidgetInterface *c : customWidgets) {
      const QString dom_xml = c->domXml();
      if (dom_xml.isEmpty()) {
         continue;
      }

      const QString pluginName = c->name();
      const QDesignerCustomWidgetData data = pm->customWidgetData(c);
      QString displayName = data.xmlDisplayName();
      if (displayName.isEmpty()) {
         displayName = pluginName;
      }

      QString cat_name = c->group();
      if (cat_name.isEmpty()) {
         cat_name = customCatName;
      } else if (cat_name == invisible) {
         continue;
      }

      int idx = findCategory(cat_name, result);
      if (idx == -1) {
         result.append(Category(cat_name));
         idx = result.size() - 1;
      }
      Category &cat = result[idx];

      const QIcon icon = c->icon();

      QString icon_name;
      if (isValidIcon(icon)) {
         icon_name = iconPrefix;
         icon_name += pluginName;
         m_pluginIcons.insert(icon_name, icon);
      }

      cat.addWidget(Widget(displayName, dom_xml, icon_name, Widget::Custom));
   }

   return result;
}

void WidgetBoxTreeWidget::adjustSubListSize(QTreeWidgetItem *cat_item)
{
   QTreeWidgetItem *embedItem = cat_item->child(0);

   if (embedItem == nullptr) {
      return;
   }

   WidgetBoxCategoryListView *list_widget = static_cast<WidgetBoxCategoryListView *>(itemWidget(embedItem, 0));
   list_widget->setFixedWidth(header()->width());
   list_widget->doItemsLayout();
   const int height = qMax(list_widget->contentsSize().height(), 1);
   list_widget->setFixedHeight(height);
   embedItem->setSizeHint(0, QSize(-1, height - 1));
}

int WidgetBoxTreeWidget::categoryCount() const
{
   return topLevelItemCount();
}

WidgetBoxTreeWidget::Category WidgetBoxTreeWidget::category(int cat_idx) const
{
   if (cat_idx >= topLevelItemCount()) {
      return Category();
   }

   QTreeWidgetItem *cat_item = topLevelItem(cat_idx);

   QTreeWidgetItem *embedItem = cat_item->child(0);
   WidgetBoxCategoryListView *categoryView = static_cast<WidgetBoxCategoryListView *>(itemWidget(embedItem, 0));

   Category result = categoryView->category();
   result.setName(cat_item->text(0));

   switch (topLevelRole(cat_item)) {
      case SCRATCHPAD_ITEM:
         result.setType(Category::Scratchpad);
         break;
      default:
         result.setType(Category::Default);
         break;
   }
   return result;
}

void WidgetBoxTreeWidget::addCategory(const Category &cat)
{
   if (cat.widgetCount() == 0) {
      return;
   }

   const bool isScratchPad = cat.type() == Category::Scratchpad;
   WidgetBoxCategoryListView *categoryView;
   QTreeWidgetItem *cat_item;

   if (isScratchPad) {
      const int idx = ensureScratchpad();
      categoryView = categoryViewAt(idx);
      cat_item = topLevelItem(idx);

   } else {
      const int existingIndex = indexOfCategory(cat.name());

      if (existingIndex == -1) {
         cat_item = new QTreeWidgetItem();
         cat_item->setText(0, cat.name());
         setTopLevelRole(NORMAL_ITEM, cat_item);

         // insert before scratchpad
         const int scratchPadIndex = indexOfScratchpad();
         if (scratchPadIndex == -1) {
            addTopLevelItem(cat_item);
         } else {
            insertTopLevelItem(scratchPadIndex, cat_item);
         }

         setItemExpanded(cat_item, true);
         categoryView = addCategoryView(cat_item, m_iconMode);

      } else {
         categoryView = categoryViewAt(existingIndex);
         cat_item = topLevelItem(existingIndex);
      }
   }

   // The same categories are read from the file $HOME, avoid duplicates
   const int widgetCount = cat.widgetCount();

   for (int i = 0; i < widgetCount; ++i) {
      const Widget w = cat.widget(i);

      if (! categoryView->containsWidget(w.name())) {
         categoryView->addWidget(w, iconForWidget(w.iconName()), isScratchPad);
      }
   }

   adjustSubListSize(cat_item);
}

void WidgetBoxTreeWidget::removeCategory(int cat_idx)
{
   if (cat_idx >= topLevelItemCount()) {
      return;
   }
   delete takeTopLevelItem(cat_idx);
}

int WidgetBoxTreeWidget::widgetCount(int cat_idx) const
{
   if (cat_idx >= topLevelItemCount()) {
      return 0;
   }

   // SDK functions want unfiltered access
   return categoryViewAt(cat_idx)->count(WidgetBoxCategoryListView::UnfilteredAccess);
}

WidgetBoxTreeWidget::Widget WidgetBoxTreeWidget::widget(int cat_idx, int wgt_idx) const
{
   if (cat_idx >= topLevelItemCount()) {
      return Widget();
   }

   // SDK functions want unfiltered access
   WidgetBoxCategoryListView *categoryView = categoryViewAt(cat_idx);
   return categoryView->widgetAt(WidgetBoxCategoryListView::UnfilteredAccess, wgt_idx);
}

void WidgetBoxTreeWidget::addWidget(int cat_idx, const Widget &wgt)
{
   if (cat_idx >= topLevelItemCount()) {
      return;
   }

   QTreeWidgetItem *cat_item = topLevelItem(cat_idx);
   WidgetBoxCategoryListView *categoryView = categoryViewAt(cat_idx);

   const bool scratch = topLevelRole(cat_item) == SCRATCHPAD_ITEM;
   categoryView->addWidget(wgt, iconForWidget(wgt.iconName()), scratch);
   adjustSubListSize(cat_item);
}

void WidgetBoxTreeWidget::removeWidget(int cat_idx, int wgt_idx)
{
   if (cat_idx >= topLevelItemCount()) {
      return;
   }

   WidgetBoxCategoryListView *categoryView = categoryViewAt(cat_idx);

   // SDK functions want unfiltered access
   const WidgetBoxCategoryListView::AccessMode am = WidgetBoxCategoryListView::UnfilteredAccess;
   if (wgt_idx >= categoryView->count(am)) {
      return;
   }

   categoryView->removeRow(am, wgt_idx);
}

void WidgetBoxTreeWidget::slotScratchPadItemDeleted()
{
   const int scratch_idx = indexOfScratchpad();
   QTreeWidgetItem *scratch_item = topLevelItem(scratch_idx);
   adjustSubListSize(scratch_item);
   save();
}

void WidgetBoxTreeWidget::slotLastScratchPadItemDeleted()
{
   // Remove the scratchpad in the next idle loop
   if (!m_scratchPadDeleteTimer) {
      m_scratchPadDeleteTimer = new QTimer(this);
      m_scratchPadDeleteTimer->setSingleShot(true);
      m_scratchPadDeleteTimer->setInterval(0);
      connect(m_scratchPadDeleteTimer, &QTimer::timeout,
         this, &WidgetBoxTreeWidget::deleteScratchpad);
   }
   if (!m_scratchPadDeleteTimer->isActive()) {
      m_scratchPadDeleteTimer->start();
   }
}

void WidgetBoxTreeWidget::deleteScratchpad()
{
   const int idx = indexOfScratchpad();
   if (idx == -1) {
      return;
   }
   delete takeTopLevelItem(idx);
   save();
}


void WidgetBoxTreeWidget::slotListMode()
{
   m_iconMode = false;
   updateViewMode();
}

void WidgetBoxTreeWidget::slotIconMode()
{
   m_iconMode = true;
   updateViewMode();
}

void WidgetBoxTreeWidget::updateViewMode()
{
   if (const int numTopLevels = topLevelItemCount()) {
      for (int i = numTopLevels - 1; i >= 0; --i) {
         QTreeWidgetItem *topLevel = topLevelItem(i);

         // Scratch pad stays in list mode
         const QListView::ViewMode viewMode  = m_iconMode &&
            (topLevelRole(topLevel) != SCRATCHPAD_ITEM) ? QListView::IconMode : QListView::ListMode;
         WidgetBoxCategoryListView *categoryView = categoryViewAt(i);

         if (viewMode != categoryView->viewMode()) {
            categoryView->setViewMode(viewMode);
            adjustSubListSize(topLevelItem(i));
         }
      }
   }

   updateGeometries();
}

void WidgetBoxTreeWidget::resizeEvent(QResizeEvent *e)
{
   QTreeWidget::resizeEvent(e);
   if (const int numTopLevels = topLevelItemCount()) {
      for (int i = numTopLevels - 1; i >= 0; --i) {
         adjustSubListSize(topLevelItem(i));
      }
   }
}

void WidgetBoxTreeWidget::contextMenuEvent(QContextMenuEvent *e)
{
   QTreeWidgetItem *item = itemAt(e->pos());

   const bool scratchpad_menu = item != nullptr && item->parent() != nullptr
         && topLevelRole(item->parent()) ==  SCRATCHPAD_ITEM;

   QMenu menu;
   menu.addAction(tr("Expand all"), this, &WidgetBoxTreeWidget::expandAll);
   menu.addAction(tr("Collapse all"), this, &WidgetBoxTreeWidget::collapseAll);
   menu.addSeparator();

   QAction *listModeAction = menu.addAction(tr("List View"));
   QAction *iconModeAction = menu.addAction(tr("Icon View"));

   listModeAction->setCheckable(true);
   iconModeAction->setCheckable(true);

   QActionGroup *viewModeGroup = new QActionGroup(&menu);
   viewModeGroup->addAction(listModeAction);
   viewModeGroup->addAction(iconModeAction);

   if (m_iconMode) {
      iconModeAction->setChecked(true);
   } else {
      listModeAction->setChecked(true);
   }
   connect(listModeAction, &QAction::triggered, this, &WidgetBoxTreeWidget::slotListMode);
   connect(iconModeAction, &QAction::triggered, this, &WidgetBoxTreeWidget::slotIconMode);

   if (scratchpad_menu) {
      menu.addSeparator();
      WidgetBoxCategoryListView *listView = dynamic_cast<WidgetBoxCategoryListView *>(itemWidget(item, 0));
      Q_ASSERT(listView);
      menu.addAction(tr("Remove"), listView, &WidgetBoxCategoryListView::removeCurrentItem);
      if (!m_iconMode) {
         menu.addAction(tr("Edit name"), listView, &WidgetBoxCategoryListView::editCurrentItem);
      }
   }

   e->accept();
   menu.exec(mapToGlobal(e->pos()));
}

void WidgetBoxTreeWidget::dropWidgets(const QList<QDesignerDnDItemInterface *> &item_list)
{
   QTreeWidgetItem *scratch_item = nullptr;
   WidgetBoxCategoryListView *categoryView = nullptr;
   bool added = false;

   for (QDesignerDnDItemInterface *item : item_list) {
      QWidget *w = item->widget();

      if (w == nullptr) {
         continue;
      }

      DomUI *dom_ui = item->domUi();
      if (dom_ui == nullptr) {
         continue;
      }

      const int scratch_idx = ensureScratchpad();
      scratch_item = topLevelItem(scratch_idx);
      categoryView = categoryViewAt(scratch_idx);

      // Temporarily remove the fake toplevel in-between
      DomWidget *fakeTopLevel = dom_ui->takeElementWidget();
      DomWidget *firstWidget  = nullptr;

      if (fakeTopLevel && !fakeTopLevel->elementWidget().isEmpty()) {
         firstWidget = fakeTopLevel->elementWidget().first();
         dom_ui->setElementWidget(firstWidget);
      } else {
         dom_ui->setElementWidget(fakeTopLevel);
         continue;
      }

      // Serialize to XML
      QString xml;
      {
         QXmlStreamWriter writer(&xml);
         writer.setAutoFormatting(true);
         writer.setAutoFormattingIndent(1);
         writer.writeStartDocument();
         dom_ui->write(writer);
         writer.writeEndDocument();
      }

      // Insert fake toplevel again
      dom_ui->takeElementWidget();
      dom_ui->setElementWidget(fakeTopLevel );

      const Widget scratchWidget = Widget(w->objectName(), xml);
      QIcon iconImage = QIcon(":/resources/form_editor/images/scratchpad-32.png");
      categoryView->addWidget(scratchWidget, iconImage, true);

      setItemExpanded(scratch_item, true);
      added = true;
   }

   if (added) {
      save();
      QApplication::setActiveWindow(this);

      // Is the new item visible in filtered mode?
      const WidgetBoxCategoryListView::AccessMode am = WidgetBoxCategoryListView::FilteredAccess;

      if (const int count = categoryView->count(am)) {
         categoryView->setCurrentItem(am, count - 1);
      }

      categoryView->adjustSize();
      adjustSubListSize(scratch_item);
   }
}

void WidgetBoxTreeWidget::filter(const QString &filterStr)
{
   QRegularExpression regexp;

   if (! filterStr.isEmpty()) {
      regexp = QRegularExpression(QRegularExpression::escape(filterStr), QPatternOption::CaseInsensitiveOption);
   }

   const int numTopLevels = topLevelItemCount();
   bool changed = false;

   for (int i = 0; i < numTopLevels; ++i) {
      QTreeWidgetItem *tl = topLevelItem(i);

      WidgetBoxCategoryListView *categoryView = categoryViewAt(i);

      // Anything changed? -> Enable the category
      const int oldCount = categoryView->count(WidgetBoxCategoryListView::FilteredAccess);
      categoryView->filter(regexp);

      const int newCount = categoryView->count(WidgetBoxCategoryListView::FilteredAccess);

      if (oldCount != newCount) {
         changed = true;
         const bool categoryEnabled = newCount > 0 || filterStr.isEmpty();

         if (categoryEnabled) {
            categoryView->adjustSize();
            adjustSubListSize(tl);
         }

         setRowHidden(i, QModelIndex(), ! categoryEnabled);
      }
   }

   if (changed) {
      updateGeometries();
   }
}

}  // namespace qdesigner_internal

