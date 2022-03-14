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

#ifndef NEWFORM_H
#define NEWFORM_H

#include <QDialog>

namespace qdesigner_internal {
class DeviceProfile;
}

class QDesignerFormEditorInterface;
class QDesignerNewFormWidgetInterface;
class QDesignerWorkbench;

class QCheckBox;
class QAbstractButton;
class QPushButton;
class QDialogButtonBox;
class QImage;
class QIODevice;

class NewForm: public QDialog
{
   CS_OBJECT(NewForm)

 public:
   NewForm(QDesignerWorkbench *workbench, QWidget *parentWidget,  const QString &fileName = QString());

   virtual ~NewForm();

   // Convenience for implementing file dialogs with preview
   static QImage grabForm(QDesignerFormEditorInterface *core, QIODevice &file,
      const QString &workingDir, const qdesigner_internal::DeviceProfile &dp);

 private:
   Q_DISABLE_COPY(NewForm)

   CS_SLOT_1(Private, void slotButtonBoxClicked(QAbstractButton *btn))
   CS_SLOT_2(slotButtonBoxClicked)

   CS_SLOT_1(Private, void recentFileChosen())
   CS_SLOT_2(recentFileChosen)

   CS_SLOT_1(Private, void slotCurrentTemplateChanged(bool templateSelected))
   CS_SLOT_2(slotCurrentTemplateChanged)

   CS_SLOT_1(Private, void slotTemplateActivated())
   CS_SLOT_2(slotTemplateActivated)

   QDialogButtonBox *createButtonBox();
   bool openTemplate(QString *ptrToErrorMessage);

   QString m_fileName;
   QDesignerNewFormWidgetInterface *m_newFormWidget;
   QDesignerWorkbench *m_workbench;
   QCheckBox *m_chkShowOnStartup;
   QPushButton *m_createButton;
   QPushButton *m_recentButton;
   QDialogButtonBox *m_buttonBox;
};

#endif
