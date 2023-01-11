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

#ifndef DIALOGGUI
#define DIALOGGUI

#include <abstract_dialoggui.h>

class QFileIconProvider;

namespace qdesigner_internal {

class DialogGui : public QDesignerDialogGuiInterface
{
 public:
   DialogGui();
   virtual ~DialogGui();

   QMessageBox::StandardButton message(QWidget *parent, Message context, QMessageBox::Icon icon,
      const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton) override;

   QMessageBox::StandardButton message(QWidget *parent, Message context, QMessageBox::Icon icon,
      const QString &title, const QString &text, const QString &informativeText,
      QMessageBox::StandardButtons buttons = QMessageBox::Ok,
      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton) override;

   QMessageBox::StandardButton message(QWidget *parent, Message context, QMessageBox::Icon icon,
      const QString &title, const QString &text, const QString &informativeText, const QString &detailedText,
      QMessageBox::StandardButtons buttons = QMessageBox::Ok,
      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton) override;

   QString getExistingDirectory(QWidget *parent = nullptr, const QString &caption = QString(),
      const QString &dir = QString(), QFileDialog::FileDialogOptions options = QFileDialog::ShowDirsOnly) override;

   QString getOpenFileName(QWidget *parent = nullptr, const QString &caption = QString(),
      const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr,
      QFileDialog::FileDialogOptions options = Qt::EmptyFlag) override;

   QStringList getOpenFileNames(QWidget *parent = nullptr, const QString &caption = QString(),
      const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr,
      QFileDialog::FileDialogOptions options = Qt::EmptyFlag) override;

   QString getSaveFileName(QWidget *parent = nullptr, const QString &caption = QString(),
      const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr,
      QFileDialog::FileDialogOptions options = Qt::EmptyFlag) override;

   QString getOpenImageFileName(QWidget *parent = nullptr, const QString &caption = QString(),
      const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr,
      QFileDialog::FileDialogOptions options = Qt::EmptyFlag) override;

   QStringList getOpenImageFileNames(QWidget *parent = nullptr, const QString &caption = QString(),
      const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr,
      QFileDialog::FileDialogOptions options = Qt::EmptyFlag) override;

 private:
   QFileIconProvider *ensureIconProvider();
   void initializeImageFileDialog(QFileDialog &fd, QFileDialog::FileDialogOptions options, QFileDialog::FileMode);

   QFileIconProvider *m_iconProvider;
};

}  // end namespace qdesigner_internal

#endif
