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

#ifndef TEXTEDIT_FINDWIDGET_H
#define TEXTEDIT_FINDWIDGET_H

#include <abstract_findwidget.h>

class QTextEdit;

class TextEditFindWidget : public AbstractFindWidget
{
   CS_OBJECT(TextEditFindWidget)

 public:
   explicit TextEditFindWidget(FindFlags flags = FindFlags(), QWidget *parent = nullptr);

   QTextEdit *textEdit() const {
      return m_textEdit;
   }

   void setTextEdit(QTextEdit *textEdit);

 protected:
   void deactivate() override;
   void find(const QString &textToFind, bool skipCurrent, bool backward, bool *found, bool *wrapped) override;

 private:
   QTextEdit *m_textEdit;
};

#endif  // TEXTEDITFINDWIDGET_H
