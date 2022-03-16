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

#ifndef BUTTON_TASKMENU_H
#define BUTTON_TASKMENU_H

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

class QDesignerFormWindowCursorInterface;

#include <QAbstractButton>
#include <QCommandLinkButton>
#include <QButtonGroup>

class QMenu;
class QActionGroup;

namespace qdesigner_internal {

// ButtonGroupMenu: Mixin menu for the 'select members'/'break group' options of
// the task menu of buttons and button group
class ButtonGroupMenu : public QObject
{
   CS_OBJECT(ButtonGroupMenu)
   Q_DISABLE_COPY(ButtonGroupMenu)
 public:
   ButtonGroupMenu(QObject *parent = nullptr);

   void initialize(QDesignerFormWindowInterface *formWindow,
      QButtonGroup *buttonGroup = 0,
      /* Current button for selection in ButtonMode */
      QAbstractButton *currentButton = 0);

   QAction *selectGroupAction() const {
      return m_selectGroupAction;
   }
   QAction *breakGroupAction() const  {
      return m_breakGroupAction;
   }

 private:
   CS_SLOT_1(Private, void selectGroup())
   CS_SLOT_2(selectGroup)
   CS_SLOT_1(Private, void breakGroup())
   CS_SLOT_2(breakGroup)

 private:
   QAction *m_selectGroupAction;
   QAction *m_breakGroupAction;

   QDesignerFormWindowInterface *m_formWindow;
   QButtonGroup *m_buttonGroup;
   QAbstractButton *m_currentButton;
};

// Task menu extension of a QButtonGroup
class ButtonGroupTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
   CS_OBJECT(ButtonGroupTaskMenu)
   Q_DISABLE_COPY(ButtonGroupTaskMenu)
   CS_INTERFACES(QDesignerTaskMenuExtension)

 public:
   explicit ButtonGroupTaskMenu(QButtonGroup *buttonGroup, QObject *parent = nullptr);

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

 private:
   QButtonGroup *m_buttonGroup;
   QList<QAction *> m_taskActions;
   mutable ButtonGroupMenu m_menu;
};

// Task menu extension of a QAbstractButton
class ButtonTaskMenu: public QDesignerTaskMenu
{
   CS_OBJECT(ButtonTaskMenu)
   Q_DISABLE_COPY(ButtonTaskMenu)
 public:
   explicit ButtonTaskMenu(QAbstractButton *button, QObject *parent = nullptr);
   virtual ~ButtonTaskMenu();

   QAction *preferredEditAction() const override;
   QList<QAction *> taskActions() const override;

   QAbstractButton *button() const;

 protected:
   void insertAction(int index, QAction *a);

 private:
   CS_SLOT_1(Private, void createGroup())
   CS_SLOT_2(createGroup)
   CS_SLOT_1(Private, void addToGroup(QAction *a))
   CS_SLOT_2(addToGroup)
   CS_SLOT_1(Private, void removeFromGroup())
   CS_SLOT_2(removeFromGroup)

 private:
   enum SelectionType {
      OtherSelection,
      UngroupedButtonSelection,
      GroupedButtonSelection
   };

   SelectionType selectionType(const QDesignerFormWindowCursorInterface *cursor, QButtonGroup **ptrToGroup = 0) const;
   bool refreshAssignMenu(const QDesignerFormWindowInterface *fw, int buttonCount, SelectionType st, QButtonGroup *currentGroup);
   QMenu *createGroupSelectionMenu(const QDesignerFormWindowInterface *fw);

   QList<QAction *> m_taskActions;
   mutable ButtonGroupMenu m_groupMenu;
   QMenu *m_assignGroupSubMenu;
   QActionGroup *m_assignActionGroup;
   QAction *m_assignToGroupSubMenuAction;
   QMenu *m_currentGroupSubMenu;
   QAction *m_currentGroupSubMenuAction;

   QAction *m_createGroupAction;
   QAction *m_preferredEditAction;
   QAction *m_removeFromGroupAction;
};

// Task menu extension of a QCommandLinkButton
class CommandLinkButtonTaskMenu: public ButtonTaskMenu
{
   CS_OBJECT(CommandLinkButtonTaskMenu)
   Q_DISABLE_COPY(CommandLinkButtonTaskMenu)
 public:
   explicit CommandLinkButtonTaskMenu(QCommandLinkButton *button, QObject *parent = nullptr);
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QButtonGroup, ButtonGroupTaskMenu> ButtonGroupTaskMenuFactory;
typedef ExtensionFactory<QDesignerTaskMenuExtension, QCommandLinkButton, CommandLinkButtonTaskMenu>  CommandLinkButtonTaskMenuFactory;
typedef ExtensionFactory<QDesignerTaskMenuExtension, QAbstractButton, ButtonTaskMenu>  ButtonTaskMenuFactory;

}   // end namespace qdesigner_internal


#endif
