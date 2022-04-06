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

#ifndef TOOLBAR_DIALOG_H
#define TOOLBAR_DIALOG_H

class QtToolBarManagerPrivate;
class QtToolBarDialogPrivate;

#include <QDialog>
#include <QListWidgetItem>
#include <QTreeWidgetItem>

class QMainWindow;
class QAction;
class QToolBar;

class QtToolBarManager : public QObject
{
   CS_OBJECT(QtToolBarManager)

 public:
   explicit QtToolBarManager(QObject *parent = nullptr);
   ~QtToolBarManager();

   void setMainWindow(QMainWindow *mainWindow);
   QMainWindow *mainWindow() const;

   void addAction(QAction *action, const QString &category);
   void removeAction(QAction *action);

   void addToolBar(QToolBar *toolBar, const QString &category);
   void removeToolBar(QToolBar *toolBar);

   QList<QToolBar *> toolBars() const;

   QByteArray saveState(int version = 0) const;
   bool restoreState(const QByteArray &state, int version = 0);

 private:
   QtToolBarManager(const QtToolBarManager &) = delete;
   QtToolBarManager &operator=(const QtToolBarManager &) = delete;

   Q_DECLARE_PRIVATE(QtToolBarManager)

   QScopedPointer<QtToolBarManagerPrivate> d_ptr;

   friend class QtToolBarDialog;
};

class QtToolBarDialog : public QDialog
{
   CS_OBJECT(QtToolBarDialog)

 public:
   explicit QtToolBarDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);
   ~QtToolBarDialog();

   void setToolBarManager(QtToolBarManager *toolBarManager);

 protected:
   void showEvent(QShowEvent *event);
   void hideEvent(QHideEvent *event);

 private:
   QScopedPointer<QtToolBarDialogPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtToolBarDialog)
   Q_DISABLE_COPY(QtToolBarDialog)

   CS_SLOT_1(Private, void newClicked())
   CS_SLOT_2(newClicked)

   CS_SLOT_1(Private, void removeClicked())
   CS_SLOT_2(removeClicked)

   CS_SLOT_1(Private, void defaultClicked())
   CS_SLOT_2(defaultClicked)

   CS_SLOT_1(Private, void okClicked())
   CS_SLOT_2(okClicked)

   CS_SLOT_1(Private, void applyClicked())
   CS_SLOT_2(applyClicked)

   CS_SLOT_1(Private, void cancelClicked())
   CS_SLOT_2(cancelClicked)

   CS_SLOT_1(Private, void upClicked())
   CS_SLOT_2(upClicked)

   CS_SLOT_1(Private, void downClicked())
   CS_SLOT_2(downClicked)

   CS_SLOT_1(Private, void leftClicked())
   CS_SLOT_2(leftClicked)

   CS_SLOT_1(Private, void rightClicked())
   CS_SLOT_2(rightClicked)

   CS_SLOT_1(Private, void renameClicked())
   CS_SLOT_2(renameClicked)

   CS_SLOT_1(Private, void toolBarRenamed(QListWidgetItem *un_named_arg1))
   CS_SLOT_2(toolBarRenamed)

   CS_SLOT_1(Private, void currentActionChanged(QTreeWidgetItem *un_named_arg1))
   CS_SLOT_2(currentActionChanged)

   CS_SLOT_1(Private, void currentToolBarChanged(QListWidgetItem *un_named_arg1))
   CS_SLOT_2(currentToolBarChanged)

   CS_SLOT_1(Private, void currentToolBarActionChanged(QListWidgetItem *un_named_arg1))
   CS_SLOT_2(currentToolBarActionChanged)
};

#endif
