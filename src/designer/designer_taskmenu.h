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

#ifndef QDESIGNER_TASKMENU_H
#define QDESIGNER_TASKMENU_H

#include <taskmenu.h>

#include <extensionfactory_p.h>

#include <QAction>
#include <QList>
#include <QObject>
#include <QPointer>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

class QSignalMapper;
class QWidget;

namespace qdesigner_internal {

class QDesignerTaskMenuPrivate;

class QDesignerTaskMenu: public QObject, public QDesignerTaskMenuExtension
{
   CS_OBJECT_MULTIPLE(QDesignerTaskMenu, QObject)

   CS_INTERFACES(QDesignerTaskMenuExtension)

 public:
   QDesignerTaskMenu(QWidget *widget, QObject *parent);

   virtual ~QDesignerTaskMenu();

   QWidget *widget() const;

   QList<QAction *> taskActions() const override;

   enum PropertyMode { CurrentWidgetMode, MultiSelectionMode };

   static bool isSlotNavigationEnabled(const QDesignerFormEditorInterface *core);
   static void navigateToSlot(QDesignerFormEditorInterface *core, QObject *o, const QString &defaultSignal = QString());

 protected:
   QDesignerFormWindowInterface *formWindow() const;
   void changeTextProperty(const QString &propertyName, const QString &windowTitle, PropertyMode pm, Qt::TextFormat desiredFormat);

   QAction *createSeparator();

   /* Retrieve the list of objects the task menu is supposed to act on. Note that a task menu can be invoked for
    * an unmanaged widget [as of 4.5], in which case it must not use the cursor selection,
    * but the unmanaged selection of the object inspector. */
   QObjectList applicableObjects(const QDesignerFormWindowInterface *fw, PropertyMode pm) const;
   QList<QWidget *> applicableWidgets(const QDesignerFormWindowInterface *fw, PropertyMode pm) const;

   void setProperty(QDesignerFormWindowInterface *fw, PropertyMode pm, const QString &name, const QVariant &newValue);

 private:
   // slot method
   void changeObjectName();

   CS_SLOT_1(Private, void changeToolTip())
   CS_SLOT_2(changeToolTip)
   CS_SLOT_1(Private, void changeWhatsThis())
   CS_SLOT_2(changeWhatsThis)
   CS_SLOT_1(Private, void changeStyleSheet())
   CS_SLOT_2(changeStyleSheet)
   CS_SLOT_1(Private, void createMenuBar())
   CS_SLOT_2(createMenuBar)
   CS_SLOT_1(Private, void addToolBar())
   CS_SLOT_2(addToolBar)
   CS_SLOT_1(Private, void createStatusBar())
   CS_SLOT_2(createStatusBar)
   CS_SLOT_1(Private, void removeStatusBar())
   CS_SLOT_2(removeStatusBar)
   CS_SLOT_1(Private, void containerFakeMethods())
   CS_SLOT_2(containerFakeMethods)
   CS_SLOT_1(Private, void slotNavigateToSlot())
   CS_SLOT_2(slotNavigateToSlot)
   CS_SLOT_1(Private, void applySize(QAction *a))
   CS_SLOT_2(applySize)
   CS_SLOT_1(Private, void slotLayoutAlignment())
   CS_SLOT_2(slotLayoutAlignment)

   QDesignerTaskMenuPrivate *d;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QWidget, QDesignerTaskMenu>  QDesignerTaskMenuFactory;

} // namespace qdesigner_internal

#endif
