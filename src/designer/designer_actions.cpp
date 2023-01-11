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

#include <abstract_formeditor.h>
#include <abstract_formeditorplugin.h>
#include <abstract_formwindow.h>
#include <abstract_formwindowcursor.h>
#include <abstract_integration.h>
#include <abstract_language.h>
#include <appfont_dialog.h>
#include <buddyeditor_plugin.h>
#include <code_dialog.h>
#include <designer.h>
#include <designer_actions.h>
#include <designer_build_info.h>
#include <designer_enums.h>
#include <designer_formbuilder.h>
#include <designer_formwindow.h>
#include <designer_formwindow_manager.h>
#include <designer_toolwindow.h>
#include <designer_utils.h>
#include <designer_workbench.h>
#include <extension.h>
#include <newform.h>
#include <plugin_manager.h>
#include <preferences_dialog.h>
#include <preview_manager.h>
#include <saveform_as_template.h>
#include <signalsloteditor_plugin.h>
#include <tabordereditor_plugin.h>
#include <utils.h>

#include <formwindowbase_p.h>
#include <formwindowbase_p.h>
#include <iconloader_p.h>
#include <metadatabase_p.h>
#include <shared_settings_p.h>

#include <QAction>
#include <QActionGroup>
#include <QBuffer>
#include <QCloseEvent>
#include <QCursor>
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDomDocument>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QLibraryInfo>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QMetaObject>
#include <QPainter>
#include <QPixmap>
#include <QPluginLoader>
#include <QPrintDialog>
#include <QPushButton>
#include <QSizeF>
#include <QStatusBar>
#include <QStyleFactory>
#include <QTimer>
#include <QTransform>

using namespace qdesigner_internal;

const QString QDesignerActions::defaultToolbarPropertyName = "__qt_defaultToolBarAction";

//#ifdef Q_OS_DARWIN
# define NONMODAL_PREVIEW
//#endif

static QAction *createSeparator(QObject *parent)
{
   QAction *rc = new QAction(parent);
   rc->setSeparator(true);
   return rc;
}

static QActionGroup *createActionGroup(QObject *parent, bool exclusive = false)
{
   QActionGroup *rc = new QActionGroup(parent);
   rc->setExclusive(exclusive);
   return rc;
}

static inline QString savedMessage(const QString &fileName)
{
   return QDesignerActions::tr("Saved %1.").formatArg(fileName);
}

// Prompt for a file and make sure an extension is added
// unless the user explicitly specifies another one.

