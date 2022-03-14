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

#ifndef LISTWIDGETEDITOR_H
#define LISTWIDGETEDITOR_H

#include <itemlist_editor.h>

#include <qdesigner_command_p.h>

class QDesignerFormWindowInterface;

#include <QWidget>

class QListWidget;
class QComboBox;

namespace qdesigner_internal {

class ListWidgetEditor: public QDialog
{
   CS_OBJECT(ListWidgetEditor)

 public:
   ListWidgetEditor(QDesignerFormWindowInterface *form, QWidget *parent);

   ListContents fillContentsFromListWidget(QListWidget *listWidget);
   ListContents fillContentsFromComboBox(QComboBox *comboBox);
   ListContents contents() const;

 private:
   ItemListEditor *m_itemsEditor;
};

}  // namespace qdesigner_internal

#endif
