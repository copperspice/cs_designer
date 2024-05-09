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

#include <abstract_formeditor.h>
#include <abstract_propertyeditor.h>
#include <action_editor.h>
#include <action_repository.h>
#include <designer_command.h>
#include <designer_menu.h>
#include <designer_object_inspector.h>
#include <designer_property_command.h>
#include <designer_propertysheet.h>
#include <designer_settings.h>
#include <designer_taskmenu.h>
#include <designer_utils.h>
#include <extension.h>
#include <newaction_dialog.h>
#include <simple_resource.h>

#include <formwindowbase_p.h>
#include <iconloader_p.h>
#include <metadatabase_p.h>

#include <QAction>
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDebug>
#include <QItemDelegate>
#include <QItemSelection>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QSignalMapper>
#include <QSplitter>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

CS_DECLARE_METATYPE(QAction *)

static const QString actionEditorViewModeKey = "ActionEditorViewMode";

static const QString iconPropertyC       = "icon";
static const QString shortcutPropertyC   = "shortcut";
static const QString toolTipPropertyC    = "toolTip";
static const QString checkablePropertyC  = "checkable";
static const QString objectNamePropertyC = "objectName";
static const QString textPropertyC       = "text";

namespace qdesigner_internal {

class ActionGroupDelegate: public QItemDelegate
{
 public:
   ActionGroupDelegate(QObject *parent)
      : QItemDelegate(parent)
   {
   }

   void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
      if (option.state & QStyle::State_Selected) {
         painter->fillRect(option.rect, option.palette.highlight());
      }

      QItemDelegate::paint(painter, option, index);
   }

   void drawFocus(QPainter *, const QStyleOptionViewItem &, const QRect &) const override
   {
   }
};

