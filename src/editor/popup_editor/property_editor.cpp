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

#include <abstract_formeditor.h>
#include <abstract_settings.h>
#include <abstract_widgetdatabase.h>
#include <designer_property.h>
#include <designer_property_command.h>
#include <designer_propertysheet.h>
#include <designer_propertysheet.h>
#include <designer_settings.h>
#include <designer_utils.h>
#include <dynamicpropertysheet.h>
#include <edit_variant_property.h>
#include <extension.h>
#include <newdynamic_property.h>
#include <property_editor.h>
#include <property_view_button.h>
#include <property_view_tree.h>
#include <widgetfactory.h>

#include <formwindowbase_p.h>
#include <iconloader_p.h>
#include <metadatabase_p.h>
#include <shared_enums_p.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTextStream>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <qalgorithms.h>

static const QString ColorKeyC            = "Colored";
static const QString ExpansionKeyC        = "ExpandedItems";
static const QString SettingsGroupC       = "PropertyEditor";
static const QString SortedKeyC           = "Sorted";
static const QString SplitterPositionKeyC = "SplitterPosition";
static const QString ViewKeyC             = "View";

enum SettingsView {
   TreeView,
   ButtonView
};

namespace qdesigner_internal {

// ----------- ElidingLabel
// QLabel does not support text eliding so we need a helper class

class ElidingLabel : public QWidget
{
 public:
   ElidingLabel(const QString &text = QString(), QWidget *parent = nullptr)
      : QWidget(parent), m_text(text), m_mode(Qt::ElideRight)
   {
      setContentsMargins(3, 2, 3, 2);
   }

   QSize sizeHint() const override;
   void paintEvent(QPaintEvent *e) override;

   void setText(const QString &text) {
      m_text = text;
      updateGeometry();
   }

   void setElidemode(Qt::TextElideMode mode) {
      m_mode = mode;
      updateGeometry();
   }

