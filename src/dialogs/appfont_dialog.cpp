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

#include <appfont_dialog.h>
#include <designer_settings.h>
#include <utils.h>

#include <iconloader_p.h>

#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QSpacerItem>
#include <QStandardItemModel>
#include <QStringList>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVector>
#include <qalgorithms.h>

#include <algorithm>

constexpr const int FileNameRole = Qt::UserRole + 1;
constexpr const int IdRole       = Qt::UserRole + 2;

static const char fontFileKeyC[] = "fontFiles";

// AppFontManager: Singleton that maintains the mapping of loaded application font
// ids to the file names (which are not stored in QFontDatabase)
// and provides API for loading/unloading fonts as well for saving/restoring settings.

class AppFontManager
{
   Q_DISABLE_COPY(AppFontManager)
   AppFontManager();

 public:
   static AppFontManager &instance();

   void save(QDesignerSettingsInterface *s, const QString &prefix) const;
   void restore(const QDesignerSettingsInterface *s, const QString &prefix);

   // Return id or -1
   int add(const QString &fontFile, QString *errorMessage);

   bool remove(int id, QString *errorMessage);
   bool remove(const QString &fontFile, QString *errorMessage);
   bool removeAt(int index, QString *errorMessage);

   // Store loaded fonts as pair of file name and Id
   typedef QPair<QString, int> FileNameFontIdPair;
   typedef QList<FileNameFontIdPair> FileNameFontIdPairs;
   const FileNameFontIdPairs &fonts() const;

 private:
   FileNameFontIdPairs m_fonts;
};

AppFontManager::AppFontManager()
{
}

AppFontManager &AppFontManager::instance()
{
   static AppFontManager rc;
   return rc;
}

void AppFontManager::save(QDesignerSettingsInterface *s, const QString &prefix) const
{
   // Store as list of file names
   QStringList fontFiles;
   const FileNameFontIdPairs::const_iterator cend = m_fonts.constEnd();

   for (FileNameFontIdPairs::const_iterator it = m_fonts.constBegin(); it != cend; ++it) {
      fontFiles.push_back(it->first);
   }

   s->beginGroup(prefix);
   s->setValue(fontFileKeyC,  fontFiles);
   s->endGroup();
}

void AppFontManager::restore(const QDesignerSettingsInterface *s, const QString &prefix)
{
   QString key = prefix + '/' + fontFileKeyC;
   const QStringList fontFiles = s->value(key, QStringList()).toStringList();

   if (!fontFiles.empty()) {
      QString errorMessage;
      const QStringList::const_iterator cend = fontFiles.constEnd();

      for (QStringList::const_iterator it = fontFiles.constBegin(); it != cend; ++it)

         if (add(*it, &errorMessage) == -1) {
            csWarning(errorMessage);
         }
   }
}

int AppFontManager::add(const QString &fontFile, QString *errorMessage)
{
   const QFileInfo inf(fontFile);

   if (!inf.isFile()) {
      *errorMessage = QCoreApplication::translate("AppFontManager", "'%1' is not a file.").formatArg(fontFile);
      return -1;
   }

   if (!inf.isReadable()) {
      *errorMessage = QCoreApplication::translate("AppFontManager",
            "The font file '%1' does not have read permissions.").formatArg(fontFile);
      return -1;
   }
   const QString fullPath = inf.absoluteFilePath();
   // Check if already loaded
   const FileNameFontIdPairs::const_iterator cend = m_fonts.constEnd();

   for (FileNameFontIdPairs::const_iterator it = m_fonts.constBegin(); it != cend; ++it) {
      if (it->first == fullPath) {
         *errorMessage = QCoreApplication::translate("AppFontManager", "The font file '%1' is already loaded.").formatArg(fontFile);
         return -1;
      }
   }

   const int id = QFontDatabase::addApplicationFont(fullPath);
   if (id == -1) {
      *errorMessage = QCoreApplication::translate("AppFontManager", "The font file '%1' could not be loaded.").formatArg(fontFile);
      return -1;
   }

   m_fonts.push_back(FileNameFontIdPair(fullPath, id));
   return id;
}

bool AppFontManager::remove(int id, QString *errorMessage)
{
   const int count = m_fonts.size();
   for (int i = 0; i < count; i++)
      if (m_fonts[i].second == id) {
         return removeAt(i, errorMessage);
      }

   *errorMessage = QCoreApplication::translate("AppFontManager", "'%1' is not a valid font id.").formatArg(id);
   return false;
}

