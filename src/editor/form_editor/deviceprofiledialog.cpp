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

#include <abstract_dialoggui.h>
#include <deviceprofiledialog.h>
#include <ui_deviceprofile_dialog.h>

#include <deviceprofile_p.h>

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStyleFactory>
#include <QFontDatabase>
#include <QFileInfo>
#include <QFile>

static const QString profileExtensionC = "qdp";

static inline QString fileFilter()
{
   return qdesigner_internal::DeviceProfileDialog::tr("Device Profiles (*.%1)").formatArg(profileExtensionC);
}

// Populate a combo with a sequence of integers an also set them as data
template <class IntIterator>
static void populateNumericCombo(IntIterator iter1, IntIterator iter2, QComboBox *cb)
{
   QString str;
   cb->setEditable(false);

   for ( ; iter1 != iter2 ; ++iter1) {
      const int n = *iter1;

      str = QString::number(n);
      cb->addItem(str, QVariant(n));
   }
}

namespace qdesigner_internal {

DeviceProfileDialog::DeviceProfileDialog(QDesignerDialogGuiInterface *dlgGui, QWidget *parent)
   : QDialog(parent), m_ui(new Ui::DeviceProfileDialog), m_dlgGui(dlgGui)
{
   setModal(true);
   m_ui->setupUi(this);

   const QList<int> standardFontSizes = QFontDatabase::standardSizes();
   populateNumericCombo(standardFontSizes.constBegin(), standardFontSizes.constEnd(), m_ui->m_systemFontSizeCombo);

   // Styles
   const QStringList styles = QStyleFactory::keys();
   m_ui->m_styleCombo->addItem(tr("Default"), QVariant(QString()));

   for (auto item : styles) {
      m_ui->m_styleCombo->addItem(item, item);
   }

   connect(m_ui->m_nameLineEdit, &QLineEdit::textChanged,     this, &DeviceProfileDialog::nameChanged);
   connect(m_ui->buttonBox,      &QDialogButtonBox::rejected, this, &QDialog::reject);

   connect(m_ui->buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked, this, &QDialog::accept);

   // Load/Save emit accepted() of the button box
   connect(m_ui->buttonBox->button(QDialogButtonBox::Save), &QAbstractButton::clicked,
         this, &DeviceProfileDialog::save);

   connect(m_ui->buttonBox->button(QDialogButtonBox::Open), &QAbstractButton::clicked,
         this, &DeviceProfileDialog::open);
}

DeviceProfileDialog::~DeviceProfileDialog()
{
   delete m_ui;
}

DeviceProfile DeviceProfileDialog::deviceProfile() const
{
   DeviceProfile rc;
   rc.setName(m_ui->m_nameLineEdit->text());
   rc.setFontFamily(m_ui->m_systemFontComboBox->currentFont().family());
   rc.setFontPointSize(m_ui->m_systemFontSizeCombo->itemData(m_ui->m_systemFontSizeCombo->currentIndex()).toInt());

   int dpiX, dpiY;
   m_ui->m_dpiChooser->getDPI(&dpiX, &dpiY);
   rc.setDpiX(dpiX);
   rc.setDpiY(dpiY);

   rc.setStyle(m_ui->m_styleCombo->itemData(m_ui->m_styleCombo->currentIndex()).toString());

   return rc;
}

void DeviceProfileDialog::setDeviceProfile(const DeviceProfile &s)
{
   m_ui->m_nameLineEdit->setText(s.name());
   m_ui->m_systemFontComboBox->setCurrentFont(QFont(s.fontFamily()));
   const int fontSizeIndex = m_ui->m_systemFontSizeCombo->findData(QVariant(s.fontPointSize()));
   m_ui->m_systemFontSizeCombo->setCurrentIndex(fontSizeIndex != -1 ? fontSizeIndex : 0);
   m_ui->m_dpiChooser->setDPI(s.dpiX(), s.dpiY());
   const int styleIndex = m_ui->m_styleCombo->findData(s.style());
   m_ui->m_styleCombo->setCurrentIndex(styleIndex != -1 ? styleIndex : 0);
}

void DeviceProfileDialog::setOkButtonEnabled(bool v)
{
   m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(v);
}

bool DeviceProfileDialog::showDialog(const QStringList &existingNames)
{
   m_existingNames = existingNames;
   m_ui->m_nameLineEdit->setFocus(Qt::OtherFocusReason);
   nameChanged(m_ui->m_nameLineEdit->text());
   return exec() == Accepted;
}

void DeviceProfileDialog::nameChanged(const QString &name)
{
   const bool invalid = name.isEmpty() || m_existingNames.indexOf(name) != -1;
   setOkButtonEnabled(!invalid);
}

void DeviceProfileDialog::save()
{
   QString fn = m_dlgGui->getSaveFileName(this, tr("Save Profile"), QString(), fileFilter());
   if (fn.isEmpty()) {
      return;
   }

   if (QFileInfo(fn).completeSuffix().isEmpty()) {
      fn += '.';
      fn += profileExtensionC;
   }

   QFile file(fn);
   if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      critical(tr("Save Profile - Error"), tr("Unable to open the file '%1' for writing: %2").formatArgs(fn, file.errorString()));
      return;
   }

   file.write(deviceProfile().toXml().toUtf8());
}

void DeviceProfileDialog::open()
{
   const QString fn = m_dlgGui->getOpenFileName(this, tr("Open profile"), QString(), fileFilter());
   if (fn.isEmpty()) {
      return;
   }

   QFile file(fn);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      critical(tr("Open Profile - Error"), tr("Unable to open the file '%1' for reading: %2").formatArgs(fn, file.errorString()));
      return;
   }
   QString errorMessage;
   DeviceProfile newSettings;
   if (!newSettings.fromXml(QString::fromUtf8(file.readAll()), &errorMessage)) {
      critical(tr("Open Profile - Error"), tr("'%1' is not a valid profile: %2").formatArgs(fn, errorMessage));
      return;
   }
   setDeviceProfile(newSettings);
}

void DeviceProfileDialog::critical(const QString &title, const QString &msg)
{
   m_dlgGui->message(this, QDesignerDialogGuiInterface::OtherMessage, QMessageBox::Critical, title, msg);
}

}   // end namespace


