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

#include <fontpanel.h>

#include <QComboBox>
#include <QLabel>
#include <QFormLayout>
#include <QSpacerItem>
#include <QFontComboBox>
#include <QTimer>
#include <QLineEdit>

FontPanel::FontPanel(QWidget *parentWidget)
   : QGroupBox(parentWidget), m_previewLineEdit(new QLineEdit), m_writingSystemComboBox(new QComboBox),
     m_familyComboBox(new QFontComboBox), m_styleComboBox(new QComboBox), m_pointSizeComboBox(new QComboBox),
     m_previewFontUpdateTimer(nullptr)
{
   setTitle(tr("Font"));

   QFormLayout *formLayout = new QFormLayout(this);

   // writing systems
   m_writingSystemComboBox->setEditable(false);

   QList<QFontDatabase::WritingSystem> writingSystems = m_fontDatabase.writingSystems();
   writingSystems.push_front(QFontDatabase::Any);

   for (QFontDatabase::WritingSystem ws : writingSystems) {
      m_writingSystemComboBox->addItem(QFontDatabase::writingSystemName(ws), QVariant(ws));
   }

   formLayout->addRow(tr("&Writing system"), m_writingSystemComboBox);
   formLayout->addRow(tr("&Family"), m_familyComboBox);

   m_styleComboBox->setEditable(false);
   formLayout->addRow(tr("&Style"), m_styleComboBox);

   m_pointSizeComboBox->setEditable(false);
   formLayout->addRow(tr("&Point size"), m_pointSizeComboBox);

   connect(m_writingSystemComboBox, cs_mp_cast<int>(&QComboBox::currentIndexChanged),
         this, &FontPanel::slotWritingSystemChanged);

   connect(m_familyComboBox, &QFontComboBox::currentFontChanged,
         this, &FontPanel::slotFamilyChanged);

   connect(m_styleComboBox, cs_mp_cast<int>(&QComboBox::currentIndexChanged),
         this, &FontPanel::slotStyleChanged);

   connect(m_pointSizeComboBox, cs_mp_cast<int>(&QComboBox::currentIndexChanged),
         this, &FontPanel::slotPointSizeChanged);

   m_previewLineEdit->setReadOnly(true);
   formLayout->addRow(m_previewLineEdit);

   setWritingSystem(QFontDatabase::Any);
}

QFont FontPanel::selectedFont() const
{
   QFont rc = m_familyComboBox->currentFont();
   const QString family = rc.family();
   rc.setPointSize(pointSize());
   const QString styleDescription = styleString();

   if (styleDescription.contains("Italic")) {
      rc.setStyle(QFont::StyleItalic);
   } else if (styleDescription.contains("Oblique")) {
      rc.setStyle(QFont::StyleOblique);
   } else {
      rc.setStyle(QFont::StyleNormal);
   }

   rc.setBold(m_fontDatabase.bold(family, styleDescription));

   // Weight < 0 asserts
   const int weight = m_fontDatabase.weight(family, styleDescription);
   if (weight >= 0) {
      rc.setWeight(weight);
   }

   return rc;
}

void FontPanel::setSelectedFont(const QFont &f)
{
   m_familyComboBox->setCurrentFont(f);

   if (m_familyComboBox->currentIndex() < 0) {
      // family not in writing system - find the corresponding one?
      QList<QFontDatabase::WritingSystem> familyWritingSystems = m_fontDatabase.writingSystems(f.family());
      if (familyWritingSystems.empty()) {
         return;
      }

      setWritingSystem(familyWritingSystems.front());
      m_familyComboBox->setCurrentFont(f);
   }

   updateFamily(family());

   const int pointSizeIndex = closestPointSizeIndex(f.pointSize());
   m_pointSizeComboBox->setCurrentIndex( pointSizeIndex);

   const QString styleString = m_fontDatabase.styleString(f);
   const int styleIndex = m_styleComboBox->findText(styleString);
   m_styleComboBox->setCurrentIndex(styleIndex);
   slotUpdatePreviewFont();
}

QFontDatabase::WritingSystem FontPanel::writingSystem() const
{
   const int currentIndex = m_writingSystemComboBox->currentIndex();
   if ( currentIndex == -1) {
      return QFontDatabase::Latin;
   }

   return static_cast<QFontDatabase::WritingSystem>(m_writingSystemComboBox->itemData(currentIndex).toInt());
}

QString FontPanel::family() const
{
   const int currentIndex = m_familyComboBox->currentIndex();
   return currentIndex != -1 ?  m_familyComboBox->currentFont().family() : QString();
}

int FontPanel::pointSize() const
{
   const int currentIndex = m_pointSizeComboBox->currentIndex();
   return currentIndex != -1 ? m_pointSizeComboBox->itemData(currentIndex).toInt() : 9;
}

