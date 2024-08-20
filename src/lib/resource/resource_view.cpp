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
#include <designer_settings.h>
#include <resource_editor.h>
#include <resource_model.h>
#include <resource_view.h>

#include <iconloader_p.h>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QDir>
#include <QDomDocument>
#include <QDrag>
#include <QFileInfo>
#include <QHeaderView>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPainter>
#include <QPushButton>
#include <QQueue>
#include <QSplitter>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>

static const QString Geometry            = "Geometry";
static const QString ResourceViewDialogC = "ResourceDialog";
static const QString SplitterPosition    = "SplitterPosition";
static const QString elementResourceData = "resource";
static const QString fileAttribute       = "file";
static const QString typeAttribute       = "type";
static const QString typeImage           = "image";
static const QString typeOther           = "other";
static const QString typeStyleSheet      = "stylesheet";

class ResourceListWidget : public QListWidget
{
 public:
   ResourceListWidget(QWidget *parent = nullptr);

 protected:
   void startDrag(Qt::DropActions supportedActions) override;
};

ResourceListWidget::ResourceListWidget(QWidget *parent) :
   QListWidget(parent)
{
   setDragEnabled(true);
}

void ResourceListWidget::startDrag(Qt::DropActions supportedActions)
{
   if (supportedActions == Qt::MoveAction) {
      return;
   }

   QListWidgetItem *item = currentItem();
   if (!item) {
      return;
   }

   const QString filePath = item->data(Qt::UserRole).toString();
   const QIcon icon = item->icon();

   QMimeData *mimeData = new QMimeData;
   const QtResourceView::ResourceType type = icon.isNull() ? QtResourceView::ResourceOther : QtResourceView::ResourceImage;
   mimeData->setText(QtResourceView::encodeMimeData(type, filePath));

   QDrag *drag = new QDrag(this);
   if (!icon.isNull()) {
      const QSize size = icon.actualSize(iconSize());
      drag->setPixmap(icon.pixmap(size));
      drag->setHotSpot(QPoint(size.width() / 2, size.height() / 2));
   }

   drag->setMimeData(mimeData);
   drag->exec(Qt::CopyAction);
}

// ---------------------------- QtResourceViewPrivate
class QtResourceViewPrivate
{
   QtResourceView *q_ptr;
   Q_DECLARE_PUBLIC(QtResourceView)
 public:
   QtResourceViewPrivate(QDesignerFormEditorInterface *core);

   void slotResourceSetActivated(QtResourceSet *resourceSet);
   void slotCurrentPathChanged(QTreeWidgetItem *);
   void slotCurrentResourceChanged(QListWidgetItem *);
   void slotResourceActivated(QListWidgetItem *);
   void slotEditResources();
   void slotReloadResources();
   void slotCopyResourcePath();

   void slotListWidgetContextMenuRequested(const QPoint &pos);
   void slotFilterChanged(const QString &pattern);
   void createPaths();
   QTreeWidgetItem *createPath(const QString &path, QTreeWidgetItem *parent);
   void createResources(const QString &path);
   void storeExpansionState();
   void applyExpansionState();
   void restoreSettings();
   void saveSettings();
   void updateActions();
   void filterOutResources();

   QPixmap makeThumbnail(const QPixmap &pix) const;

   QDesignerFormEditorInterface *m_core;
   QtResourceModel *m_resourceModel;
   QToolBar *m_toolBar;
   QWidget *m_filterWidget;
   QTreeWidget *m_treeWidget;
   QListWidget *m_listWidget;
   QSplitter *m_splitter;
   QMap<QString, QStringList>       m_pathToContents; // full path to contents file names (full path to its resource filenames)
   QMap<QString, QString>           m_pathToParentPath; // full path to full parent path
   QMap<QString, QStringList>       m_pathToSubPaths; // full path to full sub paths
   QMap<QString, QTreeWidgetItem *> m_pathToItem;
   QMap<QTreeWidgetItem *, QString> m_itemToPath;
   QMap<QString, QListWidgetItem *> m_resourceToItem;
   QMap<QListWidgetItem *, QString> m_itemToResource;
   QAction *m_editResourcesAction;
   QAction *m_reloadResourcesAction;
   QAction *m_copyResourcePathAction;

