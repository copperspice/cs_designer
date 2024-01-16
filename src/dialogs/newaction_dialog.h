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

#ifndef NEWACTIONDIALOG_P_H
#define NEWACTIONDIALOG_P_H

#include <designer_utils.h>

#include <QDialog>
#include <QKeySequence>

namespace qdesigner_internal {

namespace Ui {
class NewActionDialog;
}

class ActionEditor;

struct ActionData {

   enum ChangeMask {
      TextChanged = 0x1, NameChanged = 0x2, ToolTipChanged = 0x4,
      IconChanged = 0x8, CheckableChanged = 0x10, KeysequenceChanged = 0x20
   };

   ActionData();

   // Returns a combination of ChangeMask flags
   unsigned compare(const  ActionData &rhs) const;

   QString text;
   QString name;
   QString toolTip;
   PropertySheetIconValue icon;
   bool checkable;
   PropertySheetKeySequenceValue keysequence;
};

inline bool operator==(const ActionData &a1, const ActionData &a2)
{
   return a1.compare(a2) == 0u;
}

inline bool operator!=(const ActionData &a1, const ActionData &a2)
{
   return a1.compare(a2) != 0u;
}

class NewActionDialog: public QDialog
{
   CS_OBJECT(NewActionDialog)

 public:
   explicit NewActionDialog(ActionEditor *parent);
   virtual ~NewActionDialog();

   ActionData actionData() const;
   void setActionData(const ActionData &d);

   QString actionText() const;
   QString actionName() const;

 private:
   // slot methods
   void editActionText_textEdited(const QString &text);
   void editObjectName_textEdited(const QString &text);

   CS_SLOT_1(Private, void slotEditToolTip())
   CS_SLOT_2(slotEditToolTip)

   CS_SLOT_1(Private, void slotResetKeySequence())
   CS_SLOT_2(slotResetKeySequence)

   Ui::NewActionDialog *m_ui;
   ActionEditor *m_actionEditor;
   bool m_autoUpdateObjectName;

   void updateButtons();
};

} // namespace qdesigner_internal

#endif
