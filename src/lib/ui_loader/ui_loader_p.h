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

#ifndef QUILOADER_P_H
#define QUILOADER_P_H

#include <QString>

class QDataStream;

// This file is here for use by the form preview in Linguist. If you change anything
// here or in the code which uses it, remember to adapt Linguist accordingly.

#define PROP_GENERIC_PREFIX   "_q_notr_"
#define PROP_TOOLITEMTEXT     "_q_toolItemText_notr"
#define PROP_TOOLITEMTOOLTIP  "_q_toolItemToolTip_notr"
#define PROP_TABPAGETEXT      "_q_tabPageText_notr"
#define PROP_TABPAGETOOLTIP   "_q_tabPageToolTip_notr"
#define PROP_TABPAGEWHATSTHIS "_q_tabPageWhatsThis_notr"

class QUiTranslatableStringValue
{
 public:
   QString value() const {
      return m_value;
   }

   void setValue(const QString &value) {
      m_value = value;
   }

   QString comment() const {
      return m_comment;
   }

   void setComment(const QString &comment) {
      m_comment = comment;
   }

 private:
   friend QDataStream &operator>>(QDataStream &in, QUiTranslatableStringValue &s);

   QString m_value;
   QString m_comment;
};

struct QUiItemRolePair {
   int realRole;
   int shadowRole;
};

extern const QUiItemRolePair qUiItemRoles[];

CS_DECLARE_METATYPE(QUiTranslatableStringValue)

#endif