   QMap<QString, bool> m_expansionState;

   bool m_ignoreGuiSignals;
   QString m_settingsKey;
   bool m_resourceEditingEnabled;
   QString m_filterPattern;
};

QtResourceViewPrivate::QtResourceViewPrivate(QDesignerFormEditorInterface *core)
   : q_ptr(nullptr), m_core(core), m_resourceModel(nullptr), m_toolBar(new QToolBar),
     m_treeWidget(new QTreeWidget), m_listWidget(new ResourceListWidget), m_splitter(nullptr),
     m_editResourcesAction(nullptr), m_reloadResourcesAction(nullptr), m_copyResourcePathAction(nullptr),
     m_ignoreGuiSignals(false), m_resourceEditingEnabled(true)
{
   m_toolBar->setIconSize(QSize(22, 22));
}

void QtResourceViewPrivate::restoreSettings()
{
   if (m_settingsKey.isEmpty()) {
      return;
   }

   QDesignerSettingsInterface *settings = m_core->settingsManager();
   settings->beginGroup(m_settingsKey);

   m_splitter->restoreState(settings->value(SplitterPosition).toByteArray());
   settings->endGroup();
}

void QtResourceViewPrivate::saveSettings()
{
   if (m_settingsKey.isEmpty()) {
      return;
   }

   QDesignerSettingsInterface *settings = m_core->settingsManager();
   settings->beginGroup(m_settingsKey);

   settings->setValue(SplitterPosition, m_splitter->saveState());
   settings->endGroup();
}

void QtResourceViewPrivate::slotEditResources()
{
   const QString selectedResource = QtResourceEditorDialog::editResources(m_core,
         m_resourceModel, m_core->dialogGui(), q_ptr);

   if (! selectedResource.isEmpty()) {
      // populate the browser
      q_ptr->selectResource(selectedResource);
   }
}

void QtResourceViewPrivate::slotReloadResources()
{
   if (m_resourceModel) {
      int errorCount;

      QString errorMessages;
      m_resourceModel->reload(&errorCount, &errorMessages);

      if (errorCount) {
         QtResourceEditorDialog::displayResourceFailures(errorMessages, m_core->dialogGui(), q_ptr);
      }
   }
}

void QtResourceViewPrivate::slotCopyResourcePath()
{
   const QString path = q_ptr->selectedResource();
   QClipboard *clipboard = QApplication::clipboard();
   clipboard->setText(path);
}

void QtResourceViewPrivate::slotListWidgetContextMenuRequested(const QPoint &pos)
{
   QMenu menu(q_ptr);
   menu.addAction(m_copyResourcePathAction);
   menu.exec(m_listWidget->mapToGlobal(pos));
}

void QtResourceViewPrivate::slotFilterChanged(const QString &pattern)
{
   m_filterPattern = pattern;
   filterOutResources();
}

void QtResourceViewPrivate::storeExpansionState()
{
   QMapIterator<QString, QTreeWidgetItem *> it(m_pathToItem);
   while (it.hasNext()) {
      it.next();
      m_expansionState[it.key()] = it.value()->isExpanded();
   }
}

void QtResourceViewPrivate::applyExpansionState()
{
   QMapIterator<QString, QTreeWidgetItem *> it(m_pathToItem);
   while (it.hasNext()) {
      it.next();
      it.value()->setExpanded(m_expansionState.value(it.key(), true));
   }
}

QPixmap QtResourceViewPrivate::makeThumbnail(const QPixmap &pix) const
{
   int w = qMax(48, pix.width());
   int h = qMax(48, pix.height());
   QRect imgRect(0, 0, w, h);
   QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
   img.fill(0);
   if (!pix.isNull()) {
      QRect r(0, 0, pix.width(), pix.height());
      r.moveCenter(imgRect.center());
      QPainter p(&img);
      p.drawPixmap(r.topLeft(), pix);
   }
   return QPixmap::fromImage(img);
}

