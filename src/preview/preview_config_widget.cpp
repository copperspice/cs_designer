/***********************************************************************
*
* Copyright (c) 2021-2025 Barbara Geller
* Copyright (c) 2021-2025 Ansel Sermersheim
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

#include <designer_settings.h>
#include <preview_config_widget.h>
#include <preview_manager.h>
#include <stylesheet_editor.h>
#include <ui_preview_configuration.h>

#include <iconloader_p.h>
#include <shared_settings_p.h>

#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QMessageBox>
#include <QPair>
#include <QSharedData>
#include <QStyleFactory>

namespace qdesigner_internal {

class PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate
{
 public:
   PreviewConfigurationWidgetPrivate(QDesignerFormEditorInterface *core, QGroupBox *g);

   void slotEditAppStyleSheet();

   //   void slotDeleteSkinEntry();
   //   void slotSkinChanged(int index);

   void retrieveSettings();
   void storeSettings() const;

   QAbstractButton *appStyleSheetChangeButton() const {
      return m_ui.m_appStyleSheetChangeButton;
   }

   //   QAbstractButton *skinRemoveButton() const { return m_ui.m_skinRemoveButton; }
   //   QComboBox *skinCombo() const { return m_ui.m_skinCombo; }

   QDesignerFormEditorInterface *m_core;

 private:
   PreviewConfiguration previewConfiguration() const;
   void setPreviewConfiguration(const PreviewConfiguration &pc);

   //   QStringList userSkins() const;
   //   void addUserSkins(const QStringList &files);
   //   bool canRemoveSkin(int index) const;
   //   int browseSkin();

   const QString m_defaultStyle;
   QGroupBox *m_parent;
   Ui::PreviewConfigurationWidget m_ui;


   //   int m_firstUserSkinIndex;
   //   int m_browseSkinIndex;
   //   int m_lastSkinIndex;          // required in case browse fails
};

PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate::PreviewConfigurationWidgetPrivate(
      QDesignerFormEditorInterface *core, QGroupBox *g)
   : m_core(core), m_defaultStyle(PreviewConfigurationWidget::tr("Default")), m_parent(g)

     //  m_firstUserSkinIndex(0), m_browseSkinIndex(0), m_lastSkinIndex(0)
{
   m_ui.setupUi(g);

   // styles
   m_ui.m_styleCombo->setEditable(false);
   QStringList styleItems(m_defaultStyle);
   styleItems += QStyleFactory::keys();
   m_ui.m_styleCombo->addItems(styleItems);

   // sheet
   m_ui.m_appStyleSheetLineEdit->setTextPropertyValidationMode(qdesigner_internal::ValidationStyleSheet);
   m_ui.m_appStyleSheetClearButton->setIcon(qdesigner_internal::createIconSet(QString::fromUtf8("resetproperty.png")));
   QObject::connect(m_ui.m_appStyleSheetClearButton, &QAbstractButton::clicked,
      m_ui.m_appStyleSheetLineEdit, &qdesigner_internal::TextPropertyEditor::clear);

   /*
       m_ui.m_skinRemoveButton->setIcon(qdesigner_internal::createIconSet(QString::fromUtf8("editdelete.png")));

       // skins: find default skins (resources)
       m_ui.m_skinRemoveButton->setEnabled(false);
       Skins skins = defaultSkins();
       skins.push_front(SkinNamePath(PreviewConfigurationWidget::tr("None"), QString()));

       const Skins::const_iterator scend = skins.constEnd();
       for (Skins::const_iterator it = skins.constBegin(); it != scend; ++it)
           m_ui.m_skinCombo->addItem (it->first, QVariant(it->second));
       m_browseSkinIndex = m_firstUserSkinIndex = skins.size();
       m_ui.m_skinCombo->addItem(PreviewConfigurationWidget::tr("Browse..."), QString());

       m_ui.m_skinCombo->setMaxVisibleItems (qMax(15, 2 * m_browseSkinIndex));
       m_ui.m_skinCombo->setEditable(false);
   */

   retrieveSettings();
}