ActionEditor::ActionEditor(QDesignerFormEditorInterface *core, QWidget *parent, Qt::WindowFlags flags)
   : QDesignerActionEditorInterface(parent, flags), m_core(core), m_actionGroups(nullptr),
     m_actionView(new ActionView),
     m_actionNew(new QAction(tr("New..."), this)),
     m_actionEdit(new QAction(tr("Edit..."), this)),
     m_actionNavigateToSlot(new QAction(tr("Go to slot..."), this)),
     m_actionCopy(new QAction(tr("Copy"), this)),
     m_actionCut(new QAction(tr("Cut"), this)),
     m_actionPaste(new QAction(tr("Paste"), this)),
     m_actionSelectAll(new QAction(tr("Select all"), this)),
     m_actionDelete(new QAction(tr("Delete"), this)),
     m_viewModeGroup(new  QActionGroup(this)),
     m_iconViewAction(nullptr), m_listViewAction(nullptr), m_filterWidget(nullptr),
     m_selectAssociatedWidgetsMapper(nullptr)
{
   m_actionView->initialize(m_core);
   m_actionView->setSelectionMode(QAbstractItemView::ExtendedSelection);
   setWindowTitle(tr("Actions"));

   QVBoxLayout *l = new QVBoxLayout(this);
   l->setMargin(0);
   l->setSpacing(0);

   QToolBar *toolbar = new QToolBar;
   toolbar->setIconSize(QSize(22, 22));
   toolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
   l->addWidget(toolbar);

   // edit actions
   QIcon documentNewIcon = QIcon::fromTheme(QString("document-new"), createIconSet(QString("filenew.png")));
   m_actionNew->setIcon(documentNewIcon);
   m_actionNew->setEnabled(false);
   connect(m_actionNew, &QAction::triggered, this, &ActionEditor::slotNewAction);
   toolbar->addAction(m_actionNew);

   connect(m_actionSelectAll, &QAction::triggered, m_actionView, &ActionView::selectAll);

   m_actionCut->setEnabled(false);
   connect(m_actionCut, &QAction::triggered, this, &ActionEditor::slotCut);
   QIcon editCutIcon = QIcon::fromTheme(QString("edit-cut"), createIconSet(QString("editcut.png")));
   m_actionCut->setIcon(editCutIcon);

   m_actionCopy->setEnabled(false);
   connect(m_actionCopy, &QAction::triggered, this, &ActionEditor::slotCopy);
   QIcon editCopyIcon = QIcon::fromTheme(QString("edit-copy"), createIconSet(QString("editcopy.png")));
   m_actionCopy->setIcon(editCopyIcon);
   toolbar->addAction(m_actionCopy);

   connect(m_actionPaste, &QAction::triggered, this, &ActionEditor::slotPaste);
   QIcon editPasteIcon = QIcon::fromTheme(QString("edit-paste"), createIconSet(QString("editpaste.png")));
   m_actionPaste->setIcon(editPasteIcon);
   toolbar->addAction(m_actionPaste);

   m_actionEdit->setEnabled(false);
   connect(m_actionEdit, &QAction::triggered, this, &ActionEditor::editCurrentAction);

   connect(m_actionNavigateToSlot, &QAction::triggered, this, &ActionEditor::navigateToSlotCurrentAction);

   QIcon editDeleteIcon = QIcon::fromTheme(QString("edit-delete"), createIconSet(QString("editdelete.png")));
   m_actionDelete->setIcon(editDeleteIcon);
   m_actionDelete->setEnabled(false);
   connect(m_actionDelete, &QAction::triggered, this, &ActionEditor::slotDelete);
   toolbar->addAction(m_actionDelete);

   // Toolbutton with menu containing action group for detailed/icon view, uses the icons from the file dialog

   QMenu *configureMenu;
   toolbar->addWidget(createConfigureMenuButton(tr("Configure Action Editor"), &configureMenu));

   connect(m_viewModeGroup, &QActionGroup::triggered, this, &ActionEditor::slotViewMode);
   m_iconViewAction = m_viewModeGroup->addAction(tr("Icon View"));
   m_iconViewAction->setData(QVariant(ActionView::IconView));
   m_iconViewAction->setCheckable(true);
   m_iconViewAction->setIcon(style()->standardIcon (QStyle::SP_FileDialogListView));
   configureMenu->addAction(m_iconViewAction);

   m_listViewAction = m_viewModeGroup->addAction(tr("Detailed View"));
   m_listViewAction->setData(QVariant(ActionView::ViewMode::DetailedView));
   m_listViewAction->setCheckable(true);
   m_listViewAction->setIcon(style()->standardIcon (QStyle::SP_FileDialogDetailedView));
   configureMenu->addAction(m_listViewAction);

   // filter
   m_filterWidget = new QWidget(toolbar);

   QHBoxLayout *filterLayout = new QHBoxLayout(m_filterWidget);
   filterLayout->setContentsMargins(0, 0, 0, 0);

   QLineEdit *filterLineEdit = new QLineEdit(m_filterWidget);
   connect(filterLineEdit, &QLineEdit::textChanged, this, &ActionEditor::setFilter);
   filterLineEdit->setPlaceholderText(tr("Filter"));
   filterLineEdit->setClearButtonEnabled(true);
   filterLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Ignored));
   filterLayout->addWidget(filterLineEdit);
   m_filterWidget->setEnabled(false);
   toolbar->addWidget(m_filterWidget);

   // main layout
   QSplitter *splitter = new QSplitter(Qt::Horizontal);
   splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

   splitter->addWidget(m_actionView);
   l->addWidget(splitter);

#if 0 // ### implement me
   m_actionGroups = new QListWidget(splitter);
   splitter->addWidget(m_actionGroups);
   m_actionGroups->setItemDelegate(new ActionGroupDelegate(m_actionGroups));
   m_actionGroups->setMovement(QListWidget::Static);
   m_actionGroups->setResizeMode(QListWidget::Fixed);
   m_actionGroups->setIconSize(QSize(48, 48));
   m_actionGroups->setFlow(QListWidget::TopToBottom);
   m_actionGroups->setViewMode(QListWidget::IconMode);
   m_actionGroups->setWrapping(false);