void QtResourceViewPrivate::updateActions()
{
   bool resourceActive = false;
   if (m_resourceModel) {
      resourceActive = m_resourceModel->currentResourceSet();
   }

   m_editResourcesAction->setVisible(m_resourceEditingEnabled);
   m_editResourcesAction->setEnabled(resourceActive);
   m_reloadResourcesAction->setEnabled(resourceActive);
   m_filterWidget->setEnabled(resourceActive);
}

void QtResourceViewPrivate::slotResourceSetActivated(QtResourceSet *resourceSet)
{
   (void) resourceSet;

   updateActions();

   storeExpansionState();
   const QString currentPath = m_itemToPath.value(m_treeWidget->currentItem());
   const QString currentResource = m_itemToResource.value(m_listWidget->currentItem());
   m_treeWidget->clear();
   m_pathToContents.clear();
   m_pathToParentPath.clear();
   m_pathToSubPaths.clear();
   m_pathToItem.clear();
   m_itemToPath.clear();
   m_listWidget->clear();
   m_resourceToItem.clear();
   m_itemToResource.clear();

   createPaths();
   applyExpansionState();

   if (! currentResource.isEmpty()) {
      q_ptr->selectResource(currentResource);
   } else if (! currentPath.isEmpty()) {
      q_ptr->selectResource(currentPath);
   }
   filterOutResources();
}

void QtResourceViewPrivate::slotCurrentPathChanged(QTreeWidgetItem *item)
{
   if (m_ignoreGuiSignals) {
      return;
   }

   m_listWidget->clear();
   m_resourceToItem.clear();
   m_itemToResource.clear();

   if (!item) {
      return;
   }

   const QString currentPath = m_itemToPath.value(item);
   createResources(currentPath);
}

void QtResourceViewPrivate::slotCurrentResourceChanged(QListWidgetItem *item)
{
   m_copyResourcePathAction->setEnabled(item);
   if (m_ignoreGuiSignals) {
      return;
   }

   emit q_ptr->resourceSelected(m_itemToResource.value(item));
}

void QtResourceViewPrivate::slotResourceActivated(QListWidgetItem *item)
{
   if (m_ignoreGuiSignals) {
      return;
   }

   emit q_ptr->resourceActivated(m_itemToResource.value(item));
}

void QtResourceViewPrivate::createPaths()
{
   if (! m_resourceModel) {
      return;
   }

   // Resource root
   const QString root(QString(":/"));

   QMap<QString, QString> contents = m_resourceModel->contents();
   QMapIterator<QString, QString> itContents(contents);

   while (itContents.hasNext()) {
      const QString filePath = itContents.next().key();
      const QFileInfo fi(filePath);

      QString dirPath = fi.absolutePath();
      m_pathToContents[dirPath].append(fi.fileName());

      while (!m_pathToParentPath.contains(dirPath) && dirPath != root) { // create all parent paths
         const QFileInfo fd(dirPath);
         const QString parentDirPath = fd.absolutePath();
         m_pathToParentPath[dirPath] = parentDirPath;
         m_pathToSubPaths[parentDirPath].append(dirPath);
         dirPath = parentDirPath;
      }
   }

   QQueue<QPair<QString, QTreeWidgetItem *>> pathToParentItemQueue;
   pathToParentItemQueue.enqueue(qMakePair(root, nullptr));

   while (! pathToParentItemQueue.isEmpty()) {
      QPair<QString, QTreeWidgetItem *> pathToParentItem = pathToParentItemQueue.dequeue();
      const QString path = pathToParentItem.first;

      QTreeWidgetItem *item = createPath(path, pathToParentItem.second);
      QStringList subPaths  = m_pathToSubPaths.value(path);
      QStringListIterator itSubPaths(subPaths);

      while (itSubPaths.hasNext()) {
         pathToParentItemQueue.enqueue(qMakePair(itSubPaths.next(), item));
      }
   }
}

