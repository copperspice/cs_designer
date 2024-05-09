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

#include <gradient_manager.h>
#include <gradient_select.h>

#include <QPushButton>

QtGradientSelect::QtGradientSelect(QWidget *parent)
   : QDialog(parent)
{
   m_ui.setupUi(this);
   m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

   connect(m_ui.gradientView, &QtGradientView::currentGradientChanged,
      this, &QtGradientSelect::slotGradientSelected);

   connect(m_ui.gradientView, &QtGradientView::gradientActivated,
      this, &QtGradientSelect::slotGradientActivated);
}

void QtGradientSelect::setGradientManager(QtGradientManager *manager)
{
   m_ui.gradientView->setGradientManager(manager);
}

QGradient QtGradientSelect::getGradient(bool *ok, QtGradientManager *manager, QWidget *parent, const QString &caption)
{
   QtGradientSelect dlg(parent);
   dlg.setGradientManager(manager);
   dlg.setWindowTitle(caption);

   QGradient grad = QLinearGradient();
   const int res = dlg.exec();

   if (res == QDialog::Accepted) {
      grad = dlg.m_ui.gradientView->gradientManager()->gradients().value(dlg.m_ui.gradientView->currentGradient());
   }
   if (ok) {
      *ok = res == QDialog::Accepted;
   }
   return grad;
}

void QtGradientSelect::slotGradientSelected(const QString &id)
{
   m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!id.isEmpty());
}

void QtGradientSelect::slotGradientActivated(const QString &id)
{
   Q_UNUSED(id)
   accept();
}