static QString getSaveFileNameWithExtension(QWidget *parent, const QString &title, QString dir,
         const QString &filter, const QString &extension)
{
   const QChar dot = '.';

   QString saveFile;

   while (true) {
      saveFile = QFileDialog::getSaveFileName(parent, title, dir, filter, nullptr, QFileDialog::DontConfirmOverwrite);

      if (saveFile.isEmpty()) {
         return saveFile;
      }

      const QFileInfo fInfo(saveFile);
      if (fInfo.suffix().isEmpty() && !fInfo.fileName().endsWith(dot)) {
         saveFile += dot;
         saveFile += extension;
      }

      const QFileInfo fi(saveFile);
      if (!fi.exists()) {
         break;
      }

      const QString prompt = QDesignerActions::tr("%1 already exists.\nDo you want to replace it?").formatArg(fi.fileName());
      if (QMessageBox::warning(parent, title, prompt, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
         break;
      }

      dir = saveFile;
   }
   return saveFile;
}

QDesignerActions::QDesignerActions(QDesignerWorkbench *workbench)
   : QObject(workbench),
     m_workbench(workbench),
     m_core(workbench->core()),
     m_settings(workbench->core()),
     m_backupTimer(new QTimer(this)),
     m_fileActions(createActionGroup(this)),
     m_recentFilesActions(createActionGroup(this)),
     m_editActions(createActionGroup(this)),
     m_formActions(createActionGroup(this)),
     m_settingsActions(createActionGroup(this)),
     m_windowActions(createActionGroup(this)),
     m_toolActions(createActionGroup(this, true)),
     m_helpActions(nullptr),
     m_styleActions(nullptr),
     m_editWidgetsAction(new QAction(tr("Edit Widgets"), this)),
     m_newFormAction(new QAction(qdesigner_internal::createIconSet("filenew.png"), tr("&New..."), this)),
     m_openFormAction(new QAction(qdesigner_internal::createIconSet("fileopen.png"), tr("&Open..."), this)),
     m_saveFormAction(new QAction(qdesigner_internal::createIconSet("filesave.png"), tr("&Save"), this)),
     m_saveFormAsAction(new QAction(tr("Save &As..."), this)),
     m_saveAllFormsAction(new QAction(tr("Save A&ll"), this)),
     m_saveFormAsTemplateAction(new QAction(tr("Save As &Template..."), this)),
     m_closeFormAction(new QAction(tr("&Close"), this)),
     m_savePreviewImageAction(new QAction(tr("Save &Image..."), this)),
     m_printPreviewAction(new QAction(tr("&Print..."), this)),
     m_quitAction(new QAction(tr("&Quit"), this)),
     m_previewFormAction(nullptr),
     //   m_viewCodeAction(new QAction(tr("View &Code..."), this)),
     m_minimizeAction(new QAction(tr("&Minimize"), this)),
     m_bringAllToFrontSeparator(createSeparator(this)),
     m_bringAllToFrontAction(new QAction(tr("Bring All to Front"), this)),
     m_windowListSeparatorAction(createSeparator(this)),
     m_preferencesAction(new QAction(tr("Preferences..."), this)),
     m_appFontAction(new QAction(tr("Additional Fonts..."), this)),
     m_appFontDialog(nullptr), m_printer(nullptr), m_previewManager(nullptr)
{
#if defined (Q_OS_UNIX) && ! defined(Q_OS_DARWIN)
   m_newFormAction->setIcon(QIcon::fromTheme("document-new",        m_newFormAction->icon()));
   m_openFormAction->setIcon(QIcon::fromTheme("document-open",      m_openFormAction->icon()));
   m_saveFormAction->setIcon(QIcon::fromTheme("document-save",      m_saveFormAction->icon()));
   m_saveFormAsAction->setIcon(QIcon::fromTheme("document-save-as", m_saveFormAsAction->icon()));
   m_printPreviewAction->setIcon(QIcon::fromTheme("document-print", m_printPreviewAction->icon()));
   m_closeFormAction->setIcon(QIcon::fromTheme("window-close",      m_closeFormAction->icon()));
   m_quitAction->setIcon(QIcon::fromTheme("application-exit",       m_quitAction->icon()));
#endif

   Q_ASSERT(m_core != nullptr);

   qdesigner_internal::QDesignerFormWindowManager *ifwm = dynamic_cast<qdesigner_internal::QDesignerFormWindowManager *>
      (m_core->formWindowManager());
   Q_ASSERT(ifwm);

   m_previewManager    = ifwm->previewManager();
   m_previewFormAction = ifwm->action(QDesignerFormWindowManagerInterface::DefaultPreviewAction);
   m_styleActions      = ifwm->actionGroup(QDesignerFormWindowManagerInterface::StyledPreviewActionGroup);

   connect(ifwm, &QDesignerFormWindowManagerInterface::formWindowSettingsChanged,
      this, &QDesignerActions::formWindowSettingsChanged);

   m_editWidgetsAction->setObjectName(QString("__qt_edit_widgets_action"));
   m_newFormAction->setObjectName(QString("__qt_new_form_action"));
   m_openFormAction->setObjectName(QString("__qt_open_form_action"));
   m_saveFormAction->setObjectName(QString("__qt_save_form_action"));
   m_saveFormAsAction->setObjectName(QString("__qt_save_form_as_action"));
   m_saveAllFormsAction->setObjectName(QString("__qt_save_all_forms_action"));
   m_saveFormAsTemplateAction->setObjectName(QString("__qt_save_form_as_template_action"));
   m_closeFormAction->setObjectName(QString("__qt_close_form_action"));
   m_quitAction->setObjectName(QString("__qt_quit_action"));
   m_previewFormAction->setObjectName(QString("__qt_preview_form_action"));
   //   m_viewCodeAction->setObjectName(QString("__qt_preview_code_action"));
   m_minimizeAction->setObjectName(QString("__qt_minimize_action"));
   m_bringAllToFrontAction->setObjectName(QString("__qt_bring_all_to_front_action"));
   m_preferencesAction->setObjectName(QString("__qt_preferences_action"));

   m_helpActions = createHelpActions();

   m_newFormAction->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
   m_openFormAction->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
   m_saveFormAction->setProperty(QDesignerActions::defaultToolbarPropertyName, true);

   QDesignerFormWindowManagerInterface *formWindowManager = m_core->formWindowManager();
   Q_ASSERT(formWindowManager != nullptr);

   // file actions
   m_newFormAction->setShortcut(QKeySequence::New);
   connect(m_newFormAction, &QAction::triggered, this, &QDesignerActions::createForm);
   m_fileActions->addAction(m_newFormAction);

   m_openFormAction->setShortcut(QKeySequence::Open);
   connect(m_openFormAction, &QAction::triggered, this, &QDesignerActions::slotOpenForm);
   m_fileActions->addAction(m_openFormAction);

   m_fileActions->addAction(createRecentFilesMenu());
   m_fileActions->addAction(createSeparator(this));

   m_saveFormAction->setShortcut(QKeySequence::Save);

   connect(m_saveFormAction, &QAction::triggered, this, cs_mp_cast<>(&QDesignerActions::saveForm));
   m_fileActions->addAction(m_saveFormAction);

   connect(m_saveFormAsAction, &QAction::triggered, this, cs_mp_cast<>(&QDesignerActions::saveFormAs));
   m_fileActions->addAction(m_saveFormAsAction);

#ifdef Q_OS_DARWIN
   m_saveAllFormsAction->setShortcut(tr("ALT+CTRL+S"));
#else
   m_saveAllFormsAction->setShortcut(tr("CTRL+SHIFT+S")); // Commonly "Save As" on Mac
#endif

   connect(m_saveAllFormsAction, &QAction::triggered, this, &QDesignerActions::saveAllForms);
   m_fileActions->addAction(m_saveAllFormsAction);

   connect(m_saveFormAsTemplateAction, &QAction::triggered, this, &QDesignerActions::saveFormAsTemplate);
   m_fileActions->addAction(m_saveFormAsTemplateAction);

   m_fileActions->addAction(createSeparator(this));

   m_printPreviewAction->setShortcut(QKeySequence::Print);
   connect(m_printPreviewAction,  &QAction::triggered, this, &QDesignerActions::printPreviewImage);
   m_fileActions->addAction(m_printPreviewAction);
   m_printPreviewAction->setObjectName(QString("__print_action"));

   connect(m_savePreviewImageAction,  &QAction::triggered, this, &QDesignerActions::savePreviewImage);
   m_savePreviewImageAction->setObjectName(QString("__saveimage_action"));
   m_fileActions->addAction(m_savePreviewImageAction);
   m_fileActions->addAction(createSeparator(this));

   m_closeFormAction->setShortcut(QKeySequence::Close);
   connect(m_closeFormAction, &QAction::triggered, this, &QDesignerActions::closeForm);
   m_fileActions->addAction(m_closeFormAction);
   updateCloseAction();

   m_fileActions->addAction(createSeparator(this));

   m_quitAction->setShortcuts(QKeySequence::Quit);
   m_quitAction->setMenuRole(QAction::QuitRole);
   connect(m_quitAction, &QAction::triggered, this, &QDesignerActions::shutdown);
   m_fileActions->addAction(m_quitAction);

   // edit actions
   QAction *undoAction = formWindowManager->action(QDesignerFormWindowManagerInterface::UndoAction);
   undoAction->setObjectName("__undo_action");
   undoAction->setShortcut(QKeySequence::Undo);
   m_editActions->addAction(undoAction);

   QAction *redoAction = formWindowManager->action(QDesignerFormWindowManagerInterface::RedoAction);
   redoAction->setObjectName("__redo_action");
   redoAction->setShortcut(QKeySequence::Redo);
   m_editActions->addAction(redoAction);

   m_editActions->addAction(createSeparator(this));

   m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::CutAction));
   m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::CopyAction));
   m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::PasteAction));
   m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::DeleteAction));

   m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::SelectAllAction));

   m_editActions->addAction(createSeparator(this));

   m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::LowerAction));
   m_editActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::RaiseAction));

   formWindowManager->action(QDesignerFormWindowManagerInterface::LowerAction)->setProperty(
         QDesignerActions::defaultToolbarPropertyName, true);

   formWindowManager->action(QDesignerFormWindowManagerInterface::RaiseAction)->setProperty(
         QDesignerActions::defaultToolbarPropertyName, true);

   // edit mode actions
   m_editWidgetsAction->setCheckable(true);

   QList<QKeySequence> shortcuts;
   shortcuts.append(QKeySequence(Qt::Key_F3));
   shortcuts.append(QKeySequence(Qt::Key_Escape));
   m_editWidgetsAction->setShortcuts(shortcuts);

   m_editWidgetsAction->setIcon(QIcon(getResourceLocation() + "/widgettool.png"));

   connect(m_editWidgetsAction, &QAction::triggered, this, &QDesignerActions::editWidgetsSlot);

   m_editWidgetsAction->setChecked(true);
   m_editWidgetsAction->setEnabled(false);
   m_editWidgetsAction->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
   m_toolActions->addAction(m_editWidgetsAction);

   // F4
   SignalSlotEditorPlugin *signalEditor = new SignalSlotEditorPlugin{};
   signalEditor->initialize(core());

   QAction *actionSignal = signalEditor->action();

   if (actionSignal != nullptr) {
      actionSignal->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
      actionSignal->setCheckable(true);
      m_toolActions->addAction(actionSignal);
   }

   //
   BuddyEditorPlugin *buddyEditor = new BuddyEditorPlugin{};
   buddyEditor->initialize(core());

   QAction *actionBuddy = buddyEditor->action();

   if (actionBuddy != nullptr) {
      actionBuddy->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
      actionBuddy->setCheckable(true);
      m_toolActions->addAction(actionBuddy);
   }

   //
   TabOrderEditorPlugin *tabOrderEditor = new TabOrderEditorPlugin{};
   tabOrderEditor->initialize(core());

   QAction *actionTabOrder = tabOrderEditor->action();

   if (actionTabOrder != nullptr) {
      actionTabOrder->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
      actionTabOrder->setCheckable(true);
      m_toolActions->addAction(actionTabOrder);
   }

   connect(formWindowManager, &QDesignerFormWindowManager::activeFormWindowChanged,
         this, &QDesignerActions::activeFormWindowChanged);

