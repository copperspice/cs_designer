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

#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <abstract_formwindowcursor.h>
#include <abstract_propertyeditor.h>
#include <button_taskmenu.h>
#include <designer_formwindow_command.h>
#include <designer_object_inspector.h>
#include <inplace_editor.h>

#include <formwindowbase_p.h>
#include <metadatabase_p.h>

#include <QAbstractButton>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
#include <QMenu>
#include <QStyle>
#include <QStyleOption>

CS_DECLARE_METATYPE(QButtonGroup *)

namespace qdesigner_internal {

typedef QList<QAbstractButton *> ButtonList;
typedef QList<QButtonGroup *> ButtonGroupList;

// ButtonGroupCommand: Base for commands handling button groups and button lists
// addButtonsToGroup() and removeButtonsFromGroup() are low-level helpers for
// adding/removing members to/from existing groups.
//
// createButtonGroup()/breakButtonGroup() create and remove the groups from scratch.
// When using them in a command, the command must be executed within
// a macro since it makes the form emit objectRemoved() which might cause other components
// to add commands (for example, removal of signals and slots)
class ButtonGroupCommand : public QDesignerFormWindowCommand
{

 public:
   static QString nameList(const ButtonList &bl);
   static ButtonGroupList managedButtonGroups(const QDesignerFormWindowInterface *formWindow);

 protected:
   ButtonGroupCommand(const QString &description, QDesignerFormWindowInterface *formWindow);

   void initialize(const ButtonList &bl, QButtonGroup *buttonGroup);

   // Helper: Add the buttons to the group
   void addButtonsToGroup();

   // Helper; Remove the buttons
   void removeButtonsFromGroup();

   // Create the button group in Designer
   void createButtonGroup();

   // Remove the button group from Designer
   void breakButtonGroup();

