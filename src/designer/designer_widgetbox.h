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

#ifndef QDESIGNER_WIDGETBOX_H
#define QDESIGNER_WIDGETBOX_H

#include <abstract_widgetbox.h>

class DomUI;

namespace qdesigner_internal {

// A widget box with a load mode that allows for updating custom widgets.

class QDesignerWidgetBox : public QDesignerWidgetBoxInterface
{
   CS_OBJECT(QDesignerWidgetBox)
 public:
   enum LoadMode { LoadMerge, LoadReplace, LoadCustomWidgetsOnly };

   explicit QDesignerWidgetBox(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   LoadMode loadMode() const;
   void setLoadMode(LoadMode lm);

   virtual bool loadContents(const QString &contents) = 0;

   // Convenience to access the widget box icon of a widget. Empty category
   // matches all
   virtual QIcon iconForWidget(const QString &className,
      const QString &category = QString()) const = 0;

   // Convenience to find a widget by class name. Empty category matches all
   static bool findWidget(const QDesignerWidgetBoxInterface *wbox,
      const QString &className,
      const QString &category /* = QString()  */,
      Widget *widgetData);
   // Convenience functions to create a DomWidget from widget box xml.
   static DomUI *xmlToUi(const QString &name, const QString &xml, bool insertFakeTopLevel, QString *errorMessage);
   static DomUI *xmlToUi(const QString &name, const QString &xml, bool insertFakeTopLevel);

 private:
   LoadMode m_loadMode;
};
}  // namespace qdesigner_internal



#endif // QDESIGNER_WIDGETBOX_H
