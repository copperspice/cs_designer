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

#ifndef PALETTE_EDITOR_TOOLBUTTON_H
#define PALETTE_EDITOR_TOOLBUTTON_H

#include <abstract_formeditor.h>

#include <QPalette>
#include <QToolButton>

namespace qdesigner_internal {

class PaletteEditorButton: public QToolButton
{
   CS_OBJECT(PaletteEditorButton)

 public:
   PaletteEditorButton(QDesignerFormEditorInterface *core, const QPalette &palette, QWidget *parent = nullptr);
   virtual ~PaletteEditorButton();

   void setSuperPalette(const QPalette &palette);
   inline QPalette palette() const {
      return m_palette;
   }

   CS_SIGNAL_1(Public, void paletteChanged(const QPalette &palette))
   CS_SIGNAL_2(paletteChanged, palette)

   CS_SLOT_1(Public, void setPalette(const QPalette &palette))
   CS_SLOT_2(setPalette)

 private:
   CS_SLOT_1(Private, void showPaletteEditor())
   CS_SLOT_2(showPaletteEditor)

   QPalette m_palette;
   QPalette m_superPalette;
   QDesignerFormEditorInterface *m_core;
};

}  // namespace qdesigner_internal

#endif