 private:
   QString m_text;
   Qt::TextElideMode m_mode;
};

QSize ElidingLabel::sizeHint() const
{
   QSize size = fontMetrics().boundingRect(m_text).size();
   size += QSize(contentsMargins().left() + contentsMargins().right(),
         contentsMargins().top() + contentsMargins().bottom());
   return size;
}

void ElidingLabel::paintEvent(QPaintEvent *)
{
   QPainter painter(this);
   painter.setPen(QColor(0, 0, 0, 60));
   painter.setBrush(QColor(255, 255, 255, 40));
   painter.drawRect(rect().adjusted(0, 0, -1, -1));
   painter.setPen(palette().windowText().color());
   painter.drawText(contentsRect(), Qt::AlignLeft,
      fontMetrics().elidedText(m_text, Qt::ElideRight, width(), 0));
}

PropertyEditor::Strings::Strings()
   : m_fontProperty(QString("font")), m_qLayoutWidget(QString("QLayoutWidget")), m_designerPrefix(QString("QDesigner")),
     m_layout(QString("Layout")), m_validationModeAttribute(QString("validationMode")), m_fontAttribute(QString("font")),
     m_superPaletteAttribute(QString("superPalette")), m_enumNamesAttribute(QString("enumNames")),
     m_resettableAttribute(QString("resettable")), m_flagsAttribute(QString("flags"))
{
   m_alignmentProperties.insert(QString("alignment"));
   m_alignmentProperties.insert(QString("layoutLabelAlignment"));       // QFormLayout
   m_alignmentProperties.insert(QString("layoutFormAlignment"));
}

QDesignerMetaDataBaseItemInterface *PropertyEditor::metaDataBaseItem() const
{
   QObject *o = object();

   if (!o) {
      return nullptr;
   }

   QDesignerMetaDataBaseInterface *db = core()->metaDataBase();
   if (!db) {
      return nullptr;
   }
   return db->item(o);
}

void PropertyEditor::setupStringProperty(QtVariantProperty *property, bool isMainContainer)
{
   const TextValidPair params = textPropertyValidationMode(core(),
         m_object, property->propertyName(), isMainContainer);

   // Does a meta DB entry exist - add comment
   const bool hasComment = params.second;
   property->setAttribute(m_strings.m_validationModeAttribute, params.first);

   // assuming comment cannot appear or disappear for the same property in different object instance
   if (!hasComment) {
      qDeleteAll(property->subProperties());
   }
}

void PropertyEditor::setupPaletteProperty(QtVariantProperty *property)
{
   QPalette value = property->value().value<QPalette>();

   QPalette superPalette  = QPalette();
   QWidget *currentWidget = dynamic_cast<QWidget *>(m_object.data());

   if (currentWidget != nullptr) {
      if (currentWidget->isWindow()) {
         superPalette = QApplication::palette(currentWidget);
      } else {
         if (currentWidget->parentWidget()) {
            superPalette = currentWidget->parentWidget()->palette();
         }
      }
   }

   m_updatingBrowser = true;
   property->setAttribute(m_strings.m_superPaletteAttribute, superPalette);
   m_updatingBrowser = false;
}

static inline QToolButton *createDropDownButton(QAction *defaultAction, QWidget *parent = nullptr)
{
   QToolButton *rc = new QToolButton(parent);
   rc->setDefaultAction(defaultAction);
   rc->setPopupMode(QToolButton::InstantPopup);

   return rc;
}

PropertyEditor::PropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent, Qt::WindowFlags flags)
    : QDesignerPropertyEditor(parent, flags), m_core(core), m_propertySheet(nullptr), m_currentBrowser(nullptr),
      m_treeBrowser(nullptr), m_propertyManager(new DesignerPropertyManager(m_core, this)),
      m_dynamicGroup(nullptr), m_updatingBrowser(false), m_stackedWidget(new QStackedWidget),
      m_filterWidget(new QLineEdit), m_buttonIndex(-1), m_treeIndex(-1),
      m_addDynamicAction(new QAction(createIconSet("plus.png"), tr("Add Dynamic Property..."), this)),
      m_removeDynamicAction(new QAction(createIconSet("minus.png"), tr("Remove Dynamic Property"), this)),
      m_sortingAction(new QAction(createIconSet("sort.png"), tr("Sorting"), this)),
      m_coloringAction(new QAction(createIconSet("color.png"), tr("Color Groups"), this)),
      m_treeAction(new QAction(tr("Tree View"), this)),
      m_buttonAction(new QAction(tr("Drop Down Button View"), this)),
      m_classLabel(new ElidingLabel), m_sorting(false), m_coloring(false), m_brightness(false)
{
   QVector<QColor> colors;
   colors.push_back(QColor(255, 230, 191));      // pale orange
   colors.push_back(QColor(255, 255, 191));      // pale yellow
   colors.push_back(QColor(191, 255, 191));      // pale green
   colors.push_back(QColor(199, 255, 255));      // pale blue
   colors.push_back(QColor(234, 191, 255));      // pale purple
   colors.push_back(QColor(255, 191, 239));      // pale pink

   const int darknessFactor = 250;

   for (int i = 0; i < colors.count(); i++) {
      QColor c = colors.at(i);
      m_colors.push_back(std::make_pair(c, c.darker(darknessFactor)));
   }

   QColor dynamicColor(191, 207, 255);           // pale blue-grey
   QColor layoutColor(255, 191, 191);            // pale red

   m_dynamicColor = std::make_pair(dynamicColor, dynamicColor.darker(darknessFactor));
   m_layoutColor  = std::make_pair(layoutColor, layoutColor.darker(darknessFactor));

   updateForegroundBrightness();

   QActionGroup *actionGroup = new QActionGroup(this);

   m_treeAction->setCheckable(true);
   m_treeAction->setIcon(createIconSet("widgets/listview.png"));
   m_buttonAction->setCheckable(true);
   m_buttonAction->setIcon(createIconSet("dropdownbutton.png"));

   actionGroup->addAction(m_treeAction);
   actionGroup->addAction(m_buttonAction);

   connect(actionGroup, &QActionGroup::triggered,
         this, &PropertyEditor::slotViewTriggered);

   // Add actions
   QActionGroup *addDynamicActionGroup = new QActionGroup(this);

   connect(addDynamicActionGroup, &QActionGroup::triggered,
         this, &PropertyEditor::slotAddDynamicProperty);

   QMenu *addDynamicActionMenu = new QMenu(this);
   m_addDynamicAction->setMenu(addDynamicActionMenu);
   m_addDynamicAction->setEnabled(false);

   QAction *addDynamicAction = addDynamicActionGroup->addAction(tr("String..."));
   addDynamicAction->setData(static_cast<int>(QVariant::String));
   addDynamicActionMenu->addAction(addDynamicAction);

   addDynamicAction = addDynamicActionGroup->addAction(tr("Bool..."));
   addDynamicAction->setData(static_cast<int>(QVariant::Bool));
   addDynamicActionMenu->addAction(addDynamicAction);
   addDynamicActionMenu->addSeparator();

   addDynamicAction = addDynamicActionGroup->addAction(tr("Other..."));
   addDynamicAction->setData(static_cast<int>(QVariant::Invalid));
   addDynamicActionMenu->addAction(addDynamicAction);

   // remove
   m_removeDynamicAction->setEnabled(false);
   connect(m_removeDynamicAction, &QAction::triggered, this, &PropertyEditor::slotRemoveDynamicProperty);

   // Configure
   QAction *configureAction = new QAction(tr("Configure Property Editor"), this);
   configureAction->setIcon(createIconSet(QString("configure.png")));

   QMenu *configureMenu = new QMenu(this);
   configureAction->setMenu(configureMenu);

   m_sortingAction->setCheckable(true);
   connect(m_sortingAction, &QAction::toggled, this, &PropertyEditor::slotSorting);

   m_coloringAction->setCheckable(true);
   connect(m_coloringAction, &QAction::toggled, this, &PropertyEditor::slotColoring);

   configureMenu->addAction(m_sortingAction);
   configureMenu->addAction(m_coloringAction);

   configureMenu->addSeparator();
   configureMenu->addAction(m_treeAction);
   configureMenu->addAction(m_buttonAction);

   // Assemble toolbar
   QToolBar *toolBar = new QToolBar;
   toolBar->addWidget(m_filterWidget);
   toolBar->addWidget(createDropDownButton(m_addDynamicAction));
   toolBar->addAction(m_removeDynamicAction);
   toolBar->addWidget(createDropDownButton(configureAction));

   // Views
   QScrollArea *buttonScroll = new QScrollArea(m_stackedWidget);
   m_buttonBrowser = new QtButtonPropertyBrowser(buttonScroll);
   buttonScroll->setWidgetResizable(true);
   buttonScroll->setWidget(m_buttonBrowser);
   m_buttonIndex = m_stackedWidget->addWidget(buttonScroll);

   connect(m_buttonBrowser, &QtAbstractPropertyBrowser::currentItemChanged,
         this, &PropertyEditor::slotCurrentItemChanged);

   m_treeBrowser = new QtTreePropertyBrowser(m_stackedWidget);
   m_treeBrowser->setRootIsDecorated(false);
   m_treeBrowser->setPropertiesWithoutValueMarked(true);
   m_treeBrowser->setResizeMode(QtTreePropertyBrowser::Interactive);
   m_treeIndex = m_stackedWidget->addWidget(m_treeBrowser);

   connect(m_treeBrowser, &QtAbstractPropertyBrowser::currentItemChanged,
         this, &PropertyEditor::slotCurrentItemChanged);

   m_filterWidget->setPlaceholderText(tr("Filter"));
   m_filterWidget->setClearButtonEnabled(true);

   connect(m_filterWidget, &QLineEdit::textChanged, this, &PropertyEditor::setFilter);

   QVBoxLayout *layout = new QVBoxLayout(this);
   layout->addWidget(toolBar);
   layout->addWidget(m_classLabel);
   layout->addSpacerItem(new QSpacerItem(0, 1));
   layout->addWidget(m_stackedWidget);
   layout->setMargin(0);
   layout->setSpacing(0);

   m_treeFactory = new DesignerEditorFactory(m_core, this);
   m_treeFactory->setSpacing(0);

   m_groupFactory = new DesignerEditorFactory(m_core, this);

   QtVariantPropertyManager *variantManager = m_propertyManager;
   m_buttonBrowser->setFactoryForManager(variantManager, m_groupFactory);
   m_treeBrowser->setFactoryForManager(variantManager, m_treeFactory);

   m_stackedWidget->setCurrentIndex(m_treeIndex);
   m_currentBrowser = m_treeBrowser;
   m_treeAction->setChecked(true);

   connect(m_groupFactory, &DesignerEditorFactory::resetProperty,
      this, &PropertyEditor::slotResetProperty);

   connect(m_treeFactory, &DesignerEditorFactory::resetProperty,
      this, &PropertyEditor::slotResetProperty);

   connect(m_propertyManager, &DesignerPropertyManager::valueChanged,
      this, &PropertyEditor::slotValueChanged);

   // retrieve initial settings
   QDesignerSettingsInterface *settings = m_core->settingsManager();
   settings->beginGroup(SettingsGroupC);
   const SettingsView view = settings->value(ViewKeyC, TreeView).toInt() == TreeView ? TreeView :  ButtonView;

   // Coloring not available unless treeview and not sorted
   m_sorting  = settings->value(SortedKeyC, false).toBool();
   m_coloring = settings->value(ColorKeyC, true).toBool();

   const QVariantMap expansionState = settings->value(ExpansionKeyC, QVariantMap()).toMap();
   const int splitterPosition = settings->value(SplitterPositionKeyC, 150).toInt();
   settings->endGroup();

   // Apply settings
   m_sortingAction->setChecked(m_sorting);
   m_coloringAction->setChecked(m_coloring);
   m_treeBrowser->setSplitterPosition(splitterPosition);

   switch (view) {
      case TreeView:
         m_currentBrowser = m_treeBrowser;
         m_stackedWidget->setCurrentIndex(m_treeIndex);
         m_treeAction->setChecked(true);
         break;

      case ButtonView:
         m_currentBrowser = m_buttonBrowser;
         m_stackedWidget->setCurrentIndex(m_buttonIndex);
         m_buttonAction->setChecked(true);
         break;
   }

   // Restore expansionState from QVariant map
   if (!expansionState.empty()) {
      const QVariantMap::const_iterator cend = expansionState.constEnd();
      for (QVariantMap::const_iterator it = expansionState.constBegin(); it != cend; ++it) {
         m_expansionState.insert(it.key(), it.value().toBool());
      }
   }
   updateActionsState();
}

