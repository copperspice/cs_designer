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

#ifndef FORMWINDOWSETTINGS_H
#define FORMWINDOWSETTINGS_H

#include <QDialog>

namespace Ui {
class FormWindowSettings;
}

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

struct FormWindowData;
class FormWindowBase;

/* Dialog to edit the settings of a QDesignerFormWindowInterface.
 * It sets the dirty flag on the form window if something was changed. */

class FormWindowSettings: public QDialog
{
   CS_OBJECT(FormWindowSettings)

 public:
   explicit FormWindowSettings(QDesignerFormWindowInterface *formWindow);
   virtual ~FormWindowSettings();

   void accept() override;

 private:
   FormWindowSettings(const FormWindowSettings &) = delete;
   FormWindowSettings &operator=(const FormWindowSettings &) = delete;

   FormWindowData data() const;
   void setData(const FormWindowData &);

   Ui::FormWindowSettings *m_ui;
   FormWindowBase *m_formWindow;
   FormWindowData *m_oldData;
};

}

#endif // FORMWINDOWSETTINGS_H
