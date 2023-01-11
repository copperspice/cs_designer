/***********************************************************************
*
* Copyright (c) 2021-2023 Barbara Geller
* Copyright (c) 2021-2023 Ansel Sermersheim
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

#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include <QDialog>

class QPushButton;
class QDesignerFormEditorInterface;
class QDesignerOptionsPageInterface;

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog: public QDialog
{
   CS_OBJECT(PreferencesDialog) public:

   explicit PreferencesDialog(QDesignerFormEditorInterface *core, QWidget *parentWidget = nullptr);
   ~PreferencesDialog();

 private:
   CS_SLOT_1(Private, void slotAccepted())
   CS_SLOT_2(slotAccepted)

   CS_SLOT_1(Private, void slotRejected())
   CS_SLOT_2(slotRejected)

   CS_SLOT_1(Private, void slotApply())
   CS_SLOT_2(slotApply)

   CS_SLOT_1(Private, void slotUiModeChanged(bool modified))
   CS_SLOT_2(slotUiModeChanged)

   QPushButton *applyButton() const;
   void closeOptionPages();

   Ui::PreferencesDialog *m_ui;
   QDesignerFormEditorInterface *m_core;
   QList<QDesignerOptionsPageInterface *> m_optionsPages;
};

#endif