PropertyEditor::~PropertyEditor()
{
   storeExpansionState();
   saveSettings();
}

void PropertyEditor::saveSettings() const
{
   QDesignerSettingsInterface *settings = m_core->settingsManager();
   settings->beginGroup(SettingsGroupC);

   settings->setValue(ViewKeyC,   QVariant(m_treeAction->isChecked() ? TreeView : ButtonView));
   settings->setValue(ColorKeyC,  QVariant(m_coloring));
   settings->setValue(SortedKeyC, QVariant(m_sorting));

   // Save last expansionState as QVariant map
   QVariantMap expansionState;

   if (!m_expansionState.empty()) {
      auto cend = m_expansionState.constEnd();

      for (auto iter = m_expansionState.constBegin(); iter != cend; ++iter) {
         expansionState.insert(iter.key(), QVariant(iter.value()));
      }
   }

   settings->setValue(ExpansionKeyC, expansionState);
   settings->setValue(SplitterPositionKeyC, m_treeBrowser->splitterPosition());
   settings->endGroup();
}

void PropertyEditor::setExpanded(QtBrowserItem *item, bool expanded)
{
   if (m_buttonBrowser == m_currentBrowser) {
      m_buttonBrowser->setExpanded(item, expanded);

   } else if (m_treeBrowser == m_currentBrowser) {
      m_treeBrowser->setExpanded(item, expanded);
   }
}

bool PropertyEditor::isExpanded(QtBrowserItem *item) const
{
   if (m_buttonBrowser == m_currentBrowser) {
      return m_buttonBrowser->isExpanded(item);

   } else if (m_treeBrowser == m_currentBrowser) {
      return m_treeBrowser->isExpanded(item);
   }

   return false;
}

void PropertyEditor::setItemVisible(QtBrowserItem *item, bool visible)
{
   if (m_currentBrowser == m_treeBrowser) {
      m_treeBrowser->setItemVisible(item, visible);
   }
}

bool PropertyEditor::isItemVisible(QtBrowserItem *item) const
{
   return m_currentBrowser == m_treeBrowser ? m_treeBrowser->isItemVisible(item) : true;
}

/* Default handling of items not found in the map:
 * - Top-level items (classes) are assumed to be expanded
 * - Anything below (properties) is assumed to be collapsed
 * That is, the map is required, the state cannot be stored in a set */

void PropertyEditor::storePropertiesExpansionState(const QList<QtBrowserItem *> &items)
{
   const QChar bar = '|';
   QListIterator<QtBrowserItem *> itProperty(items);

   while (itProperty.hasNext()) {
      QtBrowserItem *propertyItem = itProperty.next();

      if (! propertyItem->children().empty()) {
         QtProperty *property = propertyItem->property();
         const QString propertyName = property->propertyName();
         const QMap<QtProperty *, QString>::const_iterator itGroup = m_propertyToGroup.constFind(property);

         if (itGroup != m_propertyToGroup.constEnd()) {
            QString key = itGroup.value();
            key += bar;
            key += propertyName;
            m_expansionState[key] = isExpanded(propertyItem);
         }
      }
   }
}

void PropertyEditor::storeExpansionState()
{
   const QList<QtBrowserItem *> items = m_currentBrowser->topLevelItems();

   if (m_sorting) {
      storePropertiesExpansionState(items);

   } else {
      QListIterator<QtBrowserItem *> itGroup(items);

      while (itGroup.hasNext()) {
         QtBrowserItem *item = itGroup.next();
         const QString groupName = item->property()->propertyName();
         QList<QtBrowserItem *> propertyItems = item->children();

         if (! propertyItems.empty()) {
            m_expansionState[groupName] = isExpanded(item);
         }

         // properties stuff here
         storePropertiesExpansionState(propertyItems);
      }
   }
}

void PropertyEditor::collapseAll()
{
   QList<QtBrowserItem *> items = m_currentBrowser->topLevelItems();
   QListIterator<QtBrowserItem *> itGroup(items);

   while (itGroup.hasNext()) {
      setExpanded(itGroup.next(), false);
   }
}

