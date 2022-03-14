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

#ifndef STRINGLISTEDITOR_H
#define STRINGLISTEDITOR_H

#include <ui_stringlist_editor.h>

#include <QStringList>

class QStringListModel;

namespace qdesigner_internal {

class StringListEditor : public QDialog, private Ui::Dialog
{
   CS_OBJECT(StringListEditor)

 public:
   ~StringListEditor();
   void setStringList(const QStringList &stringList);
   QStringList stringList() const;

   static QStringList getStringList(
      QWidget *parent, const QStringList &init = QStringList(), int *result = nullptr);

 private:
   CS_SLOT_1(Private, void on_upButton_clicked())
   CS_SLOT_2(on_upButton_clicked)
   CS_SLOT_1(Private, void on_downButton_clicked())
   CS_SLOT_2(on_downButton_clicked)
   CS_SLOT_1(Private, void on_newButton_clicked())
   CS_SLOT_2(on_newButton_clicked)
   CS_SLOT_1(Private, void on_deleteButton_clicked())
   CS_SLOT_2(on_deleteButton_clicked)
   CS_SLOT_1(Private, void on_valueEdit_textEdited(const QString &text))
   CS_SLOT_2(on_valueEdit_textEdited)
   CS_SLOT_1(Private, void currentIndexChanged(const QModelIndex &current, const QModelIndex &previous))
   CS_SLOT_2(currentIndexChanged)
   CS_SLOT_1(Private, void currentValueChanged())
   CS_SLOT_2(currentValueChanged)

   StringListEditor(QWidget *parent = nullptr);
   void updateUi();
   int currentIndex() const;
   void setCurrentIndex(int index);
   int count() const;
   QString stringAt(int index) const;
   void setStringAt(int index, const QString &value);
   void removeString(int index);
   void insertString(int index, const QString &value);
   void editString(int index);

   QStringListModel *m_model;
};

}   // end namespace qdesigner_internal

#endif
