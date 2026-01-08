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

#ifndef PREVIEW_WIDGET_H
#define PREVIEW_WIDGET_H

#include <ui_preview_widget.h>

namespace qdesigner_internal {

class PreviewWidget: public QWidget
{
   CS_OBJECT(PreviewWidget)

 public:
   explicit PreviewWidget(QWidget *parent);
   virtual ~PreviewWidget();

 private:
   Ui::PreviewWidget ui;
};

}   // end namespace qdesigner_internal

#endif