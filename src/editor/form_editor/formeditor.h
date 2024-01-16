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

#ifndef FORMEDITOR_H
#define FORMEDITOR_H

#include <abstract_formeditor.h>

class QObject;

namespace qdesigner_internal {

class FormEditor: public QDesignerFormEditorInterface
{
   CS_OBJECT(FormEditor)

 public:
   FormEditor(QObject *parent = nullptr);
   virtual ~FormEditor();

   CS_SLOT_1(Public, void slotQrcFileChangedExternally(const QString &path))
   CS_SLOT_2(slotQrcFileChangedExternally)
};

}  // namespace qdesigner_internal

#endif
