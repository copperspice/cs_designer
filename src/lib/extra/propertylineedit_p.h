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

#ifndef PROPERTYLINEEDIT_H
#define PROPERTYLINEEDIT_H

#include <QLineEdit>

namespace qdesigner_internal {

// A line edit with a special context menu allowing for adding (escaped) new  lines
class PropertyLineEdit : public QLineEdit
{
   CS_OBJECT(PropertyLineEdit)

 public:
   explicit PropertyLineEdit(QWidget *parent);
   void setWantNewLine(bool nl) {
      m_wantNewLine = nl;
   }

   bool wantNewLine() const {
      return m_wantNewLine;
   }

   bool event(QEvent *e) override;

 protected:
   void contextMenuEvent (QContextMenuEvent *event) override;

 private:
   CS_SLOT_1(Private, void insertNewLine())
   CS_SLOT_2(insertNewLine)

   void insertText(const QString &);
   bool m_wantNewLine;
};
}

#endif
