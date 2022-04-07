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

#include <ui_designer_gridpanel.h>

#include <grid_p.h>
#include <gridpanel_p.h>

namespace qdesigner_internal {

GridPanel::GridPanel(QWidget *parentWidget)
   : QWidget(parentWidget)
{
   m_ui = new Ui::GridPanel;
   m_ui->setupUi(this);

   connect(m_ui->m_resetButton, &QAbstractButton::clicked, this, &GridPanel::reset);
}

GridPanel::~GridPanel()
{
   delete m_ui;
}

void GridPanel::setGrid(const Grid &g)
{
   m_ui->m_deltaXSpinBox->setValue(g.deltaX());
   m_ui->m_deltaYSpinBox->setValue(g.deltaY());
   m_ui->m_visibleCheckBox->setCheckState(g.visible() ? Qt::Checked : Qt::Unchecked);
   m_ui->m_snapXCheckBox->setCheckState(g.snapX()  ? Qt::Checked : Qt::Unchecked);
   m_ui->m_snapYCheckBox->setCheckState(g.snapY()  ? Qt::Checked : Qt::Unchecked);
}

void GridPanel::setTitle(const QString &title)
{
   m_ui->m_gridGroupBox->setTitle(title);
}

Grid GridPanel::grid() const
{
   Grid rc;
   rc.setDeltaX(m_ui->m_deltaXSpinBox->value());
   rc.setDeltaY(m_ui->m_deltaYSpinBox->value());
   rc.setSnapX(m_ui->m_snapXCheckBox->checkState() == Qt::Checked);
   rc.setSnapY(m_ui->m_snapYCheckBox->checkState() == Qt::Checked);
   rc.setVisible(m_ui->m_visibleCheckBox->checkState() == Qt::Checked);
   return rc;
}

void GridPanel::reset()
{
   setGrid(Grid());
}

void GridPanel::setCheckable (bool c)
{
   m_ui->m_gridGroupBox->setCheckable(c);
}

bool GridPanel::isCheckable () const
{
   return m_ui->m_gridGroupBox->isCheckable ();
}

bool GridPanel::isChecked () const
{
   return m_ui->m_gridGroupBox->isChecked ();
}

void GridPanel::setChecked(bool c)
{
   m_ui->m_gridGroupBox->setChecked(c);
}

void GridPanel::setResetButtonVisible(bool v)
{
   m_ui->m_resetButton->setVisible(v);
}

}   // end namespace
