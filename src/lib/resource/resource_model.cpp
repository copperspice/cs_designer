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

#include <rcc_support.h>
#include <resource_model.h>
#include <utils.h>

#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QIODevice>
#include <QMap>
#include <QResource>
#include <QStringList>

class QtResourceSetPrivate
{
   QtResourceSet *q_ptr;
   Q_DECLARE_PUBLIC(QtResourceSet)

 public:
   QtResourceSetPrivate(QtResourceModel *model = nullptr);

   QtResourceModel *m_resourceModel;
};

QtResourceSetPrivate::QtResourceSetPrivate(QtResourceModel *model)
   : q_ptr(nullptr), m_resourceModel(model)
{
}

class QtResourceModelPrivate
{
   QtResourceModel *q_ptr;
   Q_DECLARE_PUBLIC(QtResourceModel)
   Q_DISABLE_COPY(QtResourceModelPrivate)

 public:
   QtResourceModelPrivate();

   void activate(QtResourceSet *resourceSet, const QStringList &newPaths,
         int *errorCount = nullptr, QString *errorMessages = nullptr);

   void removeOldPaths(QtResourceSet *resourceSet, const QStringList &newPaths);

   QMap<QString, bool> m_pathToModified;
   QMap<QtResourceSet *, QStringList> m_resourceSetToPaths;
   QMap<QtResourceSet *, bool> m_resourceSetToReload; // while path is recreated it needs to be reregistered

   // (it is in the new current resource set, but when the path was used in
   // other resource set
   // then later when that resource set is activated it needs to be reregistered)
   QMap<QtResourceSet *, bool> m_newlyCreated; // all created but not activated yet

   // (if was active at some point and it is not active now it will not be on that map)
   QMap<QString, QList<QtResourceSet *>> m_pathToResourceSet;
   QtResourceSet *m_currentResourceSet;

   typedef QMap<QString, const QByteArray *> PathDataMap;
   PathDataMap m_pathToData;

   QMap<QString, QStringList> m_pathToContents;  // qrc path to its contents
   QMap<QString, QString>     m_fileToQrc;       // this map contains the content of active resource set only.

   // Activating different resource set changes the contents.
   QFileSystemWatcher *m_fileWatcher;
   bool m_fileWatcherEnabled;
   QMap<QString, bool> m_fileWatchedMap;

 private:
   void registerResourceSet(QtResourceSet *resourceSet);
   void unregisterResourceSet(QtResourceSet *resourceSet);
   void setWatcherEnabled(const QString &path, bool enable);
   void addWatcher(const QString &path);
   void removeWatcher(const QString &path);

   void slotFileChanged(const QString &);

   const QByteArray *createResource(const QString &path, QStringList *contents, int *errorCount, QIODevice &errorDevice) const;
   void deleteResource(const QByteArray *data) const;
};

QtResourceModelPrivate::QtResourceModelPrivate()
   : q_ptr(nullptr), m_currentResourceSet(nullptr), m_fileWatcher(nullptr), m_fileWatcherEnabled(true)
{
}

QtResourceSet::QtResourceSet() :
   d_ptr(new QtResourceSetPrivate)
{
   d_ptr->q_ptr = this;
}

QtResourceSet::QtResourceSet(QtResourceModel *model)
   : d_ptr(new QtResourceSetPrivate(model))
{
   d_ptr->q_ptr = this;
}

QtResourceSet::~QtResourceSet()
{
}

QStringList QtResourceSet::activeResourceFilePaths() const
{
   QtResourceSet *that = const_cast<QtResourceSet *>(this);
   return d_ptr->m_resourceModel->d_ptr->m_resourceSetToPaths.value(that);
}

void QtResourceSet::activateResourceFilePaths(const QStringList &paths, int *errorCount, QString *errorMessages)
{
   d_ptr->m_resourceModel->d_ptr->activate(this, paths, errorCount, errorMessages);
}

bool QtResourceSet::isModified(const QString &path) const
{
   return d_ptr->m_resourceModel->isModified(path);
}

void QtResourceSet::setModified(const QString &path)
{
   d_ptr->m_resourceModel->setModified(path);
}

