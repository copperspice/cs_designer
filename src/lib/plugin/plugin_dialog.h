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

#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include <ui_plugin_dialog.h>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class PluginDialog : public QDialog
{
   CS_OBJECT(PluginDialog)

 public:
   explicit PluginDialog(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);

 private:

   /* emerald - temporary hold
       CS_SLOT_1(Private, void updateCustomWidgetPlugins())
       CS_SLOT_2(updateCustomWidgetPlugins)
   */

   void populateTreeWidget();
   QTreeWidgetItem *setTopLevelItem(const QString &itemName);
   QTreeWidgetItem *setPluginItem(QTreeWidgetItem *topLevelItem, const QString &itemName, const QFont &font);
   void setItem(QTreeWidgetItem *pluginItem, const QString &name, const QString &toolTip, const QString &whatsThis, const QIcon &icon);

   QDesignerFormEditorInterface *m_core;
   Ui::PluginDialog ui;
   QIcon interfaceIcon;
   QIcon featureIcon;
};

}

#endif
