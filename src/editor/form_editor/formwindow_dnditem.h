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

#ifndef FORMWINDOW_DNDITEM_H
#define FORMWINDOW_DNDITEM_H

#include <designer_dnditem.h>

namespace qdesigner_internal {

class FormWindow;

class FormWindowDnDItem : public QDesignerDnDItem
{
 public:
   FormWindowDnDItem(QDesignerDnDItemInterface::DropType type, FormWindow *form,
      QWidget *widget, const QPoint &global_mouse_pos);
   DomUI *domUi() const override;
};

}   // end namespace - qdesigner_internal

#endif