#endif

   connect(m_actionView, &ActionView::resourceImageDropped,
      this, &ActionEditor::resourceImageDropped);

   connect(m_actionView, &ActionView::currentChanged, this, &ActionEditor::slotCurrentItemChanged);
   // make it possible for vs integration to reimplement edit action dialog
   connect(m_actionView, &ActionView::activated, this, &ActionEditor::itemActivated);

   connect(m_actionView, &ActionView::selectionChanged,
      this, &ActionEditor::slotSelectionChanged);

   connect(m_actionView, &ActionView::contextMenuRequested,
      this, &ActionEditor::slotContextMenuRequested);

   connect(this, &ActionEditor::itemActivated, this, &ActionEditor::editAction);

   restoreSettings();
   updateViewModeActions();
}

// Utility to create a configure button with menu for usage on toolbars
QToolButton *ActionEditor::createConfigureMenuButton(const QString &t, QMenu **ptrToMenu)
{
   QToolButton *configureButton = new QToolButton;
   QAction *configureAction = new QAction(t, configureButton);
   QIcon configureIcon = QIcon::fromTheme(QString("document-properties"), createIconSet(QString("configure.png")));
   configureAction->setIcon(configureIcon);
   QMenu *configureMenu = new QMenu;
   configureAction->setMenu(configureMenu);
   configureButton->setDefaultAction(configureAction);
   configureButton->setPopupMode(QToolButton::InstantPopup);
   *ptrToMenu = configureMenu;
   return configureButton;
}

ActionEditor::~ActionEditor()
{
   saveSettings();
}

QAction *ActionEditor::actionNew() const
{
   return m_actionNew;
}

QAction *ActionEditor::actionDelete() const
{
   return m_actionDelete;
}

QDesignerFormWindowInterface *ActionEditor::formWindow() const
{
   return m_formWindow;
}

void ActionEditor::setFormWindow(QDesignerFormWindowInterface *formWindow)
{
   if (formWindow != nullptr && formWindow->mainContainer() == nullptr) {
      formWindow = nullptr;
   }

   // we do NOT rely on this function to update the action editor
   if (m_formWindow == formWindow) {
      return;
   }

   if (m_formWindow != nullptr) {
      const ActionList actionList = m_formWindow->mainContainer()->findChildren<QAction *>();
      for (QAction *action : actionList) {
         disconnect(action, &QAction::changed, this, &ActionEditor::slotActionChanged);
      }
   }

   m_formWindow = formWindow;

   m_actionView->model()->clearActions();
   m_actionEdit->setEnabled(false);
   m_actionCopy->setEnabled(false);
   m_actionCut->setEnabled(false);
   m_actionDelete->setEnabled(false);

   if (!formWindow || !formWindow->mainContainer()) {
      m_actionNew->setEnabled(false);
      m_filterWidget->setEnabled(false);

      return;
   }

   m_actionNew->setEnabled(true);
   m_filterWidget->setEnabled(true);

   const ActionList actionList = formWindow->mainContainer()->findChildren<QAction *>();

   for (QAction *action : actionList)
      if (!action->isSeparator() && core()->metaDataBase()->item(action) != nullptr) {
         // Show unless it has a menu. However, listen for change on menu actions also as it might be removed
         if (!action->menu()) {
            m_actionView->model()->addAction(action);
         }
         connect(action, &QAction::changed, this, &ActionEditor::slotActionChanged);
      }

   setFilter(m_filter);
}

void  ActionEditor::slotSelectionChanged(const QItemSelection &selected, const QItemSelection & /*deselected*/)
{
   const bool hasSelection = !selected.indexes().empty();

   m_actionCopy->setEnabled(hasSelection);
   m_actionCut->setEnabled(hasSelection);
   m_actionDelete->setEnabled(hasSelection);
}

void ActionEditor::slotCurrentItemChanged(QAction *action)
{
   QDesignerFormWindowInterface *fw = formWindow();
   if (! fw) {
      return;
   }

   const bool hasCurrentAction = (action != nullptr);
   m_actionEdit->setEnabled(hasCurrentAction);

   if (! action) {
      fw->clearSelection();
      return;
   }

   QDesignerObjectInspector *oi = dynamic_cast<QDesignerObjectInspector *>(core()->objectInspector());

   if (action->associatedWidgets().empty()) {
      // Special case: action not in object tree. Deselect all and set in property editor
      fw->clearSelection(false);
      if (oi) {
         oi->clearSelection();
      }
      core()->propertyEditor()->setObject(action);
   } else {
      if (oi) {
         oi->selectObject(action);
      }
   }
}