/* emerald - review if required ( builtin plugins )

   QList<QObject *> builtinPlugins = QPluginLoader::staticInstances();
   builtinPlugins += m_core->pluginManager()->instances();

   for (QObject *plugin : builtinPlugins) {
      if (QDesignerFormEditorPluginInterface *formEditorPlugin = dynamic_cast<QDesignerFormEditorPluginInterface *>(plugin)) {
         if (QAction *action = formEditorPlugin->action()) {
            m_toolActions->addAction(action);
            action->setProperty(QDesignerActions::defaultToolbarPropertyName, true);
            action->setCheckable(true);
         }
      }
   }

*/

   connect(m_preferencesAction, &QAction::triggered, this, &QDesignerActions::showPreferencesDialog);

   m_preferencesAction->setMenuRole(QAction::PreferencesRole);
   m_settingsActions->addAction(m_preferencesAction);

   connect(m_appFontAction, &QAction::triggered,  this, &QDesignerActions::showAppFontDialog);
   m_settingsActions->addAction(m_appFontAction);

   // form actions
   m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::HorizontalLayoutAction));
   m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::VerticalLayoutAction));
   m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::SplitHorizontalAction));
   m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::SplitVerticalAction));
   m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::GridLayoutAction));
   m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::FormLayoutAction));
   m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::BreakLayoutAction));
   m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::AdjustSizeAction));
   m_formActions->addAction(formWindowManager->action(QDesignerFormWindowManagerInterface::SimplifyLayoutAction));
   m_formActions->addAction(createSeparator(this));

   formWindowManager->action(QDesignerFormWindowManagerInterface::HorizontalLayoutAction)->setProperty(
      QDesignerActions::defaultToolbarPropertyName, true);
   formWindowManager->action(QDesignerFormWindowManagerInterface::VerticalLayoutAction)->setProperty(
      QDesignerActions::defaultToolbarPropertyName, true);
   formWindowManager->action(QDesignerFormWindowManagerInterface::SplitHorizontalAction)->setProperty(
      QDesignerActions::defaultToolbarPropertyName, true);
   formWindowManager->action(QDesignerFormWindowManagerInterface::SplitVerticalAction)->setProperty(
      QDesignerActions::defaultToolbarPropertyName, true);
   formWindowManager->action(QDesignerFormWindowManagerInterface::GridLayoutAction)->setProperty(
      QDesignerActions::defaultToolbarPropertyName, true);
   formWindowManager->action(QDesignerFormWindowManagerInterface::FormLayoutAction)->setProperty(
      QDesignerActions::defaultToolbarPropertyName, true);
   formWindowManager->action(QDesignerFormWindowManagerInterface::BreakLayoutAction)->setProperty(
      QDesignerActions::defaultToolbarPropertyName, true);
   formWindowManager->action(QDesignerFormWindowManagerInterface::AdjustSizeAction)->setProperty(
      QDesignerActions::defaultToolbarPropertyName, true);

   m_previewFormAction->setShortcut(tr("CTRL+R"));
   m_formActions->addAction(m_previewFormAction);

   connect(m_previewManager, &qdesigner_internal::PreviewManager::firstPreviewOpened,
      this, &QDesignerActions::updateCloseAction);
   connect(m_previewManager, &qdesigner_internal::PreviewManager::lastPreviewClosed,
      this, &QDesignerActions::updateCloseAction);

/*
   connect(m_viewCodeAction, &QAction::triggered, this, &QDesignerActions::viewCode);

   // view code, unused feature
   if (qt_extension<QDesignerLanguageExtension *>(m_core->extensionManager(), m_core) == 0) {
      m_formActions->addAction(m_viewCodeAction);
   }
*/

   m_formActions->addAction(createSeparator(this));

   m_formActions->addAction(ifwm->action(QDesignerFormWindowManagerInterface::FormWindowSettingsDialogAction));

   // window actions
   m_minimizeAction->setEnabled(false);
   m_minimizeAction->setCheckable(true);
   m_minimizeAction->setShortcut(tr("CTRL+M"));
   connect(m_minimizeAction, &QAction::triggered, m_workbench, &QDesignerWorkbench::toggleFormMinimizationState);
   m_windowActions->addAction(m_minimizeAction);

   m_windowActions->addAction(m_bringAllToFrontSeparator);
   connect(m_bringAllToFrontAction, &QAction::triggered, m_workbench, &QDesignerWorkbench::bringAllToFront);
   m_windowActions->addAction(m_bringAllToFrontAction);
   m_windowActions->addAction(m_windowListSeparatorAction);

   setWindowListSeparatorVisible(false);

   // connections
   fixActionContext();
   activeFormWindowChanged(core()->formWindowManager()->activeFormWindow());

   m_backupTimer->start(180000); // 3min
   connect(m_backupTimer, &QTimer::timeout, this, &QDesignerActions::backupForms);

   // Enable application font action
   connect(formWindowManager, &QDesignerFormWindowManagerInterface::formWindowAdded,
      this, &QDesignerActions::formWindowCountChanged);
   connect(formWindowManager, &QDesignerFormWindowManagerInterface::formWindowRemoved,
      this, &QDesignerActions::formWindowCountChanged);
   formWindowCountChanged();
}

