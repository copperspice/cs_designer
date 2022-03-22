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

#include <qtgradientdialog.h>
#include <ui_gradient_dialog.h>

#include <QPushButton>

class QtGradientDialogPrivate
{
   QtGradientDialog *q_ptr;
   Q_DECLARE_PUBLIC(QtGradientDialog)

 public:
   void slotAboutToShowDetails(bool details, int extensionWidthHint);

   Ui::QtGradientDialog m_ui;
};

void QtGradientDialogPrivate::slotAboutToShowDetails(bool details, int extensionWidthHint)
{
   if (details) {
      q_ptr->resize(q_ptr->size() + QSize(extensionWidthHint, 0));
   } else {
      q_ptr->setMinimumSize(1, 1);
      q_ptr->resize(q_ptr->size() - QSize(extensionWidthHint, 0));
      q_ptr->setMinimumSize(0, 0);
   }
}

QtGradientDialog::QtGradientDialog(QWidget *parent)
   : QDialog(parent), d_ptr(new QtGradientDialogPrivate())
{
   //    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
   d_ptr->q_ptr = this;
   d_ptr->m_ui.setupUi(this);
   QPushButton *button = d_ptr->m_ui.buttonBox->button(QDialogButtonBox::Ok);
   if (button) {
      button->setAutoDefault(false);
   }
   button = d_ptr->m_ui.buttonBox->button(QDialogButtonBox::Cancel);
   if (button) {
      button->setAutoDefault(false);
   }

   connect(d_ptr->m_ui.gradientEditor, &QtGradientEditor::aboutToShowDetails,
      this, &QtGradientDialog::slotAboutToShowDetails);
}

QtGradientDialog::~QtGradientDialog()
{
}

void QtGradientDialog::setGradient(const QGradient &gradient)
{
   d_ptr->m_ui.gradientEditor->setGradient(gradient);
}

QGradient QtGradientDialog::gradient() const
{
   return d_ptr->m_ui.gradientEditor->gradient();
}

bool QtGradientDialog::isBackgroundCheckered() const
{
   return d_ptr->m_ui.gradientEditor->isBackgroundCheckered();
}

void QtGradientDialog::setBackgroundCheckered(bool checkered)
{
   d_ptr->m_ui.gradientEditor->setBackgroundCheckered(checkered);
}

bool QtGradientDialog::detailsVisible() const
{
   return d_ptr->m_ui.gradientEditor->detailsVisible();
}

void QtGradientDialog::setDetailsVisible(bool visible)
{
   d_ptr->m_ui.gradientEditor->setDetailsVisible(visible);
}

bool QtGradientDialog::isDetailsButtonVisible() const
{
   return d_ptr->m_ui.gradientEditor->isDetailsButtonVisible();
}

void QtGradientDialog::setDetailsButtonVisible(bool visible)
{
   d_ptr->m_ui.gradientEditor->setDetailsButtonVisible(visible);
}

/*!
    Returns the current QColor::Spec used for the color sliders in the dialog.
*/
QColor::Spec QtGradientDialog::spec() const
{
   return d_ptr->m_ui.gradientEditor->spec();
}

/*!
    Sets the current QColor::Spec to \a spec used for the color sliders in the dialog.
*/
void QtGradientDialog::setSpec(QColor::Spec spec)
{
   d_ptr->m_ui.gradientEditor->setSpec(spec);
}

/*!
    Executes a modal gradient dialog, lets the user to specify a gradient, and returns that gradient.

    If the user clicks \gui OK, the gradient specified by the user is returned. If the user clicks \gui Cancel, the \a initial gradient is returned.

    The dialog is constructed with the given \a parent. \a caption is shown as the window title of the dialog and
    \a initial is the initial gradient shown in the dialog. If the \a ok parameter is not-null,
    the value it refers to is set to true if the user clicks \gui OK, and set to false if the user clicks \gui Cancel.
*/
QGradient QtGradientDialog::getGradient(bool *ok, const QGradient &initial, QWidget *parent, const QString &caption)
{
   QtGradientDialog dlg(parent);
   if (!caption.isEmpty()) {
      dlg.setWindowTitle(caption);
   }
   dlg.setGradient(initial);
   const int res = dlg.exec();
   if (ok) {
      *ok = (res == QDialog::Accepted) ? true : false;
   }
   if (res == QDialog::Accepted) {
      return dlg.gradient();
   }
   return initial;
}

/*!
    This method calls getGradient(ok, QLinearGradient(), parent, caption).
*/
QGradient QtGradientDialog::getGradient(bool *ok, QWidget *parent, const QString &caption)
{
   return getGradient(ok, QLinearGradient(), parent, caption);
}

void QtGradientDialog::slotAboutToShowDetails(bool details, int extensionWidthHint)
{
   Q_D(QtGradientDialog);
   d->slotAboutToShowDetails(details, extensionWidthHint);
}