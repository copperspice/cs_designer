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

#include <widgetbox_dnditem.h>
#include <abstract_formeditor.h>
#include <designer_utils.h>
#include <designer_dockwidget.h>
#include <designer_formbuilder.h>
#include <resource_model.h>
#include <simple_resource.h>
#include <ui4.h>

#include <widgetfactory.h>
#include <spacer_widget_p.h>
#include <formwindowbase_p.h>
#include <formwindowbase_p.h>

#include <QStyle>
#include <QApplication>

namespace qdesigner_internal {

static inline DeviceProfile currentDeviceProfile(const QDesignerFormEditorInterface *core)
{
   if (QDesignerFormWindowInterface *cfw = core->formWindowManager()->activeFormWindow())
      if (const FormWindowBase *fwb = dynamic_cast<const FormWindowBase *>(cfw)) {
         return fwb->deviceProfile();
      }
   return DeviceProfile();
}

class WidgetBoxResource : public QDesignerFormBuilder
{
 public:
   WidgetBoxResource(QDesignerFormEditorInterface *core);

   // protected->public
   QWidget *createUI(DomUI *ui, QWidget *parents) {
      return QDesignerFormBuilder::create(ui, parents);
   }

 protected:

   QWidget *create(DomWidget *ui_widget, QWidget *parents) override;
   QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name) override;
   void createCustomWidgets(DomCustomWidgets *) override;
};

WidgetBoxResource::WidgetBoxResource(QDesignerFormEditorInterface *core) :
   QDesignerFormBuilder(core, currentDeviceProfile(core))
{
}


QWidget *WidgetBoxResource::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
   if (widgetName == QString("Spacer")) {
      Spacer *spacer = new Spacer(parentWidget);
      spacer->setObjectName(name);
      return spacer;
   }

   return QDesignerFormBuilder::createWidget(widgetName, parentWidget, name);
}

QWidget *WidgetBoxResource::create(DomWidget *ui_widget, QWidget *parent)
{
   QWidget *result = QDesignerFormBuilder::create(ui_widget, parent);
   // It is possible to have a syntax error or something in custom
   // widget XML, so, try to recover here by creating an artificial
   // top level + widget.
   if (!result) {
      const QString msg = QApplication::translate("qdesigner_internal::WidgetBox",
            "Warning: Widget creation failed in the widget box. This could be caused by invalid custom widget XML.");
      csWarning(msg);
      result = new QWidget(parent);
      new QWidget(result);
   }
   result->setFocusPolicy(Qt::NoFocus);
   result->setObjectName(ui_widget->attributeName());
   return result;
}

void WidgetBoxResource::createCustomWidgets(DomCustomWidgets *dc)
{
   // Make a promotion entry in  case someone has a promoted widget
   // in the scratchpad.
   QSimpleResource::handleDomCustomWidgets(core(), dc);

}

/*******************************************************************************
** WidgetBoxResource
*/

static QSize geometryProp(const DomWidget *dw)
{
   const QList<DomProperty *> prop_list = dw->elementProperty();
   const QString geometry = QString("geometry");
   for (DomProperty *prop : prop_list) {
      if (prop->attributeName() !=  geometry) {
         continue;
      }
      DomRect *dr = prop->elementRect();
      if (dr == 0) {
         continue;
      }
      return QSize(dr->elementWidth(), dr->elementHeight());
   }
   return QSize();
}

static QSize domWidgetSize(const DomWidget *dw)
{
   QSize size = geometryProp(dw);
   if (size.isValid()) {
      return size;
   }

   for (const DomWidget *child : dw->elementWidget()) {
      size = geometryProp(child);
      if (size.isValid()) {
         return size;
      }
   }

   for (const DomLayout *dl : dw->elementLayout()) {
      for (DomLayoutItem *item : dl->elementItem()) {
         const DomWidget *child = item->elementWidget();
         if (child == 0) {
            continue;
         }
         size = geometryProp(child);
         if (size.isValid()) {
            return size;
         }
      }
   }

   return QSize();
}

static QWidget *decorationFromDomWidget(DomUI *dom_ui, QDesignerFormEditorInterface *core)
{
   WidgetBoxResource builder(core);
   // We have the builder create the articial QWidget fake top level as a tooltip
   // because the size algorithm works better at weird DPI settings
   // if the actual widget is created as a child of a container

   QWidget *fakeTopLevel = builder.createUI(dom_ui, static_cast<QWidget *>(0));
   fakeTopLevel->setParent(0, Qt::ToolTip); // Container

   // Actual widget
   const DomWidget *domW = dom_ui->elementWidget()->elementWidget().front();
   QWidget *w = fakeTopLevel->findChildren<QWidget *>().front();
   Q_ASSERT(w);

   // hack begin;
   // We set _q_dockDrag dynamic property which will be detected in drag enter event of form window.
   // Dock drop is handled in special way (highlight goes to central widget of main window)
   if (dynamic_cast<QDesignerDockWidget *>(w)) {
      fakeTopLevel->setProperty("_q_dockDrag", QVariant(true));
   }

   // hack end;
   w->setAutoFillBackground(true); // Different style for embedded
   QSize size = domWidgetSize(domW);
   const QSize minimumSize = w->minimumSizeHint();

   if (!size.isValid()) {
      size = w->sizeHint();
   }

   if (size.width() < minimumSize.width()) {
      size.setWidth(minimumSize.width());
   }

   if (size.height() < minimumSize.height()) {
      size.setHeight(minimumSize.height());
   }

   // A QWidget might have size -1,-1 if no geometry property is specified in the widget box.
   if (size.isEmpty()) {
      size = size.expandedTo(QSize(16, 16));
   }

   w->setGeometry(QRect(QPoint(0, 0), size));
   fakeTopLevel->resize(size);

   return fakeTopLevel;
}

WidgetBoxDnDItem::WidgetBoxDnDItem(QDesignerFormEditorInterface *core, DomUI *dom_ui, const QPoint &global_mouse_pos)
   :  QDesignerDnDItem(CopyDrop)
{
   QWidget *decoration = decorationFromDomWidget(dom_ui, core);
   decoration->move(global_mouse_pos - QPoint(5, 5));

   init(dom_ui, 0, decoration, global_mouse_pos);
}

}   // end namespace