void PropertyEditor::applyPropertiesExpansionState(const QList<QtBrowserItem *> &items)
{
   const QChar bar = '|';
   QListIterator<QtBrowserItem *> itProperty(items);

   while (itProperty.hasNext()) {
      const QMap<QString, bool>::const_iterator excend = m_expansionState.constEnd();
      QtBrowserItem *propertyItem = itProperty.next();

      QtProperty *property = propertyItem->property();
      const QString propertyName = property->propertyName();
      const QMap<QtProperty *, QString>::const_iterator itGroup = m_propertyToGroup.constFind(property);

      if (itGroup != m_propertyToGroup.constEnd()) {
         QString key = itGroup.value();
         key += bar;
         key += propertyName;
         const QMap<QString, bool>::const_iterator pit = m_expansionState.constFind(key);

         if (pit != excend) {
            setExpanded(propertyItem, pit.value());
         } else {
            setExpanded(propertyItem, false);
         }
      }
   }
}

void PropertyEditor::applyExpansionState()
{
   const QList<QtBrowserItem *> topList = m_currentBrowser->topLevelItems();

   if (m_sorting) {
      applyPropertiesExpansionState(topList);

   } else {

      for (auto item : topList) {
         const QString groupName = item->property()->propertyName();
         auto iter = m_expansionState.constFind(groupName);

         if (iter != m_expansionState.constEnd()) {
            setExpanded(item, iter.value());
         } else {
            setExpanded(item, true);
         }

         // properties stuff here
         applyPropertiesExpansionState(item->children());
      }
   }
}

int PropertyEditor::applyPropertiesFilter(const QList<QtBrowserItem *> &items)
{
   int showCount = 0;

   const bool matchAll = m_filterPattern.isEmpty();
   QListIterator<QtBrowserItem *> itProperty(items);

   while (itProperty.hasNext()) {
      QtBrowserItem *propertyItem = itProperty.next();
      QtProperty *property = propertyItem->property();

      const QString propertyName = property->propertyName();
      const bool showProperty = matchAll || propertyName.contains(m_filterPattern, Qt::CaseInsensitive);

      setItemVisible(propertyItem, showProperty);
      if (showProperty) {
         ++showCount;
      }
   }

   return showCount;
}

void PropertyEditor::applyFilter()
{
   const QList<QtBrowserItem *> items = m_currentBrowser->topLevelItems();

   if (m_sorting) {
      applyPropertiesFilter(items);
   } else {
      QListIterator<QtBrowserItem *> itTopLevel(items);
      while (itTopLevel.hasNext()) {
         QtBrowserItem *item = itTopLevel.next();
         setItemVisible(item, applyPropertiesFilter(item->children()));
      }
   }
}

void PropertyEditor::clearView()
{
   m_currentBrowser->clear();
}

bool PropertyEditor::event(QEvent *event)
{
   if (event->type() == QEvent::PaletteChange) {
      updateForegroundBrightness();
   }

   return QDesignerPropertyEditor::event(event);
}

void PropertyEditor::updateForegroundBrightness()
{
   QColor c = palette().color(QPalette::Text);
   bool newBrightness = qRound(0.3 * c.redF() + 0.59 * c.greenF() + 0.11 * c.blueF());

   if (m_brightness == newBrightness) {
      return;
   }

   m_brightness = newBrightness;

   updateColors();
}

QColor PropertyEditor::propertyColor(QtProperty *property) const
{
   if (! m_coloring) {
      return QColor();
   }

   QtProperty *groupProperty = property;

   auto iter = m_propertyToGroup.constFind(property);

   if (iter != m_propertyToGroup.constEnd()) {
      groupProperty = m_nameToGroup.value(iter.value());
   }

   const int groupIdx = m_groups.indexOf(groupProperty);
   QPair<QColor, QColor> pair;

   if (groupIdx != -1) {
      if (groupProperty == m_dynamicGroup) {
         pair = m_dynamicColor;

      } else if (isLayoutGroup(groupProperty)) {
         pair = m_layoutColor;

      } else {
         pair = m_colors[groupIdx % m_colors.count()];
      }
   }

   if (! m_brightness) {
      return pair.first;
   }

   return pair.second;
}

void PropertyEditor::fillView()
{
   if (m_sorting) {

      for (auto property : m_nameToProperty) {
         m_currentBrowser->addProperty(property);
      }

   } else {
      // sorting is disabled, sort by groups (is this right?)

      for (auto group : m_groups) {
         QtBrowserItem *item = m_currentBrowser->addProperty(group);

         if (m_currentBrowser == m_treeBrowser) {
            m_treeBrowser->setBackgroundColor(item, propertyColor(group));
         }

         group->setModified(m_currentBrowser == m_treeBrowser);
      }
   }
}

bool PropertyEditor::isLayoutGroup(QtProperty *group) const
{
   return group->propertyName() == m_strings.m_layout;
}

void PropertyEditor::updateActionsState()
{
   m_coloringAction->setEnabled(m_treeAction->isChecked() && ! m_sortingAction->isChecked());
}

void PropertyEditor::slotViewTriggered(QAction *action)
{
   storeExpansionState();
   collapseAll();

   {
      UpdateBlocker ub(this);
      clearView();

      int idx = 0;

      if (action == m_treeAction) {
         m_currentBrowser = m_treeBrowser;
         idx = m_treeIndex;

      } else if (action == m_buttonAction) {
         m_currentBrowser = m_buttonBrowser;
         idx = m_buttonIndex;
      }

      fillView();
      m_stackedWidget->setCurrentIndex(idx);

      applyExpansionState();
      applyFilter();
   }

   updateActionsState();
}

void PropertyEditor::slotSorting(bool sort)
{
   if (sort == m_sorting) {
      return;
   }

   storeExpansionState();
   m_sorting = sort;
   collapseAll();

   {
      UpdateBlocker ub(this);
      clearView();

      m_treeBrowser->setRootIsDecorated(sort);
      fillView();
      applyExpansionState();
      applyFilter();
   }

   updateActionsState();
}

void PropertyEditor::updateColors()
{
   if (m_treeBrowser && m_currentBrowser == m_treeBrowser) {
      QList<QtBrowserItem *> itemList = m_treeBrowser->topLevelItems();

      for (auto item : itemList) {
         m_treeBrowser->setBackgroundColor(item, propertyColor(item->property()));
      }
   }
}

void PropertyEditor::slotColoring(bool coloring)
{
   if (coloring == m_coloring) {
      return;
   }

   m_coloring = coloring;
   updateColors();
}

