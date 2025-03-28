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

#include <abstract_actioneditor.h>
#include <abstract_formeditor.h>
#include <abstract_propertyeditor.h>
#include <abstract_widgetbox.h>
#include <designer.h>
#include <designer_components.h>
#include <designer_object_inspector.h>
#include <designer_settings.h>
#include <designer_toolwindow.h>
#include <designer_workbench.h>

#include <QAction>
#include <QCloseEvent>
#include <QEvent>

ToolWindowFontSettings::ToolWindowFontSettings()
   : m_writingSystem(QFontDatabase::Any), m_useFont(false)
{
}

bool ToolWindowFontSettings::equals(const ToolWindowFontSettings &rhs) const
{
   return m_useFont == rhs.m_useFont &&
      m_writingSystem == rhs.m_writingSystem &&
      m_font == rhs.m_font;
}

QDesignerToolWindow::QDesignerToolWindow(QDesignerWorkbench *workbench, QWidget *w,
   const QString &objectName, const QString &title, const QString &actionObjectName,
   Qt::DockWidgetArea dockAreaHint, QWidget *parent, Qt::WindowFlags flags)
   : MainWindowBase(parent, flags), m_dockAreaHint(dockAreaHint),
     m_workbench(workbench), m_action(new QAction(this))
{
   setObjectName(objectName);
   setCentralWidget(w);

   setWindowTitle(title);

   m_action->setObjectName(actionObjectName);
   m_action->setShortcutContext(Qt::ApplicationShortcut);
   m_action->setText(title);
   m_action->setCheckable(true);

   connect(m_action, &QAction::triggered, this, &QDesignerToolWindow::showMe);
}

void QDesignerToolWindow::showMe(bool enable)
{
   // Access the QMdiSubWindow in MDI mode.
   if (QWidget *target = m_workbench->mode() == DockedMode ? parentWidget() : this)  {
      if (enable) {
         target->setWindowState(target->windowState() & ~Qt::WindowMinimized);
      }

      target->setVisible(enable);
   }
}

void QDesignerToolWindow::showEvent(QShowEvent *e)
{
   (void) e;

   bool blocked = m_action->blockSignals(true);
   m_action->setChecked(true);
   m_action->blockSignals(blocked);
}

void QDesignerToolWindow::hideEvent(QHideEvent *e)
{
   (void) e;

   bool blocked = m_action->blockSignals(true);
   m_action->setChecked(false);
   m_action->blockSignals(blocked);
}

QAction *QDesignerToolWindow::action() const
{
   return m_action;
}

void QDesignerToolWindow::changeEvent(QEvent *e)
{
   switch (e->type()) {
      case QEvent::WindowTitleChange:
         m_action->setText(windowTitle());
         break;
      case QEvent::WindowIconChange:
         m_action->setIcon(windowIcon());
         break;
      default:
         break;
   }
   QMainWindow::changeEvent(e);
}

QDesignerWorkbench *QDesignerToolWindow::workbench() const
{
   return m_workbench;
}

QRect QDesignerToolWindow::geometryHint() const
{
   return QRect();
}

QRect QDesignerToolWindow::availableToolWindowGeometry() const
{
   return m_workbench->availableGeometry();
}

//  ---------------------- PropertyEditorToolWindow

static inline QWidget *createPropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent = nullptr)
{
   QDesignerPropertyEditorInterface *widget = QDesignerComponents::createPropertyEditor(core, parent);
   core->setPropertyEditor(widget);

   return widget;
}

class PropertyEditorToolWindow : public QDesignerToolWindow
{
 public:
   explicit PropertyEditorToolWindow(QDesignerWorkbench *workbench);

   QRect geometryHint() const override;

 protected:
   void showEvent(QShowEvent *event) override;
};

PropertyEditorToolWindow::PropertyEditorToolWindow(QDesignerWorkbench *workbench)
   : QDesignerToolWindow(workbench, createPropertyEditor(workbench->core()),
     QString("qt_designer_propertyeditor"), QDesignerToolWindow::tr("Property Editor"),
     QString("__qt_property_editor_action"), Qt::RightDockWidgetArea)
{
   action()->setShortcut(cs_enum_cast(Qt::ControlModifier) + cs_enum_cast(Qt::Key_I));
}

QRect PropertyEditorToolWindow::geometryHint() const
{
   const QRect g     = availableToolWindowGeometry();
   const int margin  = workbench()->marginHint();
   const int spacing = 40;

   const QSize sz(g.width() * 1 / 4, g.height() * 4 / 6);

   const QRect rc = QRect((g.right() + 1 - sz.width() - margin),
         (g.top() + margin + g.height() * 1 / 6) + spacing,
         sz.width(), sz.height());

   return rc;
}

void PropertyEditorToolWindow::showEvent(QShowEvent *event)
{
   if (QDesignerPropertyEditorInterface *e = workbench()->core()->propertyEditor()) {
      // workaround to update the propertyeditor when it is not visible
      e->setObject(e->object()); // ### remove me
   }

   QDesignerToolWindow::showEvent(event);
}

static inline QWidget *createActionEditor(QDesignerFormEditorInterface *core, QWidget *parent = nullptr)
{
   QDesignerActionEditorInterface *widget = QDesignerComponents::createActionEditor(core, parent);
   core->setActionEditor(widget);
   return widget;
}

class ActionEditorToolWindow: public QDesignerToolWindow
{
 public:
   explicit ActionEditorToolWindow(QDesignerWorkbench *workbench);

   QRect geometryHint() const override;
};

ActionEditorToolWindow::ActionEditorToolWindow(QDesignerWorkbench *workbench) :
   QDesignerToolWindow(workbench,
      createActionEditor(workbench->core()),
      QString("qt_designer_actioneditor"),
      QDesignerToolWindow::tr("Action Editor"),
      QString("__qt_action_editor_tool_action"),
      Qt::RightDockWidgetArea)
{
}