bool AppFontManager::remove(const QString &fontFile, QString *errorMessage)
{
   const int count = m_fonts.size();

   for (int i = 0; i < count; i++)
      if (m_fonts[i].first == fontFile) {
         return removeAt(i, errorMessage);
      }

   *errorMessage = QCoreApplication::translate("AppFontManager",
         "There is no loaded font matching the id '%1'.").formatArg(fontFile);

   return false;
}

bool AppFontManager::removeAt(int index, QString *errorMessage)
{
   Q_ASSERT(index >= 0 && index < m_fonts.size());

   const QString fontFile = m_fonts[index].first;
   const int id = m_fonts[index].second;

   if (!QFontDatabase::removeApplicationFont(id)) {
      *errorMessage = QCoreApplication::translate("AppFontManager",
            "The font '%1' (%2) could not be unloaded.").formatArg(fontFile).formatArg(id);
      return false;
   }
   m_fonts.removeAt(index);
   return true;
}

const AppFontManager::FileNameFontIdPairs &AppFontManager::fonts() const
{
   return  m_fonts;
}

// ------------- AppFontModel
class AppFontModel : public QStandardItemModel
{
   Q_DISABLE_COPY(AppFontModel)
 public:
   AppFontModel(QObject *parent = nullptr);

   void init(const AppFontManager &mgr);
   void add(const QString &fontFile, int id);
   int idAt(const QModelIndex &idx) const;
};

AppFontModel::AppFontModel(QObject *parent) :
   QStandardItemModel(parent)
{
   setHorizontalHeaderLabels(QStringList(AppFontWidget::tr("Fonts")));
}

void AppFontModel::init(const AppFontManager &mgr)
{
   typedef AppFontManager::FileNameFontIdPairs FileNameFontIdPairs;

   const FileNameFontIdPairs &fonts = mgr.fonts();
   const FileNameFontIdPairs::const_iterator cend = fonts.constEnd();
   for (FileNameFontIdPairs::const_iterator it = fonts.constBegin(); it != cend; ++it) {
      add(it->first, it->second);
   }
}

void AppFontModel::add(const QString &fontFile, int id)
{
   const QFileInfo inf(fontFile);
   // Root item with base name
   QStandardItem *fileItem = new QStandardItem(inf.completeBaseName());
   const QString fullPath = inf.absoluteFilePath();
   fileItem->setData(fullPath, FileNameRole);
   fileItem->setToolTip(fullPath);
   fileItem->setData(id, IdRole);
   fileItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

   appendRow(fileItem);
   const QStringList families = QFontDatabase::applicationFontFamilies(id);
   const QStringList::const_iterator cend = families.constEnd();
   for (QStringList::const_iterator it = families.constBegin(); it != cend; ++it) {
      QStandardItem *familyItem = new QStandardItem(*it);
      familyItem->setToolTip(fullPath);
      familyItem->setFont(QFont(*it));
      familyItem->setFlags(Qt::ItemIsEnabled);
      fileItem->appendRow(familyItem);
   }
}

int AppFontModel::idAt(const QModelIndex &idx) const
{
   if (const QStandardItem *item = itemFromIndex(idx)) {
      return item->data(IdRole).toInt();
   }
   return -1;
}

// ------------- AppFontWidget
AppFontWidget::AppFontWidget(QWidget *parent) :
   QGroupBox(parent),
   m_view(new QTreeView),
   m_addButton(new QToolButton),
   m_removeButton(new QToolButton),
   m_removeAllButton(new QToolButton),
   m_model(new AppFontModel(this))
{
   m_model->init(AppFontManager::instance());
   m_view->setModel(m_model);
   m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
   m_view->expandAll();
   connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AppFontWidget::selectionChanged);

   m_addButton->setToolTip(tr("Add font files"));
   m_addButton->setIcon(qdesigner_internal::createIconSet("plus.png"));
   connect(m_addButton, &QAbstractButton::clicked, this, &AppFontWidget::addFiles);

   m_removeButton->setEnabled(false);
   m_removeButton->setToolTip(tr("Remove current font file"));
   m_removeButton->setIcon(qdesigner_internal::createIconSet(QString::fromUtf8("minus.png")));
   connect(m_removeButton, &QAbstractButton::clicked, this, &AppFontWidget::slotRemoveFiles);

   m_removeAllButton->setToolTip(tr("Remove all font files"));
   m_removeAllButton->setIcon(qdesigner_internal::createIconSet(QString::fromUtf8("editdelete.png")));
   connect(m_removeAllButton, &QAbstractButton::clicked, this, &AppFontWidget::slotRemoveAll);

   QHBoxLayout *hLayout = new QHBoxLayout;
   hLayout->addWidget(m_addButton);
   hLayout->addWidget(m_removeButton);
   hLayout->addWidget(m_removeAllButton);
   hLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));

   QVBoxLayout *vLayout = new QVBoxLayout;
   vLayout->addWidget(m_view);
   vLayout->addLayout(hLayout);
   setLayout(vLayout);
}