void QtResourceViewPrivate::filterOutResources()
{
   QMap<QString, bool> pathToMatchingContents; // true means the path has any matching contents
   QMap<QString, bool> pathToVisible; // true means the path has to be shown

   // 1) we go from root path recursively.
   // 2) we check every path if it contains at least one matching resource - if empty we add it
   //                 to pathToMatchingContents and pathToVisible with false, if non empty
   //                 we add it with true and change every parent path in pathToVisible to true.
   // 3) we hide these items which has pathToVisible value false.

   const bool matchAll = m_filterPattern.isEmpty();
   const QString root(QString(":/"));

   QQueue<QString> pathQueue;
   pathQueue.enqueue(root);

   while (!pathQueue.isEmpty()) {
      const QString path = pathQueue.dequeue();

      QStringList fileNames = m_pathToContents.value(path);
      QStringListIterator it(fileNames);
      bool hasContents = matchAll;
      if (!matchAll) { // the case filter is not empty - we check if the path contains anything
         while (it.hasNext()) {
            QString fileName = it.next();
            hasContents = fileName.contains(m_filterPattern, Qt::CaseInsensitive);
            if (hasContents) { // the path contains at least one resource which matches the filter
               break;
            }
         }
      }

      pathToMatchingContents[path] = hasContents;
      pathToVisible[path] = hasContents;

      if (hasContents) { // if the path is going to be shown we need to show all its parent paths
         QString parentPath = m_pathToParentPath.value(path);
         while (!parentPath.isEmpty()) {
            QString p = parentPath;
            if (pathToVisible.value(p)) { // parent path is already shown, we break the loop
               break;
            }
            pathToVisible[p] = true;
            parentPath = m_pathToParentPath.value(p);
         }
      }

      QStringList subPaths = m_pathToSubPaths.value(path); // we do the same for children paths
      QStringListIterator itSubPaths(subPaths);
      while (itSubPaths.hasNext()) {
         pathQueue.enqueue(itSubPaths.next());
      }
   }

   // we setup here new path and resource to be activated
   const QString currentPath = m_itemToPath.value(m_treeWidget->currentItem());
   QString newCurrentPath = currentPath;
   QString currentResource = m_itemToResource.value(m_listWidget->currentItem());
   if (!matchAll) {
      bool searchForNewPathWithContents = true;

      if (!currentPath.isEmpty()) { // if the currentPath is empty we will search for a new path too
         auto it = pathToMatchingContents.constFind(currentPath);
         if (it != pathToMatchingContents.constEnd() && it.value()) { // the current item has contents, we don't need to search for another path
            searchForNewPathWithContents = false;
         }
      }

      if (searchForNewPathWithContents) {
         // we find the first path with the matching contents
         auto itContents = pathToMatchingContents.constBegin();
         while (itContents != pathToMatchingContents.constEnd()) {
            if (itContents.value()) {
               newCurrentPath = itContents.key(); // the new path will be activated
               break;
            }

            ++itContents;
         }
      }

      QFileInfo fi(currentResource);
      if (!fi.fileName().contains(m_filterPattern, Qt::CaseInsensitive)) { // the case when the current resource is filtered out
         const QStringList fileNames = m_pathToContents.value(newCurrentPath);
         QStringListIterator it(fileNames);
         while (it.hasNext()) { // we try to select the first matching resource from the newCurrentPath
            QString fileName = it.next();
            if (fileName.contains(m_filterPattern, Qt::CaseInsensitive)) {
               QDir dirPath(newCurrentPath);
               currentResource = dirPath.absoluteFilePath(fileName); // the new resource inside newCurrentPath will be activated
               break;
            }
         }
      }
   }

   QTreeWidgetItem *newCurrentItem = m_pathToItem.value(newCurrentPath);
   if (currentPath != newCurrentPath) {
      m_treeWidget->setCurrentItem(newCurrentItem);
   } else {
      slotCurrentPathChanged(newCurrentItem);   // trigger filtering on the current path
   }

   QListWidgetItem *currentResourceItem = m_resourceToItem.value(currentResource);
   if (currentResourceItem) {
      m_listWidget->setCurrentItem(currentResourceItem);
      m_listWidget->scrollToItem(currentResourceItem);
   }

   QMapIterator<QString, bool> it(pathToVisible); // hide all paths filtered out

   while (it.hasNext()) {
      const QString path = it.next().key();
      QTreeWidgetItem *item = m_pathToItem.value(path);
      if (item) {
         item->setHidden(!it.value());
      }
   }
}

