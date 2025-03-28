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

#include <gradient_dialog.h>
#include <gradient_manager.h>
#include <gradient_utils.h>
#include <gradient_view.h>

#include <QClipboard>
#include <QColor>
#include <QMessageBox>
#include <QPainter>

void QtGradientView::slotGradientAdded(const QString &id, const QGradient &gradient)
{
   QListWidgetItem *item = new QListWidgetItem(QtGradientUtils::gradientPixmap(gradient), id, m_ui.listWidget);
   item->setToolTip(id);
   item->setSizeHint(QSize(72, 84));
   item->setFlags(item->flags() | Qt::ItemIsEditable);

   m_idToItem[id] = item;
   m_itemToId[item] = id;
}

void QtGradientView::slotGradientRenamed(const QString &id, const QString &newId)
{
   if (! m_idToItem.contains(id)) {
      return;
   }

   QListWidgetItem *item = m_idToItem.value(id);
   item->setText(newId);
   item->setToolTip(newId);
   m_itemToId[item] = newId;
   m_idToItem.remove(id);
   m_idToItem[newId] = item;
}

void QtGradientView::slotGradientChanged(const QString &id, const QGradient &newGradient)
{
   if (!m_idToItem.contains(id)) {
      return;
   }

   QListWidgetItem *item = m_idToItem.value(id);
   item->setIcon(QtGradientUtils::gradientPixmap(newGradient));
}

void QtGradientView::slotGradientRemoved(const QString &id)
{
   if (!m_idToItem.contains(id)) {
      return;
   }

   QListWidgetItem *item = m_idToItem.value(id);
   delete item;
   m_itemToId.remove(item);
   m_idToItem.remove(id);
}

void QtGradientView::slotNewGradient()
{
   bool ok;
   QListWidgetItem *item = m_ui.listWidget->currentItem();
   QGradient grad = QLinearGradient();
   if (item) {
      grad = m_manager->gradients().value(m_itemToId.value(item));
   }

   QGradient gradient = QtGradientDialog::getGradient(&ok, grad, this);
   if (!ok) {
      return;
   }

   QString id = m_manager->addGradient(tr("Grad"), gradient);
   m_ui.listWidget->setCurrentItem(m_idToItem.value(id));
}

void QtGradientView::slotEditGradient()
{
   bool ok;
   QListWidgetItem *item = m_ui.listWidget->currentItem();
   if (!item) {
      return;
   }

   const QString id = m_itemToId.value(item);
   QGradient grad = m_manager->gradients().value(id);
   QGradient gradient = QtGradientDialog::getGradient(&ok, grad, this);
   if (!ok) {
      return;
   }

   m_manager->changeGradient(id, gradient);
}