void PropertyEditor::slotAddDynamicProperty(QAction *action)
{
   if (! m_propertySheet) {
      return;
   }

   const QDesignerDynamicPropertySheetExtension *dynamicSheet =
      qt_extension<QDesignerDynamicPropertySheetExtension *>(m_core->extensionManager(), m_object);

   if (! dynamicSheet) {
      return;
   }

   QString newName;
   QVariant newValue;

   {
      // Make sure the dialog is closed before the signal is emitted.
      const QVariant::Type type = static_cast<QVariant::Type>(action->data().toInt());
      NewDynamicPropertyDialog dlg(core()->dialogGui(), m_currentBrowser);

      if (type != QVariant::Invalid) {
         dlg.setPropertyType(type);
      }

      QStringList reservedNames;
      const int propertyCount = m_propertySheet->count();

      for (int i = 0; i < propertyCount; i++) {
         if (! dynamicSheet->isDynamicProperty(i) || m_propertySheet->isVisible(i)) {
            reservedNames.append(m_propertySheet->propertyName(i));
         }
      }
      dlg.setReservedNames(reservedNames);

      if (dlg.exec() == QDialog::Rejected) {
         return;
      }

      newName = dlg.propertyName();
      newValue = dlg.propertyValue();
   }

   m_recentlyAddedDynamicProperty = newName;
   emit addDynamicProperty(newName, newValue);
}

QDesignerFormEditorInterface *PropertyEditor::core() const
{
   return m_core;
}

bool PropertyEditor::isReadOnly() const
{
   return false;
}

void PropertyEditor::setReadOnly(bool)
{
}

void PropertyEditor::setPropertyValue(const QString &name, const QVariant &value, bool changed)
{
   auto iter = m_nameToProperty.constFind(name);

   if (iter == m_nameToProperty.constEnd()) {
      return;
   }

   QtVariantProperty *property = iter.value();
   updateBrowserValue(property, value);
   property->setModified(changed);
}

// Quick update which assumes the actual count of properties has not changed
// N/A when executing a layout command and margin properties appear
void PropertyEditor::updatePropertySheet()
{
   if (! m_propertySheet) {
      return;
   }

   updateToolBarLabel();

   const int propertyCount = m_propertySheet->count();
   auto end_iter = m_nameToProperty.constEnd();

   for (int i = 0; i < propertyCount; ++i) {
      const QString propertyName = m_propertySheet->propertyName(i);

      QMap<QString, QtVariantProperty *>::const_iterator iter = m_nameToProperty.constFind(propertyName);

      if (iter != end_iter) {
         updateBrowserValue(iter.value(), m_propertySheet->property(i));
      }
   }
}

static inline QLayout *layoutOfQLayoutWidget(QObject *o)
{
   if (o->isWidgetType() && (o->metaObject()->className() == "QLayoutWidget")) {
      return static_cast<QWidget *>(o)->layout();
   }

   return nullptr;
}

void PropertyEditor::updateToolBarLabel()
{
   QString objectName;
   QString className;

   if (m_object) {
      if (QLayout *newLayout = layoutOfQLayoutWidget(m_object)) {
         objectName = newLayout->objectName();
      } else {
         objectName = m_object->objectName();
      }

      className = realClassName(m_object);
   }

   m_classLabel->setVisible(!objectName.isEmpty() || !className.isEmpty());
   m_classLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

   QString classLabelText;
   if (! objectName.isEmpty()) {
      classLabelText += objectName + QString(" : ");
   }
   classLabelText += className;

   m_classLabel->setText(classLabelText);
   m_classLabel->setToolTip(tr("Object: %1\nClass: %2").formatArg(objectName).formatArg(className));
}

void PropertyEditor::updateBrowserValue(QtVariantProperty *property, const QVariant &value)
{
   QVariant v = value;
   const uint type = property->propertyType();

   if (type == QtVariantPropertyManager::enumTypeId()) {

      if (v.canConvert<PropertySheetEnumValue>()) {
         const PropertySheetEnumValue e = v.value<PropertySheetEnumValue>();
         v = static_cast<int>(e.metaEnum.keys().indexOf(e.metaEnum.valueToKey(e.value)));
      }

   } else if (type == DesignerPropertyManager::designerFlagTypeId()) {

      if (v.canConvert<PropertySheetFlagValue>()) {

         const PropertySheetFlagValue f = v.value<PropertySheetFlagValue>();
         v = QVariant(f.value);
      }

   } else if (type == DesignerPropertyManager::designerAlignmentTypeId()) {

      if (v.canConvert<PropertySheetFlagValue>()) {
         const PropertySheetFlagValue f = v.value<PropertySheetFlagValue>();
         v = QVariant(f.value);
      }
   }

   QDesignerPropertySheet *sheet = dynamic_cast<QDesignerPropertySheet *>(
         m_core->extensionManager()->extension(m_object, CS_TYPEID(QDesignerPropertySheetExtension)));

   int index = -1;

   if (sheet != nullptr) {
      index = sheet->indexOf(property->propertyName());

      if (m_propertyToGroup.contains(property)) {
         // do not do this for comments since property sheets do not store them
         property->setEnabled(sheet->isEnabled(index));
      }
   }

   // Rich text string property with comment: Store/Update the font the rich text editor dialog starts out with
   if (type == QVariant::String && ! property->subProperties().empty()) {
      const int fontIndex = m_propertySheet->indexOf(m_strings.m_fontProperty);

      if (fontIndex != -1) {
         property->setAttribute(m_strings.m_fontAttribute, m_propertySheet->property(fontIndex));
      }
   }

   m_updatingBrowser = true;
   property->setValue(v);

   if (sheet != nullptr && sheet->isResourceProperty(index)) {
      property->setAttribute("defaultResource", sheet->defaultResourceProperty(index));
   }

   m_updatingBrowser = false;
}

uint PropertyEditor::toBrowserType(const QVariant &value, const QString &propertyName) const
{
   if (value.canConvert<PropertySheetFlagValue>()) {
      if (m_strings.m_alignmentProperties.contains(propertyName)) {
         return DesignerPropertyManager::designerAlignmentTypeId();
      }

      return DesignerPropertyManager::designerFlagTypeId();
   }

   if (value.canConvert<PropertySheetEnumValue>()) {
      return DesignerPropertyManager::enumTypeId();
   }

   return value.userType();
}

