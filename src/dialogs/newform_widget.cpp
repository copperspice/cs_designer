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

#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <abstract_language.h>
#include <abstract_widgetdatabase.h>
#include <extension.h>
#include <newform_widget.h>
#include <ui_new_form_widget.h>

#include <designer_formbuilder.h>
#include <sheet_delegate_p.h>
#include <widgetdatabase_p.h>
#include <shared_settings_p.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QByteArray>
#include <QBuffer>
#include <QDir>
#include <QTextStream>
#include <QApplication>
#include <QDesktopWidget>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QPainter>
#include <QPushButton>

constexpr const int INDEX_OFFSET = 1;

enum NewForm_CustomRole {
   // File name (templates from resources, paths)
   TemplateNameRole = Qt::UserRole + 100,

   // Class name (widgets from Widget data base)
   ClassNameRole = Qt::UserRole + 101
};

static const QString newFormObjectNameC = "Form";

// Create a form name for an arbitrary class
static QString formNameFromClass(const QString &className)
{
   if (! className.startsWith('Q')) {
      return newFormObjectNameC;
   }

   QString rc = className;

   rc.remove(0, 1);

   return rc;
}

namespace qdesigner_internal {

struct TemplateSize {
   const QString name;
   int width;
   int height;
};

static const struct TemplateSize templateSizes[] = {
   { "Default size",         0,       0 },
   { "Portrait  (480x640)",  480,   640 },
   { "Landscape (640x480)",  640,   480 },
   { "Portrait  (768x1024)", 768,  1024 },
   { "Landscape (1024x768)", 1024,  768 },      // emerald, add two more later on
};

NewFormWidget::NewFormWidget(QDesignerFormEditorInterface *core, QWidget *parentWidget)
   : QDesignerNewFormWidgetInterface(parentWidget), m_core(core),
     m_ui(new Ui::NewFormWidget), m_currentItem(nullptr), m_acceptedItem(nullptr)
{
   using DeviceProfileList = QList<qdesigner_internal::DeviceProfile>;

   m_ui->setupUi(this);
   m_ui->treeWidget->setItemDelegate(new qdesigner_internal::SheetDelegate(m_ui->treeWidget, this));
   m_ui->treeWidget->header()->hide();
   m_ui->treeWidget->header()->setStretchLastSection(true);

   m_ui->lblPreview->setBackgroundRole(QPalette::Base);

   QDesignerSharedSettings settings(m_core);

   QString templatePath = ":/resources/templates";
   QString uiExtension  = "ui";

   QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core);

   if (lang) {
      uiExtension  = lang->uiExtension();
   }

   // (A) Resource templates
   const QString formTemplate   = settings.formTemplate();
   QTreeWidgetItem *selectedItem = nullptr;

   loadFrom(templatePath, true, uiExtension, formTemplate, selectedItem);

   // (B) Additional template paths
   const QStringList formTemplatePaths = settings.formTemplatePaths();

   for (auto item : formTemplatePaths) {
      loadFrom(item, false, uiExtension, formTemplate, selectedItem);
   }

   // (C) Widgets/custom widgets
   if (! lang) {
      // New Form Dialog Categories
      loadFrom(tr("Widgets"), qdesigner_internal::WidgetDataBase::formWidgetClasses(core), formTemplate, selectedItem);
      loadFrom(tr("Custom Widgets"), qdesigner_internal::WidgetDataBase::customFormWidgetClasses(core), formTemplate, selectedItem);
   }

   connect(m_ui->treeWidget, &QTreeWidget::itemActivated,      this, &NewFormWidget::slot_treeWidget_itemActivated);
   connect(m_ui->treeWidget, &QTreeWidget::currentItemChanged, this, &NewFormWidget::slot_treeWidget_currentItemChanged);
   connect(m_ui->treeWidget, &QTreeWidget::itemPressed,        this, &NewFormWidget::slot_treeWidget_itemPressed);