QRect ActionEditorToolWindow::geometryHint() const
{
   const QRect g = availableToolWindowGeometry();
   const int margin = workbench()->marginHint();

   const QSize sz(g.width() * 1 / 4, g.height() * 1 / 6);

   const QRect rc = QRect((g.right() + 1 - sz.width() - margin), g.top() + margin, sz.width(), sz.height());

   return rc;
}

//  ---------------------- ObjectInspectorToolWindow

static inline QWidget *createObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent = nullptr)
{
   QDesignerObjectInspectorInterface *widget = QDesignerComponents::createObjectInspector(core, parent);
   core->setObjectInspector(widget);
   return widget;
}

class ObjectInspectorToolWindow: public QDesignerToolWindow
{
 public:
   explicit ObjectInspectorToolWindow(QDesignerWorkbench *workbench);

   QRect geometryHint() const override;
};

ObjectInspectorToolWindow::ObjectInspectorToolWindow(QDesignerWorkbench *workbench) :
   QDesignerToolWindow(workbench,
      createObjectInspector(workbench->core()),
      QString("qt_designer_objectinspector"),
      QDesignerToolWindow::tr("Object Inspector"),
      QString("__qt_object_inspector_tool_action"),
      Qt::RightDockWidgetArea)
{
}

QRect ObjectInspectorToolWindow::geometryHint() const
{
   const QRect g = availableToolWindowGeometry();
   const int margin = workbench()->marginHint();

   const QSize sz(g.width() * 1 / 4, g.height() * 1 / 6);

   const QRect rc = QRect((g.right() + 1 - sz.width() - margin), g.top() + margin, sz.width(), sz.height());

   return rc;
}

class ResourceEditorToolWindow: public QDesignerToolWindow
{
 public:
   explicit ResourceEditorToolWindow(QDesignerWorkbench *workbench);

   QRect geometryHint() const override;
};

ResourceEditorToolWindow::ResourceEditorToolWindow(QDesignerWorkbench *workbench)
   : QDesignerToolWindow(workbench,
     QDesignerComponents::createResourceEditor(workbench->core(), nullptr), "qt_designer_resourceeditor",
     QDesignerToolWindow::tr("Resource Editor"), "__qt_resource_editor_tool_action", Qt::RightDockWidgetArea)
{
}

QRect ResourceEditorToolWindow::geometryHint() const
{
   const QRect g = availableToolWindowGeometry();
   const int margin = workbench()->marginHint();

   const QSize sz(g.width() * 1 / 3, g.height() * 1 / 6);
   QRect r(QPoint(0, 0), sz);

   r.moveCenter(g.center());
   r.moveBottom(g.bottom() - margin);

   return r;
}

class SignalSlotEditorToolWindow: public QDesignerToolWindow
{
 public:
   explicit SignalSlotEditorToolWindow(QDesignerWorkbench *workbench);

   QRect geometryHint() const override;
};

SignalSlotEditorToolWindow::SignalSlotEditorToolWindow(QDesignerWorkbench *workbench)
   : QDesignerToolWindow(workbench, QDesignerComponents::createSignalSlotEditor(workbench->core(), nullptr),
        QString("qt_designer_signalsloteditor"), QDesignerToolWindow::tr("Signal/Slot Editor"),
        QString("__qt_signal_slot_editor_tool_action"), Qt::RightDockWidgetArea)
{
}

QRect SignalSlotEditorToolWindow::geometryHint() const
{
   const QRect g = availableToolWindowGeometry();
   const int margin = workbench()->marginHint();

   const QSize sz(g.width() * 1 / 3, g.height() * 1 / 6);
   QRect r(QPoint(0, 0), sz);
   r.moveCenter(g.center());
   r.moveTop(margin + g.top());

   return r;
}

static inline QWidget *createWidgetBox(QDesignerFormEditorInterface *core, QWidget *parent = nullptr)
{
   QDesignerWidgetBoxInterface *widget = QDesignerComponents::createWidgetBox(core, parent);
   core->setWidgetBox(widget);
   return widget;
}

class WidgetBoxToolWindow: public QDesignerToolWindow
{
 public:
   explicit WidgetBoxToolWindow(QDesignerWorkbench *workbench);

   QRect geometryHint() const override;
};

WidgetBoxToolWindow::WidgetBoxToolWindow(QDesignerWorkbench *workbench)
   : QDesignerToolWindow(workbench, createWidgetBox(workbench->core()),
     QString("qt_designer_widgetbox"), QDesignerToolWindow::tr("Widget Box"),
     QString("__qt_widget_box_tool_action"), Qt::LeftDockWidgetArea)
{
}

QRect WidgetBoxToolWindow::geometryHint() const
{
   const QRect g = availableToolWindowGeometry();
   const int margin = workbench()->marginHint();
   const  QRect rc = QRect(g.left() + margin, g.top() + margin, g.width() * 1 / 4, g.height() * 5 / 6);

   return rc;
}

QDesignerToolWindow *QDesignerToolWindow::createStandardToolWindow(StandardToolWindow which,
   QDesignerWorkbench *workbench)
{
   switch (which) {
      case ActionEditor:
         return new ActionEditorToolWindow(workbench);
      case ResourceEditor:
         return new ResourceEditorToolWindow(workbench);
      case SignalSlotEditor:
         return new SignalSlotEditorToolWindow(workbench);
      case PropertyEditor:
         return new PropertyEditorToolWindow(workbench);
      case ObjectInspector:
         return new ObjectInspectorToolWindow(workbench);
      case WidgetBox:
         return new WidgetBoxToolWindow(workbench);
      default:
         break;
   }
   return nullptr;
}


