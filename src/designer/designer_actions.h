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

#ifndef QDESIGNER_ACTIONS_H
#define QDESIGNER_ACTIONS_H

#include <designer_settings.h>

class AppFontDialog;
class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QDesignerWorkbench;

#include <QObject>
#include <QPointer>
#include <QPrinter>

class QDir;
class QTimer;
class QAction;
class QActionGroup;
class QRect;
class QWidget;
class QPixmap;
class QMenu;

namespace qdesigner_internal {
class PreviewConfiguration;
class PreviewManager;
}

class QDesignerActions: public QObject
{
   CS_OBJECT(QDesignerActions)

 public:
   explicit QDesignerActions(QDesignerWorkbench *mainWindow);
   virtual ~QDesignerActions();

   QDesignerWorkbench *workbench() const;
   QDesignerFormEditorInterface *core() const;

   bool saveForm(QDesignerFormWindowInterface *fw);
   bool readInForm(const QString &fileName);
   bool writeOutForm(QDesignerFormWindowInterface *formWindow, const QString &fileName, bool check = true);

   QActionGroup *fileActions() const;
   QActionGroup *recentFilesActions() const;
   QActionGroup *editActions() const;
   QActionGroup *formActions() const;
   QActionGroup *settingsActions() const;
   QActionGroup *windowActions() const;
   QActionGroup *toolActions() const;
   QActionGroup *helpActions() const;
   QActionGroup *uiMode() const;
   QActionGroup *styleActions() const;

   // file actions
   QAction *openFormAction() const;
   QAction *closeFormAction() const;

   // window actions
   QAction *minimizeAction() const;

   // edit mode actions
   QAction *editWidgets() const;

   // form actions
   QAction *previewFormAction() const;
   QAction *viewCodeAction() const;

   void setBringAllToFrontVisible(bool visible);
   void setWindowListSeparatorVisible(bool visible);

   bool openForm(QWidget *parent);

   QString uiExtension() const;

   // Boolean dynamic property set on actions to
   // show them in the default toolbar layout
   static const QString defaultToolbarPropertyName;

   CS_SLOT_1(Public, void activeFormWindowChanged(QDesignerFormWindowInterface *formWindow))
   CS_SLOT_2(activeFormWindowChanged)

   CS_SLOT_1(Public, void createForm())
   CS_SLOT_2(createForm)

   CS_SLOT_1(Public, void slotOpenForm())
   CS_SLOT_2(slotOpenForm)

   CS_SIGNAL_1(Public, void useBigIcons(bool un_named_arg1))
   CS_SIGNAL_2(useBigIcons, un_named_arg1)

 private:
   CS_SLOT_1(Private, void saveForm())
   CS_SLOT_OVERLOAD(saveForm, ())

   CS_SLOT_1(Private, void saveFormAs())
   CS_SLOT_OVERLOAD(saveFormAs, ())

   CS_SLOT_1(Private, void saveAllForms())
   CS_SLOT_2(saveAllForms)

   CS_SLOT_1(Private, void saveFormAsTemplate())
   CS_SLOT_2(saveFormAsTemplate)

   CS_SLOT_1(Private, void viewCode())
   CS_SLOT_2(viewCode)

   CS_SLOT_1(Private, void notImplementedYet())
   CS_SLOT_2(notImplementedYet)

   CS_SLOT_1(Private, void shutdown())
   CS_SLOT_2(shutdown)

   CS_SLOT_1(Private, void editWidgetsSlot())
   CS_SLOT_2(editWidgetsSlot)

   CS_SLOT_1(Private, void openRecentForm())
   CS_SLOT_2(openRecentForm)

   CS_SLOT_1(Private, void clearRecentFiles())
   CS_SLOT_2(clearRecentFiles)

   CS_SLOT_1(Private, void closeForm())
   CS_SLOT_2(closeForm)

   // help
   void showDesignerHelp();
   void aboutDesigner();

   //
   CS_SLOT_1(Private, void backupForms())
   CS_SLOT_2(backupForms)

   CS_SLOT_1(Private, void showNewFormDialog(const QString &fileName))
   CS_SLOT_2(showNewFormDialog)

   CS_SLOT_1(Private, void showPreferencesDialog())
   CS_SLOT_2(showPreferencesDialog)

   CS_SLOT_1(Private, void showAppFontDialog())
   CS_SLOT_2(showAppFontDialog)

   CS_SLOT_1(Private, void savePreviewImage())
   CS_SLOT_2(savePreviewImage)

   CS_SLOT_1(Private, void printPreviewImage())
   CS_SLOT_2(printPreviewImage)

   CS_SLOT_1(Private, void updateCloseAction())
   CS_SLOT_2(updateCloseAction)

   CS_SLOT_1(Private, void formWindowCountChanged())
   CS_SLOT_2(formWindowCountChanged)

   CS_SLOT_1(Private, void formWindowSettingsChanged(QDesignerFormWindowInterface *fw))
   CS_SLOT_2(formWindowSettingsChanged)

 private:
   QAction *createRecentFilesMenu();
   bool saveFormAs(QDesignerFormWindowInterface *fw);
   void fixActionContext();
   void updateRecentFileActions();
   void addRecentFile(const QString &fileName);
   void closePreview();

   QRect fixDialogRect(const QRect &rect) const;
   QString fixResourceFileBackupPath(QDesignerFormWindowInterface *fwi, const QDir &backupDir);
   void showStatusBarMessage(const QString &message) const;
   QActionGroup *createHelpActions();
   bool ensureBackupDirectories();
   QPixmap createPreviewPixmap(QDesignerFormWindowInterface *fw);
   qdesigner_internal::PreviewConfiguration previewConfiguration();

   static constexpr const int MAX_RECENT_FILES = 10;

   QDesignerWorkbench *m_workbench;
   QDesignerFormEditorInterface *m_core;
   QDesignerSettings m_settings;

   QString m_openDirectory;
   QString m_saveDirectory;
   QString m_backupPath;
   QString m_backupTmpPath;

   QTimer *m_backupTimer;

   QActionGroup *m_fileActions;
   QActionGroup *m_recentFilesActions;
   QActionGroup *m_editActions;
   QActionGroup *m_formActions;
   QActionGroup *m_settingsActions;
   QActionGroup *m_windowActions;
   QActionGroup *m_toolActions;
   QActionGroup *m_helpActions;
   QActionGroup *m_styleActions;

   QAction *m_editWidgetsAction;

   QAction *m_newFormAction;
   QAction *m_openFormAction;
   QAction *m_saveFormAction;
   QAction *m_saveFormAsAction;
   QAction *m_saveAllFormsAction;
   QAction *m_saveFormAsTemplateAction;
   QAction *m_closeFormAction;
   QAction *m_savePreviewImageAction;
   QAction *m_printPreviewAction;

   QAction *m_quitAction;

   QAction *m_previewFormAction;
   //   QAction *m_viewCodeAction;

   QAction *m_minimizeAction;
   QAction *m_bringAllToFrontSeparator;
   QAction *m_bringAllToFrontAction;
   QAction *m_windowListSeparatorAction;

   QAction *m_preferencesAction;
   QAction *m_appFontAction;

   QPointer<AppFontDialog> m_appFontDialog;
   QPrinter *m_printer;

   qdesigner_internal::PreviewManager *m_previewManager;
};

#endif