const QByteArray *QtResourceModelPrivate::createResource(const QString &path, QStringList *contents,
      int *errorCount, QIODevice &errorDevice) const
{
   const QByteArray *rc = nullptr;
   *errorCount = -1;

   contents->clear();

   RCC_Support library;

   library.setVerbose(true);
   library.setInputFiles(QStringList(path));
   library.setFormat(RCC_Support::Binary);

   QBuffer buffer;
   buffer.open(QIODevice::WriteOnly);

   // ignore errors
   if (! library.readFiles(true, errorDevice)) {
      return nullptr;
   }

   const RCC_Support::ResourceFile rccData = library.resourceFile();

   if (! library.output(buffer, errorDevice)) {
      return nullptr;
   }

   *errorCount = library.failedResources().size();
   *contents   = rccData.keys();

   if (! rccData.empty()) {
      buffer.close();
      rc = new QByteArray(buffer.data());
   }

   if (*errorCount != 0) {
      csWarning("createResource() " + path);
   }

   return rc;
}

void QtResourceModelPrivate::deleteResource(const QByteArray *data) const
{
   if (data) {
      delete data;
   }
}

void QtResourceModelPrivate::registerResourceSet(QtResourceSet *resourceSet)
{
   if (! resourceSet) {
      return;
   }

   // unregister old paths (all because the order of registration is important), later it can be optimized a bit
   QStringList toRegister = resourceSet->activeResourceFilePaths();
   QStringListIterator itRegister(toRegister);

   while (itRegister.hasNext()) {
      const QString path = itRegister.next();
      const PathDataMap::const_iterator itRcc = m_pathToData.constFind(path);

      if (itRcc != m_pathToData.constEnd()) { // otherwise data was not created yet
         const QByteArray *data = itRcc.value();

         if (data) {
            if (! QResource::registerResource(reinterpret_cast<const uchar *>(data->constData()))) {
               csWarning("QResource failed to register " + path);

            } else {
               QStringList contents = m_pathToContents.value(path);
               QStringListIterator itContents(contents);

               while (itContents.hasNext()) {
                  const QString filePath = itContents.next();

                  if (! m_fileToQrc.contains(filePath)) {
                     // the first loaded resource has higher priority in the resource system
                     m_fileToQrc.insert(filePath, path);
                  }
               }
            }
         }
      }
   }
}

void QtResourceModelPrivate::unregisterResourceSet(QtResourceSet *resourceSet)
{
   if (! resourceSet) {
      return;
   }

   // unregister old paths (all because the order of registration is importans), later it can be optimized a bit
   QStringList toUnregister = resourceSet->activeResourceFilePaths();
   QStringListIterator itUnregister(toUnregister);

   while (itUnregister.hasNext()) {
      const QString path = itUnregister.next();
      const PathDataMap::const_iterator itRcc = m_pathToData.constFind(path);

      if (itRcc != m_pathToData.constEnd()) { // otherwise data was not created yet
         const QByteArray *data = itRcc.value();

         if (data != nullptr) {
            if (! QResource::unregisterResource(reinterpret_cast<const uchar *>(itRcc.value()->constData()))) {
               csWarning("QResource failed to unregister " + path);
            }
         }
      }
   }

   m_fileToQrc.clear();
}

