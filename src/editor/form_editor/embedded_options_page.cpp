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

#include <abstract_dialoggui.h>
#include <abstract_formeditor.h>
#include <deviceprofiledialog.h>
#include <embedded_options_page.h>
#include <formwindowmanager.h>
#include <widgetfactory.h>

#include <deviceprofile_p.h>
#include <formwindowbase_p.h>
#include <formwindowbase_p.h>
#include <iconloader_p.h>
#include <shared_settings_p.h>

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLabel>
#include <QMessageBox>
#include <QSet>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>

namespace qdesigner_internal {

constexpr const int INDEX_OFFSET = 1;

// Sort by name. Used by template, do not make it static!
bool deviceProfileLessThan(const DeviceProfile &d1, const DeviceProfile  &d2)
{
   return d1.name().toLower() < d2.name().toLower();
}

static bool ask(QWidget *parent, QDesignerDialogGuiInterface *dlgui, const QString &title, const QString &what)
{
   return dlgui->message(parent, QDesignerDialogGuiInterface::OtherMessage,
         QMessageBox::Question, title, what,
         QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
}

class EmbeddedOptionsControlPrivate
{
 public:
   EmbeddedOptionsControlPrivate(QDesignerFormEditorInterface *core);
   void init(EmbeddedOptionsControl *q);

   bool isDirty() const {
      return m_dirty;
   }

   void loadSettings();
   void saveSettings();
   void slotAdd();
   void slotEdit();
   void slotDelete();
   void slotProfileIndexChanged(int);

 private:
   EmbeddedOptionsControlPrivate(const EmbeddedOptionsControlPrivate &) = delete;
   EmbeddedOptionsControlPrivate &operator=(const EmbeddedOptionsControlPrivate &) = delete;

   QStringList existingProfileNames() const;
   void sortAndPopulateProfileCombo();
   void updateState();
   void updateDescriptionLabel();

   QDesignerFormEditorInterface *m_core;
   QComboBox *m_profileCombo;
   QToolButton *m_addButton;
   QToolButton *m_editButton;
   QToolButton *m_deleteButton;
   QLabel *m_descriptionLabel;

   QList<DeviceProfile> m_sortedProfiles;

   EmbeddedOptionsControl *m_q;
   bool m_dirty;
   QSet<QString> m_usedProfiles;
};

EmbeddedOptionsControlPrivate::EmbeddedOptionsControlPrivate(QDesignerFormEditorInterface *core)
   : m_core(core), m_profileCombo(new QComboBox), m_addButton(new QToolButton), m_editButton(new QToolButton),
     m_deleteButton(new QToolButton), m_descriptionLabel(new QLabel), m_q(nullptr), m_dirty(false)
{
   m_descriptionLabel->setMinimumHeight(80);

   // Determine used profiles to lock them
   const QDesignerFormWindowManagerInterface *fwm = core->formWindowManager();

   if (const int fwCount = fwm->formWindowCount()) {
      for (int i = 0; i < fwCount; i++)
         if (const FormWindowBase *fwb = dynamic_cast<const FormWindowBase *>(fwm->formWindow(i))) {
            const QString deviceProfileName = fwb->deviceProfileName();

            if (!deviceProfileName.isEmpty()) {
               m_usedProfiles.insert(deviceProfileName);
            }
         }
   }
}

void EmbeddedOptionsControlPrivate::init(EmbeddedOptionsControl *q)
{
   m_q = q;

   QVBoxLayout *vLayout = new QVBoxLayout;
   QHBoxLayout *hLayout = new QHBoxLayout;
   m_profileCombo->setMinimumWidth(200);
   m_profileCombo->setEditable(false);
   hLayout->addWidget(m_profileCombo);
   m_profileCombo->addItem(EmbeddedOptionsControl::tr("None"));

   EmbeddedOptionsControl::connect(m_profileCombo, cs_mp_cast<int>(&QComboBox::currentIndexChanged),
         m_q, &EmbeddedOptionsControl::slotProfileIndexChanged);

   m_addButton->setIcon(createIconSet(QString::fromUtf8("plus.png")));
   m_addButton->setToolTip(EmbeddedOptionsControl::tr("Add a profile"));

   EmbeddedOptionsControl::connect(m_addButton, &QAbstractButton::clicked,
         m_q, &EmbeddedOptionsControl::slotAdd);

   hLayout->addWidget(m_addButton);

   EmbeddedOptionsControl::connect(m_editButton, &QAbstractButton::clicked,
         m_q, &EmbeddedOptionsControl::slotEdit);

   m_editButton->setIcon(createIconSet(QString::fromUtf8("edit.png")));
   m_editButton->setToolTip(EmbeddedOptionsControl::tr("Edit the selected profile"));
   hLayout->addWidget(m_editButton);

   m_deleteButton->setIcon(createIconSet(QString::fromUtf8("minus.png")));
   m_deleteButton->setToolTip(EmbeddedOptionsControl::tr("Delete the selected profile"));

   EmbeddedOptionsControl::connect(m_deleteButton, &QAbstractButton::clicked,
         m_q, &EmbeddedOptionsControl::slotDelete);

   hLayout->addWidget(m_deleteButton);

   hLayout->addStretch();
   vLayout->addLayout(hLayout);
   vLayout->addWidget(m_descriptionLabel);
   m_q->setLayout(vLayout);
}

QStringList EmbeddedOptionsControlPrivate::existingProfileNames() const
{
   QStringList rc;
   auto dcend = m_sortedProfiles.constEnd();

   for (auto it = m_sortedProfiles.constBegin(); it != dcend; ++it) {
      rc.push_back(it->name());
   }

   return rc;
}

void EmbeddedOptionsControlPrivate::slotAdd()
{
   DeviceProfileDialog dlg(m_core->dialogGui(), m_q);
   dlg.setWindowTitle(EmbeddedOptionsControl::tr("Add Profile"));

   // Create a new profile with a new, unique name
   DeviceProfile settings;
   settings.fromSystem();
   dlg.setDeviceProfile(settings);

   const QStringList names = existingProfileNames();
   const QString newNamePrefix = EmbeddedOptionsControl::tr("New profile");

   QString newName = newNamePrefix;

   for (int i = 2; names.contains(newName); i++) {
      newName = newNamePrefix;
      newName += QString::number(i);
   }

   settings.setName(newName);
   dlg.setDeviceProfile(settings);
   if (dlg.showDialog(names)) {
      const DeviceProfile newProfile = dlg.deviceProfile();
      m_sortedProfiles.push_back(newProfile);
      // Maintain sorted order
      sortAndPopulateProfileCombo();
      const int index = m_profileCombo->findText(newProfile.name());
      m_profileCombo->setCurrentIndex(index);
      m_dirty = true;
   }
}

void EmbeddedOptionsControlPrivate::slotEdit()
{
   const int index =  m_profileCombo->currentIndex() - INDEX_OFFSET;
   if (index < 0) {
      return;
   }

   // Edit the profile, compile a list of existing names
   // excluding current one. re-insert if changed, re-sort if name changed.
   const DeviceProfile oldProfile = m_sortedProfiles.at(index);

   const QString oldName = oldProfile.name();

   QStringList names = existingProfileNames();
   names.removeAll(oldName);

   DeviceProfileDialog dlg(m_core->dialogGui(), m_q);
   dlg.setWindowTitle(EmbeddedOptionsControl::tr("Edit Profile"));
   dlg.setDeviceProfile(oldProfile);

   if (dlg.showDialog(names)) {
      const DeviceProfile newProfile = dlg.deviceProfile();

      if (newProfile != oldProfile) {
         m_dirty = true;
         m_sortedProfiles[index] = newProfile;

         if (newProfile.name() != oldName) {
            sortAndPopulateProfileCombo();
            const int currentIndex = m_profileCombo->findText(newProfile.name());
            m_profileCombo->setCurrentIndex(currentIndex);

         } else {
            updateDescriptionLabel();
         }

      }
   }
}

void EmbeddedOptionsControlPrivate::slotDelete()
{
   const int index =  m_profileCombo->currentIndex() - INDEX_OFFSET;
   if (index < 0) {
      return;
   }
   const QString name = m_sortedProfiles.at(index).name();
   if (ask(m_q, m_core->dialogGui(),
         EmbeddedOptionsControl::tr("Delete Profile"),
         EmbeddedOptionsControl::tr("Would you like to delete the profile '%1'?").formatArg(name))) {
      m_profileCombo->setCurrentIndex(0);
      m_sortedProfiles.removeAt(index);
      m_profileCombo->removeItem(index + INDEX_OFFSET);
      m_dirty = true;
   }
}

void EmbeddedOptionsControlPrivate::sortAndPopulateProfileCombo()
{
   // Clear items until only "None" is left
   for (int i = m_profileCombo->count() - 1; i > 0; i--) {
      m_profileCombo->removeItem(i);
   }
   if (!m_sortedProfiles.empty()) {
      std::sort(m_sortedProfiles.begin(), m_sortedProfiles.end(), deviceProfileLessThan);
      m_profileCombo->addItems(existingProfileNames());
   }
}

void EmbeddedOptionsControlPrivate::loadSettings()
{
   const QDesignerSharedSettings settings(m_core);
   m_sortedProfiles = settings.deviceProfiles();
   sortAndPopulateProfileCombo();
   // Index: 0 is "None"
   const int settingsIndex = settings.currentDeviceProfileIndex();
   const int profileIndex = settingsIndex >= 0 && settingsIndex <  m_sortedProfiles.size() ? settingsIndex + INDEX_OFFSET : 0;
   m_profileCombo->setCurrentIndex(profileIndex);
   updateState();
   m_dirty = false;
}

void EmbeddedOptionsControlPrivate::saveSettings()
{
   QDesignerSharedSettings settings(m_core);
   settings.setDeviceProfiles(m_sortedProfiles);

   // Index: 0 is "None"
   settings.setCurrentDeviceProfileIndex(m_profileCombo->currentIndex() - INDEX_OFFSET);
   m_dirty = false;
}

//: Format embedded device profile description
static const QString descriptionFormat = cs_mark_string_tr(
      "EmbeddedOptionsControl",
      "<html>"
      "<table>"
      "<tr><td><b>Font</b></td><td>%1, %2</td></tr>"
      "<tr><td><b>Style</b></td><td>%3</td></tr>"
      "<tr><td><b>Resolution</b></td><td>%4 x %5</td></tr>"
      "</table>"
      "</html>"
   );

static inline QString description(const DeviceProfile &p)
{
   QString styleName = p.style();

   if (styleName.isEmpty()) {
      styleName = EmbeddedOptionsControl::tr("Default");
   }

   QString retval =  EmbeddedOptionsControl::staticMetaObject().tr(descriptionFormat);

   return retval.formatArg(p.fontFamily()).formatArg(p.fontPointSize()).formatArg(styleName)
      .formatArg(p.dpiX()).formatArg(p.dpiY());
}

void EmbeddedOptionsControlPrivate::updateDescriptionLabel()
{
   const int profileIndex = m_profileCombo->currentIndex() - INDEX_OFFSET;
   if (profileIndex >= 0) {
      m_descriptionLabel->setText(description(m_sortedProfiles.at(profileIndex)));
   } else {
      m_descriptionLabel->clear();
   }
}

void EmbeddedOptionsControlPrivate::updateState()
{
   const int profileIndex = m_profileCombo->currentIndex() - INDEX_OFFSET;
   // Allow for changing/deleting only if it is not in use
   bool modifyEnabled = false;
   if (profileIndex >= 0) {
      modifyEnabled = !m_usedProfiles.contains(m_sortedProfiles.at(profileIndex).name());
   }
   m_editButton->setEnabled(modifyEnabled);
   m_deleteButton->setEnabled(modifyEnabled);
   updateDescriptionLabel();
}

void EmbeddedOptionsControlPrivate::slotProfileIndexChanged(int)
{
   updateState();
   m_dirty = true;
}

// ------------- EmbeddedOptionsControl
EmbeddedOptionsControl::EmbeddedOptionsControl(QDesignerFormEditorInterface *core, QWidget *parent) :
   QWidget(parent),
   m_d(new EmbeddedOptionsControlPrivate(core))
{
   m_d->init(this);
}

EmbeddedOptionsControl::~EmbeddedOptionsControl()
{
   delete m_d;
}

void EmbeddedOptionsControl::slotAdd()
{
   m_d->slotAdd();
}

void EmbeddedOptionsControl::slotEdit()
{
   m_d->slotEdit();
}

void EmbeddedOptionsControl::slotDelete()
{
   m_d->slotDelete();
}

void EmbeddedOptionsControl::loadSettings()
{
   m_d->loadSettings();
}

void EmbeddedOptionsControl::saveSettings()
{
   m_d->saveSettings();
}

void EmbeddedOptionsControl::slotProfileIndexChanged(int i)
{
   m_d->slotProfileIndexChanged(i);
}

bool EmbeddedOptionsControl::isDirty() const
{
   return m_d->isDirty();
}

// EmbeddedOptionsPage:
EmbeddedOptionsPage::EmbeddedOptionsPage(QDesignerFormEditorInterface *core) :
   m_core(core)
{
}

QString EmbeddedOptionsPage::name() const
{
   // Tab in preferences dialog
   return QCoreApplication::translate("EmbeddedOptionsPage", "Embedded Design");
}

QWidget *EmbeddedOptionsPage::createPage(QWidget *parent)
{
   QWidget *optionsWidget = new QWidget(parent);

   QVBoxLayout *optionsVLayout = new QVBoxLayout();

   //: EmbeddedOptionsControl group box"
   QGroupBox *gb = new QGroupBox(QCoreApplication::translate("EmbeddedOptionsPage", "Device Profiles"));
   QVBoxLayout *gbVLayout = new QVBoxLayout();
   m_embeddedOptionsControl = new EmbeddedOptionsControl(m_core);
   m_embeddedOptionsControl->loadSettings();
   gbVLayout->addWidget(m_embeddedOptionsControl);
   gb->setLayout(gbVLayout);
   optionsVLayout->addWidget(gb);

   optionsVLayout->addStretch(1);

   // Outer layout to give it horizontal stretch
   QHBoxLayout *optionsHLayout = new QHBoxLayout();
   optionsHLayout->addLayout(optionsVLayout);
   optionsHLayout->addStretch(1);
   optionsWidget->setLayout(optionsHLayout);
   return optionsWidget;
}

void EmbeddedOptionsPage::apply()
{
   if (!m_embeddedOptionsControl || !m_embeddedOptionsControl->isDirty()) {
      return;
   }

   m_embeddedOptionsControl->saveSettings();
   if (FormWindowManager *fw = dynamic_cast<qdesigner_internal::FormWindowManager *>(m_core->formWindowManager())) {
      fw->deviceProfilesChanged();
   }
}

void EmbeddedOptionsPage::finish()
{
}
}


