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

#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include <abstract_widgetfactory.h>
#include <plugin_manager.h>

class QDesignerFormEditorInterface;
class QDesignerCustomWidgetInterface;
class QDesignerFormWindowInterface;

#include <QHash>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QVariant>

class QWidget;
class QLayout;
class QStyle;

namespace qdesigner_internal {

class WidgetFactory: public QDesignerWidgetFactoryInterface
{
   CS_OBJECT(WidgetFactory)

 public:
   explicit WidgetFactory(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
   ~WidgetFactory();

   QWidget *containerOfWidget(QWidget *widget) const override;
   QWidget *widgetOfContainer(QWidget *widget) const override;

   QObject *createObject(const QString &className, QObject *parent) const;

   QWidget *createWidget(const QString &className, QWidget *parentWidget) const override;
   QLayout *createLayout(QWidget *widget, QLayout *layout, int type) const override;

   bool isPassiveInteractor(QWidget *widget) override;
   void initialize(QObject *object) const override;
   void initializeCommon(QWidget *object) const;
   void initializePreview(QWidget *object) const;


   QDesignerFormEditorInterface *core() const override;

   static QString classNameOf(QDesignerFormEditorInterface *core, const QObject *o);

   QDesignerFormWindowInterface *currentFormWindow(QDesignerFormWindowInterface *fw);

   static QLayout *createUnmanagedLayout(QWidget *parentWidget, int type);

   // The widget factory maintains a cache of styles which it owns.
   QString styleName() const;
   void setStyleName(const QString &styleName);

   /* Return a cached style matching the name or QApplication's style if
    * it is the default. */
   QStyle *getStyle(const QString &styleName);
   // Return the current style used by the factory. This either a cached one
   // or QApplication's style */
   QStyle *style() const;

   // Apply one of the cached styles or QApplication's style to a toplevel widget.
   void applyStyleTopLevel(const QString &styleName, QWidget *w);
   static void applyStyleToTopLevel(QStyle *style, QWidget *widget);

   // Return whether object was created by the factory for the form editor.
   static bool isFormEditorObject(const QObject *o);

   // Boolean dynamic property to set on widgets to prevent custom
   // styles from interfering
   static const QString disableStyleCustomPaintingPropertyC;

   CS_SLOT_1(Public, void loadPlugins())
   CS_SLOT_2(loadPlugins)
   CS_SLOT_1(Public, void activeFormWindowChanged(QDesignerFormWindowInterface *formWindow))
   CS_SLOT_2(activeFormWindowChanged)
   CS_SLOT_1(Public, void formWindowAdded(QDesignerFormWindowInterface *formWindow))
   CS_SLOT_2(formWindowAdded)

 private:
   struct Strings { // Reduce string allocations by storing predefined strings
      Strings();
      const QString m_alignment;
      const QString m_bottomMargin;
      const QString m_geometry;
      const QString m_leftMargin;
      const QString m_line;
      const QString m_objectName;
      const QString m_spacerName;
      const QString m_orientation;
      const QString m_qAction;
      const QString m_qButtonGroup;
      const QString m_qAxWidget;
      const QString m_qDialog;
      const QString m_qDockWidget;
      const QString m_qLayoutWidget;
      const QString m_qMenu;
      const QString m_qMenuBar;
      const QString m_qWidget;
      const QString m_rightMargin;
      const QString m_sizeHint;
      const QString m_spacer;
      const QString m_text;
      const QString m_title;
      const QString m_topMargin;
      const QString m_windowIcon;
      const QString m_windowTitle;
   };

   QWidget *createCustomWidget(const QString &className, QWidget *parentWidget, bool *creationError) const;
   QDesignerFormWindowInterface *findFormWindow(QWidget *parentWidget) const;
   void setFormWindowStyle(QDesignerFormWindowInterface *formWindow);

   const Strings m_strings;
   QDesignerFormEditorInterface *m_core;
   typedef QMap<QString, QDesignerCustomWidgetInterface *> CustomWidgetFactoryMap;
   CustomWidgetFactoryMap m_customFactory;
   QDesignerFormWindowInterface *m_formWindow;

   // Points to the cached style or 0 if the default (qApp) is active
   QStyle *m_currentStyle;
   typedef QHash<QString, QStyle *> StyleCache;
   StyleCache m_styleCache;

   static QPointer<QWidget> *m_lastPassiveInteractor;
   static bool m_lastWasAPassiveInteractor;
};

} // namespace qdesigner_internal



#endif // WIDGETFACTORY_H