QString PropertyEditor::realClassName(QObject *object) const
{
   if (! object) {
      return QString();
   }

   QString className = object->metaObject()->className();
   const QDesignerWidgetDataBaseInterface *db = core()->widgetDataBase();

   if (QDesignerWidgetDataBaseItemInterface *widgetItem = db->item(db->indexOfObject(object, true))) {
      className = widgetItem->name();

      if (object->isWidgetType() && className == m_strings.m_qLayoutWidget
         && static_cast<QWidget *>(object)->layout()) {
         className = static_cast<QWidget *>(object)->layout()->metaObject()->className();
      }
   }

   if (className.startsWith(m_strings.m_designerPrefix)) {
      className.remove(1, m_strings.m_designerPrefix.size() - 1);
   }

   return className;
}

static const QString typeName(uint type)
{
   QString retval;

   if (type == QVariant::typeToTypeId<PropertySheetStringValue>()) {
      retval = "QString";
   }

   else if (type < int(QVariant::UserType)) {
      retval = QVariant::typeToName(static_cast<QVariant::Type>(type));
   }

   else if (type == QVariant::typeToTypeId<PropertySheetIconValue>()) {
      retval = "QIcon";
   }

   else if (type == QVariant::typeToTypeId<PropertySheetPixmapValue>()) {
      retval = "QPixmap";
   }

   else if (type == QVariant::typeToTypeId<PropertySheetKeySequenceValue>()) {
      retval = "QKeySequence";
   }

   else if (type == QVariant::typeToTypeId<PropertySheetFlagValue>()) {
      retval = "QFlags";
   }

   else if (type == QVariant::typeToTypeId<PropertySheetEnumValue>()) {
      retval = "enum";
   }

   else if (type == QVariant::Invalid) {
      retval = "invalid";
   }

   else if (type == QVariant::UserType) {
      retval =  "user type";
   }

   return retval;
}

static QString msgUnsupportedType(const QString &propertyName, uint type)
{
   QString rc;
   QTextStream str(&rc);

   const QString typeS = typeName(type);

   if (typeS.isEmpty()) {
      str << "The property \"" << propertyName << "\" of unknown type is not supported";

   } else  {
      str << "The property \"" << propertyName << "\" of type " << typeS << " is not supported";

   }

   return rc;
}

