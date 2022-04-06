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

#include <textedit_findwidget.h>
#include <code_dialog.h>

#include <designer_utils.h>
#include <iconloader_p.h>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QIcon>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QToolBar>
#include <QVBoxLayout>
#include <QDebug>
#include <QDir>
#include <QTemporaryFile>

namespace qdesigner_internal {

struct CodeDialog::CodeDialogPrivate {
   CodeDialogPrivate();

   QTextEdit *m_textEdit;
   TextEditFindWidget *m_findWidget;
   QString m_formFileName;
};

CodeDialog::CodeDialogPrivate::CodeDialogPrivate()
   : m_textEdit(new QTextEdit)
   , m_findWidget(new TextEditFindWidget)
{
}

// ----------------- CodeDialog
CodeDialog::CodeDialog(QWidget *parent) :
   QDialog(parent),
   m_impl(new CodeDialogPrivate)
{
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
   QVBoxLayout *vBoxLayout = new QVBoxLayout;

   // Edit tool bar
   QToolBar *toolBar = new QToolBar;

   const QIcon saveIcon = createIconSet(QString("filesave.png"));
   QAction *saveAction = toolBar->addAction(saveIcon, tr("Save..."));
   connect(saveAction, &QAction::triggered, this, &CodeDialog::slotSaveAs);

   const QIcon copyIcon = createIconSet(QString("editcopy.png"));
   QAction *copyAction = toolBar->addAction(copyIcon, tr("Copy All"));
   connect(copyAction, &QAction::triggered, this, &CodeDialog::copyAll);

   QAction *findAction = toolBar->addAction(
         TextEditFindWidget::findIconSet(),
         tr("&Find in Text..."),
         m_impl->m_findWidget, &AbstractFindWidget::activate);
   findAction->setShortcut(QKeySequence::Find);

   vBoxLayout->addWidget(toolBar);

   // Edit
   QFont font = m_impl->m_textEdit->font();
   font.setPointSize(10);
   m_impl->m_textEdit->setFont(font);

   m_impl->m_textEdit->setReadOnly(true);
   m_impl->m_textEdit->setMinimumSize(QSize(
         m_impl->m_findWidget->minimumSize().width(),
         500));
   vBoxLayout->addWidget(m_impl->m_textEdit);

   // Find
   m_impl->m_findWidget->setTextEdit(m_impl->m_textEdit);
   vBoxLayout->addWidget(m_impl->m_findWidget);

   // Button box
   QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal);
   connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

   // Disable auto default
   QPushButton *closeButton = buttonBox->button(QDialogButtonBox::Close);
   closeButton->setAutoDefault(false);
   vBoxLayout->addWidget(buttonBox);

   setLayout(vBoxLayout);
}

CodeDialog::~CodeDialog()
{
   delete m_impl;
}

void CodeDialog::setCode(const QString &code)
{
   m_impl->m_textEdit->setPlainText(code);
}

QString CodeDialog::code() const
{
   return m_impl->m_textEdit->toPlainText();
}

void CodeDialog::setFormFileName(const QString &f)
{
   m_impl->m_formFileName = f;
}

QString CodeDialog::formFileName() const
{
   return m_impl->m_formFileName;
}

bool CodeDialog::generateCode(const QDesignerFormWindowInterface *fw,
   QString *code,
   QString *errorMessage)
{
   // Generate temporary file name similar to form file name
   // (for header guards)
   QString tempPattern = QDir::tempPath();
   if (!tempPattern.endsWith(QDir::separator())) { // platform-dependant
      tempPattern += QDir::separator();
   }
   const QString fileName = fw->fileName();
   if (fileName.isEmpty()) {
      tempPattern += QString("designer");
   } else {
      tempPattern += QFileInfo(fileName).baseName();
   }
   tempPattern += QString("XXXXXX.ui");
   // Write to temp file
   QTemporaryFile tempFormFile(tempPattern);

   tempFormFile.setAutoRemove(true);
   if (!tempFormFile.open()) {
      *errorMessage = tr("A temporary form file could not be created in %1.").formatArg(QDir::tempPath());
      return false;
   }
   const QString tempFormFileName = tempFormFile.fileName();
   tempFormFile.write(fw->contents().toUtf8());
   if (!tempFormFile.flush())  {
      *errorMessage = tr("The temporary form file %1 could not be written.").formatArg(tempFormFileName);
      return false;
   }
   tempFormFile.close();
   // Run uic
   QByteArray rc;
   if (!runUIC(tempFormFileName, rc, *errorMessage)) {
      return false;
   }
   *code = QString::fromUtf8(rc);
   return true;
}

bool CodeDialog::showCodeDialog(const QDesignerFormWindowInterface *fw, QWidget *parent, QString *errorMessage)
{
   QString code;

   if (! generateCode(fw, &code, errorMessage)) {
      return false;
   }

   CodeDialog dialog(parent);
   dialog.setWindowTitle(tr("%1 - [Code]").formatArg(fw->mainContainer()->windowTitle()));
   dialog.setCode(code);
   dialog.setFormFileName(fw->fileName());
   dialog.exec();

   return true;
}

void CodeDialog::slotSaveAs()
{
   // build the default relative name 'ui_sth.h'
   const QString headerSuffix = QString('h');

   QString filter;
   const QString uiFile = formFileName();

   if (!uiFile.isEmpty()) {
      filter = QString("ui_");
      filter += QFileInfo(uiFile).baseName();
      filter += '.';
      filter += headerSuffix;
   }
   // file dialog
   while (true) {
      const QString fileName =
         QFileDialog::getSaveFileName (this, tr("Save Code"), filter, tr("Header Files (*.%1)").formatArg(headerSuffix));
      if (fileName.isEmpty()) {
         break;
      }

      QFile file(fileName);
      if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
         warning(tr("The file %1 could not be opened: %2").formatArg(fileName).formatArg(file.errorString()));
         continue;
      }
      file.write(code().toUtf8());
      if (!file.flush()) {
         warning(tr("The file %1 could not be written: %2").formatArg(fileName).formatArg(file.errorString()));
         continue;
      }
      file.close();
      break;
   }
}

void CodeDialog::warning(const QString &msg)
{
   QMessageBox::warning(this, tr("%1 - Error").formatArg(windowTitle()), msg, QMessageBox::Close);
}

void CodeDialog::copyAll()
{
   QApplication::clipboard()->setText(code());
}

} // namespace qdesigner_internal