QTreeWidgetItem *QtResourceViewPrivate::createPath(const QString &path, QTreeWidgetItem *parent)
{
   QTreeWidgetItem *item = nullptr;

   if (parent) {
      item = new QTreeWidgetItem(parent);
   } else {
      item = new QTreeWidgetItem(m_treeWidget);
   }

   m_pathToItem[path] = item;
   m_itemToPath[item] = path;

   QString substPath;
   if (parent) {
      QFileInfo di(path);
      substPath = di.fileName();
   } else {
      substPath = QString("<resource root>");
   }

   item->setText(0, substPath);
   item->setToolTip(0, path);

   return item;
}

void QtResourceViewPrivate::createResources(const QString &path)
{
   const bool matchAll = m_filterPattern.isEmpty();

   QDir dir(path);
   QStringList fileNames = m_pathToContents.value(path);
   QStringListIterator it(fileNames);

   while (it.hasNext()) {
      QString fileName = it.next();
      const bool showProperty = matchAll || fileName.contains(m_filterPattern, Qt::CaseInsensitive);
      if (showProperty) {
         QString filePath = dir.absoluteFilePath(fileName);
         QFileInfo fi(filePath);
         if (fi.isFile()) {
            QListWidgetItem *item = new QListWidgetItem(fi.fileName(), m_listWidget);
            const QPixmap pix = QPixmap(filePath);
            if (pix.isNull()) {
               item->setToolTip(filePath);
            } else {
               item->setIcon(QIcon(makeThumbnail(pix)));
               const QSize size = pix.size();
               item->setToolTip(QtResourceView::tr("Size: %1 x %2\n%3").formatArg(size.width()).formatArg(size.height()).formatArg(filePath));
            }
            item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
            item->setData(Qt::UserRole, filePath);
            m_itemToResource[item] = filePath;
            m_resourceToItem[filePath] = item;
         }
      }
   }
}

QtResourceView::QtResourceView(QDesignerFormEditorInterface *core, QWidget *parent)
   : QWidget(parent), d_ptr(new QtResourceViewPrivate(core))
{
   d_ptr->q_ptr = this;

   QIcon editIcon = QIcon::fromTheme(QString("document-properties"), qdesigner_internal::createIconSet(QString("edit.png")));
   d_ptr->m_editResourcesAction = new QAction(editIcon, tr("Edit Resources..."), this);
   d_ptr->m_toolBar->addAction(d_ptr->m_editResourcesAction);
   connect(d_ptr->m_editResourcesAction, SIGNAL(triggered()), this, SLOT(slotEditResources()));
   d_ptr->m_editResourcesAction->setEnabled(false);

   QIcon refreshIcon = QIcon::fromTheme(QString("view-refresh"), qdesigner_internal::createIconSet(QString("reload.png")));
   d_ptr->m_reloadResourcesAction = new QAction(refreshIcon, tr("Reload"), this);

   d_ptr->m_toolBar->addAction(d_ptr->m_reloadResourcesAction);
   connect(d_ptr->m_reloadResourcesAction, SIGNAL(triggered()), this, SLOT(slotReloadResources()));
   d_ptr->m_reloadResourcesAction->setEnabled(false);

   QIcon copyIcon = QIcon::fromTheme(QString("edit-copy"), qdesigner_internal::createIconSet(QString("editcopy.png")));
   d_ptr->m_copyResourcePathAction = new QAction(copyIcon, tr("Copy Path"), this);
   connect(d_ptr->m_copyResourcePathAction, SIGNAL(triggered()), this, SLOT(slotCopyResourcePath()));
   d_ptr->m_copyResourcePathAction->setEnabled(false);

   d_ptr->m_filterWidget = new QWidget(d_ptr->m_toolBar);
   QHBoxLayout *filterLayout = new QHBoxLayout(d_ptr->m_filterWidget);
   filterLayout->setContentsMargins(0, 0, 0, 0);

   QLineEdit *filterLineEdit = new QLineEdit(d_ptr->m_filterWidget);
   connect(filterLineEdit, &QLineEdit::textChanged, this, &QtResourceView::slotFilterChanged);

   filterLineEdit->setPlaceholderText(tr("Filter"));
   filterLineEdit->setClearButtonEnabled(true);
   filterLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Ignored));
   filterLayout->addWidget(filterLineEdit);
   d_ptr->m_toolBar->addWidget(d_ptr->m_filterWidget);

   d_ptr->m_splitter = new QSplitter;
   d_ptr->m_splitter->setChildrenCollapsible(false);
   d_ptr->m_splitter->addWidget(d_ptr->m_treeWidget);
   d_ptr->m_splitter->addWidget(d_ptr->m_listWidget);

   QLayout *layout = new QVBoxLayout(this);
   layout->setMargin(0);
   layout->setSpacing(0);
   layout->addWidget(d_ptr->m_toolBar);
   layout->addWidget(d_ptr->m_splitter);

   d_ptr->m_treeWidget->setColumnCount(1);
   d_ptr->m_treeWidget->header()->hide();
   d_ptr->m_treeWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding));

   d_ptr->m_listWidget->setViewMode(QListView::IconMode);
   d_ptr->m_listWidget->setResizeMode(QListView::Adjust);
   d_ptr->m_listWidget->setIconSize(QSize(48, 48));
   d_ptr->m_listWidget->setGridSize(QSize(64, 64));

   d_ptr->m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

   connect(d_ptr->m_treeWidget, &QTreeWidget::currentItemChanged,
      this, &QtResourceView::slotCurrentPathChanged);

   connect(d_ptr->m_listWidget, &QListWidget::currentItemChanged,
      this, &QtResourceView::slotCurrentResourceChanged);

   connect(d_ptr->m_listWidget, &QListWidget::itemActivated,
      this, &QtResourceView::slotResourceActivated);

   connect(d_ptr->m_listWidget, &QListWidget::customContextMenuRequested,
      this, &QtResourceView::slotListWidgetContextMenuRequested);
}

