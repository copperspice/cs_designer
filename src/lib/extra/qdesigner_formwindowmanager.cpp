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

#include <abstract_formeditor.h>

#include <qdesigner_formwindowmanager_p.h>
#include <plugindialog_p.h>

using namespace qdesigner_internal;

QDesignerFormWindowManager::QDesignerFormWindowManager(QObject *parent)
   : QDesignerFormWindowManagerInterface(parent), m_unused(0)
{
}

QDesignerFormWindowManager::~QDesignerFormWindowManager()
{
}

void QDesignerFormWindowManager::showPluginDialog()
{
   PluginDialog dlg(core(), core()->topLevel());
   dlg.exec();
}
