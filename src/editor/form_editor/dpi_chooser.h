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

#ifndef DPICHOOSER_H
#define DPICHOOSER_H

#include <QWidget>

class QSpinBox;
class QComboBox;

namespace qdesigner_internal {

struct DPI_Entry;

/* Let the user choose a DPI settings */
class DPI_Chooser : public QWidget
{
   Q_DISABLE_COPY(DPI_Chooser)
   CS_OBJECT(DPI_Chooser)

 public:
   explicit DPI_Chooser(QWidget *parent = nullptr);
   ~DPI_Chooser();

   void getDPI(int *dpiX, int *dpiY) const;
   void setDPI(int dpiX, int dpiY);

 private:
   CS_SLOT_1(Private, void syncSpinBoxes())
   CS_SLOT_2(syncSpinBoxes)

   void setUserDefinedValues(int dpiX, int dpiY);

   struct DPI_Entry *m_systemEntry;
   QComboBox *m_predefinedCombo;
   QSpinBox *m_dpiXSpinBox;
   QSpinBox *m_dpiYSpinBox;
};

}

#endif