QActionGroup *QDesignerActions::createHelpActions()
{
   QActionGroup *helpActions = createActionGroup(this);

   QAction *mainHelpAction = new QAction(tr("CS Designer &Help"), this);
   mainHelpAction->setObjectName("__designer_help_action");
   connect(mainHelpAction, &QAction::triggered, this, &QDesignerActions::showDesignerHelp);
   mainHelpAction->setShortcut(Qt::ControlModifier + Qt::Key_Question);
   helpActions->addAction(mainHelpAction);

   QAction *aboutDesignerAction = new QAction(tr("About CS Designer"), this);
   aboutDesignerAction->setMenuRole(QAction::AboutRole);
   aboutDesignerAction->setObjectName("__about_designer_action");
   connect(aboutDesignerAction, &QAction::triggered, this, &QDesignerActions::aboutDesigner);
   helpActions->addAction(aboutDesignerAction);

   QAction *aboutCsAction = new QAction(tr("About CopperSpice"), this);
   aboutCsAction->setMenuRole(QAction::AboutCsRole);
   aboutCsAction->setObjectName("__about_cs_action");
   connect(aboutCsAction, &QAction::triggered, qApp, &QApplication::aboutCs);
   helpActions->addAction(aboutCsAction);

   return helpActions;
}

QDesignerActions::~QDesignerActions()
{
   delete m_printer;
}

QString QDesignerActions::uiExtension() const
{
   QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension *>(m_core->extensionManager(), m_core);

   if (lang) {
      return lang->uiExtension();
   }

   return QString("ui");
}

QAction *QDesignerActions::createRecentFilesMenu()
{
   QMenu *menu = new QMenu;
   QAction *act;

   // Need to insert this into the QAction
   for (int i = 0; i < MAX_RECENT_FILES; ++i) {
      act = new QAction(this);
      act->setVisible(false);

      connect(act, &QAction::triggered, this, &QDesignerActions::openRecentForm);

      m_recentFilesActions->addAction(act);
      menu->addAction(act);
   }

   updateRecentFileActions();
   menu->addSeparator();

   act = new QAction(QIcon::fromTheme("edit-clear"), tr("Clear &Menu"), this);
   act->setObjectName("__qt_action_clear_menu_");

   connect(act, &QAction::triggered, this, &QDesignerActions::clearRecentFiles);

   m_recentFilesActions->addAction(act);
   menu->addAction(act);

   act = new QAction(QIcon::fromTheme("document-open-recent"), tr("&Recent Forms"), this);
   act->setMenu(menu);

   return act;
}

QActionGroup *QDesignerActions::toolActions() const
{
   return m_toolActions;
}

QDesignerWorkbench *QDesignerActions::workbench() const
{
   return m_workbench;
}

QDesignerFormEditorInterface *QDesignerActions::core() const
{
   return m_core;
}

QActionGroup *QDesignerActions::fileActions() const
{
   return m_fileActions;
}

QActionGroup *QDesignerActions::editActions() const
{
   return m_editActions;
}

QActionGroup *QDesignerActions::formActions() const
{
   return m_formActions;
}

QActionGroup *QDesignerActions::settingsActions() const
{
   return m_settingsActions;
}

QActionGroup *QDesignerActions::windowActions() const
{
   return m_windowActions;
}

QActionGroup *QDesignerActions::helpActions() const
{
   return m_helpActions;
}

QActionGroup *QDesignerActions::styleActions() const
{
   return m_styleActions;
}

QAction *QDesignerActions::previewFormAction() const
{
   return m_previewFormAction;
}

/*
QAction *QDesignerActions::viewCodeAction() const
{
   return m_viewCodeAction;
}
*/

void QDesignerActions::editWidgetsSlot()
{
   QDesignerFormWindowManagerInterface *formWindowManager = core()->formWindowManager();

   for (int i = 0; i < formWindowManager->formWindowCount(); ++i) {
      QDesignerFormWindowInterface *formWindow = formWindowManager->formWindow(i);
      formWindow->editWidgets();
   }
}

void QDesignerActions::createForm()
{
   showNewFormDialog(QString());
}

void QDesignerActions::showNewFormDialog(const QString &fileName)
{
   closePreview();
   NewForm *dlg = new NewForm(workbench(), workbench()->core()->topLevel(), fileName);

   dlg->setAttribute(Qt::WA_DeleteOnClose);
   dlg->setAttribute(Qt::WA_ShowModal);

   dlg->setGeometry(fixDialogRect(dlg->rect()));
   dlg->exec();
}

void QDesignerActions::slotOpenForm()
{
   openForm(core()->topLevel());
}

bool QDesignerActions::openForm(QWidget *parent)
{
   closePreview();
   const QString extension = uiExtension();

   const QStringList fileNames = QFileDialog::getOpenFileNames(parent, tr("Open Form"),
         m_openDirectory, tr("Designer UI files (*.%1);;All Files (*)").formatArg(extension), nullptr, QFileDialog::DontUseSheet);

   if (fileNames.isEmpty()) {
      return false;
   }

   bool atLeastOne = false;

   for (const QString &fileName : fileNames) {
      if (readInForm(fileName) && ! atLeastOne) {
         atLeastOne = true;
      }
   }

   return atLeastOne;
}

bool QDesignerActions::saveFormAs(QDesignerFormWindowInterface *fw)
{
   const QString extension = uiExtension();

   QString dir = fw->fileName();

   if (dir.isEmpty()) {
      do {
         // Build untitled name
         if (!m_saveDirectory.isEmpty()) {
            dir = m_saveDirectory;
            break;
         }

         if (!m_openDirectory.isEmpty()) {
            dir = m_openDirectory;
            break;
         }

         dir = QDir::current().absolutePath();

      } while (false);

      dir += QDir::separator();
      dir += QString("untitled.");
      dir += extension;
   }

   const  QString saveFile = getSaveFileNameWithExtension(fw, tr("Save Form As"), dir,
         tr("Designer UI files (*.%1);;All Files (*)").formatArg(extension), extension);

   if (saveFile.isEmpty()) {
      return false;
   }

   fw->setFileName(saveFile);
   return writeOutForm(fw, saveFile);
}

void QDesignerActions::saveForm()
{
   if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow()) {
      if (saveForm(fw)) {
         showStatusBarMessage(savedMessage(QFileInfo(fw->fileName()).fileName()));
      }
   }
}

