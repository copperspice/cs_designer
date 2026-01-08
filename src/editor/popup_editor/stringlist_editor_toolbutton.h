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

#ifndef STRINGLIST_TOOLBUTTON_H
#define STRINGLIST_TOOLBUTTON_H

#include <QStringList>
#include <QToolButton>

namespace qdesigner_internal {

class StringListEditorButton: public QToolButton
{
   CS_OBJECT(StringListEditorButton)

 public:
   explicit StringListEditorButton(const QStringList &stringList, QWidget *parent = nullptr);
   virtual ~StringListEditorButton();

   QStringList stringList() const {
      return m_stringList;
   }

   CS_SIGNAL_1(Public, void stringListChanged(const QStringList &stringList))
   CS_SIGNAL_2(stringListChanged, stringList)

   CS_SLOT_1(Public, void setStringList(const QStringList &stringList))
   CS_SLOT_2(setStringList)

 private:
   CS_SLOT_1(Private, void showStringListEditor())
   CS_SLOT_2(showStringListEditor)

   QStringList m_stringList;
};

}   // end namespace qdesigner_internal

#endif
