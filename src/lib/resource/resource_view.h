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

#ifndef QTRESOURCEVIEW_H
#define QTRESOURCEVIEW_H

#include <QDialog>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QWidget>

class QDesignerFormEditorInterface;
class QtResourceModel;
class QtResourceSet;

class QMimeData;

class QtResourceView : public QWidget
{
   CS_OBJECT(QtResourceView)

 public:
   explicit QtResourceView(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
   ~QtResourceView();

   void setDragEnabled(bool dragEnabled);
   bool dragEnabled() const;

   QtResourceModel *model() const;
   void setResourceModel(QtResourceModel *model);

   QString selectedResource() const;
   void selectResource(const QString &resource);

   QString settingsKey() const;
   void setSettingsKey(const QString &key);

   bool isResourceEditingEnabled() const;
   void setResourceEditingEnabled(bool enable);

   // Helpers for handling the drag originating in QtResourceView (Xml/text)
   enum ResourceType { ResourceImage, ResourceStyleSheet, ResourceOther };
   static QString encodeMimeData(ResourceType resourceType, const QString &path);

   static bool decodeMimeData(const QMimeData *md, ResourceType *t = nullptr, QString *file = nullptr);
   static bool decodeMimeData(const QString &text, ResourceType *t = nullptr, QString *file = nullptr);

   CS_SIGNAL_1(Public, void resourceSelected(const QString &resource))
   CS_SIGNAL_2(resourceSelected, resource)

   CS_SIGNAL_1(Public, void resourceActivated(const QString &resource))
   CS_SIGNAL_2(resourceActivated, resource)

 protected:
   bool event(QEvent *event) override;

 private:
   QScopedPointer<class QtResourceViewPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtResourceView)
   Q_DISABLE_COPY(QtResourceView)

   CS_SLOT_1(Private, void slotResourceSetActivated(QtResourceSet *un_named_arg1))
   CS_SLOT_2(slotResourceSetActivated)

   CS_SLOT_1(Private, void slotCurrentPathChanged(QTreeWidgetItem *un_named_arg1))
   CS_SLOT_2(slotCurrentPathChanged)

   CS_SLOT_1(Private, void slotCurrentResourceChanged(QListWidgetItem *un_named_arg1))
   CS_SLOT_2(slotCurrentResourceChanged)

   CS_SLOT_1(Private, void slotResourceActivated(QListWidgetItem *un_named_arg1))
   CS_SLOT_2(slotResourceActivated)

   CS_SLOT_1(Private, void slotEditResources())
   CS_SLOT_2(slotEditResources)

   CS_SLOT_1(Private, void slotReloadResources())
   CS_SLOT_2(slotReloadResources)

   CS_SLOT_1(Private, void slotCopyResourcePath())
   CS_SLOT_2(slotCopyResourcePath)

   CS_SLOT_1(Private, void slotListWidgetContextMenuRequested(const QPoint &pos))
   CS_SLOT_2(slotListWidgetContextMenuRequested)

   CS_SLOT_1(Private, void slotFilterChanged(const QString &pattern))
   CS_SLOT_2(slotFilterChanged)
};

class  QtResourceViewDialog : public QDialog
{
   CS_OBJECT(QtResourceViewDialog)

 public:
   explicit QtResourceViewDialog(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
   virtual ~QtResourceViewDialog();

   QString selectedResource() const;
   void selectResource(const QString &path);

   bool isResourceEditingEnabled() const;
   void setResourceEditingEnabled(bool enable);

 private:
   QScopedPointer<class QtResourceViewDialogPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtResourceViewDialog)
   Q_DISABLE_COPY(QtResourceViewDialog)

   CS_SLOT_1(Private, void slotResourceSelected(const QString &un_named_arg1))
   CS_SLOT_2(slotResourceSelected)
};

#endif