void ActionEditor::slotActionChanged()
{
   QAction *action = dynamic_cast<QAction *>(sender());
   Q_ASSERT(action != nullptr);

   ActionModel *model = m_actionView->model();
   const int row = model->findAction(action);

   if (row == -1) {
      if (action->menu() == nullptr) {
         // action got its menu deleted, create item
         model->addAction(action);
      }

   } else if (action->menu() != nullptr) {
      // action got its menu created, remove item
      model->removeRow(row);

   } else {
      // action text or icon changed, update item
      model->update(row);
   }
}

QDesignerFormEditorInterface *ActionEditor::core() const
{
   return m_core;
}

QString ActionEditor::filter() const
{
   return m_filter;
}

void ActionEditor::setFilter(const QString &f)
{
   m_filter = f;
   m_actionView->filter(m_filter);
}

// Set changed state of icon property,  reset when icon is cleared
static void refreshIconPropertyChanged(const QAction *action, QDesignerPropertySheetExtension *sheet)
{
   sheet->setChanged(sheet->indexOf(iconPropertyC), !action->icon().isNull());
}

void ActionEditor::manageAction(QAction *action)
{
   action->setParent(formWindow()->mainContainer());
   core()->metaDataBase()->add(action);

   if (action->isSeparator() || action->menu() != nullptr) {
      return;
   }

   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), action);
   sheet->setChanged(sheet->indexOf(objectNamePropertyC), true);
   sheet->setChanged(sheet->indexOf(textPropertyC), true);
   refreshIconPropertyChanged(action, sheet);

   m_actionView->setCurrentIndex(m_actionView->model()->addAction(action));
   connect(action, &QAction::changed, this, &ActionEditor::slotActionChanged);
}

void ActionEditor::unmanageAction(QAction *action)
{
   core()->metaDataBase()->remove(action);
   action->setParent(nullptr);

   disconnect(action, &QAction::changed, this, &ActionEditor::slotActionChanged);

   const int row = m_actionView->model()->findAction(action);
   if (row != -1) {
      m_actionView->model()->remove(row);
   }
}

// Set an initial property and mark it as changed in the sheet
static void setInitialProperty(QDesignerPropertySheetExtension *sheet, const QString &name, const QVariant &value)
{
   const int index = sheet->indexOf(name);
   Q_ASSERT(index != -1);
   sheet->setProperty(index, value);
   sheet->setChanged(index, true);
}

void ActionEditor::slotNewAction()
{
   NewActionDialog dlg(this);
   dlg.setWindowTitle(tr("New action"));

   if (dlg.exec() == QDialog::Accepted) {
      const ActionData actionData = dlg.actionData();
      m_actionView->clearSelection();
      QAction *action = new QAction(formWindow());
      action->setObjectName(actionData.name);
      formWindow()->ensureUniqueObjectName(action);
      action->setText(actionData.text);

      QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), action);
      if (!actionData.toolTip.isEmpty()) {
         setInitialProperty(sheet, toolTipPropertyC, actionData.toolTip);
      }

      if (actionData.checkable) {
         setInitialProperty(sheet, checkablePropertyC, QVariant(true));
      }

      if (!actionData.keysequence.value().isEmpty()) {
         setInitialProperty(sheet, shortcutPropertyC, QVariant::fromValue(actionData.keysequence));
      }

      sheet->setProperty(sheet->indexOf(iconPropertyC), QVariant::fromValue(actionData.icon));

      AddActionCommand *cmd = new AddActionCommand(formWindow());
      cmd->init(action);
      formWindow()->commandHistory()->push(cmd);
   }
}

// return a FormWindow command to apply an icon or a reset command in case it
//  is empty.

