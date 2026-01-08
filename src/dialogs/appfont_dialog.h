/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#ifndef APPFONT_DIALOG_H
#define APPFONT_DIALOG_H

#include <abstract_settings.h>

#include <QDialog>
#include <QGroupBox>
#include <QItemSelection>

class AppFontModel;

class QTreeView;
class QToolButton;

// AppFontWidget: Manages application fonts which the user can load and
// provides API for saving/restoring them.

class AppFontWidget : public QGroupBox
{
   CS_OBJECT(AppFontWidget)

 public:
   explicit AppFontWidget(QWidget *parent = nullptr);

   QStringList fontFiles() const;

   static void save(QDesignerSettingsInterface *s, const QString &prefix);
   static void restore(const QDesignerSettingsInterface *s, const QString &prefix);

 private:
   Q_DISABLE_COPY(AppFontWidget)

   CS_SLOT_1(Private, void addFiles())
   CS_SLOT_2(addFiles)

   CS_SLOT_1(Private, void slotRemoveFiles())
   CS_SLOT_2(slotRemoveFiles)

   CS_SLOT_1(Private, void slotRemoveAll())
   CS_SLOT_2(slotRemoveAll)

   CS_SLOT_1(Private, void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected))
   CS_SLOT_2(selectionChanged)

   QTreeView *m_view;
   QToolButton *m_addButton;
   QToolButton *m_removeButton;
   QToolButton *m_removeAllButton;
   AppFontModel *m_model;
};

// AppFontDialog: Non modal dialog for AppFontWidget which has Qt::WA_DeleteOnClose set.

class AppFontDialog : public QDialog
{
   CS_OBJECT(AppFontDialog)

 public:
   explicit AppFontDialog(QWidget *parent = nullptr);

 private:
   Q_DISABLE_COPY(AppFontDialog)
   AppFontWidget *m_appFontWidget;
};

#endif
