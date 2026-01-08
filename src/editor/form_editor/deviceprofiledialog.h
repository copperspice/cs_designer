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

#ifndef SYSTEMSETTINGSDIALOG_H
#define SYSTEMSETTINGSDIALOG_H

class QDesignerDialogGuiInterface;
class QDialogButtonBox;

#include <QDialog>
#include <QStringList>

namespace Ui {
class DeviceProfileDialog;
}

namespace qdesigner_internal {

class DeviceProfile;

// widget to edit system settings for embedded design
class DeviceProfileDialog : public QDialog
{
   CS_OBJECT(DeviceProfileDialog)

 public:
   explicit DeviceProfileDialog(QDesignerDialogGuiInterface *dlgGui, QWidget *parent = nullptr);
   ~DeviceProfileDialog();

   DeviceProfile deviceProfile() const;
   void setDeviceProfile(const DeviceProfile &s);

   bool showDialog(const QStringList &existingNames);

 private:
   CS_SLOT_1(Private, void setOkButtonEnabled(bool un_named_arg1))
   CS_SLOT_2(setOkButtonEnabled)

   CS_SLOT_1(Private, void nameChanged(const QString &name))
   CS_SLOT_2(nameChanged)

   CS_SLOT_1(Private, void save())
   CS_SLOT_2(save)

   CS_SLOT_1(Private, void open() override)
   CS_SLOT_2(open)

   void critical(const QString &title, const QString &msg);
   Ui::DeviceProfileDialog *m_ui;
   QDesignerDialogGuiInterface *m_dlgGui;
   QStringList m_existingNames;

   DeviceProfileDialog(const DeviceProfileDialog &) = delete;
   DeviceProfileDialog &operator=(const DeviceProfileDialog &) = delete;
};

}

#endif
