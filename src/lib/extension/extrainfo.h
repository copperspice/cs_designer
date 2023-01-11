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

#ifndef EXTRAINFO_H
#define EXTRAINFO_H

#include <extension.h>

class DomWidget;
class DomUI;
class QWidget;

class QDesignerFormEditorInterface;

class QDesignerExtraInfoExtension
{
 public:
   virtual ~QDesignerExtraInfoExtension() {}

   virtual QDesignerFormEditorInterface *core() const = 0;
   virtual QWidget *widget() const = 0;

   virtual bool saveUiExtraInfo(DomUI *ui) = 0;
   virtual bool loadUiExtraInfo(DomUI *ui) = 0;

   virtual bool saveWidgetExtraInfo(DomWidget *ui_widget) = 0;
   virtual bool loadWidgetExtraInfo(DomWidget *ui_widget) = 0;

   QString workingDirectory() const;
   void setWorkingDirectory(const QString &workingDirectory);

 private:
   QString m_workingDirectory;
};

CS_DECLARE_EXTENSION_INTERFACE(QDesignerExtraInfoExtension, "copperspice.com.CS.Designer.ExtraInfo.2")


#endif // EXTRAINFO_H
