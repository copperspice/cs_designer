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
#include <abstract_optionspage.h>
#include <designer_appearanceoptions.h>
#include <preferences_dialog.h>
#include <ui_designer_preferences.h>

#include <QFileDialog>
#include <QPushButton>

PreferencesDialog::PreferencesDialog(QDesignerFormEditorInterface *core, QWidget *parentWidget) :
   QDialog(parentWidget), m_ui(new Ui::PreferencesDialog()), m_core(core)
{
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
   m_ui->setupUi(this);

   m_optionsPages = core->optionsPages();

   m_ui->m_optionTabWidget->clear();

   for (QDesignerOptionsPageInterface *optionsPage : m_optionsPages) {
      QWidget *page = optionsPage->createPage(this);
      m_ui->m_optionTabWidget->addTab(page, optionsPage->name());
      if (QDesignerAppearanceOptionsWidget *appearanceWidget = dynamic_cast<QDesignerAppearanceOptionsWidget *>(page))
         connect(appearanceWidget, &QDesignerAppearanceOptionsWidget::uiModeChanged,
            this, &PreferencesDialog::slotUiModeChanged);
   }

   connect(m_ui->m_dialogButtonBox, &QDialogButtonBox::rejected, this, &PreferencesDialog::slotRejected);
   connect(m_ui->m_dialogButtonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::slotAccepted);
   connect(applyButton(), &QAbstractButton::clicked, this, &PreferencesDialog::slotApply);
}

PreferencesDialog::~PreferencesDialog()
{
   delete m_ui;
}

QPushButton *PreferencesDialog::applyButton() const
{
   return m_ui->m_dialogButtonBox->button(QDialogButtonBox::Apply);
}

void PreferencesDialog::slotApply()
{
   for (QDesignerOptionsPageInterface *optionsPage : m_optionsPages) {
      optionsPage->apply();
   }
}

void PreferencesDialog::slotAccepted()
{
   slotApply();
   closeOptionPages();
   accept();
}

void PreferencesDialog::slotRejected()
{
   closeOptionPages();
   reject();
}

void PreferencesDialog::slotUiModeChanged(bool modified)
{
   // Cannot "apply" a ui mode change (destroy the dialogs parent)
   applyButton()->setEnabled(!modified);
}

void PreferencesDialog::closeOptionPages()
{
   for (QDesignerOptionsPageInterface *optionsPage : m_optionsPages) {
      optionsPage->finish();
   }
}
