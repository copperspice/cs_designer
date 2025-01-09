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

#ifndef GRIDPANEL_H
#define GRIDPANEL_H

#include <QWidget>

namespace qdesigner_internal {

class Grid;

namespace Ui {
class GridPanel;
}

class  GridPanel : public QWidget
{
   CS_OBJECT(GridPanel)

 public:
   GridPanel(QWidget *parent = nullptr);
   ~GridPanel();

   void setTitle(const QString &title);

   void setGrid(const Grid &g);
   Grid grid() const;

   void setCheckable (bool c);
   bool isCheckable () const;

   bool isChecked () const;
   void setChecked(bool c);

   void setResetButtonVisible(bool v);

 private:
   CS_SLOT_1(Private, void reset())
   CS_SLOT_2(reset)

   Ui::GridPanel *m_ui;
};

} // qdesigner_internal

#endif // GRIDPANEL_H