   // no selection set, default to first item
   if (selectedItem == nullptr && m_ui->treeWidget->topLevelItemCount() != 0) {
      QTreeWidgetItem *firstTopLevel = m_ui->treeWidget->topLevelItem(0);

      if (firstTopLevel->childCount() > 0) {
         selectedItem = firstTopLevel->child(0);
      }
   }

   // Open parent, select and make visible
   if (selectedItem) {
      m_ui->treeWidget->setCurrentItem(selectedItem);
      m_ui->treeWidget->setItemSelected(selectedItem, true);
      m_ui->treeWidget->scrollToItem(selectedItem->parent());
   }

   // Fill profile combo
   m_deviceProfiles = settings.deviceProfiles();
   m_ui->profileComboBox->addItem(tr("None"));

   connect(m_ui->profileComboBox, cs_mp_cast<int>(&QComboBox::currentIndexChanged),
      this, &NewFormWidget::slotDeviceProfileIndexChanged);

   if (m_deviceProfiles.empty()) {
      m_ui->profileComboBox->setEnabled(false);

   } else {
      const DeviceProfileList::const_iterator dcend = m_deviceProfiles.constEnd();
      for (DeviceProfileList::const_iterator it = m_deviceProfiles.constBegin(); it != dcend; ++it) {
         m_ui->profileComboBox->addItem(it->name());
      }
      const int ci = settings.currentDeviceProfileIndex();
      if (ci >= 0) {
         m_ui->profileComboBox->setCurrentIndex(ci + INDEX_OFFSET);
      }
   }

   // Fill size combo
   for (const auto &item : templateSizes) {
      const QSize size = QSize(item.width, item.height);
      m_ui->sizeComboBox->addItem(item.name, size);
   }

   setTemplateSize(settings.newFormSize());
}

NewFormWidget::~NewFormWidget()
{
   QDesignerSharedSettings settings (m_core);
   settings.setNewFormSize(templateSize());

   // Do not change previously stored item if dialog was rejected
   if (m_acceptedItem) {
      settings.setFormTemplate(m_acceptedItem->text(0));
   }

   delete m_ui;
}

void NewFormWidget::slot_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
   if (current == nullptr) {
      return;
   }

   if (current->parent() == nullptr) {
      // Top level item: Ensure expanded when browsing down
      return;
   }

   m_currentItem = current;

   emit currentTemplateChanged(showCurrentItemPixmap());
}

bool NewFormWidget::showCurrentItemPixmap()
{
   bool rc = false;

   if (m_currentItem) {
      const QPixmap pixmap = formPreviewPixmap(m_currentItem);

      if (pixmap.isNull()) {
         m_ui->lblPreview->setText(tr("Error loading form"));
      } else {
         m_ui->lblPreview->setPixmap(pixmap);
         rc = true;
      }
   }

   return rc;
}

void NewFormWidget::slot_treeWidget_itemActivated(QTreeWidgetItem *item)
{
   if (item->data(0, TemplateNameRole).isValid() || item->data(0, ClassNameRole).isValid()) {
      emit templateActivated();
   }
}

QPixmap NewFormWidget::formPreviewPixmap(const QTreeWidgetItem *item)
{
   // Cache pixmaps per item/device profile
   const ItemPixmapCacheKey cacheKey(item, profileComboIndex());
   ItemPixmapCache::iterator it = m_itemPixmapCache.find(cacheKey);

   if (it == m_itemPixmapCache.end()) {

      // file or string?
      const QVariant fileName = item->data(0, TemplateNameRole);
      QPixmap rc;

      if (fileName.type() == QVariant::String) {
         rc = formPreviewPixmap(fileName.toString());

      } else {
         const QVariant classNameV = item->data(0, ClassNameRole);
         Q_ASSERT(classNameV.type() == QVariant::String);

         const QString className = classNameV.toString();

         QByteArray data = qdesigner_internal::WidgetDataBase::formTemplate(m_core, className, formNameFromClass(className)).toUtf8();

         QBuffer buffer(&data);
         buffer.open(QIODevice::ReadOnly);
         rc = formPreviewPixmap(buffer);
      }

      if (rc.isNull()) {
         // Retry invalid ones
         return rc;
      }

      it = m_itemPixmapCache.insert(cacheKey, rc);
   }

   return it.value();
}

