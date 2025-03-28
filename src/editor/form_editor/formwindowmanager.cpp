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

#include <abstract_dialoggui.h>
#include <abstract_integration.h>
#include <abstract_language.h>
#include <abstract_widgetbox.h>
#include <action_editor.h>
#include <connection_edit.h>
#include <container.h>
#include <designer_command.h>
#include <designer_dnditem.h>
#include <designer_object_inspector.h>
#include <extension.h>
#include <formeditor.h>
#include <formwindow.h>
#include <formwindow_dnditem.h>
#include <formwindowmanager.h>
#include <formwindowsettings.h>
#include <layout_info.h>
#include <layout_widget.h>
#include <preview_manager.h>
#include <previewactiongroup.h>
#include <resource_model.h>
#include <widgetfactory.h>
#include <widgetselection.h>

#include <iconloader_p.h>
#include <shared_settings_p.h>
#include <spacer_widget_p.h>
#include <widgetdatabase_p.h>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSizeGrip>
#include <QSplitter>
#include <QUndoGroup>

static inline QString whatsThisFrom(const QString &str)   /// ### implement me
{
   return str;
}

// find the first child of w in a sequence
template <class Iterator>
static inline Iterator findFirstChildOf(Iterator it, Iterator end, const QWidget *w)
{
   for  (; it != end; ++it) {
      if (w->isAncestorOf(*it)) {
         return  it;
      }
   }

   return it;
}

