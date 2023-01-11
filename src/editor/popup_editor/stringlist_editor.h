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

#ifndef STRINGLIST_EDITOR_H
#define STRINGLIST_EDITOR_H

#include <ui_edit_stringlist.h>

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
   // slots
   void newItemButton();
   void deleteItemButton();
   void upItemButton();
   void downItemButton();
   void valueChanged(const QString &text);
   void currentIndexChanged(const QModelIndex &current, const QModelIndex &previous);
   void currentValueChanged();

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