QPixmap NewFormWidget::formPreviewPixmap(const QString &fileName) const
{
   QFile f(fileName);

   if (f.open(QFile::ReadOnly)) {
      QFileInfo fi(fileName);

      const QPixmap rc = formPreviewPixmap(f, fi.absolutePath());
      f.close();

      return rc;
   }

   qWarning() << "The file " << fileName << " could not be opened: " << f.errorString();

   return QPixmap();
}

QImage NewFormWidget::grabForm(QDesignerFormEditorInterface *core, QIODevice &file,
   const QString &workingDir, const qdesigner_internal::DeviceProfile &dp)
{
   qdesigner_internal::NewFormWidgetFormBuilder formBuilder(core, dp);

   if (! workingDir.isEmpty()) {
      formBuilder.setWorkingDirectory(workingDir);
   }

   QWidget *widget = formBuilder.load(&file, nullptr);

   if (widget == nullptr) {
      return QImage();
   }

   const QPixmap pixmap = widget->grab(QRect(0, 0, -1, -1));
   widget->deleteLater();

   return pixmap.toImage();
}

QPixmap NewFormWidget::formPreviewPixmap(QIODevice &file, const QString &workingDir) const
{
   const QSizeF screenSize(QApplication::desktop()->screenGeometry(this).size());
   const int previewSize = qRound(screenSize.width() / 7.5);   // 256 on 1920px screens.
   const int margin = previewSize / 32 - 1;                    // 7 on 1920px screens.
   const int shadow = margin;

   const QImage wimage = grabForm(m_core, file, workingDir, currentDeviceProfile());

   if (wimage.isNull()) {
      return QPixmap();
   }

   const qreal devicePixelRatio = wimage.devicePixelRatioF();
   const QSize imageSize(previewSize - margin * 2, previewSize - margin * 2);

   QImage image = wimage.scaled((QSizeF(imageSize) * devicePixelRatio).toSize(),
         Qt::KeepAspectRatio, Qt::SmoothTransformation);

   image.setDevicePixelRatio(devicePixelRatio);

   QImage dest((QSizeF(previewSize, previewSize) * devicePixelRatio).toSize(), QImage::Format_ARGB32_Premultiplied);
   dest.setDevicePixelRatio(devicePixelRatio);
   dest.fill(0);

   QPainter p(&dest);
   p.drawImage(margin, margin, image);

   p.setPen(QPen(palette().brush(QPalette::WindowText), 0));

   p.drawRect(QRectF(margin - 1, margin - 1, imageSize.width() + 1.5, imageSize.height() + 1.5));

   const QColor dark(Qt::darkGray);
   const QColor light(Qt::transparent);

   // right shadow
   {
      const QRect rect(margin + imageSize.width() + 1, margin + shadow, shadow, imageSize.height() - shadow + 1);
      QLinearGradient lg(rect.topLeft(), rect.topRight());
      lg.setColorAt(0, dark);
      lg.setColorAt(1, light);
      p.fillRect(rect, lg);
   }

   // bottom shadow
   {
      const QRect rect(margin + shadow, margin + imageSize.height() + 1, imageSize.width() - shadow + 1, shadow);
      QLinearGradient lg(rect.topLeft(), rect.bottomLeft());
      lg.setColorAt(0, dark);
      lg.setColorAt(1, light);
      p.fillRect(rect, lg);
   }

   // bottom/right corner shadow
   {
      const QRect rect(margin + imageSize.width() + 1, margin + imageSize.height() + 1, shadow, shadow);
      QRadialGradient g(rect.topLeft(), shadow - 1);
      g.setColorAt(0, dark);
      g.setColorAt(1, light);
      p.fillRect(rect, g);
   }

   // top/right corner
   {
      const QRect rect(margin + imageSize.width() + 1, margin, shadow, shadow);
      QRadialGradient g(rect.bottomLeft(), shadow - 1);
      g.setColorAt(0, dark);
      g.setColorAt(1, light);
      p.fillRect(rect, g);
   }

   // bottom/left corner
   {
      const QRect rect(margin, margin + imageSize.height() + 1, shadow, shadow);
      QRadialGradient g(rect.topRight(), shadow - 1);
      g.setColorAt(0, dark);
      g.setColorAt(1, light);
      p.fillRect(rect, g);
   }

   p.end();

   return QPixmap::fromImage(dest);
}