QString FontPanel::styleString() const
{
   const int currentIndex = m_styleComboBox->currentIndex();
   return currentIndex != -1 ? m_styleComboBox->itemText(currentIndex) : QString();
}

void FontPanel::setWritingSystem(QFontDatabase::WritingSystem ws)
{
   m_writingSystemComboBox->setCurrentIndex(m_writingSystemComboBox->findData(QVariant(ws)));
   updateWritingSystem(ws);
}

void FontPanel::slotWritingSystemChanged(int)
{
   updateWritingSystem(writingSystem());
   delayedPreviewFontUpdate();
}

void FontPanel::slotFamilyChanged(const QFont &)
{
   updateFamily(family());
   delayedPreviewFontUpdate();
}

void FontPanel::slotStyleChanged(int)
{
   updatePointSizes(family(), styleString());
   delayedPreviewFontUpdate();
}

void FontPanel::slotPointSizeChanged(int)
{
   delayedPreviewFontUpdate();
}

void FontPanel::updateWritingSystem(QFontDatabase::WritingSystem ws)
{

   m_previewLineEdit->setText(QFontDatabase::writingSystemSample(ws));
   m_familyComboBox->setWritingSystem (ws);

   // Current font not in WS ... set index 0.
   if (m_familyComboBox->currentIndex() < 0) {
      m_familyComboBox->setCurrentIndex(0);
      updateFamily(family());
   }
}

void FontPanel::updateFamily(const QString &family)
{
   // Update styles and trigger update of point sizes.
   // Try to maintain selection or select normal
   const QString oldStyleString = styleString();

   const QStringList styles = m_fontDatabase.styles(family);
   const bool hasStyles = !styles.empty();

   m_styleComboBox->setCurrentIndex(-1);
   m_styleComboBox->clear();
   m_styleComboBox->setEnabled(hasStyles);

   int normalIndex = -1;
   const QString normalStyle = "Normal";

   if (hasStyles) {
      for (const QString &style : styles) {
         // try to maintain selection or select 'normal' preferably
         const int newIndex = m_styleComboBox->count();
         m_styleComboBox->addItem(style);

         if (oldStyleString == style) {
            m_styleComboBox->setCurrentIndex(newIndex);
         } else {
            if (oldStyleString ==  normalStyle) {
               normalIndex = newIndex;
            }
         }
      }

      if (m_styleComboBox->currentIndex() == -1 && normalIndex != -1) {
         m_styleComboBox->setCurrentIndex(normalIndex);
      }
   }

   updatePointSizes(family, styleString());
}

int FontPanel::closestPointSizeIndex(int desiredPointSize) const
{
   //  try to maintain selection or select closest.
   int closestIndex = -1;
   int closestAbsError = 0xFFFF;

   const int pointSizeCount = m_pointSizeComboBox->count();
   for (int i = 0; i < pointSizeCount; i++) {
      const int itemPointSize = m_pointSizeComboBox->itemData(i).toInt();
      const int absError = qAbs(desiredPointSize - itemPointSize);

      if (absError < closestAbsError) {
         closestIndex  = i;
         closestAbsError = absError;
         if (closestAbsError == 0) {
            break;
         }
      } else {    // past optimum
         if (absError > closestAbsError) {
            break;
         }
      }
   }
   return closestIndex;
}


void FontPanel::updatePointSizes(const QString &family, const QString &styleString)
{
   const int oldPointSize = pointSize();

   QList<int> pointSizes =  m_fontDatabase.pointSizes(family, styleString);
   if (pointSizes.empty()) {
      pointSizes = QFontDatabase::standardSizes();
   }

   const bool hasSizes = !pointSizes.empty();
   m_pointSizeComboBox->clear();
   m_pointSizeComboBox->setEnabled(hasSizes);
   m_pointSizeComboBox->setCurrentIndex(-1);

   //  try to maintain selection or select closest.
   if (hasSizes) {
      QString n;

      for (int pointSize : pointSizes) {
         n = QString::number(pointSize);
         m_pointSizeComboBox->addItem(n, QVariant(pointSize));
      }

      const int closestIndex = closestPointSizeIndex(oldPointSize);

      if (closestIndex != -1) {
         m_pointSizeComboBox->setCurrentIndex(closestIndex);
      }
   }
}

void FontPanel::slotUpdatePreviewFont()
{
   m_previewLineEdit->setFont(selectedFont());
}

void FontPanel::delayedPreviewFontUpdate()
{
   if (!m_previewFontUpdateTimer) {
      m_previewFontUpdateTimer = new QTimer(this);

      connect(m_previewFontUpdateTimer, &QTimer::timeout, this, &FontPanel::slotUpdatePreviewFont);

      m_previewFontUpdateTimer->setInterval(0);
      m_previewFontUpdateTimer->setSingleShot(true);
   }

   if (m_previewFontUpdateTimer->isActive()) {
      return;
   }

   m_previewFontUpdateTimer->start();
}

