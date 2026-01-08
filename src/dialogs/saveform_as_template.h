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

#ifndef SAVEFORM_AS_TEMPLATE_H
#define SAVEFORM_AS_TEMPLATE_H

#include <ui_template_saveform_as.h>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class SaveFormAsTemplate: public QDialog
{
   CS_OBJECT(SaveFormAsTemplate)

 public:
   explicit SaveFormAsTemplate(QDesignerFormEditorInterface *m_core,
      QDesignerFormWindowInterface *formWindow, QWidget *parent = nullptr);

   virtual ~SaveFormAsTemplate();

 private:
   CS_SLOT_1(Private, void accept() override)
   CS_SLOT_2(accept)

   CS_SLOT_1(Private, void updateOKButton(const QString &str))
   CS_SLOT_2(updateOKButton)

   CS_SLOT_1(Private, void checkToAddPath(int itemIndex))
   CS_SLOT_2(checkToAddPath)

   static QString chooseTemplatePath(QWidget *parent);

   Ui::SaveFormAsTemplate ui;
   QDesignerFormEditorInterface *m_core;
   QDesignerFormWindowInterface *m_formWindow;
   int m_addPathIndex;
};

#endif