void QtGradientView::slotRemoveGradient()
{
   QListWidgetItem *item = m_ui.listWidget->currentItem();
   if (!item) {
      return;
   }

   if (QMessageBox::question(this, tr("Remove Gradient"),
         tr("Are you sure you want to remove the selected gradient?"),
         QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Yes) {
      return;
   }

   const QString id = m_itemToId.value(item);
   m_manager->removeGradient(id);
}

void QtGradientView::slotRenameGradient()
{
   QListWidgetItem *item = m_ui.listWidget->currentItem();
   if (! item) {
      return;
   }

   m_ui.listWidget->editItem(item);
}

void QtGradientView::slotRenameGradient(QListWidgetItem *item)
{
   if (! item) {
      return;
   }

   const QString id = m_itemToId.value(item);
   m_manager->renameGradient(id, item->text());
}

void QtGradientView::slotCurrentItemChanged(QListWidgetItem *item)
{
   m_editAction->setEnabled(item);
   m_renameAction->setEnabled(item);
   m_removeAction->setEnabled(item);
   emit currentGradientChanged(m_itemToId.value(item));
}

void QtGradientView::slotGradientActivated(QListWidgetItem *item)
{
   const QString id = m_itemToId.value(item);
   if (!id.isEmpty()) {
      emit gradientActivated(id);
   }
}

QtGradientView::QtGradientView(QWidget *parent)
   : QWidget(parent)
{
   m_manager = nullptr;

   m_ui.setupUi(this);

   m_ui.listWidget->setViewMode(QListView::IconMode);
   m_ui.listWidget->setMovement(QListView::Static);
   m_ui.listWidget->setTextElideMode(Qt::ElideRight);
   m_ui.listWidget->setResizeMode(QListWidget::Adjust);
   m_ui.listWidget->setIconSize(QSize(64, 64));
   m_ui.listWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

   QPalette pal = m_ui.listWidget->viewport()->palette();
   int pixSize = 18;
   QPixmap pm(2 * pixSize, 2 * pixSize);

   QColor c1 = palette().color(QPalette::Midlight);
   QColor c2 = palette().color(QPalette::Dark);
   QPainter pmp(&pm);

   pmp.fillRect(0, 0, pixSize, pixSize, c1);
   pmp.fillRect(pixSize, pixSize, pixSize, pixSize, c1);
   pmp.fillRect(0, pixSize, pixSize, pixSize, c2);
   pmp.fillRect(pixSize, 0, pixSize, pixSize, c2);

   pal.setBrush(QPalette::Base, QBrush(pm));
   m_ui.listWidget->viewport()->setPalette(pal);

   connect(m_ui.listWidget, &QListWidget::itemDoubleClicked,
         this, &QtGradientView::slotGradientActivated);

   connect(m_ui.listWidget, cs_mp_cast<QListWidgetItem *>(&QListWidget::itemChanged),
         this, cs_mp_cast<QListWidgetItem *>(&QtGradientView::slotRenameGradient));

   connect(m_ui.listWidget, &QListWidget::currentItemChanged,
         this, &QtGradientView::slotCurrentItemChanged);

   m_newAction    = new QAction(QIcon(":/resources/gradient_editor/images/plus.png"), tr("New..."), this);
   m_editAction   = new QAction(QIcon(":/resources/gradient_editor/images/edit.png"), tr("Edit..."), this);
   m_renameAction = new QAction(tr("Rename"), this);
   m_removeAction = new QAction(QIcon(":/resources/gradient_editor/images/minus.png"), tr("Remove"), this);

   connect(m_newAction,    &QAction::triggered, this, &QtGradientView::slotNewGradient);
   connect(m_editAction,   &QAction::triggered, this, &QtGradientView::slotEditGradient);
   connect(m_removeAction, &QAction::triggered, this, &QtGradientView::slotRemoveGradient);
   connect(m_renameAction, &QAction::triggered, this, cs_mp_cast<>(&QtGradientView::slotRenameGradient));

   m_ui.listWidget->addAction(m_newAction);
   m_ui.listWidget->addAction(m_editAction);
   m_ui.listWidget->addAction(m_renameAction);
   m_ui.listWidget->addAction(m_removeAction);

   m_ui.newButton->setDefaultAction(m_newAction);
   m_ui.editButton->setDefaultAction(m_editAction);
   m_ui.renameButton->setDefaultAction(m_renameAction);
   m_ui.removeButton->setDefaultAction(m_removeAction);

   m_ui.listWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void QtGradientView::setGradientManager(QtGradientManager *manager)
{
   if (m_manager == manager) {
      return;
   }

   if (m_manager) {
      disconnect(m_manager, &QtGradientManager::gradientAdded,
            this, &QtGradientView::slotGradientAdded);

      disconnect(m_manager, &QtGradientManager::gradientRenamed,
            this, &QtGradientView::slotGradientRenamed);

      disconnect(m_manager, &QtGradientManager::gradientChanged,
            this, &QtGradientView::slotGradientChanged);

      disconnect(m_manager, &QtGradientManager::gradientRemoved,
            this, &QtGradientView::slotGradientRemoved);

      m_ui.listWidget->clear();
      m_idToItem.clear();
      m_itemToId.clear();
   }

   m_manager = manager;

   if (! m_manager) {
      return;
   }

   QMap<QString, QGradient> gradients = m_manager->gradients();
   QMapIterator<QString, QGradient> itGrad(gradients);

   while (itGrad.hasNext()) {
      itGrad.next();
      slotGradientAdded(itGrad.key(), itGrad.value());
   }

   connect(m_manager, &QtGradientManager::gradientAdded,
         this, &QtGradientView::slotGradientAdded);

   connect(m_manager, &QtGradientManager::gradientRenamed,
         this, &QtGradientView::slotGradientRenamed);

   connect(m_manager, &QtGradientManager::gradientChanged,
         this, &QtGradientView::slotGradientChanged);

   connect(m_manager, &QtGradientManager::gradientRemoved,
         this, &QtGradientView::slotGradientRemoved);
}

QtGradientManager *QtGradientView::gradientManager() const
{
   return m_manager;
}

void QtGradientView::setCurrentGradient(const QString &id)
{
   QListWidgetItem *item = m_idToItem.value(id);

   if (! item) {
      return;
   }

   m_ui.listWidget->setCurrentItem(item);
}

QString QtGradientView::currentGradient() const
{
   return m_itemToId.value(m_ui.listWidget->currentItem());
}
