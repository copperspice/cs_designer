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

#ifndef ABSTRACT_DIALOGGUI_H
#define ABSTRACT_DIALOGGUI_H

#include <QMessageBox>
#include <QFileDialog>

class QWidget;

class QDesignerDialogGuiInterface
{
   Q_DISABLE_COPY(QDesignerDialogGuiInterface)

 public:
   QDesignerDialogGuiInterface();
   virtual ~QDesignerDialogGuiInterface();

   enum Message { FormLoadFailureMessage, UiVersionMismatchMessage, ResourceLoadFailureMessage,
      TopLevelSpacerMessage, PropertyEditorMessage, SignalSlotEditorMessage, FormEditorMessage,
      PreviewFailureMessage, PromotionErrorMessage, ResourceEditorMessage,
      ScriptDialogMessage, SignalSlotDialogMessage, OtherMessage, FileChangedMessage
   };

   virtual QMessageBox::StandardButton message(QWidget *parent, Message context, QMessageBox::Icon icon,
      const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton) = 0;

   virtual QMessageBox::StandardButton message(QWidget *parent, Message context, QMessageBox::Icon icon,
      const QString &title, const QString &text, const QString &informativeText,
      QMessageBox::StandardButtons buttons = QMessageBox::Ok,
      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton) = 0;

   virtual QMessageBox::StandardButton message(QWidget *parent, Message context, QMessageBox::Icon icon,
      const QString &title, const QString &text, const QString &informativeText, const QString &detailedText,
      QMessageBox::StandardButtons buttons = QMessageBox::Ok,
      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton) = 0;

   virtual QString getExistingDirectory(QWidget *parent = nullptr, const QString &caption = QString(),
      const QString &dir = QString(), QFileDialog::FileDialogOptions options = QFileDialog::ShowDirsOnly) = 0;

   virtual QString getOpenFileName(QWidget *parent = nullptr, const QString &caption = QString(),
      const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr,
       QFileDialog::FileDialogOptions options = Qt::EmptyFlag) = 0;

   virtual QString getOpenImageFileName(QWidget *parent = nullptr, const QString &caption = QString(),
      const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr,
      QFileDialog::FileDialogOptions options = Qt::EmptyFlag);

   virtual QStringList getOpenFileNames(QWidget *parent = nullptr, const QString &caption = QString(),
      const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr,
      QFileDialog::FileDialogOptions options = Qt::EmptyFlag) = 0;

   virtual QStringList getOpenImageFileNames(QWidget *parent = nullptr, const QString &caption = QString(),
      const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr,
      QFileDialog::FileDialogOptions options = Qt::EmptyFlag);

   virtual QString getSaveFileName(QWidget *parent = nullptr, const QString &caption = QString(),
      const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr,
      QFileDialog::FileDialogOptions options = Qt::EmptyFlag) = 0;
};

#endif
