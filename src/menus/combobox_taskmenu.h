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

#ifndef COMBOBOX_TASKMENU_H
#define COMBOBOX_TASKMENU_H

#include <designer_taskmenu.h>
#include <extensionfactory_p.h>

class QDesignerFormWindowInterface;

#include <QComboBox>
#include <QPointer>

class QLineEdit;

namespace qdesigner_internal {

class ComboBoxTaskMenu: public QDesignerTaskMenu
{
   CS_OBJECT(ComboBoxTaskMenu)

 public:
   explicit ComboBoxTaskMenu(QComboBox *button, QObject *parent = nullptr);
   virtual ~ComboBoxTaskMenu();

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 private:
   CS_SLOT_1(Private, void editItems())
   CS_SLOT_2(editItems)
   CS_SLOT_1(Private, void updateSelection())
   CS_SLOT_2(updateSelection)

 private:
   QComboBox *m_comboBox;
   QPointer<QDesignerFormWindowInterface> m_formWindow;
   QPointer<QLineEdit> m_editor;
   mutable QList<QAction *> m_taskActions;
   QAction *m_editItemsAction;
};

class ComboBoxTaskMenuFactory : public ExtensionFactory<QDesignerTaskMenuExtension, QComboBox, ComboBoxTaskMenu>
{
 public:
   explicit ComboBoxTaskMenuFactory(const QString &iid, QExtensionManager *extensionManager);

 private:
   QComboBox *checkObject(QObject *qObject) const override;
};

}    // end namespace qdesigner_internal

#endif