/*
bool PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate::canRemoveSkin(int index) const
{
    return index >= m_firstUserSkinIndex && index != m_browseSkinIndex;
}

QStringList PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate::userSkins() const
{
    QStringList rc;
    for (int i = m_firstUserSkinIndex; i < m_browseSkinIndex; i++)
        rc.push_back(m_ui.m_skinCombo->itemData(i).toString());
    return rc;
}

void PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate::addUserSkins(const QStringList &files)
{
    if (files.empty())
        return;

    const QStringList ::const_iterator fcend = files.constEnd();

    for (QStringList::const_iterator it = files.constBegin(); it != fcend; ++it) {
        const QFileInfo fi(*it);
        if (fi.isDir() && fi.isReadable()) {
            m_ui.m_skinCombo->insertItem(m_browseSkinIndex++, fi.baseName(), QVariant(*it));
        } else {
            qWarning() << "Unable to access the skin directory " << *it;
        }
    }
}
*/


PreviewConfiguration PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate::previewConfiguration() const
{
   PreviewConfiguration  rc;
   QString style = m_ui.m_styleCombo->currentText();
   if (style == m_defaultStyle) {
      style.clear();
   }
   const QString applicationStyleSheet = m_ui.m_appStyleSheetLineEdit->text();

   /*
       // Figure out skin. 0 is None by definition..
       const int skinIndex = m_ui.m_skinCombo->currentIndex();

       QString deviceSkin;
       if (skinIndex != SkinComboNoneIndex &&  skinIndex != m_browseSkinIndex)
           deviceSkin = m_ui.m_skinCombo->itemData(skinIndex).toString();

       return PreviewConfiguration(style, applicationStyleSheet, deviceSkin);
   */

   return PreviewConfiguration(style, applicationStyleSheet, QString());
}

void PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate::setPreviewConfiguration(const PreviewConfiguration &pc)
{
   int styleIndex = m_ui.m_styleCombo->findText(pc.style());
   if (styleIndex == -1) {
      styleIndex = m_ui.m_styleCombo->findText(m_defaultStyle);
   }
   m_ui.m_styleCombo->setCurrentIndex(styleIndex);
   m_ui.m_appStyleSheetLineEdit->setText(pc.applicationStyleSheet());

   /*
       // find skin by file name. 0 is "none"
       const QString deviceSkin = pc.deviceSkin();
       int skinIndex = deviceSkin.isEmpty() ? 0 : m_ui.m_skinCombo->findData(QVariant(deviceSkin));
       if (skinIndex == -1) {
           qWarning() << "Unable to find skin " << deviceSkin;
           skinIndex = 0;
       }
       m_ui.m_skinCombo->setCurrentIndex(skinIndex);
   */

}

void  PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate::slotEditAppStyleSheet()
{
   StyleSheetEditorDialog dlg(m_core, m_parent, nullptr, StyleSheetEditorDialog::ModeGlobal);
   dlg.setText(m_ui.m_appStyleSheetLineEdit->text());

   if (dlg.exec() == QDialog::Accepted) {
      m_ui.m_appStyleSheetLineEdit->setText(dlg.text());
   }
}

/*
void  PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate::slotDeleteSkinEntry()
{
    const int index = m_ui.m_skinCombo->currentIndex();

    if (canRemoveSkin(index)) {
        m_ui.m_skinCombo->setCurrentIndex(SkinComboNoneIndex);
        m_ui.m_skinCombo->removeItem(index);
        m_browseSkinIndex--;
    }
}

void  PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate::slotSkinChanged(int index)
{
    if (index == m_browseSkinIndex) {
        m_ui.m_skinCombo->setCurrentIndex(browseSkin());
    } else {
        m_lastSkinIndex = index;
        m_ui.m_skinRemoveButton->setEnabled(canRemoveSkin(index));
        m_ui.m_skinCombo->setToolTip(index != SkinComboNoneIndex ? m_ui.m_skinCombo->itemData(index).toString() : QString());
    }

}
*/

void PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate::retrieveSettings()
{
   QDesignerSharedSettings settings(m_core);
   m_parent->setChecked(settings.isCustomPreviewConfigurationEnabled());
   setPreviewConfiguration(settings.customPreviewConfiguration());

   //    addUserSkins(settings.userDeviceSkins());
}

void PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate::storeSettings() const
{
   QDesignerSharedSettings settings(m_core);
   settings.setCustomPreviewConfigurationEnabled(m_parent->isChecked());
   settings.setCustomPreviewConfiguration(previewConfiguration());

   //    settings.setUserDeviceSkins(userSkins());
}

/*

int PreviewConfigurationWidget::PreviewConfigurationWidgetPrivate::browseSkin()
{
    QFileDialog dlg(m_parent);
    dlg.setFileMode(QFileDialog::DirectoryOnly);

    const QString title = tr("Load Custom Device Skin");
    dlg.setWindowTitle(title);
    dlg.setNameFilter(tr("All QVFB Skins (*.%1)").formatArg(QLatin1String(skinExtensionC)));

    int rc = m_lastSkinIndex;

    do {
        if (! dlg.exec()) {
            break;
        }

        const QStringList directories =  dlg.selectedFiles();
        if (directories.size() != 1)
            break;

        // check: 1) name already there
        const QString directory = directories.front();
        const QString name = QFileInfo(directory).baseName();
        const int existingIndex = m_ui.m_skinCombo->findText(name);

        if (existingIndex != -1 && existingIndex != SkinComboNoneIndex &&  existingIndex != m_browseSkinIndex) {
            const QString msgTitle = tr("%1 - Duplicate Skin").formatArg(title);
            const QString msg = tr("The skin '%1' already exists.").formatArg(name);
            QMessageBox::information(m_parent, msgTitle, msg);
            break;
        }

        // check: 2) can read
        DeviceSkinParameters parameters;
        QString readError;

        if (parameters.read(directory, DeviceSkinParameters::ReadSizeOnly, &readError)) {
            const QString name = QFileInfo(directory).baseName();
            m_ui.m_skinCombo->insertItem(m_browseSkinIndex, name, QVariant(directory));
            rc = m_browseSkinIndex++;

            break;

        } else {
            const QString msgTitle = tr("%1 - Error").formatArg(title);
            const QString msg = tr("%1 is not a valid skin directory:\n%2").formatArg(directory).formatArg(readError);
            QMessageBox::warning (m_parent, msgTitle, msg);
        }

    } while (true);

    return rc;
}

*/

// ------------- PreviewConfigurationWidget
PreviewConfigurationWidget::PreviewConfigurationWidget(QDesignerFormEditorInterface *core, QWidget *parent)
   : QGroupBox(parent),m_impl(new PreviewConfigurationWidgetPrivate(core, this))
{
   connect(m_impl->appStyleSheetChangeButton(), &QAbstractButton::clicked,
      this, &PreviewConfigurationWidget::slotEditAppStyleSheet);

   /*
       connect(m_impl->skinRemoveButton(), &QAbstractButton::clicked,
               this, &PreviewConfigurationWidget::slotDeleteSkinEntry);

       connect(m_impl->skinCombo(), cs_mp_cast<int>(&QComboBox::currentIndexChanged),
               this, &PreviewConfigurationWidget::slotSkinChanged);
   */

   m_impl->retrieveSettings();
}

PreviewConfigurationWidget::~PreviewConfigurationWidget()
{
   delete m_impl;
}

void PreviewConfigurationWidget::saveState()
{
   m_impl->storeSettings();
}

void PreviewConfigurationWidget::slotEditAppStyleSheet()
{
   m_impl->slotEditAppStyleSheet();
}

/*

void PreviewConfigurationWidget::slotDeleteSkinEntry()
{
    m_impl->slotDeleteSkinEntry();
}

void PreviewConfigurationWidget::slotSkinChanged(int index)
{
    m_impl->slotSkinChanged(index);
}

*/

}

