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

#include <abstract_formwindow.h>
#include <resource_model.h>
#include <widgetfactory.h>

#include <QAbstractButton>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QSizeGrip>
#include <QTabBar>
#include <QToolBar>
#include <QToolBox>

QDesignerFormWindowInterface::QDesignerFormWindowInterface(QWidget *parent, Qt::WindowFlags flags)
   : QWidget(parent, flags)
{
}

QDesignerFormWindowInterface::~QDesignerFormWindowInterface()
{
}

QDesignerFormEditorInterface *QDesignerFormWindowInterface::core() const
{
   return nullptr;
}

static inline bool stopFindAtTopLevel(const QObject *w, bool stopAtMenu)
{
   // Do we need to go beyond top levels when looking for the form window?
   // 1) A dialog has a window attribute at the moment it is created
   //    before it is properly embedded into a form window. The property
   //    sheet queries the layout attributes precisely at this moment.
   // 2) In the case of floating docks and toolbars, we also need to go beyond the top level window.
   // 3) In the case of menu editing, we don't want to block events from the
   //    Designer menu, so, we say stop.
   // Note that there must be no false positives for dialogs parented on
   // the form (for example, the "change object name" dialog), else, its
   // events will be blocked.

   if (stopAtMenu && w->inherits("QDesignerMenu")) {
      return true;
   }
   return !qdesigner_internal::WidgetFactory::isFormEditorObject(w);
}

QDesignerFormWindowInterface *QDesignerFormWindowInterface::findFormWindow(QWidget *w)
{
   while (w != nullptr) {
      if (QDesignerFormWindowInterface *fw = dynamic_cast<QDesignerFormWindowInterface *>(w)) {
         return fw;
      } else {
         if (w->isWindow() && stopFindAtTopLevel(w, true)) {
            break;
         }
      }

      w = w->parentWidget();
   }

   return nullptr;
}

QDesignerFormWindowInterface *QDesignerFormWindowInterface::findFormWindow(QObject *object)
{
   while (object != nullptr) {
      if (QDesignerFormWindowInterface *fw = dynamic_cast<QDesignerFormWindowInterface *>(object)) {
         return fw;

      } else {
         QWidget *w = dynamic_cast<QWidget *>(object);
         // QDesignerMenu is a window, so stopFindAtTopLevel(w) returns 0.
         // However, we want to find the form window for QActions of a menu.
         // If this check is inside stopFindAtTopLevel(w), it will break designer
         // menu editing (e.g. when clicking on items inside an opened menu)
         if (w && w->isWindow() && stopFindAtTopLevel(w, false)) {
            break;
         }

      }

      object = object->parent();
   }

   return nullptr;
}

QStringList QDesignerFormWindowInterface::activeResourceFilePaths() const
{
   return resourceSet()->activeResourceFilePaths();
}

void QDesignerFormWindowInterface::activateResourceFilePaths(const QStringList &paths, int *errorCount, QString *errorMessages)
{
   resourceSet()->activateResourceFilePaths(paths, errorCount, errorMessages);
}