void QtResourceModelPrivate::activate(QtResourceSet *resourceSet, const QStringList &newPaths,
      int *errorCountPtr, QString *errorMessages)
{
   if (errorCountPtr) {
      *errorCountPtr = 0;
   }

   if (errorMessages) {
      errorMessages->clear();
   }

   QBuffer errorStream;
   errorStream.open(QIODevice::WriteOnly);

   int errorCount     = 0;
   int generatedCount = 0;
   bool newResourceSetChanged = false;

   if (resourceSet && resourceSet->activeResourceFilePaths() != newPaths && !m_newlyCreated.contains(resourceSet)) {
      newResourceSetChanged = true;
   }

   PathDataMap newPathToData = m_pathToData;

   QStringListIterator itPath(newPaths);

   while (itPath.hasNext()) {
      const QString path = itPath.next();

      if (resourceSet && !m_pathToResourceSet[path].contains(resourceSet)) {
         m_pathToResourceSet[path].append(resourceSet);
      }

      const QMap<QString, bool>::iterator itMod = m_pathToModified.find(path);

      if (itMod == m_pathToModified.end() || itMod.value()) {
         // new path or path is already created, but needs to be recreated
         QStringList contents;
         int qrcErrorCount;
         ++generatedCount;

         const QByteArray *data = createResource(path, &contents, &qrcErrorCount, errorStream);

         newPathToData.insert(path, data);
         if (qrcErrorCount) {
            // Count single failed files as sort of 1/2 error
            ++errorCount;
         }
         addWatcher(path);

         m_pathToModified.insert(path, false);
         m_pathToContents.insert(path, contents);
         newResourceSetChanged = true;
         const QMap<QString, QList<QtResourceSet *>>::iterator itReload = m_pathToResourceSet.find(path);
         if (itReload != m_pathToResourceSet.end()) {
            QList<QtResourceSet *> resources = itReload.value();
            QListIterator<QtResourceSet *> itRes(resources);
            while (itRes.hasNext()) {
               QtResourceSet *res = itRes.next();
               if (res != resourceSet) {
                  m_resourceSetToReload[res] = true;
               }
            }
         }
      } else { // path is already created, don't need to recreate
      }
   }

   QList<const QByteArray *> oldData = m_pathToData.values();
   QList<const QByteArray *> newData = newPathToData.values();

   QList<const QByteArray *> toDelete;
   QListIterator<const QByteArray *> itOld(oldData);

   if (itOld.hasNext()) {
      const QByteArray *array = itOld.next();
      if (array && !newData.contains(array)) {
         toDelete.append(array);
      }
   }

   // Nothing can fail below here?
   if (generatedCount) {
      if (errorCountPtr) {
         *errorCountPtr = errorCount;
      }

      errorStream.close();
      const QString stderrOutput = QString::fromUtf8(errorStream.data());

      if (errorMessages) {
         *errorMessages = stderrOutput;
      }
   }

   // register
   const QMap<QtResourceSet *, bool>::iterator itReload = m_resourceSetToReload.find(resourceSet);
   if (itReload != m_resourceSetToReload.end()) {
      if (itReload.value()) {
         newResourceSetChanged = true;
         m_resourceSetToReload.insert(resourceSet, false);
      }
   }

   QStringList oldActivePaths;
   if (m_currentResourceSet) {
      oldActivePaths = m_currentResourceSet->activeResourceFilePaths();
   }

   const bool needReregister = (oldActivePaths != newPaths) || newResourceSetChanged;

   QMap<QtResourceSet *, bool>::iterator itNew = m_newlyCreated.find(resourceSet);
   if (itNew != m_newlyCreated.end()) {
      m_newlyCreated.remove(resourceSet);
      if (needReregister) {
         newResourceSetChanged = true;
      }
   }

   if (!newResourceSetChanged && !needReregister && (m_currentResourceSet == resourceSet)) {
      for (const QByteArray *data : toDelete) {
         deleteResource(data);
      }

      return; // nothing changed
   }

   if (needReregister) {
      unregisterResourceSet(m_currentResourceSet);
   }

   for (const QByteArray *data : toDelete) {
      deleteResource(data);
   }

   m_pathToData = newPathToData;
   m_currentResourceSet = resourceSet;

   if (resourceSet) {
      removeOldPaths(resourceSet, newPaths);
   }

   if (needReregister) {
      registerResourceSet(m_currentResourceSet);
   }

   emit q_ptr->resourceSetActivated(m_currentResourceSet, newResourceSetChanged);

   // deactivates the paths from old current resource set
   // add new paths to the new current resource set
   // reloads all paths which are marked as modified from the current resource set;
   // activates the paths from current resource set
   // emits resourceSetActivated() (don't emit only in case when old resource set is the same as new one
   // AND no path was reloaded AND the list of paths is exactly the same)
}

