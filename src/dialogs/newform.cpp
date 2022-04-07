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
#include <newform.h>
#include <designer_workbench.h>
#include <designer_actions.h>
#include <designer_formwindow.h>
#include <designer_settings.h>
#include <newform_widget.h>

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QTemporaryFile>
#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMenu>
#include <QCheckBox>
#include <QFrame>
#include <QMessageBox>

NewForm::NewForm(QDesignerWorkbench *workbench, QWidget *parentWidget, const QString &fileName)
   : QDialog(parentWidget, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
     m_fileName(fileName),
     m_newFormWidget(QDesignerNewFormWidgetInterface::createNewFormWidget(workbench->core())),
     m_workbench(workbench),
     m_chkShowOnStartup(new QCheckBox(tr("Show this Dialog on Startup"))),
     m_createButton(new QPushButton(QApplication::translate("NewForm", "C&reate", nullptr))),
     m_recentButton(new QPushButton(QApplication::translate("NewForm", "Recent", nullptr))),
     m_buttonBox(nullptr)
{
   setWindowTitle(tr("New Form"));
   QDesignerSettings settings(m_workbench->core());

   QVBoxLayout *vBoxLayout = new QVBoxLayout;

   connect(m_newFormWidget, &QDesignerNewFormWidgetInterface::templateActivated,
         this, &NewForm::slotTemplateActivated);

   connect(m_newFormWidget, &QDesignerNewFormWidgetInterface::currentTemplateChanged,
         this, &NewForm::slotCurrentTemplateChanged);

   vBoxLayout->addWidget(m_newFormWidget);

   QFrame *horizontalLine = new QFrame;
   horizontalLine->setFrameShape(QFrame::HLine);
   horizontalLine->setFrameShadow(QFrame::Sunken);
   vBoxLayout->addWidget(horizontalLine);

   m_chkShowOnStartup->setChecked(settings.showNewFormOnStartup());
   vBoxLayout->addWidget(m_chkShowOnStartup);

   m_buttonBox = createButtonBox();
   vBoxLayout->addWidget(m_buttonBox);
   setLayout(vBoxLayout);

   resize(500, 400);
   slotCurrentTemplateChanged(m_newFormWidget->hasCurrentTemplate());
}

QDialogButtonBox *NewForm::createButtonBox()
{
   // Dialog buttons with 'recent files'
   QDialogButtonBox *buttonBox = new QDialogButtonBox;
   buttonBox->addButton(QApplication::translate("NewForm", "&Close", nullptr), QDialogButtonBox::RejectRole);

   buttonBox->addButton(m_createButton, QDialogButtonBox::AcceptRole);
   buttonBox->addButton(QApplication::translate("NewForm", "&Open...", nullptr), QDialogButtonBox::ActionRole);

   buttonBox->addButton(m_recentButton, QDialogButtonBox::ActionRole);
   QDesignerActions *da   = m_workbench->actionManager();
   QMenu *recentFilesMenu = new QMenu(tr("&Recent Forms"), m_recentButton);

   // Pop the "Recent Files" stuff in here.
   const QList<QAction *> recentActions = da->recentFilesActions()->actions();

   if (! recentActions.empty()) {
      for (auto item : recentActions) {
         recentFilesMenu->addAction(item);
         connect(item, &QAction::triggered, this, &NewForm::recentFileChosen);
      }
   }

   m_recentButton->setMenu(recentFilesMenu);

   connect(buttonBox, &QDialogButtonBox::clicked, this, &NewForm::slotButtonBoxClicked);

   return buttonBox;
}

NewForm::~NewForm()
{
   QDesignerSettings settings (m_workbench->core());
   settings.setShowNewFormOnStartup(m_chkShowOnStartup->isChecked());
}

void NewForm::recentFileChosen()
{
   QAction *action = dynamic_cast<QAction *>(sender());
   if (!action) {
      return;
   }
   if (action->objectName() == QString("__qt_action_clear_menu_")) {
      return;
   }
   close();
}

void NewForm::slotCurrentTemplateChanged(bool templateSelected)
{
   if (templateSelected) {
      m_createButton->setEnabled(true);
      m_createButton->setDefault(true);
   } else {
      m_createButton->setEnabled(false);
   }
}

void NewForm::slotTemplateActivated()
{
   m_createButton->animateClick(0);
}

void NewForm::slotButtonBoxClicked(QAbstractButton *btn)
{
   switch (m_buttonBox->buttonRole(btn)) {
      case QDialogButtonBox::RejectRole:
         reject();
         break;

      case QDialogButtonBox::ActionRole:
         if (btn != m_recentButton) {
            m_fileName.clear();
            if (m_workbench->actionManager()->openForm(this)) {
               accept();
            }
         }
         break;

      case QDialogButtonBox::AcceptRole: {
         QString errorMessage;
         if (openTemplate(&errorMessage)) {
            accept();
         }  else {
            QMessageBox::warning(this, tr("Read error"), errorMessage);
         }
      }
      break;

      default:
         break;
   }
}

bool NewForm::openTemplate(QString *ptrToErrorMessage)
{
   const QString contents = m_newFormWidget->currentTemplate(ptrToErrorMessage);

   if (contents.isEmpty()) {
      return false;
   }

   // Write to temporary file and open
   QString tempPattern = QDir::tempPath();

   if (! tempPattern.endsWith(QDir::separator())) {
      // platform-dependant
      tempPattern += QDir::separator();
   }

   tempPattern += QString("XXXXXX.ui");
   QTemporaryFile tempFormFile(tempPattern);

   tempFormFile.setAutoRemove(true);
   if (! tempFormFile.open()) {
      *ptrToErrorMessage = tr("A temporary form file could not be created in %1.").formatArg(QDir::tempPath());
      return false;
   }

   const QString tempFormFileName = tempFormFile.fileName();
   tempFormFile.write(contents.toUtf8());
   if (!tempFormFile.flush())  {
      *ptrToErrorMessage = tr("The temporary form file %1 could not be written.").formatArg(tempFormFileName);
      return false;
   }

   tempFormFile.close();

   return m_workbench->openTemplate(tempFormFileName, m_fileName, ptrToErrorMessage);
}

QImage NewForm::grabForm(QDesignerFormEditorInterface *core, QIODevice &file,
   const QString &workingDir, const qdesigner_internal::DeviceProfile &dp)
{
   return qdesigner_internal::NewFormWidget::grabForm(core, file, workingDir, dp);
}