QtResourceView::~QtResourceView()
{
   if (!d_ptr->m_settingsKey.isEmpty()) {
      d_ptr->saveSettings();
   }
}

bool QtResourceView::event(QEvent *event)
{
   if (event->type() == QEvent::Show) {
      d_ptr->m_listWidget->scrollToItem(d_ptr->m_listWidget->currentItem());
      d_ptr->m_treeWidget->scrollToItem(d_ptr->m_treeWidget->currentItem());
   }
   return QWidget::event(event);
}

QtResourceModel *QtResourceView::model() const
{
   return d_ptr->m_resourceModel;
}

QString QtResourceView::selectedResource() const
{
   QListWidgetItem *item = d_ptr->m_listWidget->currentItem();
   return d_ptr->m_itemToResource.value(item);
}

void QtResourceView::selectResource(const QString &resource)
{
   if (resource.isEmpty()) {
      return;
   }

   QFileInfo fi(resource);
   QDir dir = fi.absoluteDir();

   if (fi.isDir()) {
      dir = QDir(resource);
   }

   QString dirPath = dir.absolutePath();
   QMap<QString, QTreeWidgetItem *>::const_iterator it;

   while ((it = d_ptr->m_pathToItem.find(dirPath)) == d_ptr->m_pathToItem.constEnd()) {
      if (! dir.cdUp()) {
         break;
      }

      dirPath = dir.absolutePath();
   }

   if (it != d_ptr->m_pathToItem.constEnd()) {
      QTreeWidgetItem *treeItem = it.value();

      d_ptr->m_treeWidget->setCurrentItem(treeItem);
      d_ptr->m_treeWidget->scrollToItem(treeItem);

      QListWidgetItem *item = d_ptr->m_resourceToItem.value(resource);

      if (item) {
         d_ptr->m_listWidget->setCurrentItem(item);
         d_ptr->m_listWidget->scrollToItem(item);
      }
   }
}

QString QtResourceView::settingsKey() const
{
   return d_ptr->m_settingsKey;
}

void QtResourceView::setSettingsKey(const QString &key)
{
   if (d_ptr->m_settingsKey == key) {
      return;
   }

   d_ptr->m_settingsKey = key;

   if (key.isEmpty()) {
      return;
   }

   d_ptr->restoreSettings();
}