void NewFormWidget::loadFrom(const QString &path, bool resourceFile, const QString &uiExtension,
   const QString &selectedItem, QTreeWidgetItem *&selectedItemFound)
{
   const QDir dir(path);

   if (!dir.exists()) {
      return;
   }

   // Iterate through the directory and add the templates
   const QFileInfoList list = dir.entryInfoList(QStringList(QString("*.") + uiExtension),
         QDir::Files);

   if (list.isEmpty()) {
      return;
   }

   const QChar separator = resourceFile ? QChar('/') : QDir::separator();
   QTreeWidgetItem *root = new QTreeWidgetItem(m_ui->treeWidget);

   root->setFlags(root->flags() & ~Qt::ItemIsSelectable);

   // Try to get something that is easy to read.
   QString visiblePath = path;
   int index = visiblePath.lastIndexOf(separator);

   if (index != -1) {
      // try to find a second slash, just to be a bit better.
      const int index2 = visiblePath.lastIndexOf(separator, index - 1);
      if (index2 != -1) {
         index = index2;
      }
      visiblePath = visiblePath.mid(index + 1);
      visiblePath = QDir::toNativeSeparators(visiblePath);
   }

   const QChar underscore = '_';
   const QChar blank      = ' ';
   root->setText(0, visiblePath.replace(underscore, blank));
   root->setToolTip(0, path);

   const QFileInfoList::const_iterator lcend = list.constEnd();
   for (QFileInfoList::const_iterator it = list.constBegin(); it != lcend; ++it) {
      if (!it->isFile()) {
         continue;
      }

      QTreeWidgetItem *item = new QTreeWidgetItem(root);
      const QString text = it->baseName().replace(underscore, blank);

      if (selectedItemFound == nullptr && text == selectedItem) {
         selectedItemFound = item;
      }
      item->setText(0, text);
      item->setData(0, TemplateNameRole, it->absoluteFilePath());
   }
}

void NewFormWidget::loadFrom(const QString &title, const QStringList &nameList,
   const QString &selectedItem, QTreeWidgetItem *&selectedItemFound)
{
   if (nameList.empty()) {
      return;
   }

   QTreeWidgetItem *root = new QTreeWidgetItem(m_ui->treeWidget);
   root->setFlags(root->flags() & ~Qt::ItemIsSelectable);
   root->setText(0, title);

   const QStringList::const_iterator cend = nameList.constEnd();

   for (QStringList::const_iterator it = nameList.constBegin(); it != cend; ++it) {
      const QString text = *it;

      QTreeWidgetItem *item = new QTreeWidgetItem(root);
      item->setText(0, text);

      if (selectedItemFound == nullptr && text == selectedItem) {
         selectedItemFound = item;
      }
      item->setData(0, ClassNameRole, *it);
   }
}

void NewFormWidget::slot_treeWidget_itemPressed(QTreeWidgetItem *item)
{
   if (item && !item->parent()) {
      m_ui->treeWidget->setItemExpanded(item, ! m_ui->treeWidget->isItemExpanded(item));
   }
}