static QDesignerFormWindowCommand *setIconPropertyCommand(const PropertySheetIconValue &newIcon, QAction *action,
   QDesignerFormWindowInterface *fw)
{
   const QString iconProperty = iconPropertyC;
   if (newIcon.isEmpty()) {
      ResetPropertyCommand *cmd = new ResetPropertyCommand(fw);
      cmd->init(action, iconProperty);
      return cmd;
   }
   SetPropertyCommand *cmd = new SetPropertyCommand(fw);
   cmd->init(action, iconProperty, QVariant::fromValue(newIcon));
   return cmd;
}

// return a FormWindow command to apply a QKeySequence or a reset command
// in case it is empty.

static QDesignerFormWindowCommand *setKeySequencePropertyCommand(const PropertySheetKeySequenceValue &ks, QAction *action,
   QDesignerFormWindowInterface *fw)
{
   const QString shortcutProperty = shortcutPropertyC;
   if (ks.value().isEmpty()) {
      ResetPropertyCommand *cmd = new ResetPropertyCommand(fw);
      cmd->init(action, shortcutProperty);
      return cmd;
   }
   SetPropertyCommand *cmd = new SetPropertyCommand(fw);
   cmd->init(action, shortcutProperty, QVariant::fromValue(ks));
   return cmd;
}

// return a FormWindow command to apply a POD value or reset command in case
// it is equal to the default value.

template <class T>
QDesignerFormWindowCommand *setPropertyCommand(const QString &name, T value, T defaultValue,
   QObject *o, QDesignerFormWindowInterface *fw)
{
   if (value == defaultValue) {
      ResetPropertyCommand *cmd = new ResetPropertyCommand(fw);
      cmd->init(o, name);
      return cmd;
   }

   SetPropertyCommand *cmd = new SetPropertyCommand(fw);
   cmd->init(o, name, QVariant(value));
   return cmd;
}

// Return the text value of a string property via PropertySheetStringValue
static inline QString textPropertyValue(const QDesignerPropertySheetExtension *sheet, const QString &name)
{
   const int index = sheet->indexOf(name);
   Q_ASSERT(index != -1);

   QVariant data = sheet->property(index);
   const PropertySheetStringValue ps = data.value<PropertySheetStringValue>();

   return ps.value();
}

void ActionEditor::editAction(QAction *action)
{
   if (!action) {
      return;
   }

   NewActionDialog dlg(this);
   dlg.setWindowTitle(tr("Edit action"));

   ActionData oldActionData;
   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), action);
   oldActionData.name = action->objectName();
   oldActionData.text = action->text();
   oldActionData.toolTip = textPropertyValue(sheet, toolTipPropertyC);

   QVariant data = sheet->property(sheet->indexOf(iconPropertyC));
   oldActionData.icon = data.value<PropertySheetIconValue>();

   oldActionData.keysequence = ActionModel::actionShortCut(sheet);
   oldActionData.checkable =  action->isCheckable();
   dlg.setActionData(oldActionData);

   if (! dlg.exec()) {
      return;
   }

   // figure out changes and whether to start a macro
   const ActionData newActionData = dlg.actionData();
   const unsigned changeMask = newActionData.compare(oldActionData);

   if (changeMask == 0u) {
      return;
   }

   const bool severalChanges = (changeMask != ActionData::TextChanged) && (changeMask != ActionData::NameChanged)
      && (changeMask != ActionData::ToolTipChanged)   && (changeMask != ActionData::IconChanged)
      && (changeMask != ActionData::CheckableChanged) && (changeMask != ActionData::KeysequenceChanged);

   QDesignerFormWindowInterface *fw = formWindow();
   QUndoStack *undoStack = fw->commandHistory();
   if (severalChanges) {
      fw->beginCommand(QString("Edit action"));
   }

   if (changeMask & ActionData::NameChanged) {
      undoStack->push(createTextPropertyCommand(objectNamePropertyC, newActionData.name, action, fw));
   }

   if (changeMask & ActionData::TextChanged) {
      undoStack->push(createTextPropertyCommand(textPropertyC, newActionData.text, action, fw));
   }

   if (changeMask & ActionData::ToolTipChanged) {
      undoStack->push(createTextPropertyCommand(toolTipPropertyC, newActionData.toolTip, action, fw));
   }

   if (changeMask & ActionData::IconChanged) {
      undoStack->push(setIconPropertyCommand(newActionData.icon, action, fw));
   }

   if (changeMask & ActionData::CheckableChanged) {
      undoStack->push(setPropertyCommand(checkablePropertyC, newActionData.checkable, false, action, fw));
   }

   if (changeMask & ActionData::KeysequenceChanged) {
      undoStack->push(setKeySequencePropertyCommand(newActionData.keysequence, action, fw));
   }

   if (severalChanges) {
      fw->endCommand();
   }
}