void QDesignerActions::saveAllForms()
{
   QString fileNames;
   QDesignerFormWindowManagerInterface *formWindowManager = core()->formWindowManager();

   if (const int totalWindows = formWindowManager->formWindowCount()) {
      const QString separator = QString(", ");

      for (int i = 0; i < totalWindows; ++i) {
         QDesignerFormWindowInterface *fw = formWindowManager->formWindow(i);
         if (fw && fw->isDirty()) {
            formWindowManager->setActiveFormWindow(fw);

            if (saveForm(fw)) {
               if (!fileNames.isEmpty()) {
                  fileNames += separator;
               }

               fileNames += QFileInfo(fw->fileName()).fileName();

            } else {
               break;
            }
         }
      }
   }

   if (!fileNames.isEmpty()) {
      showStatusBarMessage(savedMessage(fileNames));
   }
}

bool QDesignerActions::saveForm(QDesignerFormWindowInterface *fw)
{
   bool ret;
   if (fw->fileName().isEmpty()) {
      ret = saveFormAs(fw);
   } else {
      ret =  writeOutForm(fw, fw->fileName());
   }
   return ret;
}

void QDesignerActions::closeForm()
{
   if (m_previewManager->previewCount()) {
      closePreview();
      return;
   }

   if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow())
      if (QWidget *parent = fw->parentWidget()) {
         if (QMdiSubWindow *mdiSubWindow = dynamic_cast<QMdiSubWindow *>(parent->parentWidget())) {
            mdiSubWindow->close();
         } else {
            parent->close();
         }
      }
}

void QDesignerActions::saveFormAs()
{
   if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow()) {
      if (saveFormAs(fw)) {
         showStatusBarMessage(savedMessage(fw->fileName()));
      }
   }
}

void QDesignerActions::saveFormAsTemplate()
{
   if (QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow()) {
      SaveFormAsTemplate dlg(core(), fw, fw->window());
      dlg.exec();
   }
}

void QDesignerActions::notImplementedYet()
{
   QMessageBox::information(core()->topLevel(), tr("Designer"), tr("Feature not implemented yet!"));
}

void QDesignerActions::closePreview()
{
   m_previewManager->closeAllPreviews();
}

void  QDesignerActions::viewCode()
{
   QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow();

   if (fw == nullptr) {
      return;
   }

   QString errorMessage;

   if (! qdesigner_internal::CodeDialog::showCodeDialog(fw, fw, &errorMessage)) {
      csError(tr("Code generation failed"), errorMessage);
   }
}

void QDesignerActions::fixActionContext()
{
   QList<QAction *> actions;
   actions += m_fileActions->actions();
   actions += m_editActions->actions();
   actions += m_toolActions->actions();
   actions += m_formActions->actions();
   actions += m_windowActions->actions();
   actions += m_helpActions->actions();

   for (QAction *a : actions) {
      a->setShortcutContext(Qt::ApplicationShortcut);
   }
}

bool QDesignerActions::readInForm(const QString &fileName)
{
   QString fn = fileName;

   // First make sure file is not open already
   QDesignerFormWindowManagerInterface *formWindowManager = core()->formWindowManager();
   const int totalWindows = formWindowManager->formWindowCount();

   for (int i = 0; i < totalWindows; ++i) {
      QDesignerFormWindowInterface *w = formWindowManager->formWindow(i);

      if (w->fileName() == fn) {
         w->raise();
         formWindowManager->setActiveFormWindow(w);
         addRecentFile(fn);

         return true;
      }
   }

   // Otherwise load it
   do {
      QString errorMessage;

      if (workbench()->openForm(fn, &errorMessage)) {
         addRecentFile(fn);
         m_openDirectory = QFileInfo(fn).absolutePath();
         return true;

      } else {
         // prompt to reload

         QMessageBox box(QMessageBox::Warning, tr("Read error"),
            tr("%1\nDo you want to update the file location or generate a new form?").formatArg(errorMessage),
            QMessageBox::Cancel, core()->topLevel());

         QPushButton *updateButton = box.addButton(tr("&Update"), QMessageBox::ActionRole);
         QPushButton *newButton    = box.addButton(tr("&New Form"), QMessageBox::ActionRole);
         box.exec();
         if (box.clickedButton() == box.button(QMessageBox::Cancel)) {
            return false;
         }

         if (box.clickedButton() == updateButton) {
            const QString extension = uiExtension();

            fn = QFileDialog::getOpenFileName(core()->topLevel(),
                  tr("Open Form"), m_openDirectory,
                  tr("Designer UI files (*.%1);;All Files (*)").formatArg(extension), nullptr, QFileDialog::DontUseSheet);

            if (fn.isEmpty()) {
               return false;
            }

         } else if (box.clickedButton() == newButton) {
            // If the file does not exist but the directory is valid, open the template with the editor file
            // name set to it. (called from command line).

            QString newFormFileName;
            const  QFileInfo fInfo(fn);

            if (! fInfo.exists()) {
               // Normalize file name
               const QString directory = fInfo.absolutePath();

               if (QDir(directory).exists()) {
                  newFormFileName = directory;
                  newFormFileName  += '/';
                  newFormFileName  += fInfo.fileName();
               }
            }

            showNewFormDialog(newFormFileName);
            return false;
         }
      }

   } while (true);

   return true;
}

static QString createBackup(const QString &fileName)
{
   const QString suffix = QString(".bak");
   QString backupFile = fileName + suffix;
   QFileInfo fi(backupFile);
   int i = 0;

   while (fi.exists()) {
      backupFile = fileName + suffix + QString::number(++i);
      fi.setFile(backupFile);
   }

   if (QFile::copy(fileName, backupFile)) {
      return backupFile;
   }
   return QString();
}

static void removeBackup(const QString &backupFile)
{
   if (!backupFile.isEmpty()) {
      QFile::remove(backupFile);
   }
}

