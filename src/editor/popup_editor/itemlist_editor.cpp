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

#include <abstract_formbuilder.h>
#include <abstract_formwindow.h>
#include <designer_property.h>
#include <itemlist_editor.h>
#include <property_view_tree.h>

#include <formwindowbase_p.h>
#include <iconloader_p.h>

#include <QCoreApplication>
#include <QSplitter>

namespace qdesigner_internal {

class ItemPropertyBrowser : public QtTreePropertyBrowser
{
 public:
   ItemPropertyBrowser() {
      setResizeMode(Interactive);

      // Sample string to determinate the width for the first column of the list item property browser
      const QString widthSampleString = QCoreApplication::translate("ItemPropertyBrowser", "XX Icon Selected off");

      m_width = fontMetrics().width(widthSampleString);
      setSplitterPosition(m_width);

      m_width += fontMetrics().width("/this/is/some/random/path");
   }

   QSize sizeHint() const override {
      return QSize(m_width, 1);
   }

 private:
   int m_width;
};

AbstractItemEditor::AbstractItemEditor(QDesignerFormWindowInterface *form, QWidget *parent)
   : QWidget(parent), m_iconCache(dynamic_cast<FormWindowBase *>(form)->iconCache()),
     m_updatingBrowser(false)
{
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
   m_propertyManager = new DesignerPropertyManager(form->core(), this);

   m_editorFactory   = new DesignerEditorFactory(form->core(), this);
   m_editorFactory->setSpacing(0);

   m_propertyBrowser = new ItemPropertyBrowser;
   m_propertyBrowser->setFactoryForManager((QtVariantPropertyManager *)m_propertyManager, m_editorFactory);

   connect(m_editorFactory, &DesignerEditorFactory::resetProperty,
         this, &AbstractItemEditor::resetProperty);

   connect(m_propertyManager, &DesignerPropertyManager::valueChanged,
         this, &AbstractItemEditor::propertyChanged);

   connect(iconCache(), &DesignerIconCache::reloaded,
         this, &AbstractItemEditor::cacheReloaded);
}

AbstractItemEditor::~AbstractItemEditor()
{
   m_propertyBrowser->unsetFactoryForManager(m_propertyManager);
}

static const QStringList itemFlagNamesList = {
   cs_mark_string_tr("AbstractItemEditor", "Selectable"),
   cs_mark_string_tr("AbstractItemEditor", "Editable"),
   cs_mark_string_tr("AbstractItemEditor", "DragEnabled"),
   cs_mark_string_tr("AbstractItemEditor", "DropEnabled"),
   cs_mark_string_tr("AbstractItemEditor", "UserCheckable"),
   cs_mark_string_tr("AbstractItemEditor", "Enabled"),
   cs_mark_string_tr("AbstractItemEditor", "Tristate"),
};

static const QStringList checkStateNamesList = {
   cs_mark_string_tr("AbstractItemEditor", "Unchecked"),
   cs_mark_string_tr("AbstractItemEditor", "PartiallyChecked"),
   cs_mark_string_tr("AbstractItemEditor", "Checked"),
};

void AbstractItemEditor::setupProperties(PropertyDefinition *propList)
{
   for (int i = 0; ! propList[i].name.isEmpty(); i++) {
      int type = propList[i].typeFunc ? propList[i].typeFunc() : propList[i].type;
      int role = propList[i].role;

      QtVariantProperty *prop = m_propertyManager->addProperty(type, propList[i].name);
      Q_ASSERT(prop);

      if (role == Qt::ToolTipPropertyRole || role == Qt::WhatsThisPropertyRole) {
         prop->setAttribute("validationMode", ValidationRichText);
      }

      else if (role == Qt::DisplayPropertyRole) {
         prop->setAttribute("validationMode", ValidationMultiLine);
      }

      else if (role == Qt::StatusTipPropertyRole) {
         prop->setAttribute("validationMode", ValidationSingleLine);
      }

      else if (role == ItemFlagsShadowRole) {
         prop->setAttribute("flagNames", itemFlagNamesList);
      }

      else if (role == Qt::CheckStateRole) {
         prop->setAttribute("enumNames", checkStateNamesList);
      }

      prop->setAttribute("resettable", true);
      m_properties.append(prop);
      m_rootProperties.append(prop);
      m_propertyToRole.insert(prop, role);
   }
}

void AbstractItemEditor::setupObject(QWidget *object)
{
   m_propertyManager->setObject(object);
   QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(object);
   FormWindowBase *fwb = dynamic_cast<FormWindowBase *>(formWindow);
   m_editorFactory->setFormWindowBase(fwb);
}

void AbstractItemEditor::setupEditor(QWidget *object, PropertyDefinition *propList)
{
   setupProperties(propList);
   setupObject(object);
}

void AbstractItemEditor::propertyChanged(QtProperty *property)
{
   if (m_updatingBrowser) {
      return;
   }

   BoolBlocker block(m_updatingBrowser);
   QtVariantProperty *prop = m_propertyManager->variantProperty(property);
   int role;

   if ((role = m_propertyToRole.value(prop, -1)) == -1) {
      // Subproperty
      return;
   }

   QVariant data1 = prop->value();

   if ((role == ItemFlagsShadowRole && data1.toInt() == (int)QListWidgetItem().flags())
         || (role == Qt::DecorationPropertyRole && ! data1.value<PropertySheetIconValue>().mask())
         || (role == Qt::FontRole && ! data1.value<QFont>().resolve())) {

      prop->setModified(false);
      setItemData(role, QVariant());

   } else {
      prop->setModified(true);
      setItemData(role, prop->value());
   }

   switch (role) {
      case Qt::DecorationPropertyRole: {
         QVariant data2 = prop->value();
         setItemData(Qt::DecorationRole, QVariant::fromValue(iconCache()->icon(data2.value<PropertySheetIconValue>())));
         break;
      }

      case Qt::DisplayPropertyRole: {
         QVariant data3 = prop->value();
         setItemData(Qt::EditRole, QVariant::fromValue(data3.value<PropertySheetStringValue>().value()));
         break;
      }

      case Qt::ToolTipPropertyRole: {
         QVariant data4 = prop->value();
         setItemData(Qt::ToolTipRole, QVariant::fromValue(data4.value<PropertySheetStringValue>().value()));
         break;
      }

      case Qt::StatusTipPropertyRole: {
         QVariant data5 = prop->value();
         setItemData(Qt::StatusTipRole, QVariant::fromValue(data5.value<PropertySheetStringValue>().value()));
         break;
      }

      case Qt::WhatsThisPropertyRole: {
         QVariant data6 = prop->value();
         setItemData(Qt::WhatsThisRole, QVariant::fromValue(data6.value<PropertySheetStringValue>().value()));
         break;
      }

      default:
         break;
   }

   prop->setValue(getItemData(role));
}

void AbstractItemEditor::resetProperty(QtProperty *property)
{
   if (m_propertyManager->resetFontSubProperty(property)) {
      return;
   }

   if (m_propertyManager->resetIconSubProperty(property)) {
      return;
   }

   BoolBlocker block(m_updatingBrowser);

   QtVariantProperty *prop = m_propertyManager->variantProperty(property);
   int role = m_propertyToRole.value(prop);

   if (role == ItemFlagsShadowRole) {
      prop->setValue(QVariant::fromValue((int)QListWidgetItem().flags()));
   } else {
      prop->setValue(QVariant(prop->valueType(), nullptr));
   }
   prop->setModified(false);

   setItemData(role, QVariant());
   if (role == Qt::DecorationPropertyRole) {
      setItemData(Qt::DecorationRole, QVariant::fromValue(QIcon()));
   }

   if (role == Qt::DisplayPropertyRole) {
      setItemData(Qt::EditRole, QVariant::fromValue(QString()));
   }

   if (role == Qt::ToolTipPropertyRole) {
      setItemData(Qt::ToolTipRole, QVariant::fromValue(QString()));
   }

   if (role == Qt::StatusTipPropertyRole) {
      setItemData(Qt::StatusTipRole, QVariant::fromValue(QString()));
   }

   if (role == Qt::WhatsThisPropertyRole) {
      setItemData(Qt::WhatsThisRole, QVariant::fromValue(QString()));
   }
}

void AbstractItemEditor::cacheReloaded()
{
   BoolBlocker block(m_updatingBrowser);
   m_propertyManager->reloadResourceProperties();
}

void AbstractItemEditor::updateBrowser()
{
   BoolBlocker block(m_updatingBrowser);

   for (QtVariantProperty *prop : m_properties) {
      int role = m_propertyToRole.value(prop);
      QVariant val = getItemData(role);

      if (!val.isValid()) {
         if (role == ItemFlagsShadowRole) {
            val = QVariant::fromValue((int)QListWidgetItem().flags());
         } else {
            val = QVariant((int)prop->value().userType(), nullptr);
         }
         prop->setModified(false);

      } else {
         prop->setModified(true);
      }

      prop->setValue(val);
   }

   if (m_propertyBrowser->topLevelItems().isEmpty())
      for (QtVariantProperty *prop : m_rootProperties) {
         m_propertyBrowser->addProperty(prop);
      }
}

void AbstractItemEditor::injectPropertyBrowser(QWidget *parent, QWidget *widget)
{
   // It is impossible to design a splitter with just one widget, so we do it by hand.
   m_propertySplitter = new QSplitter;
   m_propertySplitter->addWidget(widget);
   m_propertySplitter->addWidget(m_propertyBrowser);
   m_propertySplitter->setStretchFactor(0, 1);
   m_propertySplitter->setStretchFactor(1, 0);
   parent->layout()->addWidget(m_propertySplitter);
}

ItemListEditor::ItemListEditor(QDesignerFormWindowInterface *form, QWidget *parent)
   : AbstractItemEditor(form, parent), m_updating(false)
{
   ui.setupUi(this);

   injectPropertyBrowser(this, ui.widget);
   setPropertyBrowserVisible(false);

   QIcon plusIcon  = createIconSet(QString::fromUtf8("plus.png"));
   QIcon minusIcon = createIconSet(QString::fromUtf8("minus.png"));
   QIcon upIcon    = createIconSet(QString::fromUtf8("up.png"));
   QIcon downIcon  = createIconSet(QString::fromUtf8("down.png"));

   ui.newListItemButton->setIcon(plusIcon);
   ui.deleteListItemButton->setIcon(minusIcon);
   ui.moveListItemUpButton->setIcon(upIcon);
   ui.moveListItemDownButton->setIcon(downIcon);

   connect(ui.newListItemButton, &QPushButton::clicked,
         this, &ItemListEditor::newListItemButton);

   connect(ui.deleteListItemButton, &QPushButton::clicked,
         this, &ItemListEditor::deleteListItemButton);

   connect(ui.moveListItemUpButton, &QPushButton::clicked,
         this, &ItemListEditor::moveListItemUpButton);

   connect(ui.moveListItemDownButton, &QPushButton::clicked,
         this, &ItemListEditor::moveListItemDownButton);

   connect(iconCache(), &DesignerIconCache::reloaded,
         this, &AbstractItemEditor::cacheReloaded);

   connect(ui.showPropertiesButton, &QAbstractButton::clicked,
         this, &ItemListEditor::togglePropertyBrowser);

   connect(ui.listWidget, &QListWidget::itemChanged,
         this, &ItemListEditor::listWidget_itemChanged);
}

void ItemListEditor::setupEditor(QWidget *object, PropertyDefinition *propList)
{
   AbstractItemEditor::setupEditor(object, propList);

   if (ui.listWidget->count() > 0) {
      ui.listWidget->setCurrentRow(0);
   } else {
      updateEditor();
   }
}

void ItemListEditor::setCurrentIndex(int idx)
{
   m_updating = true;
   ui.listWidget->setCurrentRow(idx);
   m_updating = false;
}

void ItemListEditor::newListItemButton()
{
   int row = ui.listWidget->currentRow() + 1;

   QListWidgetItem *item = new QListWidgetItem(m_newItemText);
   item->setData(Qt::DisplayPropertyRole, QVariant::fromValue(PropertySheetStringValue(m_newItemText)));
   item->setFlags(item->flags() | Qt::ItemIsEditable);

   if (row < ui.listWidget->count()) {
      ui.listWidget->insertItem(row, item);
   } else {
      ui.listWidget->addItem(item);
   }

   emit itemInserted(row);

   ui.listWidget->setCurrentItem(item);
   ui.listWidget->editItem(item);
}

void ItemListEditor::deleteListItemButton()
{
   int row = ui.listWidget->currentRow();

   if (row != -1) {
      delete ui.listWidget->takeItem(row);
      emit itemDeleted(row);
   }

   if (row == ui.listWidget->count()) {
      --row;
   }

   if (row < 0) {
      updateEditor();
   } else {
      ui.listWidget->setCurrentRow(row);
   }
}

void ItemListEditor::moveListItemUpButton()
{
   int row = ui.listWidget->currentRow();
   if (row <= 0) {
      return;   // nothing to do
   }

   ui.listWidget->insertItem(row - 1, ui.listWidget->takeItem(row));
   ui.listWidget->setCurrentRow(row - 1);
   emit itemMovedUp(row);
}

void ItemListEditor::moveListItemDownButton()
{
   int row = ui.listWidget->currentRow();
   if (row == -1 || row == ui.listWidget->count() - 1) {
      return;   // nothing to do
   }

   ui.listWidget->insertItem(row + 1, ui.listWidget->takeItem(row));
   ui.listWidget->setCurrentRow(row + 1);
   emit itemMovedDown(row);
}

void ItemListEditor::on_listWidget_currentRowChanged(int)
{
   // emerald - remove passed data type

   updateEditor();
   if (! m_updating) {
      emit indexChanged(ui.listWidget->currentRow());
   }
}

void ItemListEditor::listWidget_itemChanged(QListWidgetItem *item)
{
   if (m_updatingBrowser) {
      return;
   }

   QVariant data1 = item->data(Qt::DisplayPropertyRole);
   PropertySheetStringValue val = data1.value<PropertySheetStringValue>();

   val.setValue(item->text());

   BoolBlocker block(m_updatingBrowser);
   item->setData(Qt::DisplayPropertyRole, QVariant::fromValue(val));

   // checkState could change, but if this signal is connected then checkState
   // is not in the list since we are editing a header item
   emit itemChanged(ui.listWidget->currentRow(), Qt::DisplayPropertyRole, QVariant::fromValue(val));

   updateBrowser();
}

void ItemListEditor::togglePropertyBrowser()
{
   setPropertyBrowserVisible(!m_propertyBrowser->isVisible());
}

void ItemListEditor::setPropertyBrowserVisible(bool v)
{
   ui.showPropertiesButton->setText(v ? tr("Properties &>>") : tr("Properties &<<"));
   m_propertyBrowser->setVisible(v);
}

void ItemListEditor::setItemData(int role, const QVariant &v)
{
   QListWidgetItem *item = ui.listWidget->currentItem();
   bool reLayout = false;

   if ((role == Qt::EditRole && (v.toString().count(QLatin1Char('\n')) != item->data(role).toString().count(QLatin1Char('\n'))))
      || role == Qt::FontRole) {
      reLayout = true;
   }

   QVariant newValue = v;
   if (role == Qt::FontRole && newValue.type() == QVariant::Font) {
      QFont oldFont = ui.listWidget->font();
      QFont newFont = newValue.value<QFont>().resolve(oldFont);
      newValue = QVariant::fromValue(newFont);
      item->setData(role, QVariant()); // force the right font with the current resolve mask is set (item view bug)
   }

   item->setData(role, newValue);
   if (reLayout) {
      ui.listWidget->doItemsLayout();
   }
   emit itemChanged(ui.listWidget->currentRow(), role, newValue);
}

QVariant ItemListEditor::getItemData(int role) const
{
   return ui.listWidget->currentItem()->data(role);
}

void ItemListEditor::cacheReloaded()
{
   reloadIconResources(iconCache(), ui.listWidget);
}

void ItemListEditor::updateEditor()
{
   bool currentItemEnabled = false;

   bool moveRowUpEnabled = false;
   bool moveRowDownEnabled = false;

   QListWidgetItem *item = ui.listWidget->currentItem();
   if (item) {
      currentItemEnabled = true;
      int currentRow = ui.listWidget->currentRow();
      if (currentRow > 0) {
         moveRowUpEnabled = true;
      }
      if (currentRow < ui.listWidget->count() - 1) {
         moveRowDownEnabled = true;
      }
   }

   ui.moveListItemUpButton->setEnabled(moveRowUpEnabled);
   ui.moveListItemDownButton->setEnabled(moveRowDownEnabled);
   ui.deleteListItemButton->setEnabled(currentItemEnabled);

   if (item) {
      updateBrowser();
   } else {
      m_propertyBrowser->clear();
   }
}

} // namespace qdesigner_internal
