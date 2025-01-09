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

#ifndef QTRESOURCEEDITOR_H
#define QTRESOURCEEDITOR_H

#include <abstract_dialoggui.h>
#include <abstract_formeditor.h>
#include <resource_model.h>

#include <QDialog>
#include <QListWidgetItem>
#include <QScopedPointer>
#include <QStandardItem>

class QtQrcFile;
class QtResourceFile;
class QtResourcePrefix;

class QtResourceEditorDialog : public QDialog
{
   CS_OBJECT(QtResourceEditorDialog)

 public:
   QtResourceModel *model() const;
   void setResourceModel(QtResourceModel *model);

   QString selectedResource() const;

   static QString editResources(QDesignerFormEditorInterface *core, QtResourceModel *model,
      QDesignerDialogGuiInterface *dlgGui, QWidget *parent = nullptr);

   // Helper to display a message box with rcc logs in case of errors.
   static void displayResourceFailures(const QString &logOutput, QDesignerDialogGuiInterface *dlgGui, QWidget *parent = nullptr);

   CS_SLOT_1(Public, void accept() override)
   CS_SLOT_2(accept)

 private:
   QtResourceEditorDialog(QDesignerFormEditorInterface *core, QDesignerDialogGuiInterface *dlgGui, QWidget *parent = nullptr);
   ~QtResourceEditorDialog();

   QScopedPointer<class QtResourceEditorDialogPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtResourceEditorDialog)
   Q_DISABLE_COPY(QtResourceEditorDialog)

   CS_SLOT_1(Private, void slotQrcFileInserted(QtQrcFile *qrcFile))
   CS_SLOT_2(slotQrcFileInserted)

   CS_SLOT_1(Private, void slotQrcFileMoved(QtQrcFile *uqrcFile))
   CS_SLOT_2(slotQrcFileMoved)

   CS_SLOT_1(Private, void slotQrcFileRemoved(QtQrcFile *qrcFile))
   CS_SLOT_2(slotQrcFileRemoved)

   CS_SLOT_1(Private, void slotResourcePrefixInserted(QtResourcePrefix *un_named_arg1))
   CS_SLOT_2(slotResourcePrefixInserted)

   CS_SLOT_1(Private, void slotResourcePrefixMoved(QtResourcePrefix *un_named_arg1))
   CS_SLOT_2(slotResourcePrefixMoved)

   CS_SLOT_1(Private, void slotResourcePrefixChanged(QtResourcePrefix *un_named_arg1))
   CS_SLOT_2(slotResourcePrefixChanged)

   CS_SLOT_1(Private, void slotResourceLanguageChanged(QtResourcePrefix *un_named_arg1))
   CS_SLOT_2(slotResourceLanguageChanged)

   CS_SLOT_1(Private, void slotResourcePrefixRemoved(QtResourcePrefix *un_named_arg1))
   CS_SLOT_2(slotResourcePrefixRemoved)

   CS_SLOT_1(Private, void slotResourceFileInserted(QtResourceFile *un_named_arg1))
   CS_SLOT_2(slotResourceFileInserted)

   CS_SLOT_1(Private, void slotResourceFileMoved(QtResourceFile *un_named_arg1))
   CS_SLOT_2(slotResourceFileMoved)

   CS_SLOT_1(Private, void slotResourceAliasChanged(QtResourceFile *un_named_arg1))
   CS_SLOT_2(slotResourceAliasChanged)

   CS_SLOT_1(Private, void slotResourceFileRemoved(QtResourceFile *un_named_arg1))
   CS_SLOT_2(slotResourceFileRemoved)

   CS_SLOT_1(Private, void slotCurrentQrcFileChanged(QListWidgetItem *un_named_arg1))
   CS_SLOT_2(slotCurrentQrcFileChanged)

   CS_SLOT_1(Private, void slotCurrentTreeViewItemChanged(const QModelIndex &un_named_arg1))
   CS_SLOT_2(slotCurrentTreeViewItemChanged)

   CS_SLOT_1(Private, void slotListWidgetContextMenuRequested(const QPoint &un_named_arg1))
   CS_SLOT_2(slotListWidgetContextMenuRequested)

   CS_SLOT_1(Private, void slotTreeViewContextMenuRequested(const QPoint &un_named_arg1))
   CS_SLOT_2(slotTreeViewContextMenuRequested)

   CS_SLOT_1(Private, void slotTreeViewItemChanged(QStandardItem *un_named_arg1))
   CS_SLOT_2(slotTreeViewItemChanged)

   CS_SLOT_1(Private, void slotNewQrcFile())
   CS_SLOT_2(slotNewQrcFile)

   CS_SLOT_1(Private, void slotImportQrcFile())
   CS_SLOT_2(slotImportQrcFile)

   CS_SLOT_1(Private, void slotRemoveQrcFile())
   CS_SLOT_2(slotRemoveQrcFile)

   CS_SLOT_1(Private, void slotMoveUpQrcFile())
   CS_SLOT_2(slotMoveUpQrcFile)

   CS_SLOT_1(Private, void slotMoveDownQrcFile())
   CS_SLOT_2(slotMoveDownQrcFile)

   CS_SLOT_1(Private, void slotNewPrefix())
   CS_SLOT_2(slotNewPrefix)

   CS_SLOT_1(Private, void slotAddFiles())
   CS_SLOT_2(slotAddFiles)

   CS_SLOT_1(Private, void slotChangePrefix())
   CS_SLOT_2(slotChangePrefix)

   CS_SLOT_1(Private, void slotChangeLanguage())
   CS_SLOT_2(slotChangeLanguage)

   CS_SLOT_1(Private, void slotChangeAlias())
   CS_SLOT_2(slotChangeAlias)

   CS_SLOT_1(Private, void slotClonePrefix())
   CS_SLOT_2(slotClonePrefix)

   CS_SLOT_1(Private, void slotRemove())
   CS_SLOT_2(slotRemove)

   CS_SLOT_1(Private, void slotMoveUp())
   CS_SLOT_2(slotMoveUp)

   CS_SLOT_1(Private, void slotMoveDown())
   CS_SLOT_2(slotMoveDown)
};

#endif