bool QDesignerActions::writeOutForm(QDesignerFormWindowInterface *fw, const QString &saveFile, bool check)
{
   Q_ASSERT(fw && !saveFile.isEmpty());

   QString backupFile;
   QFileInfo fi(saveFile);
   if (fi.exists()) {
      backupFile = createBackup(saveFile);
   }

   if (check) {
      const QStringList problems = fw->checkContents();
      if (! problems.isEmpty()) {
         QMessageBox::information(fw->window(), tr("CS Designer"), problems.join("<br>"));
      }
   }

   QString contents = fw->contents();
   if (qdesigner_internal::FormWindowBase *fwb = dynamic_cast<qdesigner_internal::FormWindowBase *>(fw)) {
      if (fwb->lineTerminatorMode() == qdesigner_internal::FormWindowBase::CRLFLineTerminator) {
         contents.replace('\n', "\r\n");
      }
   }

   const QByteArray utf8Array = contents.toUtf8();
   m_workbench->updateBackup(fw);

   QFile f(saveFile);
   while (!f.open(QFile::WriteOnly)) {
      QMessageBox box(QMessageBox::Warning,
         tr("Save Form?"),
         tr("Could not open file"),
         QMessageBox::NoButton, fw);

      box.setWindowModality(Qt::WindowModal);
      box.setInformativeText(tr("The file %1 could not be opened."
            "\nReason: %2"
            "\nWould you like to retry or select a different file?")
         .formatArg(f.fileName()).formatArg(f.errorString()));

      QPushButton *retryButton = box.addButton(QMessageBox::Retry);
      retryButton->setDefault(true);
      QPushButton *switchButton = box.addButton(tr("Select New File"), QMessageBox::AcceptRole);
      QPushButton *cancelButton = box.addButton(QMessageBox::Cancel);
      box.exec();

      if (box.clickedButton() == cancelButton) {
         removeBackup(backupFile);
         return false;
      } else if (box.clickedButton() == switchButton) {
         QString extension = uiExtension();
         const QString fileName = QFileDialog::getSaveFileName(fw, tr("Save Form As"),
               QDir::current().absolutePath(),
               QString("*.") + extension);
         if (fileName.isEmpty()) {
            removeBackup(backupFile);
            return false;
         }
         if (f.fileName() != fileName) {
            removeBackup(backupFile);
            fi.setFile(fileName);
            backupFile.clear();
            if (fi.exists()) {
               backupFile = createBackup(fileName);
            }
         }
         f.setFileName(fileName);
         fw->setFileName(fileName);
      }

   }

   while (f.write(utf8Array) != utf8Array.size()) {

      QMessageBox box(QMessageBox::Warning, tr("Save Form?"),
         tr("Could not write file"), QMessageBox::Retry | QMessageBox::Cancel, fw);

      box.setWindowModality(Qt::WindowModal);
      box.setInformativeText(tr("Unable to write the entire file %1 to disk."
            "\nReason:%2\nWould you like to retry?")
         .formatArg(f.fileName()).formatArg(f.errorString()));

      box.setDefaultButton(QMessageBox::Retry);

      switch (box.exec()) {
         case QMessageBox::Retry:
            f.resize(0);
            break;

         default:
            return false;
      }
   }
   f.close();
   removeBackup(backupFile);
   addRecentFile(saveFile);
   m_saveDirectory = QFileInfo(f).absolutePath();

   fw->setDirty(false);
   fw->parentWidget()->setWindowModified(false);
   return true;
}

void QDesignerActions::shutdown()
{
   // Follow the idea from the Mac, i.e. send the Application a close event
   // and if it's accepted, quit.
   QCloseEvent ev;
   QApplication::sendEvent(qDesignerApp, &ev);

   if (ev.isAccepted()) {
      qDesignerApp->quit();
   }
}

void QDesignerActions::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
   const bool enable = formWindow != nullptr;

   m_saveFormAction->setEnabled(enable);
   m_saveFormAsAction->setEnabled(enable);
   m_saveAllFormsAction->setEnabled(enable);
   m_saveFormAsTemplateAction->setEnabled(enable);
   m_closeFormAction->setEnabled(enable);
   m_savePreviewImageAction->setEnabled(enable);
   m_printPreviewAction->setEnabled(enable);

   m_editWidgetsAction->setEnabled(enable);

   m_previewFormAction->setEnabled(enable);
   //   m_viewCodeAction->setEnabled(enable);
   m_styleActions->setEnabled(enable);
}

void QDesignerActions::formWindowSettingsChanged(QDesignerFormWindowInterface *fw)
{
   if (QDesignerFormWindow *window = m_workbench->findFormWindow(fw)) {
      window->updateChanged();
   }
}

void QDesignerActions::updateRecentFileActions()
{
   QStringList files = m_settings.recentFilesList();
   const int originalSize = files.size();
   int numRecentFiles = qMin(files.size(), int(MAX_RECENT_FILES));
   const QList<QAction *> recentFilesActs = m_recentFilesActions->actions();

   for (int i = 0; i < numRecentFiles; ++i) {
      const QFileInfo fi(files[i]);

      if (! fi.exists()) {
         files.removeAt(i);
         --i;
         numRecentFiles = qMin(files.size(), int(MAX_RECENT_FILES));

         continue;
      }
      const QString text = fi.fileName();
      recentFilesActs[i]->setText(text);
      recentFilesActs[i]->setIconText(files[i]);
      recentFilesActs[i]->setVisible(true);
   }

   for (int j = numRecentFiles; j < MAX_RECENT_FILES; ++j) {
      recentFilesActs[j]->setVisible(false);
   }

   // If there's been a change, right it back
   if (originalSize != files.size()) {
      m_settings.setRecentFilesList(files);
   }
}

void QDesignerActions::openRecentForm()
{
   if (const QAction *action = dynamic_cast<const QAction *>(sender())) {
      if (!readInForm(action->iconText())) {
         updateRecentFileActions();            // File does not exist, remove it from settings
      }
   }
}

void QDesignerActions::clearRecentFiles()
{
   m_settings.setRecentFilesList(QStringList());
   updateRecentFileActions();
}

QActionGroup *QDesignerActions::recentFilesActions() const
{
   return m_recentFilesActions;
}

void QDesignerActions::addRecentFile(const QString &fileName)
{
   QStringList files = m_settings.recentFilesList();
   files.removeAll(fileName);
   files.prepend(fileName);

   while (files.size() > MAX_RECENT_FILES) {
      files.removeLast();
   }

   m_settings.setRecentFilesList(files);
   updateRecentFileActions();
}

QAction *QDesignerActions::openFormAction() const
{
   return m_openFormAction;
}

QAction *QDesignerActions::closeFormAction() const
{
   return m_closeFormAction;
}

QAction *QDesignerActions::minimizeAction() const
{
   return m_minimizeAction;
}

void QDesignerActions::showDesignerHelp()
{
   // display html help file using client browser
   bool ok = QDesktopServices::openUrl(QUrl("https://www.copperspice.com/docs/cs_api/tools-designer.html"));

   if (! ok)  {
      csError("CS Designer Documentation", "Unable to display Documentation\n");
   }
}