void QtResourceModelPrivate::removeOldPaths(QtResourceSet *resourceSet, const QStringList &newPaths)
{
   QStringList oldPaths = m_resourceSetToPaths.value(resourceSet);
   if (oldPaths != newPaths) {
      // remove old
      QStringListIterator itOldPaths(oldPaths);

      while (itOldPaths.hasNext()) {
         QString oldPath = itOldPaths.next();

         if (!newPaths.contains(oldPath)) {
            const QMap<QString, QList<QtResourceSet *>>::iterator itRemove = m_pathToResourceSet.find(oldPath);

            if (itRemove != m_pathToResourceSet.end()) {
               const int idx = itRemove.value().indexOf(resourceSet);

               if (idx >= 0) {
                  itRemove.value().removeAt(idx);
               }

               if (itRemove.value().count() == 0) {
                  PathDataMap::iterator it = m_pathToData.find(oldPath);
                  if (it != m_pathToData.end()) {
                     deleteResource(it.value());
                  }
                  m_pathToResourceSet.erase(itRemove);
                  m_pathToModified.remove(oldPath);
                  m_pathToContents.remove(oldPath);
                  m_pathToData.remove(oldPath);
                  removeWatcher(oldPath);
               }
            }
         }
      }
      m_resourceSetToPaths[resourceSet] = newPaths;
   }
}

void QtResourceModelPrivate::setWatcherEnabled(const QString &path, bool enable)
{
   if (!enable) {
      m_fileWatcher->removePath(path);
      return;
   }

   QFileInfo fi(path);
   if (fi.exists()) {
      m_fileWatcher->addPath(path);
   }
}

void QtResourceModelPrivate::addWatcher(const QString &path)
{
   auto it = m_fileWatchedMap.constFind(path);
   if (it != m_fileWatchedMap.constEnd() && it.value() == false) {
      return;
   }

   m_fileWatchedMap.insert(path, true);
   if (!m_fileWatcherEnabled) {
      return;
   }
   setWatcherEnabled(path, true);
}

void QtResourceModelPrivate::removeWatcher(const QString &path)
{
   if (!m_fileWatchedMap.contains(path)) {
      return;
   }

   m_fileWatchedMap.remove(path);
   if (!m_fileWatcherEnabled) {
      return;
   }
   setWatcherEnabled(path, false);
}

void QtResourceModelPrivate::slotFileChanged(const QString &path)
{
   setWatcherEnabled(path, false);
   emit q_ptr->qrcFileModifiedExternally(path);
   setWatcherEnabled(path, true); //readd
}

// ----------------------- QtResourceModel
QtResourceModel::QtResourceModel(QObject *parent) :
   QObject(parent), d_ptr(new QtResourceModelPrivate)
{
   d_ptr->q_ptr         = this;
   d_ptr->m_fileWatcher = new QFileSystemWatcher(this);

   connect(d_ptr->m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &QtResourceModel::slotFileChanged);
}

QtResourceModel::~QtResourceModel()
{
   blockSignals(true);
   QList<QtResourceSet *> resourceList = resourceSets();
   QListIterator<QtResourceSet *> it(resourceList);
   while (it.hasNext()) {
      removeResourceSet(it.next());
   }
   blockSignals(false);
}

QStringList QtResourceModel::loadedQrcFiles() const
{
   return d_ptr->m_pathToModified.keys();
}

bool QtResourceModel::isModified(const QString &path) const
{
   QMap<QString, bool>::const_iterator it = d_ptr->m_pathToModified.find(path);
   if (it != d_ptr->m_pathToModified.constEnd()) {
      return it.value();
   }
   return true;
}

void QtResourceModel::setModified(const QString &path)
{
   QMap<QString, bool>::const_iterator itMod = d_ptr->m_pathToModified.find(path);
   if (itMod == d_ptr->m_pathToModified.constEnd()) {
      return;
   }

   d_ptr->m_pathToModified[path] = true;
   QMap<QString, QList<QtResourceSet *>>::const_iterator it = d_ptr->m_pathToResourceSet.constFind(path);
   if (it == d_ptr->m_pathToResourceSet.constEnd()) {
      return;
   }

   QList<QtResourceSet *> resourceList = it.value();
   QListIterator<QtResourceSet *> itReload(resourceList);
   while (itReload.hasNext()) {
      d_ptr->m_resourceSetToReload.insert(itReload.next(), true);
   }
}

QList<QtResourceSet *> QtResourceModel::resourceSets() const
{
   return d_ptr->m_resourceSetToPaths.keys();
}

QtResourceSet *QtResourceModel::currentResourceSet() const
{
   return d_ptr->m_currentResourceSet;
}

void QtResourceModel::setCurrentResourceSet(QtResourceSet *resourceSet, int *errorCount, QString *errorMessages)
{
   d_ptr->activate(resourceSet, d_ptr->m_resourceSetToPaths.value(resourceSet), errorCount, errorMessages);
}