void PropertyEditor::setObject(QObject *object)
{
   QDesignerFormWindowInterface *oldFormWindow = QDesignerFormWindowInterface::findFormWindow(m_object);

   // in the first setObject() call following the addition of a dynamic property, focus and edit it
   const bool editNewDynamicProperty = object != nullptr && m_object == object &&
         ! m_recentlyAddedDynamicProperty.isEmpty();

   m_object = object;
   m_propertyManager->setObject(object);


   QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(m_object);
   FormWindowBase *fwb = dynamic_cast<FormWindowBase *>(formWindow);
   m_treeFactory->setFormWindowBase(fwb);
   m_groupFactory->setFormWindowBase(fwb);

   storeExpansionState();

   UpdateBlocker ub(this);

   updateToolBarLabel();

   QMap<QString, QtVariantProperty *> toRemove = m_nameToProperty;

   const QDesignerDynamicPropertySheetExtension *dynamicSheet =
            qt_extension<QDesignerDynamicPropertySheetExtension *>(m_core->extensionManager(), m_object);

   const QDesignerPropertySheet *sheet = dynamic_cast<QDesignerPropertySheet *>(
            m_core->extensionManager()->extension(m_object, CS_TYPEID(QDesignerPropertySheetExtension)));

   // Optimizization: Instead of rebuilding the complete list every time, compile a list of properties
   // to remove and do so, then traverse the sheet and if it exists set a value, otherwise create it
   QExtensionManager *m = m_core->extensionManager();

   m_propertySheet = dynamic_cast<QDesignerPropertySheetExtension *>(
            m->extension(object, CS_TYPEID(QDesignerPropertySheetExtension)));

   if (m_propertySheet != nullptr) {
      const int propertyCount = m_propertySheet->count();

      for (int i = 0; i < propertyCount; ++i) {

         if (! m_propertySheet->isVisible(i)) {
            continue;
         }

         const QString propertyName = m_propertySheet->propertyName(i);
         const QString groupName    = m_propertySheet->propertyGroup(i);

         auto iter_remove = toRemove.constFind(propertyName);

         if (iter_remove != toRemove.constEnd()) {
            QtVariantProperty *property = iter_remove.value();

            if (m_propertyToGroup.value(property) == groupName &&
                  toBrowserType(m_propertySheet->property(i), propertyName) == property->propertyType()) {
               toRemove.remove(propertyName);
            }
         }
      }
   }

   QMapIterator<QString, QtVariantProperty *> itRemove(toRemove);

   while (itRemove.hasNext()) {
      itRemove.next();

      QtVariantProperty *property = itRemove.value();
      m_nameToProperty.remove(itRemove.key());
      m_propertyToGroup.remove(property);

      delete property;
   }

   if (oldFormWindow != formWindow) {
      reloadResourceProperties();
   }

   bool isMainContainer = false;
   if (QWidget *widget = dynamic_cast<QWidget *>(object)) {
      if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(widget)) {
         isMainContainer = (fw->mainContainer() == widget);
      }
   }

   m_groups.clear();

   if (m_propertySheet != nullptr) {
      const QString className = WidgetFactory::classNameOf(formWindow->core(), m_object);
      const QDesignerCustomWidgetData customData = formWindow->core()->pluginManager()->customWidgetData(className);

      QtProperty *lastGroup = nullptr;

      const int propertyCount = m_propertySheet->count();

      for (int i = 0; i < propertyCount; ++i) {
         if (! m_propertySheet->isVisible(i)) {
            continue;
         }

         const QString propertyName = m_propertySheet->propertyName(i);

         if (m_propertySheet->indexOf(propertyName) != i) {
            continue;
         }

         const QVariant value = m_propertySheet->property(i);
         const uint type = toBrowserType(value, propertyName);

         QtVariantProperty *property = m_nameToProperty.value(propertyName, nullptr);
         bool newProperty = false;

         if (property == nullptr) {
            newProperty = true;

            property = m_propertyManager->addProperty(type, propertyName);

            if (property != nullptr) {

               if (type == DesignerPropertyManager::enumTypeId()) {
                  const PropertySheetEnumValue e = value.value<PropertySheetEnumValue>();
                  QStringList names;

                  for (const QString &name : e.metaEnum.keys() ) {
                     names.append(name);
                  }

                  m_updatingBrowser = true;
                  property->setAttribute(m_strings.m_enumNamesAttribute, names);
                  m_updatingBrowser = false;

               } else if (type == DesignerPropertyManager::designerFlagTypeId()) {

                  if (value.canConvert<PropertySheetFlagValue>()) {

                     const PropertySheetFlagValue f = value.value<PropertySheetFlagValue>();
                     QList<QPair<QString, uint>> flags;

                     for (const QString &name : f.metaFlags.keys()) {
                        const uint val = f.metaFlags.keyToValue(name);
                        flags.append(std::make_pair(name, val));
                     }

                     m_updatingBrowser = true;
                     property->setAttribute(m_strings.m_flagsAttribute, QVariant::fromValue(flags));
                     m_updatingBrowser = false;
                  }
               }
            }
         }

         if (property == nullptr) {
            csWarning(msgUnsupportedType(propertyName, type));

         } else {

            const bool dynamicProperty = (dynamicSheet && dynamicSheet->isDynamicProperty(i))
               || (sheet && sheet->isDefaultDynamicProperty(i));

            QString descriptionToolTip;

            if (! dynamicProperty && ! customData.isNull()) {
               descriptionToolTip = customData.propertyToolTip(propertyName);
            }

            if (descriptionToolTip.isEmpty()) {
               const QString typeS = typeName(type);

               if (! typeS.isEmpty()) {
                  descriptionToolTip  = propertyName + " (" + typeS + ")";
               }
            }
            if (! descriptionToolTip.isEmpty()) {
               property->setDescriptionToolTip(descriptionToolTip);
            }

            switch (type) {
               case QVariant::Palette:
                  setupPaletteProperty(property);
                  break;

               case QVariant::KeySequence:
                  // addCommentProperty(property, propertyName);
                  break;

               default:
                  break;
            }

            if (type == QVariant::String || type == QVariant::typeToTypeId<PropertySheetStringValue>()) {
               setupStringProperty(property, isMainContainer);
            }
            property->setAttribute(m_strings.m_resettableAttribute, m_propertySheet->hasReset(i));

            const QString groupName = m_propertySheet->propertyGroup(i);
            QtVariantProperty *groupProperty = nullptr;

            if (newProperty) {
               auto itPrev = m_nameToProperty.insert(propertyName, property);
               m_propertyToGroup[property] = groupName;

               if (m_sorting) {
                  QtProperty *previous = nullptr;

                  if (itPrev != m_nameToProperty.constBegin()) {
                     previous = (--itPrev).value();
                  }
                  m_currentBrowser->insertProperty(property, previous);
               }
            }
            const QMap<QString, QtVariantProperty *>::const_iterator gnit = m_nameToGroup.constFind(groupName);

            if (gnit != m_nameToGroup.constEnd()) {
               groupProperty = gnit.value();

            } else {
               groupProperty = m_propertyManager->addProperty(QtVariantPropertyManager::groupTypeId(), groupName);
               QtBrowserItem *item = nullptr;

               if (! m_sorting) {
                  item = m_currentBrowser->insertProperty(groupProperty, lastGroup);
               }

               m_nameToGroup[groupName] = groupProperty;
               m_groups.append(groupProperty);

               if (dynamicProperty) {
                  m_dynamicGroup = groupProperty;
               }

               if (m_currentBrowser == m_treeBrowser && item) {
                  m_treeBrowser->setBackgroundColor(item, propertyColor(groupProperty));
                  groupProperty->setModified(true);
               }
            }

            //  Group changed or new group. Append to last subproperty of that group. There are
            //  cases in which a derived  property sheet appends fake properties for the class
            //  which will appear after the layout group properties  (QWizardPage). To make them
            //  appear at the end of the  actual class group, goto last element.

            if (lastGroup != groupProperty) {
               lastGroup = groupProperty;
               const QList<QtProperty *> subProperties = lastGroup->subProperties();
               lastGroup = groupProperty;
            }

            if (! m_groups.contains(groupProperty)) {
               m_groups.append(groupProperty);
            }

            if (newProperty) {
               groupProperty->insertSubProperty(property, nullptr);
            }

            updateBrowserValue(property, value);
            property->setModified(m_propertySheet->isChanged(i));

            if (propertyName == "geometry" && type == QVariant::Rect) {
               QList<QtProperty *> subProperties = property->subProperties();

               for (QtProperty *subProperty : subProperties) {
                  const QString subPropertyName = subProperty->propertyName();

                  if (subPropertyName == "X" || subPropertyName == "Y") {
                     subProperty->setEnabled(!isMainContainer);
                  }
               }
            }
         }
      }
   }

   // add sorting for m_groups based on the group name
   const static QHash<QString, int> groupOrder = {
      {"QObject", 0}, {"QWidget", 1}, {"QFrame", 2},
      {"QAbstractButton", 3}, {"QAbstractScrollArea", 4}, {"QAbstractSpinBox", 5},
      {"QAbstractSlider", 6}, {"QAbstractItemView", 7},

      {"QCheckBox", 30}, {"QPushButton", 31}, {"QToolButton", 32},
      {"QComboBox", 33}, {"QFontCombBox", 34},
      {"QCommandLinkButton", 35}, {"QDialogButtonBox", 36},

      {"QSpinBox", 40}, {"QDoubleSpinBox", 41},
      {"QDateTimeEdit", 42}, {"QDateEdit", 43}, {"QTimeEdit", 44},
      {"QCalendarWidget", 45},

      {"QSlider", 50}, {"QDial", 51}, {"QProgressBar", 52}, {"QLine", 53},

      {"QLabel", 60}, {"QLineEdit", 61}, {"QTextEdit", 62}, {"QPlainTextEdit", 63},
      {"QTextBrowser", 64}, {"QGraphicsView", 65},
      {"QKeySequenceEdit", 66}, {"QLCDNumber", 67},

      {"QListView", 80}, {"QTreeView", 81},
      {"QTableView", 82}, {"QColumnView", 83},
      {"QListWidget", 84}, {"QTreeWidget", 85}, {"QTableWidget", 86},

      {"QGroupBox", 90}, {"QToolBox", 91}, {"QTabWidget", 92}, {"QStackedWidget", 93},
      {"QMdiArea", 94}, {"QDockWidget", 95},
   };

   if (! m_groups.isEmpty()) {
      std::sort(m_groups.begin(), m_groups.end(), [] (auto a, auto b)
         {
            auto index_a = groupOrder.value(a->propertyName(), -1);
            auto index_b = groupOrder.value(b->propertyName(), -1);

            if (index_a == -1 && index_b == -1) {
               return a->propertyName() < b->propertyName();

            } else if (index_a == -1) {
               return false;

            } else if (index_b == -1) {
               return true;

            } else {
               return index_a < index_b;
            }
         } );

      clearView();
      fillView();
   }

   QMap<QString, QtVariantProperty *> groups = m_nameToGroup;
   QMapIterator<QString, QtVariantProperty *> itGroup(groups);

   while (itGroup.hasNext()) {
      QtVariantProperty *groupProperty = itGroup.next().value();

      if (groupProperty->subProperties().empty()) {
         if (groupProperty == m_dynamicGroup) {
            m_dynamicGroup = nullptr;
         }

         delete groupProperty;
         m_nameToGroup.remove(itGroup.key());
      }
   }

   const bool addEnabled = dynamicSheet ? dynamicSheet->dynamicPropertiesAllowed() : false;
   m_addDynamicAction->setEnabled(addEnabled);
   m_removeDynamicAction->setEnabled(false);

   applyExpansionState();
   applyFilter();

   // In the first setObject() call following the addition of a dynamic property, focus and edit it
   if (editNewDynamicProperty) {
      // Have QApplication process the events related to completely closing the modal 'add' dialog,
      // otherwise, we cannot focus the property editor in docked mode.
      QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
      editProperty(m_recentlyAddedDynamicProperty);
   }

   m_recentlyAddedDynamicProperty.clear();
   m_filterWidget->setEnabled(object);
}

