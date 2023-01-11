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

#ifndef FONTPANEL_H
#define FONTPANEL_H

#include <QGroupBox>
#include <QFont>
#include <QFontDatabase>

class QComboBox;
class QFontComboBox;
class QTimer;
class QLineEdit;

class FontPanel: public QGroupBox
{
   CS_OBJECT(FontPanel)

 public:
   FontPanel(QWidget *parentWidget = nullptr);

   QFont selectedFont() const;
   void setSelectedFont(const QFont &);

   QFontDatabase::WritingSystem writingSystem() const;
   void setWritingSystem(QFontDatabase::WritingSystem ws);

 private:
   CS_SLOT_1(Private, void slotWritingSystemChanged(int un_named_arg1))
   CS_SLOT_2(slotWritingSystemChanged)

   CS_SLOT_1(Private, void slotFamilyChanged(const QFont &un_named_arg1))
   CS_SLOT_2(slotFamilyChanged)

   CS_SLOT_1(Private, void slotStyleChanged(int un_named_arg1))
   CS_SLOT_2(slotStyleChanged)

   CS_SLOT_1(Private, void slotPointSizeChanged(int un_named_arg1))
   CS_SLOT_2(slotPointSizeChanged)

   CS_SLOT_1(Private, void slotUpdatePreviewFont())
   CS_SLOT_2(slotUpdatePreviewFont)

   QString family() const;
   QString styleString() const;
   int pointSize() const;
   int closestPointSizeIndex(int ps) const;

   void updateWritingSystem(QFontDatabase::WritingSystem ws);
   void updateFamily(const QString &family);
   void updatePointSizes(const QString &family, const QString &style);
   void delayedPreviewFontUpdate();

   QFontDatabase m_fontDatabase;
   QLineEdit *m_previewLineEdit;
   QComboBox *m_writingSystemComboBox;
   QFontComboBox *m_familyComboBox;
   QComboBox *m_styleComboBox;
   QComboBox *m_pointSizeComboBox;
   QTimer *m_previewFontUpdateTimer;
};

#endif
