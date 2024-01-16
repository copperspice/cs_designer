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

#include <dialoggui_p.h>

#include <QFile>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QPixmap>
#include <QSet>

// QFileDialog on X11 does not provide an image preview. Display icons.
#if defined(Q_OS_UNIX) && ! defined(Q_OS_DARWIN)
#  define IMAGE_PREVIEW
#endif

namespace qdesigner_internal {

// Icon provider that reads out the known image formats
class IconProvider : public QFileIconProvider
{
   Q_DISABLE_COPY(IconProvider)

 public:
   IconProvider();
   QIcon icon (const QFileInfo &info) const override;

   inline bool loadCheck(const QFileInfo &info) const;
   QImage loadImage(const QString &fileName) const;

 private:
   QSet<QString> m_imageFormats;
};

IconProvider::IconProvider()
{
   // Determine a list of readable extensions (upper and lower case)

   const QList<QString> fmts = QImageReader::supportedImageFormats();
   auto cend = fmts.constEnd();

   for (auto it = fmts.constBegin(); it != cend; ++it) {
      const QString suffix =  *it;

      m_imageFormats.insert(suffix.toLower());
      m_imageFormats.insert(suffix.toUpper());
   }
}

// Check by extension and type if this appears to be a loadable image
bool IconProvider::loadCheck(const QFileInfo &info) const
{
   if (info.isFile() && info.isReadable()) {
      const QString suffix = info.suffix();

      if (!suffix.isEmpty()) {
         return m_imageFormats.contains(suffix);
      }
   }
   return false;
}

QImage IconProvider::loadImage(const QString &fileName) const
{
   QFile file(fileName);
   if (file.open(QIODevice::ReadOnly)) {
      QImageReader imgReader(&file);
      if (imgReader.canRead()) {
         QImage image;
         if (imgReader.read(&image)) {
            return image;
         }
      }
   }
   return QImage();
}

QIcon IconProvider::icon (const QFileInfo &info) const
{
   // Don't get stuck on large images.
   const qint64 maxSize = 131072;

   if (loadCheck(info) && info.size() < maxSize) {
      const QImage image = loadImage(info.absoluteFilePath());
      if (!image.isNull()) {
         return QIcon(QPixmap::fromImage(image, Qt::ThresholdDither | Qt::AutoColor));
      }
   }

   return QFileIconProvider::icon(info);
}

DialogGui::DialogGui()
   : m_iconProvider(nullptr)
{
}

DialogGui::~DialogGui()
{
   delete m_iconProvider;
}

QFileIconProvider *DialogGui::ensureIconProvider()
{
   if (!m_iconProvider) {
      m_iconProvider = new IconProvider;
   }
   return m_iconProvider;
}

QMessageBox::StandardButton DialogGui::message(QWidget *parent, Message /*context*/, QMessageBox::Icon icon,
   const QString &title, const QString &text, QMessageBox::StandardButtons buttons,
   QMessageBox::StandardButton defaultButton)
{
   QMessageBox::StandardButton rc = QMessageBox::NoButton;
   switch (icon) {
      case QMessageBox::Information:
         rc = QMessageBox::information(parent, title, text, buttons, defaultButton);
         break;
      case QMessageBox::Warning:
         rc = QMessageBox::warning(parent, title, text, buttons, defaultButton);
         break;
      case QMessageBox::Critical:
         rc = QMessageBox::critical(parent, title, text, buttons, defaultButton);
         break;
      case QMessageBox::Question:
         rc = QMessageBox::question(parent, title, text, buttons, defaultButton);
         break;
      case QMessageBox::NoIcon:
         break;
   }
   return rc;
}

QMessageBox::StandardButton DialogGui::message(QWidget *parent, Message /*context*/, QMessageBox::Icon icon,
   const QString &title, const QString &text, const QString &informativeText,
   QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
   QMessageBox msgBox(icon, title, text, buttons, parent);
   msgBox.setDefaultButton(defaultButton);
   msgBox.setInformativeText(informativeText);
   return static_cast<QMessageBox::StandardButton>(msgBox.exec());
}

QMessageBox::StandardButton DialogGui::message(QWidget *parent, Message /*context*/, QMessageBox::Icon icon,
   const QString &title, const QString &text, const QString &informativeText, const QString &detailedText,
   QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
   QMessageBox msgBox(icon, title, text, buttons, parent);
   msgBox.setDefaultButton(defaultButton);
   msgBox.setInformativeText(informativeText);
   msgBox.setDetailedText(detailedText);
   return static_cast<QMessageBox::StandardButton>(msgBox.exec());
}

QString DialogGui::getExistingDirectory(QWidget *parent, const QString &caption, const QString &dir,
   QFileDialog::FileDialogOptions options)
{
   return QFileDialog::getExistingDirectory(parent, caption, dir, options);
}

QString DialogGui::getOpenFileName(QWidget *parent, const QString &caption, const QString &dir,
   const QString &filter, QString *selectedFilter, QFileDialog::FileDialogOptions options)
{
   return QFileDialog::getOpenFileName(parent, caption, dir, filter, selectedFilter, options);
}

QStringList DialogGui::getOpenFileNames(QWidget *parent, const QString &caption, const QString &dir,
   const QString &filter, QString *selectedFilter, QFileDialog::FileDialogOptions options)
{
   return QFileDialog::getOpenFileNames(parent, caption, dir, filter, selectedFilter, options);
}

QString DialogGui::getSaveFileName(QWidget *parent, const QString &caption, const QString &dir,
   const QString &filter, QString *selectedFilter, QFileDialog::FileDialogOptions options)
{
   return QFileDialog::getSaveFileName(parent, caption, dir, filter, selectedFilter, options);
}

void DialogGui::initializeImageFileDialog(QFileDialog &fileDialog,
   QFileDialog::FileDialogOptions options, QFileDialog::FileMode fm)
{
   fileDialog.setConfirmOverwrite( !(options & QFileDialog::DontConfirmOverwrite) );
   fileDialog.setResolveSymlinks( !(options & QFileDialog::DontResolveSymlinks) );
   fileDialog.setIconProvider(ensureIconProvider());
   fileDialog.setFileMode(fm);
}

QString DialogGui::getOpenImageFileName(QWidget *parent, const QString &caption, const QString &dir,
   const QString &filter, QString *selectedFilter, QFileDialog::FileDialogOptions options )
{
#ifdef IMAGE_PREVIEW
   QFileDialog fileDialog(parent, caption, dir, filter);
   initializeImageFileDialog(fileDialog, options, QFileDialog::ExistingFile);
   if (fileDialog.exec() != QDialog::Accepted) {
      return QString();
   }

   const QStringList selectedFiles = fileDialog.selectedFiles();
   if (selectedFiles.empty()) {
      return QString();
   }

   if (selectedFilter) {
      *selectedFilter =  fileDialog.selectedNameFilter();
   }

   return selectedFiles.front();
#else
   return getOpenFileName(parent, caption, dir, filter, selectedFilter, options);
#endif
}

QStringList DialogGui::getOpenImageFileNames(QWidget *parent, const QString &caption, const QString &dir,
   const QString &filter, QString *selectedFilter, QFileDialog::FileDialogOptions options )
{
#ifdef IMAGE_PREVIEW
   QFileDialog fileDialog(parent, caption, dir, filter);
   initializeImageFileDialog(fileDialog, options, QFileDialog::ExistingFiles);
   if (fileDialog.exec() != QDialog::Accepted) {
      return QStringList();
   }

   const QStringList selectedFiles = fileDialog.selectedFiles();
   if (!selectedFiles.empty() && selectedFilter) {
      *selectedFilter =  fileDialog.selectedNameFilter();
   }

   return selectedFiles;
#else
   return getOpenFileNames(parent, caption, dir, filter, selectedFilter, options);
#endif
}

}