void QtResourceView::setResourceModel(QtResourceModel *model)
{
   if (d_ptr->m_resourceModel) {
      disconnect(d_ptr->m_resourceModel, SIGNAL(resourceSetActivated(QtResourceSet *, bool)),
         this, SLOT(slotResourceSetActivated(QtResourceSet *)));
   }

   // clear here
   d_ptr->m_treeWidget->clear();
   d_ptr->m_listWidget->clear();

   d_ptr->m_resourceModel = model;

   if (!d_ptr->m_resourceModel) {
      return;
   }

   connect(d_ptr->m_resourceModel, SIGNAL(resourceSetActivated(QtResourceSet *, bool)),
      this, SLOT(slotResourceSetActivated(QtResourceSet *)));

   // fill new here
   d_ptr->slotResourceSetActivated(d_ptr->m_resourceModel->currentResourceSet());
}

bool QtResourceView::isResourceEditingEnabled() const
{
   return d_ptr->m_resourceEditingEnabled;
}

void QtResourceView::setResourceEditingEnabled(bool enable)
{
   d_ptr->m_resourceEditingEnabled = enable;
   d_ptr->updateActions();
}

void QtResourceView::setDragEnabled(bool dragEnabled)
{
   d_ptr->m_listWidget->setDragEnabled(dragEnabled);
}

bool QtResourceView::dragEnabled() const
{
   return d_ptr->m_listWidget->dragEnabled();
}

QString QtResourceView::encodeMimeData(ResourceType resourceType, const QString &path)
{
   QDomDocument doc;
   QDomElement elem = doc.createElement(elementResourceData);

   switch (resourceType) {
      case ResourceImage:
         elem.setAttribute(typeAttribute, typeImage);
         break;
      case ResourceStyleSheet:
         elem.setAttribute(typeAttribute, typeStyleSheet);
         break;
      case ResourceOther:
         elem.setAttribute(typeAttribute, typeOther);
         break;
   }
   elem.setAttribute(fileAttribute, path);
   doc.appendChild(elem);
   return doc.toString();
}

bool QtResourceView::decodeMimeData(const QMimeData *md, ResourceType *t, QString *file)
{
   return md->hasText() ? decodeMimeData(md->text(), t, file) : false;
}

bool QtResourceView::decodeMimeData(const QString &text, ResourceType *t, QString *file)
{

   const QString docElementName = elementResourceData;
   static const QString docElementString = QLatin1Char('<') + docElementName;

   if (text.isEmpty() || text.indexOf(docElementString) == -1) {
      return false;
   }

   QDomDocument doc;
   if (!doc.setContent(text)) {
      return false;
   }

   const QDomElement domElement = doc.documentElement();
   if (domElement.tagName() != docElementName) {
      return false;
   }

   if (t) {
      const QString typeAttr = typeAttribute;
      if (domElement.hasAttribute (typeAttr)) {
         const QString typeValue = domElement.attribute(typeAttr, typeOther);
         if (typeValue == typeImage) {
            *t = ResourceImage;
         } else {
            *t = typeValue == typeStyleSheet ? ResourceStyleSheet : ResourceOther;
         }
      }
   }
   if (file) {
      const QString fileAttr = fileAttribute;
      if (domElement.hasAttribute(fileAttr)) {
         *file = domElement.attribute(fileAttr, QString());
      } else {
         file->clear();
      }
   }
   return true;
}

class QtResourceViewDialogPrivate
{
   QtResourceViewDialog *q_ptr;
   Q_DECLARE_PUBLIC(QtResourceViewDialog)

 public:
   QtResourceViewDialogPrivate(QDesignerFormEditorInterface *core);

   void slotResourceSelected(const QString &resource) {
      setOkButtonEnabled(!resource.isEmpty());
   }
   void setOkButtonEnabled(bool v)                    {
      m_box->button(QDialogButtonBox::Ok)->setEnabled(v);
   }

   QDesignerFormEditorInterface *m_core;
   QtResourceView *m_view;
   QDialogButtonBox *m_box;
};