void ActionEditor::editCurrentAction()
{
   if (QAction *a = m_actionView->currentAction()) {
      editAction(a);
   }
}

void ActionEditor::navigateToSlotCurrentAction()
{
   if (QAction *a = m_actionView->currentAction()) {
      QDesignerTaskMenu::navigateToSlot(m_core, a, QString("triggered()"));
   }
}

void ActionEditor::deleteActions(QDesignerFormWindowInterface *fw, const ActionList &actions)
{
   // We need a macro even in the case of single action because the commands might cause the
   // scheduling of other commands (signal slots connections)
   const QString description = actions.size() == 1 ?
      tr("Remove action '%1'").formatArg(actions.front()->objectName()) : tr("Remove actions");
   fw->beginCommand(description);
   for (QAction *action : actions) {
      RemoveActionCommand *cmd = new RemoveActionCommand(fw);
      cmd->init(action);
      fw->commandHistory()->push(cmd);
   }
   fw->endCommand();
}

void ActionEditor::copyActions(QDesignerFormWindowInterface *fwi, const ActionList &actions)
{
   FormWindowBase *fw = dynamic_cast<FormWindowBase *>(fwi);
   if (!fw ) {
      return;
   }

   FormBuilderClipboard clipboard;
   clipboard.m_actions = actions;

   if (clipboard.empty()) {
      return;
   }

   QEditorFormBuilder *formBuilder = fw->createFormBuilder();
   Q_ASSERT(formBuilder);

   QBuffer buffer;
   if (buffer.open(QIODevice::WriteOnly)) {
      if (formBuilder->copy(&buffer, clipboard)) {
         qApp->clipboard()->setText(QString::fromUtf8(buffer.buffer()), QClipboard::Clipboard);
      }
   }

   delete formBuilder;
}

void ActionEditor::slotDelete()
{
   QDesignerFormWindowInterface *fw =  formWindow();
   if (! fw) {
      return;
   }

   const ActionView::ActionList selection = m_actionView->selectedActions();
   if (selection.empty()) {
      return;
   }

   deleteActions(fw, selection);
}

QString ActionEditor::actionTextToName(const QString &text, const QString &prefix)
{
   QString name = text;

   if (name.isEmpty()) {
      return QString();
   }

   name.replace(0, 1, name.at(0).toUpper());
   name.prepend(prefix);

   const QString underscore = QChar('_');

   name.replace(QRegularExpression("[^a-zA-Z_0-9]"), underscore);
   name.replace(QRegularExpression("__*"), underscore);

   if (name.endsWith(underscore.at(0))) {
      name.truncate(name.size() - 1);
   }

   return name;
}

void  ActionEditor::resourceImageDropped(const QString &path, QAction *action)
{
   QDesignerFormWindowInterface *fw =  formWindow();

   if (! fw) {
      return;
   }

   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), action);

   QVariant data = sheet->property(sheet->indexOf(iconPropertyC));
   const PropertySheetIconValue oldIcon = data.value<PropertySheetIconValue>();

   PropertySheetIconValue newIcon;
   newIcon.setPixmap(QIcon::Normal, QIcon::Off, PropertySheetPixmapValue(path));

   if (newIcon.paths().isEmpty() || newIcon.paths() == oldIcon.paths()) {
      return;
   }

   fw->commandHistory()->push(setIconPropertyCommand(newIcon, action, fw));
}

void ActionEditor::mainContainerChanged()
{
   // Invalidate references to objects kept in model
   if (sender() == formWindow()) {
      setFormWindow(nullptr);
   }
}

void ActionEditor::slotViewMode(QAction *a)
{
   m_actionView->setViewMode(a->data().toInt());
   updateViewModeActions();
}

