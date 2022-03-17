/***********************************************************************
*
* Copyright (c) 2021-2022 Barbara Geller
* Copyright (c) 2021-2022 Ansel Sermersheim
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

#ifndef GRADIENTMANAGER_H
#define GRADIENTMANAGER_H

#include <QDomDocument>
#include <QDomElement>
#include <QGradient>
#include <QMap>
#include <QObject>
#include <QSize>

class QtGradientManager : public QObject
{
   CS_OBJECT(QtGradientManager)

 public:
   QtGradientManager(QObject *parent = nullptr);

   QMap<QString, QGradient> gradients() const;
   QString uniqueId(const QString &id) const;

   CS_SLOT_1(Public, QString addGradient(const QString &id, const QGradient &gradient))
   CS_SLOT_2(addGradient)
   CS_SLOT_1(Public, void renameGradient(const QString &id, const QString &newId))
   CS_SLOT_2(renameGradient)
   CS_SLOT_1(Public, void changeGradient(const QString &id, const QGradient &newGradient))
   CS_SLOT_2(changeGradient)
   CS_SLOT_1(Public, void removeGradient(const QString &id))
   CS_SLOT_2(removeGradient)

   //utils
   CS_SLOT_1(Public, void clear())
   CS_SLOT_2(clear)

   CS_SIGNAL_1(Public, void gradientAdded(const QString &id, const QGradient &gradient))
   CS_SIGNAL_2(gradientAdded, id, gradient)
   CS_SIGNAL_1(Public, void gradientRenamed(const QString &id, const QString &newId))
   CS_SIGNAL_2(gradientRenamed, id, newId)
   CS_SIGNAL_1(Public, void gradientChanged(const QString &id, const QGradient &newGradient))
   CS_SIGNAL_2(gradientChanged, id, newGradient)
   CS_SIGNAL_1(Public, void gradientRemoved(const QString &id))
   CS_SIGNAL_2(gradientRemoved, id)

 private:

   QMap<QString, QGradient> m_idToGradient;
};



#endif
