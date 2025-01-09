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

#ifndef PROPERTY_VIEW_GROUPBOX_H
#define PROPERTY_VIEW_GROUPBOX_H

#include <property_view.h>

class QtGroupBoxPropertyBrowserPrivate;

class QtGroupBoxPropertyBrowser : public QtAbstractPropertyBrowser
{
   CS_OBJECT(QtGroupBoxPropertyBrowser)

 public:
   QtGroupBoxPropertyBrowser(QWidget *parent = nullptr);
   ~QtGroupBoxPropertyBrowser();

 protected:
   void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem) override;
   void itemRemoved(QtBrowserItem *item) override;
   void itemChanged(QtBrowserItem *item) override;

 private:
   QScopedPointer<QtGroupBoxPropertyBrowserPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtGroupBoxPropertyBrowser)
   Q_DISABLE_COPY(QtGroupBoxPropertyBrowser)

   CS_SLOT_1(Private, void slotUpdate())
   CS_SLOT_2(slotUpdate)

   CS_SLOT_1(Private, void slotEditorDestroyed())
   CS_SLOT_2(slotEditorDestroyed)
};

#endif
