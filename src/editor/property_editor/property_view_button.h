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

#ifndef PROPERTY_VIEW_BUTTON_H
#define PROPERTY_VIEW_BUTTON_H

#include <property_view.h>

class QtButtonPropertyBrowserPrivate;

class QtButtonPropertyBrowser : public QtAbstractPropertyBrowser
{
   CS_OBJECT(QtButtonPropertyBrowser)

 public:
   QtButtonPropertyBrowser(QWidget *parent = nullptr);
   ~QtButtonPropertyBrowser();

   void setExpanded(QtBrowserItem *browserItem, bool expanded);
   bool isExpanded(QtBrowserItem *browserItem) const;

   CS_SIGNAL_1(Public, void collapsed(QtBrowserItem *item))
   CS_SIGNAL_2(collapsed, item)

   CS_SIGNAL_1(Public, void expanded(QtBrowserItem *item))
   CS_SIGNAL_2(expanded, item)

 protected:
   void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem) override;
   void itemRemoved(QtBrowserItem *item) override;
   void itemChanged(QtBrowserItem *item) override;

 private:
   QScopedPointer<QtButtonPropertyBrowserPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtButtonPropertyBrowser)
   Q_DISABLE_COPY(QtButtonPropertyBrowser)

   CS_SLOT_1(Private, void slotUpdate())
   CS_SLOT_2(slotUpdate)

   CS_SLOT_1(Private, void slotEditorDestroyed())
   CS_SLOT_2(slotEditorDestroyed)

   CS_SLOT_1(Private, void slotToggled(bool un_named_arg1))
   CS_SLOT_2(slotToggled)
};

#endif