QSize NewFormWidget::templateSize() const
{
   return m_ui->sizeComboBox->itemData(m_ui->sizeComboBox->currentIndex()).toSize();
}

void NewFormWidget::setTemplateSize(const QSize &s)
{
   const int index = s.isNull() ? 0 : m_ui->sizeComboBox->findData(s);

   if (index != -1) {
      m_ui->sizeComboBox->setCurrentIndex(index);
   }
}

static QString readAll(const QString &fileName, QString *errorMessage)
{
   QFile file(fileName);

   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      *errorMessage = NewFormWidget::tr("Unable to open the form template file '%1': %2")
         .formatArgs(fileName, file.errorString());
      return QString();
   }

   return QString::fromUtf8(file.readAll());
}

QString NewFormWidget::itemToTemplate(const QTreeWidgetItem *item, QString *errorMessage) const
{
   const QSize size = templateSize();

   // file name or string contents?
   const QVariant templateFileName = item->data(0, TemplateNameRole);

   if (templateFileName.type() == QVariant::String) {
      const QString fileName = templateFileName.toString();

      // No fixed size: just open.
      if (size.isNull()) {
         return readAll(fileName, errorMessage);
      }

      // try to find a file matching the size, like "../640x480/xx.ui"
      const QFileInfo fiBase(fileName);
      QString sizeFileName;

      QTextStream(&sizeFileName) << fiBase.path() << QDir::separator()
          << size.width() << 'x' << size.height() << QDir::separator()
          << fiBase.fileName();

      if (QFileInfo(sizeFileName).isFile()) {
         return readAll(sizeFileName, errorMessage);
      }

      // Nothing found, scale via DOM/temporary file
      QString contents = readAll(fileName, errorMessage);
      if (!contents.isEmpty()) {
         contents = qdesigner_internal::WidgetDataBase::scaleFormTemplate(contents, size, false);
      }

      return contents;
   }

   // Content
   const QString className = item->data(0, ClassNameRole).toString();
   QString contents = qdesigner_internal::WidgetDataBase::formTemplate(m_core, className, formNameFromClass(className));

   if (! size.isNull()) {
      contents = qdesigner_internal::WidgetDataBase::scaleFormTemplate(contents, size, false);
   }

   return contents;
}

void NewFormWidget::slotDeviceProfileIndexChanged(int idx)
{
   // Store index for form windows to take effect and refresh pixmap
   QDesignerSharedSettings settings(m_core);
   settings.setCurrentDeviceProfileIndex(idx - INDEX_OFFSET);
   showCurrentItemPixmap();
}

int NewFormWidget::profileComboIndex() const
{
   return m_ui->profileComboBox->currentIndex();
}

qdesigner_internal::DeviceProfile NewFormWidget::currentDeviceProfile() const
{
   const int ci = profileComboIndex();
   if (ci > 0) {
      return m_deviceProfiles.at(ci - INDEX_OFFSET);
   }
   return qdesigner_internal::DeviceProfile();
}

bool NewFormWidget::hasCurrentTemplate() const
{
   return m_currentItem != nullptr;
}

QString NewFormWidget::currentTemplateI(QString *ptrToErrorMessage)
{
   if (m_currentItem == nullptr) {
      *ptrToErrorMessage = tr("Internal error: No template selected.");
      return QString();
   }

   const QString contents = itemToTemplate(m_currentItem, ptrToErrorMessage);
   if (contents.isEmpty()) {
      return contents;
   }

   m_acceptedItem = m_currentItem;

   return contents;
}

QString NewFormWidget::currentTemplate(QString *ptrToErrorMessage)
{
   if (ptrToErrorMessage) {
      return currentTemplateI(ptrToErrorMessage);
   }

   // Do not loose the error
   QString errorMessage;
   const QString contents = currentTemplateI(&errorMessage);

   if (! errorMessage.isEmpty()) {
      qWarning("%s", csPrintable(errorMessage));
   }
   return contents;
}

}