 private:
   ButtonList m_buttonList;
   QButtonGroup *m_buttonGroup;
};

ButtonGroupCommand::ButtonGroupCommand(const QString &description, QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(description, formWindow), m_buttonGroup(nullptr)
{
}

void ButtonGroupCommand::initialize(const ButtonList &bl, QButtonGroup *buttonGroup)
{
   m_buttonList  = bl;
   m_buttonGroup = buttonGroup;
}

void ButtonGroupCommand::addButtonsToGroup()
{
   for (auto item : m_buttonList) {
      m_buttonGroup->addButton(item);
   }
}

void ButtonGroupCommand::removeButtonsFromGroup()
{
   for (auto item : m_buttonList) {
      m_buttonGroup->removeButton(item);
   }
}

void ButtonGroupCommand::createButtonGroup()
{
   QDesignerFormWindowInterface *fw   = formWindow();
   QDesignerFormEditorInterface *core = fw->core();
   core->metaDataBase()->add(m_buttonGroup);
   addButtonsToGroup();

   // Make button group visible
   core->objectInspector()->setFormWindow(fw);
}

void ButtonGroupCommand::breakButtonGroup()
{
   QDesignerFormWindowInterface *fw = formWindow();
   QDesignerFormEditorInterface *core = fw->core();

   // Button group was selected, that is, break was invoked via its context menu. Remove it from property editor, select the buttons
   if (core->propertyEditor()->object() == m_buttonGroup) {
      fw->clearSelection(false);

      for (auto item : m_buttonList) {
         fw->selectWidget(item, true);
      }
   }

   // Now remove and refresh object inspector
   removeButtonsFromGroup();

   // Notify components (for example, signal slot editor)
   if (qdesigner_internal::FormWindowBase *fwb = dynamic_cast<qdesigner_internal::FormWindowBase *>(fw)) {
      fwb->emitObjectRemoved(m_buttonGroup);
   }

   core->metaDataBase()->remove(m_buttonGroup);
   core->objectInspector()->setFormWindow(fw);
}

QString ButtonGroupCommand::nameList(const ButtonList &bl)
{
   QString rc;

   const QChar quote       = '\'';
   const QString separator = ", ";

   const int size = bl.size();

   for (int i = 0; i < size; ++i) {
      if (i) {
         rc += separator;
      }

      rc += quote;
      rc += bl[i]->objectName();
      rc += quote;
   }

   return rc;
}

ButtonGroupList ButtonGroupCommand::managedButtonGroups(const QDesignerFormWindowInterface *formWindow)
{
   const QDesignerMetaDataBaseInterface *mdb = formWindow->core()->metaDataBase();
   ButtonGroupList bl;

   // Check 1st order children for managed button groups
   const QObjectList children = formWindow->mainContainer()->children();
   const QObjectList::const_iterator cend =  children.constEnd();

   for (auto it =  children.constBegin(); it != cend; ++it) {

      if (! (*it)->isWidgetType()) {
         if (QButtonGroup *bg = dynamic_cast<QButtonGroup *>(*it))
            if (mdb->item(bg)) {
               bl.push_back(bg);
            }
      }
   }

   return bl;
}

// This command might be executed in a macro with a remove
// command to move buttons from one group to a new one.
class CreateButtonGroupCommand : public ButtonGroupCommand
{
 public:
   CreateButtonGroupCommand(QDesignerFormWindowInterface *formWindow);
   bool init(const ButtonList &bl);

   void undo() override {
      breakButtonGroup();
   }

   void redo() override {
      createButtonGroup();
   }
};

CreateButtonGroupCommand::CreateButtonGroupCommand(QDesignerFormWindowInterface *formWindow)
   : ButtonGroupCommand(QApplication::translate("Command", "Create button group"), formWindow)
{
}

bool CreateButtonGroupCommand::init(const ButtonList &bl)
{
   if (bl.empty()) {
      return false;
   }

   QDesignerFormWindowInterface *fw = formWindow();
   QButtonGroup *buttonGroup = new QButtonGroup(fw->mainContainer());
   buttonGroup->setObjectName(QString("buttonGroup"));
   fw->ensureUniqueObjectName(buttonGroup);
   initialize(bl, buttonGroup);

   return true;
}

class BreakButtonGroupCommand : public ButtonGroupCommand
{
 public:
   BreakButtonGroupCommand(QDesignerFormWindowInterface *formWindow);
   bool init(QButtonGroup *group);

   void undo() override {
      createButtonGroup();
   }

   void redo() override {
      breakButtonGroup();
   }
};

BreakButtonGroupCommand::BreakButtonGroupCommand(QDesignerFormWindowInterface *formWindow) :
   ButtonGroupCommand(QApplication::translate("Command", "Break button group"), formWindow)
{
}

bool BreakButtonGroupCommand::init(QButtonGroup *group)
{
   if (! group) {
      return false;
   }

   initialize(group->buttons(), group);
   setText(QApplication::translate("Command", "Break button group '%1'").formatArg(group->objectName()));

   return true;
}

// --------------- AddButtonsToGroupCommand
// This command might be executed in a macro with a remove
// command to move buttons from one group to a new one.
class AddButtonsToGroupCommand : public ButtonGroupCommand
{
 public:
   AddButtonsToGroupCommand(QDesignerFormWindowInterface *formWindow);
   void init(const ButtonList &bl, QButtonGroup *group);

   void undo() override {
      removeButtonsFromGroup();
   }

   void redo() override {
      addButtonsToGroup();
   }
};

AddButtonsToGroupCommand::AddButtonsToGroupCommand(QDesignerFormWindowInterface *formWindow) :
   ButtonGroupCommand(QApplication::translate("Command", "Add buttons to group"), formWindow)
{
}

void AddButtonsToGroupCommand::init(const ButtonList &bl, QButtonGroup *group)
{
   initialize(bl, group);
   //: Command description for adding buttons to a QButtonGroup
   setText(QApplication::translate("Command", "Add '%1' to '%2'").formatArgs(nameList(bl), group->objectName()));
}

class RemoveButtonsFromGroupCommand : public ButtonGroupCommand
{
 public:
   RemoveButtonsFromGroupCommand(QDesignerFormWindowInterface *formWindow);
   bool init(const ButtonList &bl);

   void undo() override {
      addButtonsToGroup();
   }

   void redo() override {
      removeButtonsFromGroup();
   }
};

RemoveButtonsFromGroupCommand::RemoveButtonsFromGroupCommand(QDesignerFormWindowInterface *formWindow) :
   ButtonGroupCommand(QApplication::translate("Command", "Remove buttons from group"), formWindow)
{
}

bool RemoveButtonsFromGroupCommand::init(const ButtonList &bl)
{
   if (bl.empty()) {
      return false;
   }

   QButtonGroup *group = bl.front()->group();

   if (! group) {
      return false;
   }

   if (bl.size() >= group->buttons().size()) {
      return false;
   }

   initialize(bl, group);

   // Command description for removing buttons from a QButtonGroup
   setText(QApplication::translate("Command", "Remove '%1' from '%2'").formatArgs(nameList(bl), group->objectName()));

   return true;
}

// --------  ButtonGroupMenu
ButtonGroupMenu::ButtonGroupMenu(QObject *parent)
   : QObject(parent), m_selectGroupAction(new QAction(tr("Select members"), this)),
     m_breakGroupAction(new QAction(tr("Break"), this)),
     m_formWindow(nullptr), m_buttonGroup(nullptr), m_currentButton(nullptr)
{
   connect(m_breakGroupAction,  &QAction::triggered, this, &ButtonGroupMenu::breakGroup);
   connect(m_selectGroupAction, &QAction::triggered, this, &ButtonGroupMenu::selectGroup);
}

void ButtonGroupMenu::initialize(QDesignerFormWindowInterface *formWindow, QButtonGroup *buttonGroup, QAbstractButton *currentButton)
{
   m_buttonGroup   = buttonGroup;
   m_currentButton = currentButton;
   m_formWindow    = formWindow;
   Q_ASSERT(m_formWindow);

   const bool canBreak = buttonGroup != nullptr;
   m_breakGroupAction->setEnabled(canBreak);
   m_selectGroupAction->setEnabled(canBreak);
}

void ButtonGroupMenu::selectGroup()
{
   // Select and make current button "current" again by selecting it last (if there is any)
   const ButtonList buttons = m_buttonGroup->buttons();
   m_formWindow->clearSelection(false);

   const ButtonList::const_iterator cend = buttons.constEnd();

   for (ButtonList::const_iterator it = buttons.constBegin(); it != cend; ++it) {
      if (*it != m_currentButton) {
         m_formWindow->selectWidget(*it, true);
      }
   }

   if (m_currentButton) {
      m_formWindow->selectWidget(m_currentButton, true);
   }
}

void ButtonGroupMenu::breakGroup()
{
   BreakButtonGroupCommand *cmd = new BreakButtonGroupCommand(m_formWindow);

   if (cmd->init(m_buttonGroup)) {
      // Need a macro since the command might trigger additional commands
      QUndoStack *history = m_formWindow->commandHistory();
      history->beginMacro(cmd->text());
      history->push(cmd);
      history->endMacro();

   } else {
      delete cmd;
   }
}

ButtonGroupTaskMenu::ButtonGroupTaskMenu(QButtonGroup *buttonGroup, QObject *parent)
   : QObject(parent), m_buttonGroup(buttonGroup)
{
   m_taskActions.push_back(m_menu.breakGroupAction());
   m_taskActions.push_back(m_menu.selectGroupAction());
}

QAction *ButtonGroupTaskMenu::preferredEditAction() const
{
   return m_menu.selectGroupAction();
}

QList<QAction *> ButtonGroupTaskMenu::taskActions() const
{
   m_menu.initialize(QDesignerFormWindowInterface::findFormWindow(m_buttonGroup), m_buttonGroup);
   return m_taskActions;
}

// -------- Text area editor
class ButtonTextTaskMenuInlineEditor : public  TaskMenuInlineEditor
{
 public:
   ButtonTextTaskMenuInlineEditor(QAbstractButton *button, QObject *parent);

 protected:
   QRect editRectangle() const override;
};

ButtonTextTaskMenuInlineEditor::ButtonTextTaskMenuInlineEditor(QAbstractButton *button, QObject *parent) :
   TaskMenuInlineEditor(button, ValidationMultiLine, QString("text"), parent)
{
}

QRect ButtonTextTaskMenuInlineEditor::editRectangle() const
{
   QWidget *w = widget();

   QStyleOptionButton opt;
   opt.initFrom(w);

   return w->style()->subElementRect(QStyle::SE_PushButtonContents, &opt, w);
}

class LinkDescriptionTaskMenuInlineEditor : public  TaskMenuInlineEditor
{
 public:
   LinkDescriptionTaskMenuInlineEditor(QAbstractButton *button, QObject *parent);

 protected:
   QRect editRectangle() const override;
};

LinkDescriptionTaskMenuInlineEditor::LinkDescriptionTaskMenuInlineEditor(QAbstractButton *button, QObject *parent)
   : TaskMenuInlineEditor(button, ValidationMultiLine, QString("description"), parent)
{
}

QRect LinkDescriptionTaskMenuInlineEditor::editRectangle() const
{
   QWidget *w = widget();          // what is the exact description area?

   QStyleOptionButton opt;
   opt.initFrom(w);

   return w->style()->subElementRect(QStyle::SE_PushButtonContents, &opt, w);
}

ButtonTaskMenu::ButtonTaskMenu(QAbstractButton *button, QObject *parent)
   : QDesignerTaskMenu(button, parent),
     m_assignGroupSubMenu(new QMenu),
     m_assignActionGroup(nullptr),
     m_assignToGroupSubMenuAction(new QAction(tr("Assign to button group"), this)),
     m_currentGroupSubMenu(new QMenu),
     m_currentGroupSubMenuAction(new QAction(tr("Button group"), this)),
     m_createGroupAction(new QAction(tr("New button group"), this)),
     m_preferredEditAction(new QAction(tr("Change text..."), this)),
     m_removeFromGroupAction(new QAction(tr("None"), this))
{
   connect(m_createGroupAction, &QAction::triggered, this, &ButtonTaskMenu::createGroup);
   TaskMenuInlineEditor *textEditor = new ButtonTextTaskMenuInlineEditor(button, this);
   connect(m_preferredEditAction, &QAction::triggered, textEditor, &TaskMenuInlineEditor::editText);
   connect(m_removeFromGroupAction, &QAction::triggered, this, &ButtonTaskMenu::removeFromGroup);

   m_assignToGroupSubMenuAction->setMenu(m_assignGroupSubMenu);

   m_currentGroupSubMenu->addAction(m_groupMenu.breakGroupAction());
   m_currentGroupSubMenu->addAction(m_groupMenu.selectGroupAction());
   m_currentGroupSubMenuAction->setMenu(m_currentGroupSubMenu);

   m_taskActions.append(m_preferredEditAction);
   m_taskActions.append(m_assignToGroupSubMenuAction);
   m_taskActions.append(m_currentGroupSubMenuAction);
   m_taskActions.append(createSeparator());
}

ButtonTaskMenu::~ButtonTaskMenu()
{
   delete m_assignGroupSubMenu;
   delete m_currentGroupSubMenu;
}

QAction *ButtonTaskMenu::preferredEditAction() const
{
   return m_preferredEditAction;
}

bool ButtonTaskMenu::refreshAssignMenu(const QDesignerFormWindowInterface *fw, int buttonCount, SelectionType st,
   QButtonGroup *currentGroup)
{
   // clear
   if (m_assignActionGroup) {
      delete m_assignActionGroup;
      m_assignActionGroup = nullptr;
   }

   m_assignGroupSubMenu->clear();
   if (st == OtherSelection) {
      return false;
   }

   // Assign to new: Need several
   const bool canAssignToNewGroup = buttonCount > 1;

   m_createGroupAction->setEnabled(canAssignToNewGroup);

   if (canAssignToNewGroup) {
      m_assignGroupSubMenu->addAction(m_createGroupAction);
   }

   // Assign to other
   const ButtonGroupList bl = ButtonGroupCommand::managedButtonGroups(fw);

   // Groups: Any groups to add to except the current?
   const int groupCount = bl.size();
   const bool hasAddGroups = groupCount > 1 || (groupCount == 1 && !bl.contains(currentGroup));

   if (hasAddGroups) {
      if (!m_assignGroupSubMenu->isEmpty()) {
         m_assignGroupSubMenu->addSeparator();
      }
      // Create a new action group
      m_assignActionGroup = new QActionGroup(this);
      connect(m_assignActionGroup, &QActionGroup::triggered, this, &ButtonTaskMenu::addToGroup);

      const ButtonGroupList::const_iterator cend = bl.constEnd();
      for (ButtonGroupList::const_iterator it = bl.constBegin(); it != cend; ++it) {
         QButtonGroup *bg = *it;
         if (*it != currentGroup) {
            QAction *a = new QAction(bg->objectName(), m_assignGroupSubMenu);
            a->setData(QVariant::fromValue(bg));
            m_assignActionGroup->addAction(a);
            m_assignGroupSubMenu->addAction(a);
         }
      }
   }

   // Can remove: A homogenous selection of another group that does not completely break it.
   const bool canRemoveFromGroup = st == GroupedButtonSelection;
   m_removeFromGroupAction->setEnabled(canRemoveFromGroup);

   if (canRemoveFromGroup) {
      if (!m_assignGroupSubMenu->isEmpty()) {
         m_assignGroupSubMenu->addSeparator();
      }

      m_assignGroupSubMenu->addAction(m_removeFromGroupAction);
   }

   return !m_assignGroupSubMenu->isEmpty();
}

QList<QAction *> ButtonTaskMenu::taskActions() const
{
   ButtonTaskMenu *ncThis = const_cast<ButtonTaskMenu *>(this);
   QButtonGroup *buttonGroup = nullptr;

   QDesignerFormWindowInterface *fw = formWindow();
   const SelectionType st = selectionType(fw->cursor(), &buttonGroup);

   m_groupMenu.initialize(fw, buttonGroup, button());
   const bool hasAssignOptions = ncThis->refreshAssignMenu(fw, fw->cursor()->selectedWidgetCount(), st, buttonGroup);
   m_assignToGroupSubMenuAction->setVisible(hasAssignOptions);

   // add/remove
   switch (st) {
      case UngroupedButtonSelection:
      case OtherSelection:
         m_currentGroupSubMenuAction->setVisible(false);
         break;

      case GroupedButtonSelection:
         m_currentGroupSubMenuAction->setText(tr("Button group '%1'").formatArg(buttonGroup->objectName()));
         m_currentGroupSubMenuAction->setVisible(true);
         break;
   }

   return m_taskActions + QDesignerTaskMenu::taskActions();
}


void ButtonTaskMenu::insertAction(int index, QAction *a)
{
   m_taskActions.insert(index, a);
}

/* Create a button list from the cursor selection */
static ButtonList buttonList(const QDesignerFormWindowCursorInterface *cursor)
{
   ButtonList rc;
   const int selectionCount = cursor->selectedWidgetCount();

   for (int i = 0; i < selectionCount; ++i) {
      QAbstractButton *ab = dynamic_cast<QAbstractButton *>(cursor->selectedWidget(i));
      Q_ASSERT(ab);
      rc += ab;
   }

   return rc;
}

// Create a command to remove the buttons from their group
// If it would leave an empty or 1-member group behind, create a break command instead

static QUndoCommand *createRemoveButtonsCommand(QDesignerFormWindowInterface *fw, const ButtonList &bl)
{
   QButtonGroup *bg = bl.front()->group();

   // Complete group or 1-member group?
   if (bl.size() >= bg->buttons().size() - 1) {
      BreakButtonGroupCommand *breakCmd = new BreakButtonGroupCommand(fw);

      if (! breakCmd->init(bg)) {
         delete breakCmd;
         return nullptr;
      }

      return breakCmd;
   }

   // Just remove the buttons
   RemoveButtonsFromGroupCommand *removeCmd  = new RemoveButtonsFromGroupCommand(fw);

   if (! removeCmd->init(bl)) {
      delete removeCmd;
      return nullptr;
   }

   return removeCmd;
}

void ButtonTaskMenu::createGroup()
{
   QDesignerFormWindowInterface *fw = formWindow();
   const ButtonList bl = buttonList(fw->cursor());

   // Do we need to remove the buttons from an existing group?
   QUndoCommand *removeCmd = nullptr;

   if (bl.front()->group()) {
      removeCmd = createRemoveButtonsCommand(fw, bl);
      if (!removeCmd) {
         return;
      }
   }

   // Add cmd
   CreateButtonGroupCommand *addCmd = new CreateButtonGroupCommand(fw);

   if (! addCmd->init(bl)) {
      delete addCmd;
      return;
   }

   // Need a macro [even if we only have the add command] since the command might trigger additional commands
   QUndoStack *history = fw->commandHistory();
   history->beginMacro(addCmd->text());

  if (removeCmd) {
      history->push(removeCmd);
   }

   history->push(addCmd);
   history->endMacro();
}

QAbstractButton *ButtonTaskMenu::button() const
{
   return dynamic_cast<QAbstractButton *>(widget());
}

// Figure out if we have a homogenous selections (buttons of the same group or no group)
ButtonTaskMenu::SelectionType ButtonTaskMenu::selectionType(const QDesignerFormWindowCursorInterface *cursor,
      QButtonGroup **ptrToGroup) const
{
   const int selectionCount = cursor->selectedWidgetCount();
   if (! selectionCount) {
      return OtherSelection;
   }

   QButtonGroup *commonGroup = nullptr;

   for (int i = 0; i < selectionCount; i++) {
      if (const QAbstractButton *ab = dynamic_cast<const QAbstractButton *>(cursor->selectedWidget(i))) {
         QButtonGroup *buttonGroup = ab->group();

         if (i) {
            if (buttonGroup != commonGroup) {
               return OtherSelection;
            }

         } else {
            commonGroup = buttonGroup;
         }

      } else {
         return OtherSelection;
      }
   }

   if (ptrToGroup) {
      *ptrToGroup = commonGroup;
   }

   return commonGroup ? GroupedButtonSelection : UngroupedButtonSelection;
}

void ButtonTaskMenu::addToGroup(QAction *a)
{
   QVariant data = a->data();
   QButtonGroup *bg = data.value<QButtonGroup *>();

   Q_ASSERT(bg);

   QDesignerFormWindowInterface *fw = formWindow();
   const ButtonList bl = buttonList(fw->cursor());

   // Do we need to remove the buttons from an existing group?
   QUndoCommand *removeCmd = nullptr;

   if (bl.front()->group()) {
      removeCmd = createRemoveButtonsCommand(fw, bl);
      if (! removeCmd) {
         return;
      }
   }

   AddButtonsToGroupCommand *addCmd = new AddButtonsToGroupCommand(fw);
   addCmd->init(bl, bg);

   QUndoStack *history = fw->commandHistory();
   if (removeCmd) {
      history->beginMacro(addCmd->text());
      history->push(removeCmd);
      history->push(addCmd);
      history->endMacro();
   } else {
      history->push(addCmd);
   }
}

void ButtonTaskMenu::removeFromGroup()
{
   QDesignerFormWindowInterface *fw = formWindow();

   if (QUndoCommand *cmd = createRemoveButtonsCommand(fw, buttonList(fw->cursor()))) {
      fw->commandHistory()->push(cmd);
   }
}

CommandLinkButtonTaskMenu::CommandLinkButtonTaskMenu(QCommandLinkButton *button, QObject *parent)
   : ButtonTaskMenu(button, parent)
{
   TaskMenuInlineEditor *descriptonEditor = new LinkDescriptionTaskMenuInlineEditor(button, this);
   QAction *descriptionAction = new QAction(tr("Change description..."), this);
   connect(descriptionAction, &QAction::triggered, descriptonEditor, &TaskMenuInlineEditor::editText);
   insertAction(1, descriptionAction);
}

}   // end namespace
