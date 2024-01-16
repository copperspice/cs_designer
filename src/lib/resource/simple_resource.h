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

#ifndef QSIMPLERESOURCE_H
#define QSIMPLERESOURCE_H

#include <abstract_formbuilder.h>

class QDesignerFormEditorInterface;
class DomScript;
class DomCustomWidgets;
class DomCustomWidget;
class DomSlots;

#include <QStringList>

namespace qdesigner_internal {

class WidgetDataBaseItem;

class QSimpleResource : public QAbstractFormBuilder
{
 public:
   explicit QSimpleResource(QDesignerFormEditorInterface *core);
   virtual ~QSimpleResource();

   QBrush setupBrush(DomBrush *brush);
   DomBrush *saveBrush(const QBrush &brush);

   QDesignerFormEditorInterface *core() const {
      return m_core;
   }

   // Query extensions for additional data
   static void addExtensionDataToDOM(QAbstractFormBuilder *afb,
      QDesignerFormEditorInterface *core,
      DomWidget *ui_widget, QWidget *widget);

   static void applyExtensionDataFromDOM(QAbstractFormBuilder *afb,
      QDesignerFormEditorInterface *core,
      DomWidget *ui_widget, QWidget *widget);

   // Return the script returned by the CustomWidget codeTemplate API
   static QString customWidgetScript(QDesignerFormEditorInterface *core, QObject *object);
   static QString customWidgetScript(QDesignerFormEditorInterface *core, const QString &className);
   static bool hasCustomWidgetScript(QDesignerFormEditorInterface *core, QObject *object);

   // Implementation for FormBuilder::createDomCustomWidgets() that adds
   // the custom widgets to the widget database
   static void handleDomCustomWidgets(const QDesignerFormEditorInterface *core,
      const DomCustomWidgets *dom_custom_widgets);

 protected:
   enum ScriptSource { ScriptDesigner, ScriptExtension, ScriptCustomWidgetPlugin };
   static DomScript *createScript(const QString &script, ScriptSource source);
   typedef QList<DomScript *> DomScripts;
   static void addScript(const QString &script, ScriptSource source, DomScripts &domScripts);

   static bool addFakeMethods(const DomSlots *domSlots, QStringList &fakeSlots, QStringList &fakeSignals);

 private:
   static void addCustomWidgetsToWidgetDatabase(const QDesignerFormEditorInterface *core,
      QList<DomCustomWidget *> &custom_widget_list);
   static void addFakeMethodsToWidgetDataBase(const DomCustomWidget *domCustomWidget, WidgetDataBaseItem *item);

   static bool m_warningsEnabled;
   QDesignerFormEditorInterface *m_core;
};

// Contents of clipboard for formbuilder copy and paste operations
// (Actions and widgets)
struct FormBuilderClipboard {
   typedef QList<QAction *> ActionList;

   FormBuilderClipboard() {}
   FormBuilderClipboard(QWidget *w);

   bool empty() const;

   QWidgetList m_widgets;
   ActionList m_actions;
};

// Base class for a form builder used in the editor that
// provides copy and paste.(move into base interface)
class QEditorFormBuilder : public QSimpleResource
{
 public:
   explicit QEditorFormBuilder(QDesignerFormEditorInterface *core) : QSimpleResource(core) {}

   virtual bool copy(QIODevice *dev, const FormBuilderClipboard &selection) = 0;
   virtual DomUI *copy(const FormBuilderClipboard &selection) = 0;

   // A widget parent needs to be specified, otherwise, the widget factory cannot locate the form window via parent
   // and thus is not able to construct special widgets (QLayoutWidget).
   virtual FormBuilderClipboard paste(DomUI *ui, QWidget *widgetParent, QObject *actionParent = nullptr) = 0;
   virtual FormBuilderClipboard paste(QIODevice *dev, QWidget *widgetParent, QObject *actionParent = nullptr) = 0;
};

}   // end namespace qdesigner_internal

#endif
