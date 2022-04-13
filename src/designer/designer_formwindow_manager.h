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

#ifndef QDESIGNER_FORMWINDOMANAGER_H
#define QDESIGNER_FORMWINDOMANAGER_H

#include <abstract_formwindowmanager.h>

#include <formwindowbase_p.h>

namespace qdesigner_internal {

class PreviewManager;

//
// Convenience methods to manage form previews (ultimately forwarded to PreviewManager).
//
class QDesignerFormWindowManager : public QDesignerFormWindowManagerInterface
{
   CS_OBJECT(QDesignerFormWindowManager)

 public:
   explicit QDesignerFormWindowManager(QObject *parent = nullptr);
   virtual ~QDesignerFormWindowManager();

   virtual PreviewManager *previewManager() const = 0;

   void showPluginDialog() override;

 private:
   void *m_unused;
};

} // namespace qdesigner_internal

#endif
