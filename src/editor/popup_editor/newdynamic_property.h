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

#ifndef NEWDYNAMIC_PROPERTY_H
#define NEWDYNAMIC_PROPERTY_H

#include <QDialog>
#include <QVariant>

class QAbstractButton;
class QDesignerDialogGuiInterface;

namespace qdesigner_internal {

namespace Ui {
class NewDynamicPropertyDialog;
}

class NewDynamicPropertyDialog: public QDialog
{
   CS_OBJECT(NewDynamicPropertyDialog)

 public:
   explicit NewDynamicPropertyDialog(QDesignerDialogGuiInterface *dialogGui, QWidget *parent = nullptr);
   ~NewDynamicPropertyDialog();

   void setReservedNames(const QStringList &names);
   void setPropertyType(QVariant::Type t);

   QString propertyName() const;
   QVariant propertyValue() const;

 private:
   // slot
   void slot_buttonBox_clicked(QAbstractButton *btn);

   CS_SLOT_1(Private, void nameChanged(const QString &un_named_arg1))
   CS_SLOT_2(nameChanged)

   bool validatePropertyName(const QString &name);
   void setOkButtonEnabled(bool e);
   void information(const QString &message);

   QDesignerDialogGuiInterface *m_dialogGui;
   Ui::NewDynamicPropertyDialog *m_ui;
   QStringList m_reservedNames;
};

}  // namespace qdesigner_internal

#endif // NEWDYNAMICPROPERTYDIALOG_P_H
