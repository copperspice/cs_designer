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

#include <abstract_formeditor.h>
#include <abstract_integration.h>
#include <abstract_widgetdatabase.h>
#include <customwidget.h>
#include <plugin_dialog.h>
#include <plugin_manager.h>

#include <iconloader_p.h>

#include <QStyle>
#include <QHeaderView>
#include <QPushButton>
#include <QFileInfo>
#include <QPluginLoader>

namespace qdesigner_internal {

PluginDialog::PluginDialog(QDesignerFormEditorInterface *core, QWidget *parent)
   : QDialog(parent

#ifdef Q_OS_DARWIN
        , Qt::Tool
#endif

     ), m_core(core)
{
   ui.setupUi(this);

   ui.message->hide();

   const QStringList headerLabels(tr("Components"));

   ui.treeWidget->setAlternatingRowColors(false);
   ui.treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
   ui.treeWidget->setHeaderLabels(headerLabels);
   ui.treeWidget->header()->hide();

   interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon),   QIcon::Normal, QIcon::On);
   interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon), QIcon::Normal, QIcon::Off);
   featureIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileIcon));

   setWindowTitle(tr("Plugin Information"));
   populateTreeWidget();

   QPushButton *updateButton = new QPushButton(tr("Refresh"));
   const QString tooltip = tr("Scan for newly installed custom widget plugins.");
   updateButton->setToolTip(tooltip);
   updateButton->setWhatsThis(tooltip);

   // emerald - temporary hold
   // connect(updateButton, &QAbstractButton::clicked, this, &PluginDialog::updateCustomWidgetPlugins);

   ui.buttonBox->addButton(updateButton, QDialogButtonBox::ActionRole);
}

void PluginDialog::populateTreeWidget()
{
   ui.treeWidget->clear();
   QDesignerPluginManager *pluginManager = m_core->pluginManager();
   const QStringList fileNames = pluginManager->registeredPlugins();

   if (! fileNames.isEmpty()) {
      QTreeWidgetItem *topLevelItem = setTopLevelItem(tr("Loaded Plugins"));
      QFont boldFont = topLevelItem->font(0);

      for (const QString &fileName : fileNames) {
         QPluginLoader loader(fileName);
         const QFileInfo fileInfo(fileName);

         QTreeWidgetItem *pluginItem = setPluginItem(topLevelItem, fileInfo.fileName(), boldFont);

         if (QObject *plugin = loader.instance()) {
            if (const QDesignerCustomWidgetCollectionInterface *c = dynamic_cast<QDesignerCustomWidgetCollectionInterface *>(plugin)) {
               for (const QDesignerCustomWidgetInterface *p : c->customWidgets()) {
                  setItem(pluginItem, p->name(), p->toolTip(), p->whatsThis(), p->icon());
               }

            } else {
               if (const QDesignerCustomWidgetInterface *p = dynamic_cast<QDesignerCustomWidgetInterface *>(plugin)) {
                  setItem(pluginItem, p->name(), p->toolTip(), p->whatsThis(), p->icon());
               }
            }
         }
      }
   }

   const QStringList notLoadedPlugins = pluginManager->failedPlugins();

   if (! notLoadedPlugins.isEmpty()) {
      QTreeWidgetItem *topLevelItem = setTopLevelItem(tr("Failed Plugins"));
      const QFont boldFont = topLevelItem->font(0);

      for (const QString &plugin : notLoadedPlugins) {
         const QString failureReason = pluginManager->failureReason(plugin);
         QTreeWidgetItem *pluginItem = setPluginItem(topLevelItem, plugin, boldFont);
         setItem(pluginItem, failureReason, failureReason, QString(), QIcon());
      }
   }

   if (ui.treeWidget->topLevelItemCount() == 0) {
      ui.label->setText(tr("CS Designer could not find any plugins"));
      ui.treeWidget->hide();

   } else {
      ui.label->setText(tr("CS Designer found the following plugins"));
   }
}

QTreeWidgetItem *PluginDialog::setTopLevelItem(const QString &itemName)
{
   QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(ui.treeWidget);
   topLevelItem->setText(0, itemName);
   ui.treeWidget->setItemExpanded(topLevelItem, true);
   topLevelItem->setIcon(0, style()->standardPixmap(QStyle::SP_DirOpenIcon));

   QFont boldFont = topLevelItem->font(0);
   boldFont.setBold(true);
   topLevelItem->setFont(0, boldFont);

   return topLevelItem;
}

QTreeWidgetItem *PluginDialog::setPluginItem(QTreeWidgetItem *topLevelItem,
   const QString &itemName, const QFont &font)
{
   QTreeWidgetItem *pluginItem = new QTreeWidgetItem(topLevelItem);
   pluginItem->setFont(0, font);
   pluginItem->setText(0, itemName);
   ui.treeWidget->setItemExpanded(pluginItem, true);
   pluginItem->setIcon(0, style()->standardPixmap(QStyle::SP_DirOpenIcon));

   return pluginItem;
}

void PluginDialog::setItem(QTreeWidgetItem *pluginItem, const QString &name,
   const QString &toolTip, const QString &whatsThis, const QIcon &icon)
{
   QTreeWidgetItem *item = new QTreeWidgetItem(pluginItem);
   item->setText(0, name);
   item->setToolTip(0, toolTip);
   item->setWhatsThis(0, whatsThis);

   if (icon.isNull()) {
      QIcon iconImage = QIcon(":/resources/form_editor/images/scratchpad-32.png");
      item->setIcon(0, iconImage);
   } else {
      item->setIcon(0, icon);
   }
}

/* emerald - temporary hold

void  PluginDialog::updateCustomWidgetPlugins()
{
    const int before = m_core->widgetDataBase()->count();
    m_core->integration()->updateCustomWidgetPlugins();
    const int after = m_core->widgetDataBase()->count();
    if (after >  before) {
        ui.message->setText(tr("New custom widget plugins have been found."));
        ui.message->show();
    } else {
        ui.message->setText(QString());
    }
    populateTreeWidget();
}

*/

}
