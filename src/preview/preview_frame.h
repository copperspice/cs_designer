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

#ifndef PREVIEW_FRAME_H
#define PREVIEW_FRAME_H

#include <QFrame>
#include <QPointer>

class QMdiArea;
class QMdiSubWindow;

namespace qdesigner_internal {

class PreviewFrame: public QFrame
{
   CS_OBJECT(PreviewFrame)

 public:
   explicit PreviewFrame(QWidget *parent);

   void setPreviewPalette(const QPalette &palette);
   void setSubWindowActive(bool active);

 private:
   // The user can on some platforms close the mdi child by invoking the system menu.
   // Ensure a child is present.
   QMdiSubWindow *ensureMdiSubWindow();
   QMdiArea *m_mdiArea;
   QPointer<QMdiSubWindow> m_mdiSubWindow;
};

}   // end namespace qdesigner_internal

#endif
