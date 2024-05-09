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
#include <designer_settings.h>
#include <plaintext_editor.h>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

static const QString PlainTextDialogC = "PlainTextDialog";
static const QString Geometry = "Geometry";

namespace qdesigner_internal {

PlainTextEditorDialog::PlainTextEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent)  :
   QDialog(parent), m_editor(new QPlainTextEdit), m_core(core)
{
   setWindowTitle(tr("Edit text"));
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   QVBoxLayout *vlayout = new QVBoxLayout(this);
   vlayout->addWidget(m_editor);

   QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
   QPushButton *ok_button = buttonBox->button(QDialogButtonBox::Ok);
   ok_button->setDefault(true);
   connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
   connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
   vlayout->addWidget(buttonBox);

   QDesignerSettingsInterface *settings = core->settingsManager();
   settings->beginGroup(PlainTextDialogC);

   if (settings->contains(Geometry)) {
      restoreGeometry(settings->value(Geometry).toByteArray());
   }

   settings->endGroup();
}

PlainTextEditorDialog::~PlainTextEditorDialog()
{
   QDesignerSettingsInterface *settings = m_core->settingsManager();
   settings->beginGroup(PlainTextDialogC);

   settings->setValue(Geometry, saveGeometry());
   settings->endGroup();
}

int PlainTextEditorDialog::showDialog()
{
   m_editor->setFocus();
   return exec();
}

void PlainTextEditorDialog::setDefaultFont(const QFont &font)
{
   m_editor->setFont(font);
}

void PlainTextEditorDialog::setText(const QString &text)
{
   m_editor->setPlainText(text);
}

QString PlainTextEditorDialog::text() const
{
   return m_editor->toPlainText();
}

} // namespace qdesigner_internal

