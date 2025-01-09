/***********************************************************************
*
* Copyright (c) 2021-2025 Barbara Geller
* Copyright (c) 2021-2025 Ansel Sermersheim
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

#include <stringlist_editor.h>

#include <iconloader_p.h>

#include <QPushButton>
#include <QStringListModel>

using namespace qdesigner_internal;

StringListEditor::StringListEditor(QWidget *parent)
   : QDialog(parent), m_model(new QStringListModel(this))
{
   setupUi(this);

   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
   listView->setModel(m_model);

   QIcon plusIcon  = createIconSet(QString::fromUtf8("plus.png"));
   QIcon minusIcon = createIconSet(QString::fromUtf8("minus.png"));
   QIcon upIcon    = createIconSet(QString::fromUtf8("up.png"));
   QIcon downIcon  = createIconSet(QString::fromUtf8("down.png"));

   newButton->setIcon(plusIcon);
   deleteButton->setIcon(minusIcon);
   upButton->setIcon(upIcon);
   downButton->setIcon(downIcon);

   connect(newButton, &QPushButton::clicked,
         this, &StringListEditor::newItemButton);

   connect(deleteButton, &QPushButton::clicked,
         this, &StringListEditor::deleteItemButton);

   connect(upButton, &QPushButton::clicked,
         this, &StringListEditor::upItemButton);

   connect(downButton, &QPushButton::clicked,
         this, &StringListEditor::downItemButton);

   connect(valueEdit, &QLineEdit::textEdited,
         this, &StringListEditor::valueChanged);

   connect(listView->selectionModel(), &QItemSelectionModel::currentChanged,
         this, &StringListEditor::currentIndexChanged);

   connect(listView->itemDelegate(), &QAbstractItemDelegate::closeEditor,
         this, &StringListEditor::currentValueChanged);

   updateUi();
}

StringListEditor::~StringListEditor()
{
}

QStringList StringListEditor::getStringList(QWidget *parent, const QStringList &init, int *result)
{
   StringListEditor dlg(parent);
   dlg.setStringList(init);

   int res = dlg.exec();

   if (result) {
      *result = res;
   }
   return (res == QDialog::Accepted) ? dlg.stringList() : init;
}

void StringListEditor::setStringList(const QStringList &stringList)
{
   m_model->setStringList(stringList);
   updateUi();
}

QStringList StringListEditor::stringList() const
{
   return m_model->stringList();
}

void StringListEditor::currentIndexChanged(const QModelIndex &current, const QModelIndex &previous)
{
   (void) previous;
   setCurrentIndex(current.row());
   updateUi();
}

void StringListEditor::currentValueChanged()
{
   setCurrentIndex(currentIndex());
   updateUi();
}

void StringListEditor::newItemButton()
{
   int to = currentIndex();
   if (to == -1) {
      to = count() - 1;
   }

   ++to;
   insertString(to, QString());
   setCurrentIndex(to);
   updateUi();
   editString(to);
}

void StringListEditor::deleteItemButton()
{
   removeString(currentIndex());
   setCurrentIndex(currentIndex());
   updateUi();
}

void StringListEditor::upItemButton()
{
   int from = currentIndex();
   int to   = currentIndex() - 1;

   QString value = stringAt(from);

   removeString(from);
   insertString(to, value);
   setCurrentIndex(to);
   updateUi();
}

void StringListEditor::downItemButton()
{
   int from = currentIndex();
   int to = currentIndex() + 1;

   QString value = stringAt(from);

   removeString(from);
   insertString(to, value);
   setCurrentIndex(to);
   updateUi();
}

void StringListEditor::valueChanged(const QString &text)
{
   setStringAt(currentIndex(), text);
}

void StringListEditor::updateUi()
{
   deleteButton->setEnabled(currentIndex() != -1);

   upButton->setEnabled((count() > 1) && (currentIndex() > 0));
   downButton->setEnabled((count() > 1) && (currentIndex() >= 0) && (currentIndex() < (count() - 1)));

   valueEdit->setEnabled(currentIndex() != -1);
}

int StringListEditor::currentIndex() const
{
   return listView->currentIndex().row();
}

void StringListEditor::setCurrentIndex(int index)
{
   QModelIndex modelIndex = m_model->index(index, 0);
   if (listView->currentIndex() != modelIndex) {
      listView->setCurrentIndex(modelIndex);
   }
   valueEdit->setText(stringAt(index));
}

int StringListEditor::count() const
{
   return m_model->rowCount();
}

QString StringListEditor::stringAt(int index) const
{
   QVariant tmp = m_model->data(m_model->index(index, 0), Qt::DisplayRole);
   return tmp.value<QString>();
}

void StringListEditor::setStringAt(int index, const QString &value)
{
   m_model->setData(m_model->index(index, 0), value);
}

void StringListEditor::removeString(int index)
{
   m_model->removeRows(index, 1);
}

void StringListEditor::insertString(int index, const QString &value)
{
   m_model->insertRows(index, 1);
   m_model->setData(m_model->index(index, 0), value);
}

void StringListEditor::editString(int index)
{
   listView->edit(m_model->index(index, 0));
}

