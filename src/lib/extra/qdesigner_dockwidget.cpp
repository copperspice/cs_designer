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

#include <abstract_formwindow.h>
#include <abstract_formeditor.h>
#include <container.h>
#include <extension.h>
#include <extension_manager.h>
#include <abstract_formwindowcursor.h>

#include <qdesigner_dockwidget_p.h>
#include <layoutinfo_p.h>

#include <QMainWindow>
#include <QLayout>

QDesignerDockWidget::QDesignerDockWidget(QWidget *parent)
   : QDockWidget(parent)
{
}

QDesignerDockWidget::~QDesignerDockWidget()
{
}

bool QDesignerDockWidget::docked() const
{
   return dynamic_cast<QMainWindow *>(parentWidget()) != 0;
}

void QDesignerDockWidget::setDocked(bool b)
{
   if (QMainWindow *mainWindow = findMainWindow()) {
      QDesignerFormEditorInterface *core = formWindow()->core();
      QDesignerContainerExtension *c;
      c = qt_extension<QDesignerContainerExtension *>(core->extensionManager(), mainWindow);
      if (b && !docked()) {
         // Dock it
         // ### undo/redo stack
         setParent(0);
         c->addWidget(this);
         formWindow()->selectWidget(this, formWindow()->cursor()->isWidgetSelected(this));
      } else if (!b && docked()) {
         // Undock it
         for (int i = 0; i < c->count(); ++i) {
            if (c->widget(i) == this) {
               c->remove(i);
               break;
            }
         }
         // #### restore the position
         setParent(mainWindow->centralWidget());
         show();
         formWindow()->selectWidget(this, formWindow()->cursor()->isWidgetSelected(this));
      }
   }
}

Qt::DockWidgetArea QDesignerDockWidget::dockWidgetArea() const
{
   if (QMainWindow *mainWindow = dynamic_cast<QMainWindow *>(parentWidget())) {
      return mainWindow->dockWidgetArea(const_cast<QDesignerDockWidget *>(this));
   }

   return Qt::LeftDockWidgetArea;
}

void QDesignerDockWidget::setDockWidgetArea(Qt::DockWidgetArea dockWidgetArea)
{
   if (QMainWindow *mainWindow = dynamic_cast<QMainWindow *>(parentWidget())) {
      if ((dockWidgetArea != Qt::NoDockWidgetArea)
         && isAreaAllowed(dockWidgetArea)) {
         mainWindow->addDockWidget(dockWidgetArea, this);
      }
   }
}

bool QDesignerDockWidget::inMainWindow() const
{
   QMainWindow *mw = findMainWindow();
   if (mw && !mw->centralWidget()->layout()) {
      if (mw == parentWidget()) {
         return true;
      }
      if (mw->centralWidget() == parentWidget()) {
         return true;
      }
   }
   return false;
}

QDesignerFormWindowInterface *QDesignerDockWidget::formWindow() const
{
   return QDesignerFormWindowInterface::findFormWindow(const_cast<QDesignerDockWidget *>(this));
}

QMainWindow *QDesignerDockWidget::findMainWindow() const
{
   if (QDesignerFormWindowInterface *fw = formWindow()) {
      return dynamic_cast<QMainWindow *>(fw->mainContainer());
   }

   return nullptr;
}
