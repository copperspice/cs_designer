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

#include <dpi_chooser.h>

#include <deviceprofile_p.h>

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

constexpr const int MIN_DPI = 50;
constexpr const int MAX_DPI = 400;

namespace qdesigner_internal {

// Entry struct for predefined values
struct DPI_Entry {
   int dpiX;
   int dpiY;
   const char *description;
};

const struct DPI_Entry dpiEntries[] = {
   // Embedded device standard screen resolution
   {  96,   96, cs_mark_tr("DPI_Chooser", "Standard (96 x 96)") },

   // Embedded device screen resolution
   { 179,  185, cs_mark_tr("DPI_Chooser", "Greenphone (179 x 185)") },

   // Embedded device high definition screen resolution
   { 192,  192, cs_mark_tr("DPI_Chooser", "High (192 x 192)") }
};

}   // end namespace qdesigner_internal

CS_DECLARE_METATYPE(const struct qdesigner_internal::DPI_Entry *);

namespace qdesigner_internal {

DPI_Chooser::DPI_Chooser(QWidget *parent)
   : QWidget(parent), m_systemEntry(new DPI_Entry), m_predefinedCombo(new QComboBox),
     m_dpiXSpinBox(new QSpinBox), m_dpiYSpinBox(new QSpinBox)
{
   // Predefined settings: System
   DeviceProfile::systemResolution(&(m_systemEntry->dpiX), &(m_systemEntry->dpiY));
   m_systemEntry->description = nullptr;
   const struct DPI_Entry *systemEntry = m_systemEntry;

   // System resolution
   m_predefinedCombo->addItem(tr("System (%1 x %2)").formatArg(m_systemEntry->dpiX).formatArg(m_systemEntry->dpiY),
      QVariant::fromValue(systemEntry));

   // Devices. Exclude the system values as not to duplicate the entries
   const int predefinedCount = sizeof(dpiEntries) / sizeof(DPI_Entry);
   const struct DPI_Entry *ecend = dpiEntries + predefinedCount;

   for (const struct DPI_Entry *it = dpiEntries; it < ecend; ++it) {
      if (it->dpiX != m_systemEntry->dpiX || it->dpiY != m_systemEntry->dpiY) {
         m_predefinedCombo->addItem(tr(it->description), QVariant::fromValue(it));
      }
   }
   m_predefinedCombo->addItem(tr("User defined"));

   setFocusProxy(m_predefinedCombo);
   m_predefinedCombo->setEditable(false);
   m_predefinedCombo->setCurrentIndex(0);

   connect(m_predefinedCombo, cs_mp_cast<int>(&QComboBox::currentIndexChanged),
         this, &DPI_Chooser::syncSpinBoxes);

   // top row with predefined settings
   QVBoxLayout *vBoxLayout = new QVBoxLayout;
   vBoxLayout->setMargin(0);
   vBoxLayout->addWidget(m_predefinedCombo);

   // Spin box row
   QHBoxLayout *hBoxLayout = new QHBoxLayout;
   hBoxLayout->setMargin(0);

   m_dpiXSpinBox->setMinimum(MIN_DPI);
   m_dpiXSpinBox->setMaximum(MAX_DPI);
   hBoxLayout->addWidget(m_dpiXSpinBox);

   // DPI X/Y separator
   hBoxLayout->addWidget(new QLabel(tr(" x ")));

   m_dpiYSpinBox->setMinimum(MIN_DPI);
   m_dpiYSpinBox->setMaximum(MAX_DPI);
   hBoxLayout->addWidget(m_dpiYSpinBox);

   hBoxLayout->addStretch();
   vBoxLayout->addLayout(hBoxLayout);
   setLayout(vBoxLayout);

   syncSpinBoxes();
}

DPI_Chooser::~DPI_Chooser()
{
   delete m_systemEntry;
}

void DPI_Chooser::getDPI(int *dpiX, int *dpiY) const
{
   *dpiX = m_dpiXSpinBox->value();
   *dpiY = m_dpiYSpinBox->value();
}

void DPI_Chooser::setDPI(int dpiX, int dpiY)
{
   // Default to system if it is something odd
   const bool valid = dpiX >= MIN_DPI && dpiX <= MAX_DPI &&  dpiY >= MIN_DPI && dpiY <= MAX_DPI;
   if (!valid) {
      m_predefinedCombo->setCurrentIndex(0);
      return;
   }
   // Try to find the values among the predefined settings
   const int count = m_predefinedCombo->count();
   int predefinedIndex = -1;

   for (int i = 0; i < count; i++) {
      const QVariant data = m_predefinedCombo->itemData(i);
      if (data.type() != QVariant::Invalid) {
         const struct DPI_Entry *entry = data.value<const struct DPI_Entry *>();
         if (entry->dpiX == dpiX && entry->dpiY == dpiY) {
            predefinedIndex = i;
            break;
         }
      }
   }
   if (predefinedIndex != -1) {
      m_predefinedCombo->setCurrentIndex(predefinedIndex); // triggers syncSpinBoxes()
   } else {
      setUserDefinedValues(dpiX, dpiY);
   }
}

void DPI_Chooser::setUserDefinedValues(int dpiX, int dpiY)
{
   const bool blocked = m_predefinedCombo->blockSignals(true);
   m_predefinedCombo->setCurrentIndex(m_predefinedCombo->count() - 1);
   m_predefinedCombo->blockSignals(blocked);

   m_dpiXSpinBox->setEnabled(true);
   m_dpiYSpinBox->setEnabled(true);
   m_dpiXSpinBox->setValue(dpiX);
   m_dpiYSpinBox->setValue(dpiY);
}

void DPI_Chooser::syncSpinBoxes()
{
   const int predefIdx = m_predefinedCombo->currentIndex();
   const QVariant data1 = m_predefinedCombo->itemData(predefIdx);

   // Predefined mode in which spin boxes are disabled or user defined?
   const bool userSetting = data1.type() == QVariant::Invalid;
   m_dpiXSpinBox->setEnabled(userSetting);
   m_dpiYSpinBox->setEnabled(userSetting);

   if (!userSetting) {
      const struct DPI_Entry *entry = data1.value<const struct DPI_Entry *>();
      m_dpiXSpinBox->setValue(entry->dpiX);
      m_dpiYSpinBox->setValue(entry->dpiY);
   }
}

}   // end namespace - qdesigner_internal