void AppFontWidget::addFiles()
{
   const QStringList files =
      QFileDialog::getOpenFileNames(this, tr("Add Font Files"), QString(),
         tr("Font files (*.ttf)"));
   if (files.empty()) {
      return;
   }

   QString errorMessage;

   AppFontManager &fmgr = AppFontManager::instance();
   const QStringList::const_iterator cend = files.constEnd();
   for (QStringList::const_iterator it = files.constBegin(); it != cend; ++it) {
      const int id = fmgr.add(*it, &errorMessage);
      if (id != -1) {
         m_model->add(*it, id);
      } else {
         QMessageBox::critical(this, tr("Error Adding Fonts"), errorMessage);
      }
   }
   m_view->expandAll();
}

static void removeFonts(const QModelIndexList &selectedIndexes, AppFontModel *model, QWidget *dialogParent)
{
   if (selectedIndexes.empty()) {
      return;
   }

   // Reverse sort top level rows and remove
   AppFontManager &fmgr = AppFontManager::instance();
   QVector<int> rows;
   rows.reserve(selectedIndexes.size());

   QString errorMessage;
   const QModelIndexList::const_iterator cend = selectedIndexes.constEnd();
   for (QModelIndexList::const_iterator it = selectedIndexes.constBegin(); it != cend; ++it) {
      const int id = model->idAt(*it);
      if (id != -1) {
         if (fmgr.remove(id, &errorMessage)) {
            rows.push_back(it->row());
         } else {
            QMessageBox::critical(dialogParent, AppFontWidget::tr("Error Removing Fonts"), errorMessage);
         }
      }
   }

   std::stable_sort(rows.begin(), rows.end());

   for (int i = rows.size() - 1; i >= 0; --i) {
      model->removeRow(rows[i]);
   }
}

void AppFontWidget::slotRemoveFiles()
{
   removeFonts(m_view->selectionModel()->selectedIndexes(), m_model, this);
}

void AppFontWidget::slotRemoveAll()
{
   const int count = m_model->rowCount();
   if (!count) {
      return;
   }

   const QMessageBox::StandardButton answer =
      QMessageBox::question(this, tr("Remove Fonts"), tr("Would you like to remove all fonts?"),
         QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
   if (answer == QMessageBox::No) {
      return;
   }

   QModelIndexList topLevels;
   for (int i = 0; i < count; i++) {
      topLevels.push_back(m_model->index(i, 0));
   }
   removeFonts(topLevels, m_model, this);
}

void AppFontWidget::selectionChanged(const QItemSelection &selected, const QItemSelection & /*deselected*/)
{
   m_removeButton->setEnabled(!selected.indexes().empty());
}

void AppFontWidget::save(QDesignerSettingsInterface *s, const QString &prefix)
{
   AppFontManager::instance().save(s, prefix);
}

void AppFontWidget::restore(const QDesignerSettingsInterface *s, const QString &prefix)
{
   AppFontManager::instance().restore(s, prefix);
}

AppFontDialog::AppFontDialog(QWidget *parent)
   : QDialog(parent), m_appFontWidget(new AppFontWidget)
{
   setAttribute(Qt::WA_DeleteOnClose, true);
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
   setWindowTitle(tr("Additional Fonts"));
   setModal(false);

   QVBoxLayout *vl = new  QVBoxLayout;
   vl->addWidget(m_appFontWidget);

   QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal);

   QDialog::connect(bb, &QDialogButtonBox::rejected, this, &AppFontDialog::reject);
   vl->addWidget(bb);
   setLayout(vl);
}
