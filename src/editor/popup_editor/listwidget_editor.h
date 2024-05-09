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

#ifndef LISTWIDGETEDITOR_H
#define LISTWIDGETEDITOR_H

#include <designer_command.h>
#include <itemlist_editor.h>

#include <QWidget>

class QDesignerFormWindowInterface;

class QComboBox;
class QListWidget;

namespace qdesigner_internal {

class ListWidgetEditor: public QDialog
{
   CS_OBJECT(ListWidgetEditor)

 public:
   ListWidgetEditor(QDesignerFormWindowInterface *form, QWidget *parent);

   ListData fillContentsFromListWidget(QListWidget *listWidget);
   ListData fillContentsFromComboBox(QComboBox *comboBox);
   ListData contents() const;

 private:
   ItemListEditor *m_itemsEditor;
};

}   // end namespace qdesigner_internal

#endif
