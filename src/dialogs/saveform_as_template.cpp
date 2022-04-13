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

#include <saveform_as_template.h>
#include <designer_settings.h>

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

#include <abstract_formeditor.h>
#include <abstract_formwindow.h>

SaveFormAsTemplate::SaveFormAsTemplate(QDesignerFormEditorInterface *core,
   QDesignerFormWindowInterface *formWindow, QWidget *parent)
   : QDialog(parent, Qt::Sheet), m_core(core), m_formWindow(formWindow)
{
   ui.setupUi(this);
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   ui.templateNameEdit->setText(formWindow->mainContainer()->objectName());
   ui.templateNameEdit->selectAll();

   ui.templateNameEdit->setFocus();

   QStringList paths = QDesignerSettings(m_core).formTemplatePaths();
   ui.categoryCombo->addItems(paths);
   ui.categoryCombo->addItem(tr("Add path..."));
   m_addPathIndex = ui.categoryCombo->count() - 1;
   connect(ui.templateNameEdit, &QLineEdit::textChanged,
      this, &SaveFormAsTemplate::updateOKButton);

   connect(ui.categoryCombo, cs_mp_cast<int>(&QComboBox::activated),
      this, &SaveFormAsTemplate::checkToAddPath);
}

SaveFormAsTemplate::~SaveFormAsTemplate()
{
}

void SaveFormAsTemplate::accept()
{
   QString templateFileName = ui.categoryCombo->currentText() + '/';

   const QString name = ui.templateNameEdit->text();
   templateFileName   +=  name;

   const QString extension = ".ui";

   if (!templateFileName.endsWith(extension)) {
      templateFileName.append(extension);
   }

   QFile file(templateFileName);

   if (file.exists()) {
      QMessageBox msgBox(QMessageBox::Information, tr("Template Exists"),
         tr("A template with the name %1 already exists.\n"
            "Do you want overwrite the template?").formatArg(name), QMessageBox::Cancel, m_formWindow);

      msgBox.setDefaultButton(QMessageBox::Cancel);
      QPushButton *overwriteButton = msgBox.addButton(tr("Overwrite Template"), QMessageBox::AcceptRole);
      msgBox.exec();

      if (msgBox.clickedButton() != overwriteButton) {
         return;
      }
   }

   while (!file.open(QFile::WriteOnly)) {
      if (QMessageBox::information(m_formWindow, tr("Open Error"),
            tr("There was an error opening template %1 for writing. Reason: %2").formatArg(name).formatArg(file.errorString()),
            QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel) {
         return;
      }
   }

   const QString origName = m_formWindow->fileName();
   // ensure m_formWindow->contents() will convert properly resource paths to relative paths
   // (relative to template location, not to the current form location)
   m_formWindow->setFileName(templateFileName);

   QByteArray ba = m_formWindow->contents().toUtf8();
   m_formWindow->setFileName(origName);

   while (file.write(ba) != ba.size()) {
      if (QMessageBox::information(m_formWindow, tr("Write Error"),
            tr("There was an error writing the template %1 to disk. Reason: %2").formatArg(name).formatArg(file.errorString()),
            QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel) {
         file.close();
         file.remove();
         return;
      }
      file.reset();
   }
   // update the list of places too...
   QStringList sl;
   for (int i = 0; i < m_addPathIndex; ++i) {
      sl << ui.categoryCombo->itemText(i);
   }

   QDesignerSettings(m_core).setFormTemplatePaths(sl);

   QDialog::accept();
}

void SaveFormAsTemplate::updateOKButton(const QString &str)
{
   QPushButton *okButton = ui.buttonBox->button(QDialogButtonBox::Ok);
   okButton->setEnabled(!str.isEmpty());
}

QString  SaveFormAsTemplate::chooseTemplatePath(QWidget *parent)
{
   QString rc = QFileDialog::getExistingDirectory(parent,
         tr("Pick a directory to save templates in"));
   if (rc.isEmpty()) {
      return rc;
   }

   if (rc.endsWith(QDir::separator())) {
      rc.remove(rc.size() - 1, 1);
   }
   return rc;
}

void SaveFormAsTemplate::checkToAddPath(int itemIndex)
{
   if (itemIndex != m_addPathIndex) {
      return;
   }

   const QString dir = chooseTemplatePath(this);
   if (dir.isEmpty()) {
      ui.categoryCombo->setCurrentIndex(0);
      return;
   }

   ui.categoryCombo->insertItem(m_addPathIndex, dir);
   ui.categoryCombo->setCurrentIndex(m_addPathIndex);
   ++m_addPathIndex;
}


