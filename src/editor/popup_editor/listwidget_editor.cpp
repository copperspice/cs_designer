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
#include <abstract_formbuilder.h>
#include <designer_property.h>
#include <designer_settings.h>
#include <listwidget_editor.h>

#include <QComboBox>
#include <QGroupBox>
#include <QDialogButtonBox>

using namespace qdesigner_internal;

ListWidgetEditor::ListWidgetEditor(QDesignerFormWindowInterface *form, QWidget *parent)
   : QDialog(parent)
{
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
   QDialogButtonBox *buttonBox = new QDialogButtonBox;
   buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
   connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
   connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

   m_itemsEditor = new ItemListEditor(form, nullptr);
   m_itemsEditor->layout()->setMargin(0);
   m_itemsEditor->setNewItemText(tr("New Item"));

   QFrame *sep = new QFrame;
   sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QBoxLayout *box = new QVBoxLayout(this);
   box->addWidget(m_itemsEditor);
   box->addWidget(sep);
   box->addWidget(buttonBox);

   // Numbers copied from itemlist_editor.ui
   // (Automatic resizing doesn't work because ui has parent).
   resize(550, 360);
}

static AbstractItemEditor::PropertyDefinition listBoxPropList[] = {
   { Qt::DisplayPropertyRole,      0, DesignerPropertyManager::designerStringTypeId, "text" },
   { Qt::DecorationPropertyRole,   0, DesignerPropertyManager::designerIconTypeId,   "icon" },
   { Qt::ToolTipPropertyRole,      0, DesignerPropertyManager::designerStringTypeId, "toolTip"   },
   { Qt::StatusTipPropertyRole,    0, DesignerPropertyManager::designerStringTypeId, "statusTip" },
   { Qt::WhatsThisPropertyRole,    0, DesignerPropertyManager::designerStringTypeId, "whatsThis" },
   { Qt::FontRole,                 QVariant::Font,  nullptr, "font" },
   { Qt::TextAlignmentRole,        0, DesignerPropertyManager::designerAlignmentTypeId, "textAlignment" },
   { Qt::BackgroundRole,           QVariant::Brush, nullptr, "background" },
   { Qt::ForegroundRole,           QVariant::Brush, nullptr, "foreground" },
   { ItemFlagsShadowRole,          0, QtVariantPropertyManager::flagTypeId, "flags" },
   { Qt::CheckStateRole,           0, QtVariantPropertyManager::enumTypeId, "checkState" },
   { 0, 0, nullptr, QString() }
};

ListData ListWidgetEditor::fillContentsFromListWidget(QListWidget *listWidget)
{
   setWindowTitle(tr("Edit List Widget"));

   ListData retVal;
   retVal.createFromListWidget(listWidget, false);
   retVal.applyToListWidget(m_itemsEditor->listWidget(), m_itemsEditor->iconCache(), true);

   m_itemsEditor->setupEditor(listWidget, listBoxPropList);

   return retVal;
}

static AbstractItemEditor::PropertyDefinition comboBoxPropList[] = {
   { Qt::DisplayPropertyRole,    0, DesignerPropertyManager::designerStringTypeId, "text" },
   { Qt::DecorationPropertyRole, 0, DesignerPropertyManager::designerIconTypeId,   "icon" },
   { 0, 0, nullptr, QString() }
};

ListData ListWidgetEditor::fillContentsFromComboBox(QComboBox *comboBox)
{
   setWindowTitle(tr("Edit Combobox"));

   ListData retVal;
   retVal.createFromComboBox(comboBox);
   retVal.applyToListWidget(m_itemsEditor->listWidget(), m_itemsEditor->iconCache(), true);

   m_itemsEditor->setupEditor(comboBox, comboBoxPropList);

   return retVal;
}

ListData ListWidgetEditor::contents() const
{
   ListData retVal;
   retVal.createFromListWidget(m_itemsEditor->listWidget(), true);
   return retVal;
}
