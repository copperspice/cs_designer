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

#ifndef CODEPREVIEWDIALOG_H
#define CODEPREVIEWDIALOG_H

#include <QDialog>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

// Dialog for viewing code.
class CodeDialog : public QDialog
{
   CS_OBJECT(CodeDialog)
   explicit CodeDialog(QWidget *parent = nullptr);

 public:
   virtual ~CodeDialog();

   static bool generateCode(const QDesignerFormWindowInterface *fw,
      QString *code,
      QString *errorMessage);

   static bool showCodeDialog(const QDesignerFormWindowInterface *fw,
      QWidget *parent,
      QString *errorMessage);

 private:
   CS_SLOT_1(Private, void slotSaveAs())
   CS_SLOT_2(slotSaveAs)

   CS_SLOT_1(Private, void copyAll())
   CS_SLOT_2(copyAll)

   void setCode(const QString &code);
   QString code() const;
   void setFormFileName(const QString &f);
   QString formFileName() const;

   void warning(const QString &msg);

   struct CodeDialogPrivate;
   CodeDialogPrivate *m_impl;
};

} // namespace qdesigner_internal

#endif
