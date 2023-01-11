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

#include <abstract_dialoggui.h>
#include <newdynamic_property.h>
#include <ui_edit_dynamicproperty.h>

#include <designer_propertysheet.h>

#include <QDateTime>
#include <QPushButton>

namespace qdesigner_internal {

NewDynamicPropertyDialog::NewDynamicPropertyDialog(QDesignerDialogGuiInterface *dialogGui, QWidget *parent)
   : QDialog(parent), m_dialogGui(dialogGui), m_ui(new Ui::NewDynamicPropertyDialog)
{
   m_ui->setupUi(this);
   connect(m_ui->m_lineEdit, &QLineEdit::textChanged, this, &NewDynamicPropertyDialog::nameChanged);

   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   m_ui->m_comboBox->addItem("String",      QVariant(QString{}));
   m_ui->m_comboBox->addItem("StringList",  QVariant(QStringList{}));
   m_ui->m_comboBox->addItem("Char",        QVariant(char{}));
   m_ui->m_comboBox->addItem("ByteArray",   QVariant(QByteArray{}));
   m_ui->m_comboBox->addItem("Url",         QVariant(QUrl{}));
   m_ui->m_comboBox->addItem("Bool",        QVariant(bool{}));
   m_ui->m_comboBox->addItem("Int",         QVariant(int{}));
   m_ui->m_comboBox->addItem("UInt",        QVariant(uint{}));
   m_ui->m_comboBox->addItem("LongLong",    QVariant(qint64{}));
   m_ui->m_comboBox->addItem("ULongLong",   QVariant(quint64{}));
   m_ui->m_comboBox->addItem("Double",      QVariant(double{}));
   m_ui->m_comboBox->addItem("Size",        QVariant(QSize{}));
   m_ui->m_comboBox->addItem("SizeF",       QVariant(QSizeF{}));
   m_ui->m_comboBox->addItem("Point",       QVariant(QPoint{}));
   m_ui->m_comboBox->addItem("PointF",      QVariant(QPointF{}));
   m_ui->m_comboBox->addItem("Rect",        QVariant(QRect{}));
   m_ui->m_comboBox->addItem("RectF",       QVariant(QRectF{}));
   m_ui->m_comboBox->addItem("Date",        QVariant(QDate{}));
   m_ui->m_comboBox->addItem("Time",        QVariant(QTime{}));
   m_ui->m_comboBox->addItem("DateTime",    QVariant(QDateTime{}));
   m_ui->m_comboBox->addItem("Font",        QVariant(QFont{}));
   m_ui->m_comboBox->addItem("Palette",     QVariant(QPalette{}));
   m_ui->m_comboBox->addItem("Color",       QVariant(QColor{}));
   m_ui->m_comboBox->addItem("Pixmap",      QVariant(QPixmap{}));
   m_ui->m_comboBox->addItem("Icon",        QVariant(QIcon{}));
   m_ui->m_comboBox->addItem("Cursor",      QVariant(QCursor{}));
   m_ui->m_comboBox->addItem("SizePolicy",  QVariant(QSizePolicy{}));
   m_ui->m_comboBox->addItem("KeySequence", QVariant(QKeySequence{}));

   m_ui->m_comboBox->setCurrentIndex(0);
   setOkButtonEnabled(false);

   connect(m_ui->m_buttonBox, &QDialogButtonBox::clicked,
         this, &NewDynamicPropertyDialog::slot_buttonBox_clicked);
}

void NewDynamicPropertyDialog::setOkButtonEnabled(bool e)
{
   m_ui->m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(e);
}

NewDynamicPropertyDialog::~NewDynamicPropertyDialog()
{
   delete m_ui;
}

void NewDynamicPropertyDialog::setReservedNames(const QStringList &names)
{
   m_reservedNames = names;
}

void NewDynamicPropertyDialog::setPropertyType(QVariant::Type t)
{
   const int index = m_ui->m_comboBox->findData(QVariant(t));

   if (index != -1) {
      m_ui->m_comboBox->setCurrentIndex(index);
   }
}

QString NewDynamicPropertyDialog::propertyName() const
{
   return m_ui->m_lineEdit->text();
}

QVariant NewDynamicPropertyDialog::propertyValue() const
{
   const int index = m_ui->m_comboBox->currentIndex();
   if (index == -1) {
      return QVariant();
   }
   return m_ui->m_comboBox->itemData(index);
}

void NewDynamicPropertyDialog::information(const QString &message)
{
   m_dialogGui->message(this, QDesignerDialogGuiInterface::PropertyEditorMessage, QMessageBox::Information, tr("Set Property Name"),
      message);
}

void NewDynamicPropertyDialog::nameChanged(const QString &s)
{
   setOkButtonEnabled(!s.isEmpty());
}

bool NewDynamicPropertyDialog::validatePropertyName(const QString &name)
{
   if (m_reservedNames.contains(name)) {
      information(tr("The current object already has a property named '%1'.\nPlease select another unique name.").formatArg(name));
      return false;
   }

   if (! QDesignerPropertySheet::internalDynamicPropertiesEnabled() && name.startsWith("_q_")) {
      information(tr("The '_q_' prefix is reserved, please select another name."));
      return false;
   }

   return true;
}

void NewDynamicPropertyDialog::slot_buttonBox_clicked(QAbstractButton *btn)
{
   const int role = m_ui->m_buttonBox->buttonRole(btn);

   switch (role) {
      case QDialogButtonBox::RejectRole:
         reject();
         break;

      case QDialogButtonBox::AcceptRole:
         if (validatePropertyName(propertyName())) {
            accept();
         }
         break;
   }
}

}   // end namespace - qdesigner_internal