namespace qdesigner_internal {

FormWindowManager::FormWindowManager(QDesignerFormEditorInterface *core, QObject *parent)
   : QDesignerFormWindowManager(parent), m_core(core), m_activeFormWindow(nullptr),
     m_previewManager(new PreviewManager(PreviewManager::SingleFormNonModalPreview, this)),
     m_createLayoutContext(LayoutContainer), m_morphLayoutContainer(nullptr),
     m_actionGroupPreviewInStyle(nullptr), m_actionShowFormWindowSettingsDialog(nullptr)
{
   setupActions();
   qApp->installEventFilter(this);
}

FormWindowManager::~FormWindowManager()
{
   qDeleteAll(m_formWindows);
}

QDesignerFormEditorInterface *FormWindowManager::core() const
{
   return m_core;
}

QDesignerFormWindowInterface *FormWindowManager::activeFormWindow() const
{
   return m_activeFormWindow;
}

int FormWindowManager::formWindowCount() const
{
   return m_formWindows.size();
}

QDesignerFormWindowInterface *FormWindowManager::formWindow(int index) const
{
   return m_formWindows.at(index);
}

bool FormWindowManager::eventFilter(QObject *o, QEvent *e)
{
   if (! o->isWidgetType()) {
      return false;
   }

   // If we don't have an active form, we only listen for WindowActivate to speed up integrations
   const QEvent::Type eventType = e->type();

   if (m_activeFormWindow == nullptr && eventType != QEvent::WindowActivate) {
      return false;
   }

   switch (eventType) {
      case QEvent::Create:
      case QEvent::Destroy:
      case QEvent::ActionAdded:
      case QEvent::ActionChanged:
      case QEvent::ActionRemoved:
      case QEvent::ChildAdded:
      case QEvent::ChildPolished:
      case QEvent::ChildRemoved:
      case QEvent::Clipboard:
      case QEvent::ContentsRectChange:
      case QEvent::DeferredDelete:
      case QEvent::FileOpen:
      case QEvent::LanguageChange:
      case QEvent::MetaCall:
      case QEvent::ModifiedChange:
      case QEvent::Paint:
      case QEvent::PaletteChange:
      case QEvent::ParentAboutToChange:
      case QEvent::ParentChange:
      case QEvent::Polish:
      case QEvent::PolishRequest:
      case QEvent::QueryWhatsThis:
      case QEvent::StatusTip:
      case QEvent::StyleChange:
      case QEvent::Timer:
      case QEvent::ToolBarChange:
      case QEvent::ToolTip:
      case QEvent::WhatsThis:
      case QEvent::WhatsThisClicked:
      case QEvent::WinIdChange:
      case QEvent::DynamicPropertyChange:
      case QEvent::HoverEnter:
      case QEvent::HoverLeave:
      case QEvent::HoverMove:
      case QEvent::AcceptDropsChange:
         return false;

      default:
         break;
   }

   QWidget *widget = static_cast<QWidget *>(o);

   if (dynamic_cast<WidgetHandle *>(widget)) {
      // remove me ?
      return false;
   }

   FormWindow *fw = FormWindow::findFormWindow(widget);
   if (fw == nullptr) {
      return false;
   }

   if (QWidget *managedWidget = findManagedWidget(fw, widget)) {
      // Prevent MDI subwindows from being closed by clicking at the title bar
      if (managedWidget != widget && eventType == QEvent::Close) {
         e->ignore();
         return true;
      }

      switch (eventType) {

         case QEvent::WindowActivate: {
            if (fw->parentWidget()->isWindow() && fw->isMainContainer(managedWidget) && activeFormWindow() != fw) {
               setActiveFormWindow(fw);
            }
         }
         break;

         case QEvent::WindowDeactivate: {
            if (o == fw && o == activeFormWindow()) {
               fw->repaintSelection();
            }
         }
         break;

         case QEvent::KeyPress: {
            QKeyEvent *ke = static_cast<QKeyEvent *>(e);
            if (ke->key() == Qt::Key_Escape) {
               ke->accept();
               return true;
            }
         }
         [[fallthrough]];

         // Embedded Design: Drop on different form: Make sure the right form
         // window/device is active before having the widget created by the factory

         case QEvent::Drop:
            if (activeFormWindow() != fw) {
               setActiveFormWindow(fw);
            }
            [[fallthrough]];

         default: {
            if (fw->handleEvent(widget, managedWidget, e)) {
               return true;
            }
         }
         break;

      }
   }

   return false;
}

void FormWindowManager::addFormWindow(QDesignerFormWindowInterface *w)
{
   FormWindow *formWindow = dynamic_cast<FormWindow *>(w);

   if (! formWindow || m_formWindows.contains(formWindow)) {
      return;
   }

   connect(formWindow, &QDesignerFormWindowInterface::selectionChanged,
      this, &FormWindowManager::slotUpdateActions);

   connect(formWindow->commandHistory(), &QUndoStack::indexChanged,
      this, &FormWindowManager::slotUpdateActions);

   connect(formWindow, &QDesignerFormWindowInterface::toolChanged,
      this, &FormWindowManager::slotUpdateActions);

   if (ActionEditor *ae = dynamic_cast<ActionEditor *>(m_core->actionEditor())) {
      connect(w, &QDesignerFormWindowInterface::mainContainerChanged,
         ae, &ActionEditor::mainContainerChanged);
   }
   if (QDesignerObjectInspector *oi = dynamic_cast<QDesignerObjectInspector *>(m_core->objectInspector()))
      connect(w, &QDesignerFormWindowInterface::mainContainerChanged,
         oi, &QDesignerObjectInspector::mainContainerChanged);

   m_formWindows.append(formWindow);
   emit formWindowAdded(formWindow);
}

void FormWindowManager::removeFormWindow(QDesignerFormWindowInterface *w)
{
   FormWindow *formWindow = dynamic_cast<FormWindow *>(w);

   int idx = m_formWindows.indexOf(formWindow);
   if (!formWindow || idx == -1) {
      return;
   }

   formWindow->disconnect(this);
   m_formWindows.removeAt(idx);
   emit formWindowRemoved(formWindow);

   if (formWindow == m_activeFormWindow) {
      setActiveFormWindow(nullptr);
   }

   // Make sure that widget box is enabled by default
   if (m_formWindows.size() == 0 && m_core->widgetBox()) {
      m_core->widgetBox()->setEnabled(true);
   }
}

void FormWindowManager::setActiveFormWindow(QDesignerFormWindowInterface *w)
{
   FormWindow *formWindow = dynamic_cast<FormWindow *>(w);

   if (formWindow == m_activeFormWindow) {
      return;
   }

   FormWindow *old = m_activeFormWindow;

   m_activeFormWindow = formWindow;

   QtResourceSet *resourceSet = nullptr;
   if (formWindow) {
      resourceSet = formWindow->resourceSet();
   }
   m_core->resourceModel()->setCurrentResourceSet(resourceSet);

   slotUpdateActions();

   if (m_activeFormWindow) {
      m_activeFormWindow->repaintSelection();

      if (old) {
         old->repaintSelection();
      }
   }

   emit activeFormWindowChanged(m_activeFormWindow);

   if (m_activeFormWindow != nullptr) {
      m_activeFormWindow->emitSelectionChanged();
      m_activeFormWindow->commandHistory()->setActive();

      // Trigger setActiveSubWindow on mdi area unless we are in toplevel mode
      QMdiSubWindow *mdiSubWindow = nullptr;

      if (QWidget *formwindow = m_activeFormWindow->parentWidget()) {
         mdiSubWindow = dynamic_cast<QMdiSubWindow *>(formwindow->parentWidget());
      }

      if (mdiSubWindow) {
         for (QWidget *parent = mdiSubWindow->parentWidget(); parent; parent = parent->parentWidget()) {
            if (QMdiArea *mdiArea = dynamic_cast<QMdiArea *>(parent)) {
               mdiArea->setActiveSubWindow(mdiSubWindow);
               break;
            }
         }
      }
   }
}

void FormWindowManager::closeAllPreviews()
{
   m_previewManager->closeAllPreviews();
}

QWidget *FormWindowManager::findManagedWidget(FormWindow *fw, QWidget *w)
{
   while (w && w != fw) {
      if (fw->isManaged(w)) {
         break;
      }
      w = w->parentWidget();
   }
   return w;
}

void FormWindowManager::setupActions()
{
   const QIcon cutIcon = QIcon::fromTheme(QString("edit-cut"), createIconSet(QString("editcut.png")));
   m_actionCut = new QAction(cutIcon, tr("Cu&t"), this);
   m_actionCut->setObjectName(QString("__qt_cut_action"));
   m_actionCut->setShortcut(QKeySequence::Cut);
   m_actionCut->setStatusTip(tr("Cuts the selected widgets and puts them on the clipboard"));
   m_actionCut->setWhatsThis(whatsThisFrom(QString("Edit|Cut")));
   connect(m_actionCut, &QAction::triggered, this, &FormWindowManager::slotActionCutActivated);
   m_actionCut->setEnabled(false);

   const QIcon copyIcon = QIcon::fromTheme(QString("edit-copy"), createIconSet(QString("editcopy.png")));
   m_actionCopy = new QAction(copyIcon, tr("&Copy"), this);
   m_actionCopy->setObjectName(QString("__qt_copy_action"));
   m_actionCopy->setShortcut(QKeySequence::Copy);
   m_actionCopy->setStatusTip(tr("Copies the selected widgets to the clipboard"));
   m_actionCopy->setWhatsThis(whatsThisFrom(QString("Edit|Copy")));
   connect(m_actionCopy, &QAction::triggered, this, &FormWindowManager::slotActionCopyActivated);
   m_actionCopy->setEnabled(false);

   const QIcon pasteIcon = QIcon::fromTheme(QString("edit-paste"), createIconSet(QString("editpaste.png")));
   m_actionPaste = new QAction(pasteIcon, tr("&Paste"), this);
   m_actionPaste->setObjectName(QString("__qt_paste_action"));
   m_actionPaste->setShortcut(QKeySequence::Paste);
   m_actionPaste->setStatusTip(tr("Pastes the clipboard's contents"));
   m_actionPaste->setWhatsThis(whatsThisFrom(QString("Edit|Paste")));
   connect(m_actionPaste, &QAction::triggered, this, &FormWindowManager::slotActionPasteActivated);
   m_actionPaste->setEnabled(false);

   m_actionDelete = new QAction(QIcon::fromTheme(QString("edit-delete")), tr("&Delete"), this);
   m_actionDelete->setObjectName(QString("__qt_delete_action"));
   m_actionDelete->setStatusTip(tr("Deletes the selected widgets"));
   m_actionDelete->setWhatsThis(whatsThisFrom(QString("Edit|Delete")));
   connect(m_actionDelete, &QAction::triggered, this, &FormWindowManager::slotActionDeleteActivated);
   m_actionDelete->setEnabled(false);

   m_actionSelectAll = new QAction(tr("Select &All"), this);
   m_actionSelectAll->setObjectName(QString("__qt_select_all_action"));
   m_actionSelectAll->setShortcut(QKeySequence::SelectAll);
   m_actionSelectAll->setStatusTip(tr("Selects all widgets"));
   m_actionSelectAll->setWhatsThis(whatsThisFrom(QString("Edit|Select All")));
   connect(m_actionSelectAll, &QAction::triggered, this, &FormWindowManager::slotActionSelectAllActivated);
   m_actionSelectAll->setEnabled(false);

   m_actionRaise = new QAction(createIconSet(QString("editraise.png")), tr("Bring to &Front"), this);
   m_actionRaise->setObjectName(QString("__qt_raise_action"));
   m_actionRaise->setShortcut(cs_enum_cast(Qt::ControlModifier) + cs_enum_cast(Qt::Key_L));
   m_actionRaise->setStatusTip(tr("Raises the selected widgets"));
   m_actionRaise->setWhatsThis(tr("Raises the selected widgets"));
   connect(m_actionRaise, &QAction::triggered, this, &FormWindowManager::slotActionRaiseActivated);
   m_actionRaise->setEnabled(false);

   m_actionLower = new QAction(createIconSet(QString("editlower.png")), tr("Send to &Back"), this);
   m_actionLower->setObjectName(QString("__qt_lower_action"));
   m_actionLower->setShortcut(cs_enum_cast(Qt::ControlModifier) + cs_enum_cast(Qt::Key_K));
   m_actionLower->setStatusTip(tr("Lowers the selected widgets"));
   m_actionLower->setWhatsThis(tr("Lowers the selected widgets"));
   connect(m_actionLower, &QAction::triggered, this, &FormWindowManager::slotActionLowerActivated);
   m_actionLower->setEnabled(false);

   m_actionAdjustSize = new QAction(createIconSet(QString("adjustsize.png")), tr("Adjust &Size"), this);
   m_actionAdjustSize->setObjectName(QString("__qt_adjust_size_action"));
   m_actionAdjustSize->setShortcut(cs_enum_cast(Qt::ControlModifier) + cs_enum_cast(Qt::Key_J));
   m_actionAdjustSize->setStatusTip(tr("Adjusts the size of the selected widget"));
   m_actionAdjustSize->setWhatsThis(whatsThisFrom(QString("Layout|Adjust Size")));
   connect(m_actionAdjustSize, &QAction::triggered, this, &FormWindowManager::slotActionAdjustSizeActivated);
   m_actionAdjustSize->setEnabled(false);


   m_actionHorizontalLayout = new QAction(createIconSet(QString("edithlayout.png")), tr("Lay Out &Horizontally"), this);
   m_actionHorizontalLayout->setObjectName(QString("__qt_horizontal_layout_action"));
   m_actionHorizontalLayout->setShortcut(cs_enum_cast(Qt::ControlModifier) + cs_enum_cast(Qt::Key_1));
   m_actionHorizontalLayout->setStatusTip(tr("Lays out the selected widgets horizontally"));
   m_actionHorizontalLayout->setWhatsThis(whatsThisFrom(QString("Layout|Lay Out Horizontally")));
   m_actionHorizontalLayout->setData(LayoutInfo::HBox);
   m_actionHorizontalLayout->setEnabled(false);
   connect(m_actionHorizontalLayout, &QAction::triggered, this, &FormWindowManager::createLayout);

   m_actionVerticalLayout = new QAction(createIconSet(QString("editvlayout.png")), tr("Lay Out &Vertically"), this);
   m_actionVerticalLayout->setObjectName(QString("__qt_vertical_layout_action"));
   m_actionVerticalLayout->setShortcut(cs_enum_cast(Qt::ControlModifier) + cs_enum_cast(Qt::Key_2));
   m_actionVerticalLayout->setStatusTip(tr("Lays out the selected widgets vertically"));
   m_actionVerticalLayout->setWhatsThis(whatsThisFrom(QString("Layout|Lay Out Vertically")));
   m_actionVerticalLayout->setData(LayoutInfo::VBox);
   m_actionVerticalLayout->setEnabled(false);
   connect(m_actionVerticalLayout, &QAction::triggered, this, &FormWindowManager::createLayout);

   QIcon formIcon = QIcon::fromTheme(QString("designer-form-layout"), createIconSet(QString("editform.png")));
   m_actionFormLayout = new QAction(formIcon, tr("Lay Out in a &Form Layout"), this);
   m_actionFormLayout->setObjectName(QString("__qt_form_layout_action"));
   m_actionFormLayout->setShortcut(cs_enum_cast(Qt::ControlModifier) + cs_enum_cast(Qt::Key_6));
   m_actionFormLayout->setStatusTip(tr("Lays out the selected widgets in a form layout"));
   m_actionFormLayout->setWhatsThis(whatsThisFrom(QString("Layout|Lay Out in a Form")));
   m_actionFormLayout->setData(LayoutInfo::Form);
   m_actionFormLayout->setEnabled(false);
   connect(m_actionFormLayout, &QAction::triggered, this, &FormWindowManager::createLayout);

   m_actionGridLayout = new QAction(createIconSet(QString("editgrid.png")), tr("Lay Out in a &Grid"), this);
   m_actionGridLayout->setObjectName(QString("__qt_grid_layout_action"));
   m_actionGridLayout->setShortcut(cs_enum_cast(Qt::ControlModifier) + cs_enum_cast(Qt::Key_5));
   m_actionGridLayout->setStatusTip(tr("Lays out the selected widgets in a grid"));
   m_actionGridLayout->setWhatsThis(whatsThisFrom(QString("Layout|Lay Out in a Grid")));
   m_actionGridLayout->setData(LayoutInfo::Grid);
   m_actionGridLayout->setEnabled(false);
   connect(m_actionGridLayout, &QAction::triggered, this, &FormWindowManager::createLayout);

   m_actionSplitHorizontal = new QAction(createIconSet(QString("edithlayoutsplit.png")),
      tr("Lay Out Horizontally in S&plitter"), this);
   m_actionSplitHorizontal->setObjectName(QString("__qt_split_horizontal_action"));
   m_actionSplitHorizontal->setShortcut(cs_enum_cast(Qt::ControlModifier) + cs_enum_cast(Qt::Key_3));
   m_actionSplitHorizontal->setStatusTip(tr("Lays out the selected widgets horizontally in a splitter"));
   m_actionSplitHorizontal->setWhatsThis(whatsThisFrom(QString("Layout|Lay Out Horizontally in Splitter")));
   m_actionSplitHorizontal->setData(LayoutInfo::HSplitter);
   m_actionSplitHorizontal->setEnabled(false);
   connect(m_actionSplitHorizontal, &QAction::triggered, this, &FormWindowManager::createLayout);

   m_actionSplitVertical = new QAction(createIconSet(QString("editvlayoutsplit.png")),
      tr("Lay Out Vertically in Sp&litter"), this);
   m_actionSplitVertical->setObjectName(QString("__qt_split_vertical_action"));
   m_actionSplitVertical->setShortcut(cs_enum_cast(Qt::ControlModifier) + cs_enum_cast(Qt::Key_4));
   m_actionSplitVertical->setStatusTip(tr("Lays out the selected widgets vertically in a splitter"));
   m_actionSplitVertical->setWhatsThis(whatsThisFrom(QString("Layout|Lay Out Vertically in Splitter")));
   connect(m_actionSplitVertical, &QAction::triggered, this, &FormWindowManager::createLayout);
   m_actionSplitVertical->setData(LayoutInfo::VSplitter);

   m_actionSplitVertical->setEnabled(false);

   m_actionBreakLayout = new QAction(createIconSet(QString("editbreaklayout.png")), tr("&Break Layout"), this);
   m_actionBreakLayout->setObjectName(QString("__qt_break_layout_action"));
   m_actionBreakLayout->setShortcut(cs_enum_cast(Qt::ControlModifier) + cs_enum_cast(Qt::Key_0));
   m_actionBreakLayout->setStatusTip(tr("Breaks the selected layout"));
   m_actionBreakLayout->setWhatsThis(whatsThisFrom(QString("Layout|Break Layout")));
   connect(m_actionBreakLayout, &QAction::triggered, this, &FormWindowManager::slotActionBreakLayoutActivated);
   m_actionBreakLayout->setEnabled(false);

   m_actionSimplifyLayout = new QAction(tr("Si&mplify Grid Layout"), this);
   m_actionSimplifyLayout->setObjectName(QString("__qt_simplify_layout_action"));
   m_actionSimplifyLayout->setStatusTip(tr("Removes empty columns and rows"));
   m_actionSimplifyLayout->setWhatsThis(whatsThisFrom(QString("Layout|Simplify Layout")));
   connect(m_actionSimplifyLayout, &QAction::triggered, this, &FormWindowManager::slotActionSimplifyLayoutActivated);
   m_actionSimplifyLayout->setEnabled(false);

   m_actionDefaultPreview = new QAction(tr("&Preview..."), this);
   m_actionDefaultPreview->setObjectName(QString("__qt_default_preview_action"));
   m_actionDefaultPreview->setStatusTip(tr("Preview current form"));
   m_actionDefaultPreview->setWhatsThis(whatsThisFrom(QString("Form|Preview")));
   connect(m_actionDefaultPreview, &QAction::triggered,
      this, &FormWindowManager::showPreview);

   m_undoGroup = new QUndoGroup(this);

   m_actionUndo = m_undoGroup->createUndoAction(this);
   m_actionUndo->setEnabled(false);
   m_actionUndo->setIcon(createIconSet("undo.png"));

   m_actionRedo = m_undoGroup->createRedoAction(this);
   m_actionRedo->setEnabled(false);
   m_actionRedo->setIcon(createIconSet("redo.png"));

   m_actionShowFormWindowSettingsDialog = new QAction(tr("Form &Settings..."), this);
   m_actionShowFormWindowSettingsDialog->setObjectName(QString("__qt_form_settings_action"));

   connect(m_actionShowFormWindowSettingsDialog, &QAction::triggered,
      this, &FormWindowManager::slotActionShowFormWindowSettingsDialog);

   m_actionShowFormWindowSettingsDialog->setEnabled(false);
}

void FormWindowManager::slotActionCutActivated()
{
   m_activeFormWindow->cut();
}

void FormWindowManager::slotActionCopyActivated()
{
   m_activeFormWindow->copy();
   slotUpdateActions();
}

void FormWindowManager::slotActionPasteActivated()
{
   m_activeFormWindow->paste();
}

void FormWindowManager::slotActionDeleteActivated()
{
   m_activeFormWindow->deleteWidgets();
}

void FormWindowManager::slotActionLowerActivated()
{
   m_activeFormWindow->lowerWidgets();
}

void FormWindowManager::slotActionRaiseActivated()
{
   m_activeFormWindow->raiseWidgets();
}

static inline QWidget *findLayoutContainer(const FormWindow *fw)
{
   QList<QWidget *> l(fw->selectedWidgets());
   fw->simplifySelection(&l);
   return l.empty() ? fw->mainContainer() : l.front();
}

void FormWindowManager::createLayout()
{
   QAction *a = dynamic_cast<QAction *>(sender());

   if (! a) {
      return;
   }

   const int type = a->data().toInt();

   switch (m_createLayoutContext) {
      case LayoutContainer:
         // Cannot create a splitter on a container
         if (type != LayoutInfo::HSplitter && type != LayoutInfo::VSplitter) {
            m_activeFormWindow->createLayout(type, findLayoutContainer(m_activeFormWindow));
         }
         break;

      case LayoutSelection:
         m_activeFormWindow->createLayout(type);
         break;

      case MorphLayout:
         m_activeFormWindow->morphLayout(m_morphLayoutContainer, type);
         break;
   }
}

void FormWindowManager::slotActionBreakLayoutActivated()
{
   const QList<QWidget *> layouts = layoutsToBeBroken();
   if (layouts.isEmpty()) {
      return;
   }

   m_activeFormWindow->beginCommand(tr("Break Layout"));

   for (QWidget *layout : layouts) {
      m_activeFormWindow->breakLayout(layout);
   }

   m_activeFormWindow->endCommand();
}

void FormWindowManager::slotActionSimplifyLayoutActivated()
{
   Q_ASSERT(m_activeFormWindow != nullptr);

   QWidgetList selectedWidgets = m_activeFormWindow->selectedWidgets();
   m_activeFormWindow->simplifySelection(&selectedWidgets);

   if (selectedWidgets.size() != 1) {
      return;
   }
   SimplifyLayoutCommand *cmd = new SimplifyLayoutCommand(m_activeFormWindow);
   if (cmd->init(selectedWidgets.front())) {
      m_activeFormWindow->commandHistory()->push(cmd);
   } else {
      delete cmd;
   }
}

void FormWindowManager::slotActionAdjustSizeActivated()
{
   Q_ASSERT(m_activeFormWindow != nullptr);

   m_activeFormWindow->beginCommand(tr("Adjust Size"));

   QList<QWidget *> selectedWidgets = m_activeFormWindow->selectedWidgets();
   m_activeFormWindow->simplifySelection(&selectedWidgets);

   if (selectedWidgets.isEmpty()) {
      Q_ASSERT(m_activeFormWindow->mainContainer() != nullptr);
      selectedWidgets.append(m_activeFormWindow->mainContainer());
   }

   // Always count the main container as unlaid-out
   for (QWidget *widget : selectedWidgets) {
      bool unlaidout = LayoutInfo::layoutType(core(), widget->parentWidget()) == LayoutInfo::NoLayout;
      bool isMainContainer = m_activeFormWindow->isMainContainer(widget);

      if (unlaidout || isMainContainer) {
         AdjustWidgetSizeCommand *cmd = new AdjustWidgetSizeCommand(m_activeFormWindow);
         cmd->init(widget);
         m_activeFormWindow->commandHistory()->push(cmd);
      }
   }

   m_activeFormWindow->endCommand();
}

void FormWindowManager::slotActionSelectAllActivated()
{
   m_activeFormWindow->selectAll();
}

void FormWindowManager::showPreview()
{
   slotActionGroupPreviewInStyle(QString(), -1);
}

void FormWindowManager::slotActionGroupPreviewInStyle(const QString &style, int deviceProfileIndex)
{
   QDesignerFormWindowInterface *fw = activeFormWindow();
   if (!fw) {
      return;
   }

   QString errorMessage;
   if (!m_previewManager->showPreview(fw, style, deviceProfileIndex, &errorMessage)) {
      const QString title = tr("Could not create form preview", "Title of warning message box");
      core()->dialogGui()->message(fw, QDesignerDialogGuiInterface::FormEditorMessage, QMessageBox::Warning,
         title, errorMessage);
   }
}

// The user might click on a layout child or the actual layout container.
QWidgetList FormWindowManager::layoutsToBeBroken(QWidget *w) const
{
   if (! w) {
      return QList<QWidget *>();
   }

   QWidget *parent = w->parentWidget();
   if (m_activeFormWindow->isMainContainer(w)) {
      parent = nullptr;
   }

   QWidget *widget = core()->widgetFactory()->containerOfWidget(w);

   // maybe we want to remove following block
   const QDesignerWidgetDataBaseInterface *db = m_core->widgetDataBase();
   const QDesignerWidgetDataBaseItemInterface *item = db->item(db->indexOfObject(widget));

   if (! item) {
      return layoutsToBeBroken(parent);
   }

   const bool layoutContainer = (item->isContainer() || m_activeFormWindow->isMainContainer(widget));

   if (! layoutContainer) {
      return layoutsToBeBroken(parent);
   }

   QLayout *widgetLayout = widget->layout();
   QLayout *managedLayout = LayoutInfo::managedLayout(m_core, widgetLayout);

   if (! managedLayout) {
      if (dynamic_cast<const QSplitter *>(widget)) {
         QList<QWidget *> list = layoutsToBeBroken(parent);
         list.append(widget);

         return list;
      }

      return QList<QWidget *>();
   }

   if (managedLayout) {
      QList<QWidget *> list;

      if (dynamic_cast<const QLayoutWidget *>(widget)) {
         list = layoutsToBeBroken(parent);
      }

      list.append(widget);
      return list;
   }

   return QList<QWidget *>();
}

QMap<QWidget *, bool> FormWindowManager::getUnsortedLayoutsToBeBroken(bool firstOnly) const
{
   // Return a set of layouts to be broken.
   QMap<QWidget *, bool> layouts;

   QList<QWidget *> selection = m_activeFormWindow->selectedWidgets();
   if (selection.isEmpty() && m_activeFormWindow->mainContainer()) {
      selection.append(m_activeFormWindow->mainContainer());
   }

   const QList<QWidget *>::const_iterator scend = selection.constEnd();
   for (QList<QWidget *>::const_iterator sit = selection.constBegin(); sit != scend; ++sit) {
      // find all layouts
      const QList<QWidget *> list = layoutsToBeBroken(*sit);
      if (!list.empty()) {
         const QList<QWidget *>::const_iterator lbcend = list.constEnd();
         for (QList<QWidget *>::const_iterator lbit = list.constBegin(); lbit != lbcend; ++lbit) {
            layouts.insert(*lbit, true);
         }
         if (firstOnly) {
            return layouts;
         }
      }
   }
   return layouts;
}

bool FormWindowManager::hasLayoutsToBeBroken() const
{
   // Quick check for layouts to be broken
   return !getUnsortedLayoutsToBeBroken(true).isEmpty();
}

QWidgetList FormWindowManager::layoutsToBeBroken() const
{
   // Get all layouts. This is a list of all 'red' layouts (QLayoutWidgets)
   // up to the first 'real' widget with a layout in hierarchy order.
   QMap<QWidget *, bool> unsortedLayouts = getUnsortedLayoutsToBeBroken(false);
   // Sort in order of hierarchy
   QList<QWidget *> orderedLayoutList;
   const QMap<QWidget *, bool>::const_iterator lscend  = unsortedLayouts.constEnd();
   for (QMap<QWidget *, bool>::const_iterator itLay = unsortedLayouts.constBegin(); itLay != lscend; ++itLay) {
      QWidget *wToBeInserted = itLay.key();
      if (!orderedLayoutList.contains(wToBeInserted)) {
         // try to find first child, use as insertion position, else append
         const QList<QWidget *>::iterator firstChildPos = findFirstChildOf(orderedLayoutList.begin(), orderedLayoutList.end(), wToBeInserted);
         if (firstChildPos == orderedLayoutList.end()) {
            orderedLayoutList.push_back(wToBeInserted);
         } else {
            orderedLayoutList.insert(firstChildPos, wToBeInserted);
         }
      }
   }
   return orderedLayoutList;
}

static inline bool hasManagedLayoutItems(const QDesignerFormEditorInterface *core, QWidget *w)
{
   if (const QLayout *ml = LayoutInfo::managedLayout(core, w)) {
      // Try to find managed items, ignore dummy grid spacers
      const int count = ml->count();

      for (int i = 0; i < count; i++)
         if (! LayoutInfo::isEmptyItem(ml->itemAt(i))) {
            return true;
         }
   }

   return false;
}

void FormWindowManager::slotUpdateActions()
{
   m_createLayoutContext  = LayoutSelection;
   m_morphLayoutContainer = nullptr;

   bool canMorphIntoVBoxLayout = false;
   bool canMorphIntoHBoxLayout = false;
   bool canMorphIntoGridLayout = false;
   bool canMorphIntoFormLayout = false;

   int selectedWidgetCount  = 0;
   int unlaidoutWidgetCount = 0;

   bool pasteAvailable    = false;
   bool layoutAvailable   = false;
   bool breakAvailable    = false;
   bool simplifyAvailable = false;
   bool layoutContainer   = false;
   bool canChangeZOrder   = true;

   do {
      if (m_activeFormWindow == nullptr || m_activeFormWindow->currentTool() != 0) {
         break;
      }

      breakAvailable = hasLayoutsToBeBroken();

      QWidgetList simplifiedSelection = m_activeFormWindow->selectedWidgets();

      selectedWidgetCount = simplifiedSelection.count();
      pasteAvailable = qApp->clipboard()->mimeData() && qApp->clipboard()->mimeData()->hasText();
      m_activeFormWindow->simplifySelection(&simplifiedSelection);

      QWidget *mainContainer = m_activeFormWindow->mainContainer();
      if (simplifiedSelection.isEmpty() && mainContainer) {
         simplifiedSelection.append(mainContainer);
      }

      // Always count the main container as unlaid-out
      const QWidgetList::const_iterator cend = simplifiedSelection.constEnd();

      for (QWidgetList::const_iterator it = simplifiedSelection.constBegin(); it != cend; ++it) {

         if (*it != mainContainer && LayoutInfo::isWidgetLaidout(m_core, *it)) {
            // do nothing

         } else {
            ++unlaidoutWidgetCount;
         }

         if (dynamic_cast<const QLayoutWidget *>(*it) || dynamic_cast<const Spacer *>(*it)) {
            canChangeZOrder = false;
         }
      }

      // Figure out layouts: Looking at a group of dangling widgets
      if (simplifiedSelection.count() != 1) {
         layoutAvailable = unlaidoutWidgetCount > 1;
         // breakAvailable = false;
         break;
      }

      // Manipulate layout of a single widget
      m_createLayoutContext = LayoutSelection;
      QWidget *widget = core()->widgetFactory()->containerOfWidget(simplifiedSelection.first());

      if (widget == nullptr) {
         // looking at a page-based container with 0 pages
         break;
      }

      const QDesignerWidgetDataBaseInterface *db = m_core->widgetDataBase();
      const QDesignerWidgetDataBaseItemInterface *item = db->item(db->indexOfObject(widget));

      if (item == nullptr) {
         break;
      }

      QLayout *widgetLayout  = LayoutInfo::internalLayout(widget);
      QLayout *managedLayout = LayoutInfo::managedLayout(m_core, widgetLayout);

      // do not touch a layout created by a custom widget
      if (widgetLayout != nullptr && managedLayout == nullptr) {
         break;
      }

      layoutContainer = (item->isContainer() || m_activeFormWindow->isMainContainer(widget));

      layoutAvailable   = layoutContainer && m_activeFormWindow->hasInsertedChildren(widget) && managedLayout == nullptr;
      simplifyAvailable = SimplifyLayoutCommand::canSimplify(m_core, widget);

      if (layoutAvailable) {
         m_createLayoutContext = LayoutContainer;

      } else {
         /* Cannot create a layout, have some layouts to be broken and
          * exactly one, non-empty layout with selected: check the morph layout options
          * (Note that there might be > 1 layouts to broken if the selection
          * is a red layout, however, we want the inner-most layout here). */

         if (breakAvailable && simplifiedSelection.size() == 1
               && hasManagedLayoutItems(m_core, widget)) {

            int type;
            m_morphLayoutContainer = widget; // Was: page of first selected
            m_createLayoutContext = MorphLayout;

            if (MorphLayoutCommand::canMorph(m_activeFormWindow, m_morphLayoutContainer, &type)) {
               canMorphIntoVBoxLayout = type != LayoutInfo::VBox;
               canMorphIntoHBoxLayout = type != LayoutInfo::HBox;
               canMorphIntoGridLayout = type != LayoutInfo::Grid;
               canMorphIntoFormLayout = type != LayoutInfo::Form;
            }
         }
      }

   } while (false);

   m_actionCut->setEnabled(selectedWidgetCount > 0);
   m_actionCopy->setEnabled(selectedWidgetCount > 0);
   m_actionPaste->setEnabled(pasteAvailable);

   m_actionDelete->setEnabled(selectedWidgetCount > 0);
   m_actionLower->setEnabled(canChangeZOrder && selectedWidgetCount > 0);
   m_actionRaise->setEnabled(canChangeZOrder && selectedWidgetCount > 0);


   m_actionSelectAll->setEnabled(m_activeFormWindow != nullptr);

   m_actionAdjustSize->setEnabled(unlaidoutWidgetCount > 0);

   m_actionHorizontalLayout->setEnabled(layoutAvailable || canMorphIntoHBoxLayout);
   m_actionVerticalLayout->setEnabled(layoutAvailable || canMorphIntoVBoxLayout);
   m_actionSplitHorizontal->setEnabled(layoutAvailable && !layoutContainer);
   m_actionSplitVertical->setEnabled(layoutAvailable && !layoutContainer);
   m_actionFormLayout->setEnabled(layoutAvailable || canMorphIntoFormLayout);
   m_actionGridLayout->setEnabled(layoutAvailable || canMorphIntoGridLayout);

   m_actionBreakLayout->setEnabled(breakAvailable);
   m_actionSimplifyLayout->setEnabled(simplifyAvailable);
   m_actionShowFormWindowSettingsDialog->setEnabled(m_activeFormWindow != nullptr);
}

QDesignerFormWindowInterface *FormWindowManager::createFormWindow(QWidget *parentWidget, Qt::WindowFlags flags)
{
   FormEditor *object     = dynamic_cast<FormEditor *>(core());
   FormWindow *formWindow = new FormWindow(object, parentWidget, flags);

   formWindow->setProperty(WidgetFactory::disableStyleCustomPaintingPropertyC, QVariant(true));
   addFormWindow(formWindow);

   return formWindow;
}

QPixmap FormWindowManager::createPreviewPixmap() const
{
   const QDesignerFormWindowInterface *fw = activeFormWindow();

   if (!fw) {
      return QPixmap();
   }

   QString errorMessage;
   const QPixmap pix = m_previewManager->createPreviewPixmap(fw, QString(), &errorMessage);

   if (pix.isNull() && !errorMessage.isEmpty()) {
      qWarning("FormWindowManager::createPreviewPixmap() Preview pixmap creation failed: %s", csPrintable(errorMessage));
   }

   return pix;
}

void FormWindowManager::deviceProfilesChanged()
{
   if (m_actionGroupPreviewInStyle) {
      m_actionGroupPreviewInStyle->updateDeviceProfiles();
   }
}

// DnD stuff

void FormWindowManager::dragItems(const QList<QDesignerDnDItemInterface *> &item_list)
{
   QDesignerMimeData::execDrag(item_list, m_core->topLevel());
}

QUndoGroup *FormWindowManager::undoGroup() const
{
   return m_undoGroup;
}

void FormWindowManager::slotActionShowFormWindowSettingsDialog()
{
   QDesignerFormWindowInterface *fw = activeFormWindow();
   if (! fw) {
      return;
   }

   QDialog *settingsDialog = nullptr;
   const bool wasDirty = fw->isDirty();

   // Ask the language extension for a dialog. If not, create our own
   if (QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension *>(m_core->extensionManager(), m_core)) {
      settingsDialog = lang->createFormWindowSettingsDialog(fw, nullptr);
   }

   if (!settingsDialog) {
      settingsDialog = new FormWindowSettings(fw);
   }

   QString title = QFileInfo(fw->fileName()).fileName();
   if (title.isEmpty()) // Grab the title from the outer window if no filename
      if (const QWidget *window = m_core->integration()->containerWindow(fw)) {
         title = window->windowTitle();
      }

   settingsDialog->setWindowTitle(tr("Form Settings - %1").formatArg(title));
   if (settingsDialog->exec())
      if (fw->isDirty() != wasDirty) {
         emit formWindowSettingsChanged(fw);
      }

   delete settingsDialog;
}

QAction *FormWindowManager::action(Action action) const
{
   switch (action) {

      case QDesignerFormWindowManagerInterface::CutAction:
         return m_actionCut;

      case QDesignerFormWindowManagerInterface::CopyAction:
         return m_actionCopy;

      case QDesignerFormWindowManagerInterface::PasteAction:
         return m_actionPaste;

      case QDesignerFormWindowManagerInterface::DeleteAction:
         return m_actionDelete;

      case QDesignerFormWindowManagerInterface::SelectAllAction:
         return m_actionSelectAll;

      case QDesignerFormWindowManagerInterface::LowerAction:
         return m_actionLower;

      case QDesignerFormWindowManagerInterface::RaiseAction:
         return m_actionRaise;

      case QDesignerFormWindowManagerInterface::UndoAction:
         return m_actionUndo;

      case QDesignerFormWindowManagerInterface::RedoAction:
         return m_actionRedo;

      case QDesignerFormWindowManagerInterface::HorizontalLayoutAction:
         return m_actionHorizontalLayout;

      case QDesignerFormWindowManagerInterface::VerticalLayoutAction:
         return m_actionVerticalLayout;

      case QDesignerFormWindowManagerInterface::SplitHorizontalAction:
         return m_actionSplitHorizontal;

      case QDesignerFormWindowManagerInterface::SplitVerticalAction:
         return m_actionSplitVertical;

      case QDesignerFormWindowManagerInterface::GridLayoutAction:
         return m_actionGridLayout;

      case QDesignerFormWindowManagerInterface::FormLayoutAction:
         return m_actionFormLayout;

      case QDesignerFormWindowManagerInterface::BreakLayoutAction:
         return m_actionBreakLayout;

      case QDesignerFormWindowManagerInterface::AdjustSizeAction:
         return m_actionAdjustSize;

      case QDesignerFormWindowManagerInterface::SimplifyLayoutAction:
         return m_actionSimplifyLayout;

      case QDesignerFormWindowManagerInterface::DefaultPreviewAction:
         return m_actionDefaultPreview;

      case QDesignerFormWindowManagerInterface::FormWindowSettingsDialogAction:
         return m_actionShowFormWindowSettingsDialog;
   }

   qWarning("FormWindowManager::action() Unhandled enumeration value %d", action);

   return nullptr;
}

QActionGroup *FormWindowManager::actionGroup(ActionGroup actionGroup) const
{
   switch (actionGroup) {
      case QDesignerFormWindowManagerInterface::StyledPreviewActionGroup:

         if (m_actionGroupPreviewInStyle == nullptr) {
            QObject *parent = const_cast<FormWindowManager *>(this);

            m_actionGroupPreviewInStyle = new PreviewActionGroup(m_core, parent);

            connect(m_actionGroupPreviewInStyle, &PreviewActionGroup::preview,
               this, &FormWindowManager::slotActionGroupPreviewInStyle);
         }

         return m_actionGroupPreviewInStyle;
   }

   qWarning("FormWindowManager::actionGroup() Unhandled enumeration value %d", actionGroup);

   return nullptr;
}

}