void ActionEditor::slotSelectAssociatedWidget(QWidget *w)
{
   QDesignerFormWindowInterface *fw = formWindow();
   if (! fw ) {
      return;
   }

   QDesignerObjectInspector *oi = dynamic_cast<QDesignerObjectInspector *>(core()->objectInspector());
   if (! oi) {
      return;
   }

   fw->clearSelection(); // Actually, there are no widgets selected due to focus in event handling. Just to be sure.
   oi->selectObject(w);
}

void ActionEditor::restoreSettings()
{
   QDesignerSettingsInterface *settings = m_core->settingsManager();
   m_actionView->setViewMode(settings->value(actionEditorViewModeKey, 0).toInt());
   updateViewModeActions();
}

void ActionEditor::saveSettings()
{
   QDesignerSettingsInterface *settings = m_core->settingsManager();
   settings->setValue(actionEditorViewModeKey, m_actionView->viewMode());
}

void ActionEditor::updateViewModeActions()
{
   switch (m_actionView->viewMode()) {
      case ActionView::ViewMode::IconView:
         m_iconViewAction->setChecked(true);
         break;

      case ActionView::ViewMode::DetailedView:
         m_listViewAction->setChecked(true);
         break;
   }
}

void ActionEditor::slotCopy()
{
   QDesignerFormWindowInterface *fw = formWindow();
   if (!fw ) {
      return;
   }

   const ActionView::ActionList selection = m_actionView->selectedActions();
   if (selection.empty()) {
      return;
   }

   copyActions(fw, selection);
}

void ActionEditor::slotCut()
{
   QDesignerFormWindowInterface *fw = formWindow();
   if (!fw ) {
      return;
   }

   const ActionView::ActionList selection = m_actionView->selectedActions();
   if (selection.empty()) {
      return;
   }

   copyActions(fw, selection);
   deleteActions(fw, selection);
}

void ActionEditor::slotPaste()
{
   FormWindowBase *fw = dynamic_cast<FormWindowBase *>(formWindow());
   if (!fw) {
      return;
   }
   m_actionView->clearSelection();
   fw->paste(FormWindowBase::PasteActionsOnly);
}

void ActionEditor::slotContextMenuRequested(QContextMenuEvent *e, QAction *item)
{
   // set up signal mapper
   if (! m_selectAssociatedWidgetsMapper) {
      m_selectAssociatedWidgetsMapper = new QSignalMapper(this);

      connect(m_selectAssociatedWidgetsMapper, cs_mp_cast<QWidget *>(&QSignalMapper::mapped),
            this, &ActionEditor::slotSelectAssociatedWidget);
   }

   QMenu menu(this);
   menu.addAction(m_actionNew);
   menu.addSeparator();
   menu.addAction(m_actionEdit);

   if (QDesignerTaskMenu::isSlotNavigationEnabled(m_core)) {
      menu.addAction(m_actionNavigateToSlot);
   }

   // Associated Widgets
   if (QAction *action = m_actionView->currentAction()) {
      const QWidgetList associatedWidgets = ActionModel::associatedWidgets(action);

      if (! associatedWidgets.empty()) {
         QMenu *associatedWidgetsSubMenu =  menu.addMenu(tr("Used In"));

         for (QWidget *w : associatedWidgets) {
            QAction *action = associatedWidgetsSubMenu->addAction(w->objectName());

            m_selectAssociatedWidgetsMapper->setMapping(action, w);
            connect(action, &QAction::triggered,
               m_selectAssociatedWidgetsMapper, cs_mp_cast<>(&QSignalMapper::map));
         }
      }
   }

   menu.addSeparator();
   menu.addAction(m_actionCut);
   menu.addAction(m_actionCopy);
   menu.addAction(m_actionPaste);
   menu.addAction(m_actionSelectAll);
   menu.addAction(m_actionDelete);

   menu.addSeparator();

   menu.addAction(m_iconViewAction);
   menu.addAction(m_listViewAction);

   emit contextMenuRequested(&menu, item);

   menu.exec(e->globalPos());
   e->accept();
}

}   // end namespace qdesigner_internal
