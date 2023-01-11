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

#include <gradient_manager.h>

#include <QPixmap>
#include <QMetaEnum>

QtGradientManager::QtGradientManager(QObject *parent)
   : QObject(parent)
{
}

QMap<QString, QGradient> QtGradientManager::gradients() const
{
   return m_idToGradient;
}

QString QtGradientManager::uniqueId(const QString &id) const
{
   if (!m_idToGradient.contains(id)) {
      return id;
   }

   QString base = id;
   while (base.count() > 0 && base.at(base.count() - 1).isDigit()) {
      base = base.left(base.count() - 1);
   }
   QString newId = base;
   int counter = 0;
   while (m_idToGradient.contains(newId)) {
      ++counter;
      newId = base + QString::number(counter);
   }
   return newId;
}

QString QtGradientManager::addGradient(const QString &id, const QGradient &gradient)
{
   QString newId = uniqueId(id);

   m_idToGradient[newId] = gradient;

   emit gradientAdded(newId, gradient);

   return newId;
}

void QtGradientManager::removeGradient(const QString &id)
{
   if (!m_idToGradient.contains(id)) {
      return;
   }

   emit gradientRemoved(id);

   m_idToGradient.remove(id);
}

void QtGradientManager::renameGradient(const QString &id, const QString &newId)
{
   if (!m_idToGradient.contains(id)) {
      return;
   }

   if (newId == id) {
      return;
   }

   QString changedId = uniqueId(newId);
   QGradient gradient = m_idToGradient.value(id);

   emit gradientRenamed(id, changedId);

   m_idToGradient.remove(id);
   m_idToGradient[changedId] = gradient;
}

void QtGradientManager::changeGradient(const QString &id, const QGradient &newGradient)
{
   if (!m_idToGradient.contains(id)) {
      return;
   }

   if (m_idToGradient.value(id) == newGradient) {
      return;
   }

   emit gradientChanged(id, newGradient);

   m_idToGradient[id] = newGradient;
}

void QtGradientManager::clear()
{
   QMap<QString, QGradient> grads = gradients();
   QMapIterator<QString, QGradient> itGrad(grads);
   while (itGrad.hasNext()) {
      removeGradient(itGrad.next().key());
   }
}

