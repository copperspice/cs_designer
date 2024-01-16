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

#ifndef GRADIENTVIEW_H
#define GRADIENTVIEW_H

#include <ui_gradient_view.h>

class QtGradientManager;

#include <QWidget>
#include <QMap>

class QListViewItem;
class QAction;

class QtGradientView : public QWidget
{
   CS_OBJECT(QtGradientView)

 public:
   QtGradientView(QWidget *parent = nullptr);

   void setGradientManager(QtGradientManager *manager);
   QtGradientManager *gradientManager() const;

   void setCurrentGradient(const QString &id);
   QString currentGradient() const;

   CS_SIGNAL_1(Public, void currentGradientChanged(const QString &id))
   CS_SIGNAL_2(currentGradientChanged, id)
   CS_SIGNAL_1(Public, void gradientActivated(const QString &id))
   CS_SIGNAL_2(gradientActivated, id)

 private:
   CS_SLOT_1(Private, void slotGradientAdded(const QString &id, const QGradient &gradient))
   CS_SLOT_2(slotGradientAdded)

   CS_SLOT_1(Private, void slotGradientRenamed(const QString &id, const QString &newId))
   CS_SLOT_2(slotGradientRenamed)

   CS_SLOT_1(Private, void slotGradientChanged(const QString &id, const QGradient &newGradient))
   CS_SLOT_2(slotGradientChanged)

   CS_SLOT_1(Private, void slotGradientRemoved(const QString &id))
   CS_SLOT_2(slotGradientRemoved)

   CS_SLOT_1(Private, void slotNewGradient())
   CS_SLOT_2(slotNewGradient)

   CS_SLOT_1(Private, void slotEditGradient())
   CS_SLOT_2(slotEditGradient)

   CS_SLOT_1(Private, void slotRemoveGradient())
   CS_SLOT_2(slotRemoveGradient)

   CS_SLOT_1(Private, void slotRenameGradient())
   CS_SLOT_OVERLOAD(slotRenameGradient, ())

   CS_SLOT_1(Private, void slotRenameGradient(QListWidgetItem *item))
   CS_SLOT_OVERLOAD(slotRenameGradient, (QListWidgetItem *))

   CS_SLOT_1(Private, void slotCurrentItemChanged(QListWidgetItem *item))
   CS_SLOT_2(slotCurrentItemChanged)

   CS_SLOT_1(Private, void slotGradientActivated(QListWidgetItem *item))
   CS_SLOT_2(slotGradientActivated)

   QMap<QString, QListWidgetItem *> m_idToItem;
   QMap<QListWidgetItem *, QString> m_itemToId;

   QAction *m_newAction;
   QAction *m_editAction;
   QAction *m_renameAction;
   QAction *m_removeAction;

   QtGradientManager *m_manager;
   Ui::QtGradientView m_ui;
};

#endif