void QDesignerActions::aboutDesigner()
{
   QString textBody =
      "<font color='#000080'>"
      "<table style=margin-right:25>"
      "   <tr><td><nobr>Developed by Barbara Geller</nobr></td>"
      "       <td>barbara@copperspice.com</td></tr>"
      "   <tr><td style=padding-right:25><nobr>Developed by Ansel Sermersheim</nobr></td>"
      "       <td>ansel@copperspice.com</td></tr>"
      "</table></font>"
      "<br>"

      "<p><small>Copyright 2021-2023 BG Consulting, released under the LGPL 2.1 license<br>"
      "This program is provided AS IS with no warranty of any kind.<br><br>"
      "Copyright (c) 2015 The Qt Company Ltd.</small></p>";

   QMessageBox msgB;
   msgB.setIcon(QMessageBox::NoIcon);
   msgB.setWindowIcon(QIcon(":/resources/designer.png"));
   msgB.setMinimumWidth(2000);

   msgB.setWindowTitle(tr("About CS Designer"));

   msgB.setText(QString("<p style=margin-right:25><center><h5>Version: %1<br>Build # %2</h5></center></p>")
      .formatArgs(QString::fromLatin1(versionString), QString::fromLatin1(buildDate)));
   msgB.setInformativeText(textBody);

   msgB.setStandardButtons(QMessageBox::Ok);
   msgB.setDefaultButton(QMessageBox::Ok);

   msgB.exec();
}

QAction *QDesignerActions::editWidgets() const
{
   return m_editWidgetsAction;
}

void QDesignerActions::updateCloseAction()
{
   if (m_previewManager->previewCount()) {
      m_closeFormAction->setText(tr("&Close Preview"));
   } else {
      m_closeFormAction->setText(tr("&Close"));
   }
}

void QDesignerActions::backupForms()
{
   const int count = m_workbench->formWindowCount();
   if (!count || !ensureBackupDirectories()) {
      return;
   }


   QStringList tmpFiles;
   QMap<QString, QString> backupMap;
   QDir backupDir(m_backupPath);
   for (int i = 0; i < count; ++i) {
      QDesignerFormWindow *fw = m_workbench->formWindow(i);
      QDesignerFormWindowInterface *fwi = fw->editor();

      QString formBackupName;
      QTextStream(&formBackupName) << m_backupPath << QDir::separator()
         << QString("backup") << i << QString(".bak");

      QString fwn = QDir::toNativeSeparators(fwi->fileName());
      if (fwn.isEmpty()) {
         fwn = fw->windowTitle();
      }

      backupMap.insert(fwn, formBackupName);

      QFile file(formBackupName.replace(m_backupPath, m_backupTmpPath));
      if (file.open(QFile::WriteOnly)) {
         QString contents = fixResourceFileBackupPath(fwi, backupDir);

         if (qdesigner_internal::FormWindowBase *fwb = dynamic_cast<qdesigner_internal::FormWindowBase *>(fwi)) {
            if (fwb->lineTerminatorMode() == qdesigner_internal::FormWindowBase::CRLFLineTerminator) {
               contents.replace('\n', "\r\n");
            }
         }

         const QByteArray utf8Array = contents.toUtf8();

         if (file.write(utf8Array) != utf8Array.size()) {
            backupMap.remove(fwn);
            csWarning(tr("The backup file %1 could not be written.").formatArg(file.fileName()));
         } else {
            tmpFiles.append(formBackupName);
         }

         file.close();
      }
   }
   if (!tmpFiles.isEmpty()) {
      const QStringList backupFiles = backupDir.entryList(QDir::Files);
      if (!backupFiles.isEmpty()) {
         QStringListIterator it(backupFiles);
         while (it.hasNext()) {
            backupDir.remove(it.next());
         }
      }

      QStringListIterator it(tmpFiles);
      while (it.hasNext()) {
         const QString tmpName = it.next();
         QString name(tmpName);
         name.replace(m_backupTmpPath, m_backupPath);
         QFile tmpFile(tmpName);
         if (!tmpFile.copy(name)) {
            csWarning(tr("The backup file %1 could not be written.").formatArg(name));
         }
         tmpFile.remove();
      }

      m_settings.setBackup(backupMap);
   }
}

QString QDesignerActions::fixResourceFileBackupPath(QDesignerFormWindowInterface *fwi, const QDir &backupDir)
{
   const QString content = fwi->contents();
   QDomDocument domDoc("backup");

   if (!domDoc.setContent(content)) {
      return content;
   }

   const QDomNodeList list = domDoc.elementsByTagName("resources");
   if (list.isEmpty()) {
      return content;
   }

   for (int i = 0; i < list.count(); i++) {
      const QDomNode node = list.at(i);

      if (!node.isNull()) {
         const QDomElement element = node.toElement();

         if (!element.isNull() && element.tagName() == "resources") {
            QDomNode childNode = element.firstChild();

            while (!childNode.isNull()) {
               QDomElement childElement = childNode.toElement();
               if (!childElement.isNull() && childElement.tagName() == "include") {

                  const QString attr = childElement.attribute("location");
                  const QString path = fwi->absoluteDir().absoluteFilePath(attr);
                  childElement.setAttribute(QString("location"), backupDir.relativeFilePath(path));
               }
               childNode = childNode.nextSibling();
            }
         }
      }
   }


   return domDoc.toString();
}

QRect QDesignerActions::fixDialogRect(const QRect &rect) const
{
   QRect frameGeometry;
   const QRect availableGeometry = QApplication::desktop()->availableGeometry(core()->topLevel());

   if (workbench()->mode() == DockedMode) {
      frameGeometry = core()->topLevel()->frameGeometry();
   } else {
      frameGeometry = availableGeometry;
   }

   QRect dlgRect = rect;
   dlgRect.moveCenter(frameGeometry.center());

   // make sure that parts of the dialog are not outside of screen
   dlgRect.moveBottom(qMin(dlgRect.bottom(), availableGeometry.bottom()));
   dlgRect.moveRight(qMin(dlgRect.right(), availableGeometry.right()));
   dlgRect.moveLeft(qMax(dlgRect.left(), availableGeometry.left()));
   dlgRect.moveTop(qMax(dlgRect.top(), availableGeometry.top()));

   return dlgRect;
}

void QDesignerActions::showStatusBarMessage(const QString &message) const
{
   if (workbench()->mode() == DockedMode) {
      QStatusBar *bar = qDesignerApp->mainWindow()->statusBar();

      if (bar && ! bar->isHidden()) {
         bar->showMessage(message, 3000);
      }
   }
}

void QDesignerActions::setBringAllToFrontVisible(bool visible)
{
   m_bringAllToFrontSeparator->setVisible(visible);
   m_bringAllToFrontAction->setVisible(visible);
}

void QDesignerActions::setWindowListSeparatorVisible(bool visible)
{
   m_windowListSeparatorAction->setVisible(visible);
}