void PropertyEditor::reloadResourceProperties()
{
   m_updatingBrowser = true;
   m_propertyManager->reloadResourceProperties();
   m_updatingBrowser = false;
}

QtBrowserItem *PropertyEditor::nonFakePropertyBrowserItem(QtBrowserItem *item) const
{
   // Top-level properties are QObject/QWidget groups, etc. Find first item property below
   // which should be nonfake
   const QList<QtBrowserItem *> topLevelItems = m_currentBrowser->topLevelItems();

   do {
      if (topLevelItems.contains(item->parent())) {
         return item;
      }

      item = item->parent();
   } while (item);

   return nullptr;
}

QString PropertyEditor::currentPropertyName() const
{
   if (QtBrowserItem *browserItem = m_currentBrowser->currentItem()) {

      if (QtBrowserItem *topLevelItem = nonFakePropertyBrowserItem(browserItem)) {
         return topLevelItem->property()->propertyName();
      }
   }

   return QString();
}

void PropertyEditor::slotResetProperty(QtProperty *property)
{
   QDesignerFormWindowInterface *form = m_core->formWindowManager()->activeFormWindow();
   if (! form) {
      return;
   }

   if (m_propertyManager->resetFontSubProperty(property)) {
      return;
   }

   if (m_propertyManager->resetIconSubProperty(property)) {
      return;
   }

   if (! m_propertyToGroup.contains(property)) {
      return;
   }

   emit resetProperty(property->propertyName());
}

void PropertyEditor::slotValueChanged(QtProperty *property, const QVariant &value, bool enableSubPropertyHandling)
{
   if (m_updatingBrowser) {
      return;
   }

   if (! m_propertySheet) {
      return;
   }

   QtVariantProperty *varProp = m_propertyManager->variantProperty(property);

   if (! varProp) {
      return;
   }

   if (! m_propertyToGroup.contains(property)) {
      return;
   }

   if (varProp->propertyType() == QtVariantPropertyManager::enumTypeId()) {

      QVariant tmp = m_propertySheet->property(m_propertySheet->indexOf(property->propertyName()));
      PropertySheetEnumValue e = tmp.value<PropertySheetEnumValue>();

      const int val = value.toInt();
      const QString valName = varProp->attributeValue(m_strings.m_enumNamesAttribute).toStringList().at(val);
      bool ok = false;
      e.value = e.metaEnum.parseEnum(valName, &ok);

      Q_ASSERT(ok);

      QVariant v = QVariant::fromValue(e);
      emitPropertyValueChanged(property->propertyName(), v, true);

      return;
   }

   emitPropertyValueChanged(property->propertyName(), value, enableSubPropertyHandling);
}

bool PropertyEditor::isDynamicProperty(const QtBrowserItem *item) const
{
   if (! item) {
      return false;
   }

   const QDesignerDynamicPropertySheetExtension *dynamicSheet =
      qt_extension<QDesignerDynamicPropertySheetExtension *>(m_core->extensionManager(), m_object);

   if (!dynamicSheet) {
      return false;
   }

   if (m_propertyToGroup.contains(item->property())
      && dynamicSheet->isDynamicProperty(m_propertySheet->indexOf(item->property()->propertyName()))) {
      return true;
   }
   return false;
}

void PropertyEditor::editProperty(const QString &name)
{
   // find the browser item belonging to the property, make it current and edit it
   QtBrowserItem *browserItem = nullptr;

   if (QtVariantProperty *property = m_nameToProperty.value(name, nullptr)) {
      const QList<QtBrowserItem *> items = m_currentBrowser->items(property);
      if (items.size() == 1) {
         browserItem = items.front();
      }
   }

   if (browserItem == nullptr) {
      return;
   }

   m_currentBrowser->setFocus(Qt::OtherFocusReason);

   if (m_currentBrowser == m_treeBrowser) {
      // edit is currently only supported in tree view
      m_treeBrowser->editItem(browserItem);
   } else {
      m_currentBrowser->setCurrentItem(browserItem);
   }
}

void PropertyEditor::slotCurrentItemChanged(QtBrowserItem *item)
{
   m_removeDynamicAction->setEnabled(isDynamicProperty(item));

}

void PropertyEditor::slotRemoveDynamicProperty()
{
   if (QtBrowserItem *item = m_currentBrowser->currentItem())
      if (isDynamicProperty(item)) {
         emit removeDynamicProperty(item->property()->propertyName());
      }
}

void PropertyEditor::setFilter(const QString &pattern)
{
   m_filterPattern = pattern;
   applyFilter();
}
}
