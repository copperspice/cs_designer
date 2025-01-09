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

#include <abstract_dialoggui.h>

QDesignerDialogGuiInterface::QDesignerDialogGuiInterface()
{
}

QDesignerDialogGuiInterface::~QDesignerDialogGuiInterface()
{
}

QString QDesignerDialogGuiInterface::getOpenImageFileName(QWidget *parent, const QString &caption,
   const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::FileDialogOptions options)
{
   return getOpenFileName(parent, caption, dir, filter, selectedFilter, options);
}

QStringList QDesignerDialogGuiInterface::getOpenImageFileNames(QWidget *parent, const QString &caption,
   const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::FileDialogOptions options)
{
   return getOpenFileNames(parent, caption, dir, filter, selectedFilter, options);
}