QtResourceViewDialogPrivate::QtResourceViewDialogPrivate(QDesignerFormEditorInterface *core)
   : q_ptr(nullptr), m_core(core), m_view(new QtResourceView(core)),
     m_box(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal))
{
   m_view->setSettingsKey(ResourceViewDialogC);
}

QtResourceViewDialog::QtResourceViewDialog(QDesignerFormEditorInterface *core, QWidget *parent)
   : QDialog(parent), d_ptr(new QtResourceViewDialogPrivate(core))
{
   setWindowTitle(tr("Select Resource"));
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
   d_ptr->q_ptr = this;

   QVBoxLayout *layout = new QVBoxLayout(this);
   layout->addWidget(d_ptr->m_view);
   layout->addWidget(d_ptr->m_box);

   connect(d_ptr->m_box,  &QDialogButtonBox::accepted,        this, &QtResourceViewDialog::accept);
   connect(d_ptr->m_box,  &QDialogButtonBox::rejected,        this, &QtResourceViewDialog::reject);
   connect(d_ptr->m_view, &QtResourceView::resourceActivated, this, &QtResourceViewDialog::accept);
   connect(d_ptr->m_view, &QtResourceView::resourceSelected,  this, &QtResourceViewDialog::slotResourceSelected);

   d_ptr->setOkButtonEnabled(false);
   d_ptr->m_view->setResourceModel(core->resourceModel());

   QDesignerSettingsInterface *settings = core->settingsManager();
   settings->beginGroup(ResourceViewDialogC);

   const QVariant geometry = settings->value(Geometry);

   if (geometry.type() == QVariant::ByteArray) {
      // Used to be a QRect
      restoreGeometry(geometry.toByteArray());
   }

   settings->endGroup();
}

QtResourceViewDialog::~QtResourceViewDialog()
{
   QDesignerSettingsInterface *settings = d_ptr->m_core->settingsManager();
   settings->beginGroup(ResourceViewDialogC);

   settings->setValue(Geometry, saveGeometry());

   settings->endGroup();
}

QString QtResourceViewDialog::selectedResource() const
{
   return d_ptr->m_view->selectedResource();
}

void QtResourceViewDialog::selectResource(const QString &path)
{
   d_ptr->m_view->selectResource(path);
}

bool QtResourceViewDialog::isResourceEditingEnabled() const
{
   return d_ptr->m_view->isResourceEditingEnabled();
}

void QtResourceViewDialog::setResourceEditingEnabled(bool enable)
{
   d_ptr->m_view->setResourceEditingEnabled(enable);
}

void QtResourceView::slotResourceSetActivated(QtResourceSet *un_named_arg1)
{
   Q_D(QtResourceView);
   d->slotResourceSetActivated(un_named_arg1);
}

void QtResourceView::slotCurrentPathChanged(QTreeWidgetItem *un_named_arg1)
{
   Q_D(QtResourceView);
   d->slotCurrentPathChanged(un_named_arg1);
}

void QtResourceView::slotCurrentResourceChanged(QListWidgetItem *un_named_arg1)
{
   Q_D(QtResourceView);
   d->slotCurrentResourceChanged(un_named_arg1);
}

void QtResourceView::slotResourceActivated(QListWidgetItem *un_named_arg1)
{
   Q_D(QtResourceView);
   d->slotResourceActivated(un_named_arg1);
}

void QtResourceView::slotEditResources()
{
   Q_D(QtResourceView);
   d->slotEditResources();
}

void QtResourceView::slotReloadResources()
{
   Q_D(QtResourceView);
   d->slotReloadResources();
}

void QtResourceView::slotCopyResourcePath()
{
   Q_D(QtResourceView);
   d->slotCopyResourcePath();
}

void QtResourceView::slotListWidgetContextMenuRequested(const QPoint &pos)
{
   Q_D(QtResourceView);
   d->slotListWidgetContextMenuRequested(pos);
}

void QtResourceView::slotFilterChanged(const QString &pattern)
{
   Q_D(QtResourceView);
   d->slotFilterChanged(pattern);
}

void QtResourceViewDialog::slotResourceSelected(const QString &un_named_arg1)
{
   Q_D(QtResourceViewDialog);
   d->slotResourceSelected(un_named_arg1);
}