bool QDesignerActions::ensureBackupDirectories()
{

   if (m_backupPath.isEmpty()) {
      // create names
      m_backupPath = QDir::homePath();
      m_backupPath += QDir::separator();
      m_backupPath += QString(".designer");
      m_backupPath += QDir::separator();
      m_backupPath += QString("backup");
      m_backupPath = QDir::toNativeSeparators(m_backupPath);

      m_backupTmpPath = m_backupPath;
      m_backupTmpPath += QDir::separator();
      m_backupTmpPath += QString("tmp");
      m_backupTmpPath = QDir::toNativeSeparators(m_backupTmpPath);
   }

   // ensure directories
   const QDir backupDir(m_backupPath);
   const QDir backupTmpDir(m_backupTmpPath);

   if (!backupDir.exists()) {
      if (!backupDir.mkpath(m_backupPath)) {
         csWarning(tr("The backup directory %1 could not be created.").formatArg(m_backupPath));
         return false;
      }
   }
   if (!backupTmpDir.exists()) {
      if (!backupTmpDir.mkpath(m_backupTmpPath)) {
         csWarning(tr("The temporary backup directory %1 could not be created.").formatArg(m_backupTmpPath));
         return false;
      }
   }
   return true;
}

void QDesignerActions::showPreferencesDialog()
{
   {
      PreferencesDialog preferencesDialog(workbench()->core(), m_core->topLevel());
      preferencesDialog.exec();
   }   // Make sure the preference dialog is destroyed before switching UI modes.
   m_workbench->applyUiSettings();
}

void QDesignerActions::showAppFontDialog()
{
   if (!m_appFontDialog) { // Might get deleted when switching ui modes
      m_appFontDialog = new AppFontDialog(core()->topLevel());
   }
   m_appFontDialog->show();
   m_appFontDialog->raise();
}

QPixmap QDesignerActions::createPreviewPixmap(QDesignerFormWindowInterface *fw)
{
   const QCursor oldCursor = core()->topLevel()->cursor();
   core()->topLevel()->setCursor(Qt::WaitCursor);

   QString errorMessage;
   const QPixmap pixmap = m_previewManager->createPreviewPixmap(fw, QString(), &errorMessage);
   core()->topLevel()->setCursor(oldCursor);
   if (pixmap.isNull()) {
      QMessageBox::warning(fw, tr("Preview failed"), errorMessage);
   }
   return pixmap;
}

qdesigner_internal::PreviewConfiguration QDesignerActions::previewConfiguration()
{
   qdesigner_internal::PreviewConfiguration pc;
   QDesignerSharedSettings settings(core());
   if (settings.isCustomPreviewConfigurationEnabled()) {
      pc = settings.customPreviewConfiguration();
   }
   return pc;
}

void QDesignerActions::savePreviewImage()
{
   const QString format = "png";

   QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow();
   if (!fw) {
      return;
   }

   QImage image;
   const QString extension = format;
   const QString filter = tr("Image files (*.%1)").formatArg(extension);

   QString suggestion = fw->fileName();

   if (!suggestion.isEmpty()) {
      suggestion = QFileInfo(suggestion).baseName();
      suggestion += '.';
      suggestion += extension;
   }
   do {
      const QString fileName  = getSaveFileNameWithExtension(fw, tr("Save Image"), suggestion, filter, extension);
      if (fileName.isEmpty()) {
         break;
      }

      if (image.isNull()) {
         const QPixmap pixmap = createPreviewPixmap(fw);

         if (pixmap.isNull()) {
            break;
         }

         image = pixmap.toImage();
      }

      if (image.save(fileName, format)) {
         showStatusBarMessage(tr("Saved image %1.").formatArg(QFileInfo(fileName).fileName()));
         break;
      }

      QMessageBox box(QMessageBox::Warning, tr("Save Image"),
         tr("The file %1 could not be written.").formatArg( fileName),
         QMessageBox::Retry | QMessageBox::Cancel, fw);

      if (box.exec() == QMessageBox::Cancel) {
         break;
      }

   } while (true);
}

void QDesignerActions::formWindowCountChanged()
{
   const bool enabled = m_core->formWindowManager()->formWindowCount() == 0;
   /* Disable the application font action if there are form windows open
    * as the reordering of the fonts sets font properties to 'changed'
    * and overloaded fonts are not updated. */
   static const QString disabledTip = tr("Please close all forms to enable the loading of additional fonts.");
   m_appFontAction->setEnabled(enabled);
   m_appFontAction->setStatusTip(enabled ? QString() : disabledTip);
}

void QDesignerActions::printPreviewImage()
{
   QDesignerFormWindowInterface *fw = core()->formWindowManager()->activeFormWindow();
   if (!fw) {
      return;
   }

   if (!m_printer) {
      m_printer = new QPrinter(QPrinter::HighResolution);
   }

   m_printer->setFullPage(false);

   // Grab the image to be able to a suggest suitable orientation
   const QPixmap pixmap = createPreviewPixmap(fw);
   if (pixmap.isNull()) {
      return;
   }

   const QSizeF pixmapSize = pixmap.size();
   m_printer->setOrientation( pixmapSize.width() > pixmapSize.height() ?  QPageLayout::Landscape :  QPageLayout::Portrait);

   // Printer parameters
   QPrintDialog dialog(m_printer, fw);
   if (!dialog.exec()) {
      return;
   }

   const QCursor oldCursor = core()->topLevel()->cursor();
   core()->topLevel()->setCursor(Qt::WaitCursor);

   // Estimate of required scaling to make form look the same on screen and printer.
   const double suggestedScaling = static_cast<double>(m_printer->physicalDpiX()) /  static_cast<double>(fw->physicalDpiX());

   QPainter painter(m_printer);
   painter.setRenderHint(QPainter::SmoothPixmapTransform);

   // Clamp to page
   const QRectF page =  painter.viewport();
   const double maxScaling = qMin(page.size().width() / pixmapSize.width(), page.size().height() / pixmapSize.height());
   const double scaling = qMin(suggestedScaling, maxScaling);

   const double xOffset = page.left() + qMax(0.0, (page.size().width()  - scaling * pixmapSize.width())  / 2.0);
   const double yOffset = page.top()  + qMax(0.0, (page.size().height() - scaling * pixmapSize.height()) / 2.0);

   // Draw.
   painter.translate(xOffset, yOffset);
   painter.scale(scaling, scaling);
   painter.drawPixmap(0, 0, pixmap);
   core()->topLevel()->setCursor(oldCursor);

   showStatusBarMessage(tr("Printed %1.").formatArg(QFileInfo(fw->fileName()).fileName()));
}