QtResourceSet *QtResourceModel::addResourceSet(const QStringList &paths)
{
   QtResourceSet *newResource = new QtResourceSet(this);
   d_ptr->m_resourceSetToPaths.insert(newResource, paths);
   d_ptr->m_resourceSetToReload.insert(newResource, false);
   d_ptr->m_newlyCreated.insert(newResource, true);
   QStringListIterator it(paths);

   while (it.hasNext()) {
      const QString path = it.next();
      d_ptr->m_pathToResourceSet[path].append(newResource);
   }
   return newResource;
}

// TODO
void QtResourceModel::removeResourceSet(QtResourceSet *resourceSet)
{
   if (!resourceSet) {
      return;
   }

   if (currentResourceSet() == resourceSet) {
      setCurrentResourceSet(nullptr);
   }

   // remove rcc files for those paths which are not used in any other resource set
   d_ptr->removeOldPaths(resourceSet, QStringList());

   d_ptr->m_resourceSetToPaths.remove(resourceSet);
   d_ptr->m_resourceSetToReload.remove(resourceSet);
   d_ptr->m_newlyCreated.remove(resourceSet);
   delete resourceSet;
}

void QtResourceModel::reload(const QString &path, int *errorCount, QString *errorMessages)
{
   setModified(path);

   d_ptr->activate(d_ptr->m_currentResourceSet, d_ptr->m_resourceSetToPaths.value(d_ptr->m_currentResourceSet), errorCount,
      errorMessages);
}

void QtResourceModel::reload(int *errorCount, QString *errorMessages)
{
   QMap<QString, bool>::iterator it = d_ptr->m_pathToModified.begin();
   QMap<QString, bool>::iterator itEnd = d_ptr->m_pathToModified.end(); // will it be valid when I iterate the map and change it???
   while (it != itEnd) {
      it = d_ptr->m_pathToModified.insert(it.key(), true);
      ++it;
   }

   QMap<QtResourceSet *, bool>::iterator itReload = d_ptr->m_resourceSetToReload.begin();
   QMap<QtResourceSet *, bool>::iterator itReloadEnd = d_ptr->m_resourceSetToReload.end();
   while (itReload != itReloadEnd) {
      itReload = d_ptr->m_resourceSetToReload.insert(itReload.key(), true); // empty resourceSets could be omitted here
      ++itReload;
   }

   d_ptr->activate(d_ptr->m_currentResourceSet, d_ptr->m_resourceSetToPaths.value(d_ptr->m_currentResourceSet), errorCount,
      errorMessages);
}

QMap<QString, QString> QtResourceModel::contents() const
{
   return d_ptr->m_fileToQrc;
}

QString QtResourceModel::qrcPath(const QString &file) const
{
   return d_ptr->m_fileToQrc.value(file);
}

void QtResourceModel::setWatcherEnabled(bool enable)
{
   if (d_ptr->m_fileWatcherEnabled == enable) {
      return;
   }

   d_ptr->m_fileWatcherEnabled = enable;

   auto iter = d_ptr->m_fileWatchedMap.begin();

   if (iter != d_ptr->m_fileWatchedMap.end()) {
      d_ptr->setWatcherEnabled(iter.key(), d_ptr->m_fileWatcherEnabled);
   }
}

bool QtResourceModel::isWatcherEnabled() const
{
   return d_ptr->m_fileWatcherEnabled;
}

void QtResourceModel::setWatcherEnabled(const QString &path, bool enable)
{
   auto it = d_ptr->m_fileWatchedMap.find(path);

   if (it == d_ptr->m_fileWatchedMap.end()) {
      return;
   }

   if (it.value() == enable) {
      return;
   }

   it.value() = enable;

   if (!d_ptr->m_fileWatcherEnabled) {
      return;
   }

   d_ptr->setWatcherEnabled(it.key(), enable);
}

bool QtResourceModel::isWatcherEnabled(const QString &path)
{
   return d_ptr->m_fileWatchedMap.value(path, false);
}

void QtResourceModel::slotFileChanged(const QString &un_named_arg1)
{
   Q_D(QtResourceModel);
   d->slotFileChanged(un_named_arg1);
}