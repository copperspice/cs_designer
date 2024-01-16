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

#ifndef QTRESOURCEMODEL_H
#define QTRESOURCEMODEL_H

class QtResourceModel;

#include <QMap>
#include <QObject>
#include <QScopedPointer>

class QtResourceSet // one instance per one form
{
 public:
   QStringList activeResourceFilePaths() const;

   // activateQrcPaths(): if this QtResourceSet is active it emits resourceSetActivated();
   // otherwise only in case if active QtResource set contains one of
   // paths which was marked as modified by this resource set, the signal
   // is emitted (with reload = true);
   // If new path appears on the list it is automatically added to
   // loaded list of QtResourceModel. In addition it is marked as modified in case
   // QtResourceModel didn't contain the path.
   // If some path is removed from that list (and is not used in any other
   // resource set) it is automatically unloaded. The removed file can also be
   // marked as modified (later when another resource set which contains
   // removed path is activated will be reloaded)
   void activateResourceFilePaths(const QStringList &paths, int *errorCount = nullptr, QString *errorMessages = nullptr);

   // for all paths in resource model (redundant here, maybe it should be removed from here)
   bool isModified(const QString &path) const;

   // for all paths in resource model (redundant here, maybe it should be removed from here)
   void setModified(const QString &path);

 private:
   QtResourceSet();
   QtResourceSet(QtResourceModel *model);
   ~QtResourceSet();

   friend class QtResourceModel;

   QScopedPointer<class QtResourceSetPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtResourceSet)
   Q_DISABLE_COPY(QtResourceSet)
};

class QtResourceModel : public QObject // one instance per whole designer
{
   CS_OBJECT(QtResourceModel)

 public:
   QtResourceModel(QObject *parent = nullptr);
   ~QtResourceModel();

   QStringList loadedQrcFiles() const;
   bool isModified(const QString &path) const; // only for paths which are on loadedQrcFiles() list
   void setModified(const QString &path);      // only for paths which are on loadedQrcPaths() list

   QList<QtResourceSet *> resourceSets() const;

   QtResourceSet *currentResourceSet() const;
   void setCurrentResourceSet(QtResourceSet *resourceSet, int *errorCount = nullptr, QString *errorMessages = nullptr);

   QtResourceSet *addResourceSet(const QStringList &paths);
   void removeResourceSet(QtResourceSet *resourceSet);

   void reload(const QString &path, int *errorCount = nullptr, QString *errorMessages = nullptr);
   void reload(int *errorCount = nullptr, QString *errorMessages = nullptr);

   // Contents of the current resource set (content file to qrc path)
   QMap<QString, QString> contents() const;

   // Find the qrc file belonging to the contained file (from current resource set)
   QString qrcPath(const QString &file) const;

   void setWatcherEnabled(bool enable);
   bool isWatcherEnabled() const;

   void setWatcherEnabled(const QString &path, bool enable);
   bool isWatcherEnabled(const QString &path);

   CS_SIGNAL_1(Public, void resourceSetActivated(QtResourceSet *resourceSet, bool resourceSetChanged))
   CS_SIGNAL_2(resourceSetActivated, resourceSet, resourceSetChanged) // resourceSetChanged since last time it was activated!

   CS_SIGNAL_1(Public, void qrcFileModifiedExternally(const QString &path))
   CS_SIGNAL_2(qrcFileModifiedExternally, path)

 private:
   friend class QtResourceSet;

   QScopedPointer<class QtResourceModelPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtResourceModel)
   Q_DISABLE_COPY(QtResourceModel)

   CS_SLOT_1(Private, void slotFileChanged(const QString &un_named_arg1))
   CS_SLOT_2(slotFileChanged)
};

#endif
