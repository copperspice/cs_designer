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

#include <palette_editor_toolbutton.h>
#include <palette_editor.h>

#include <QDebug>

using namespace qdesigner_internal;

PaletteEditorButton::PaletteEditorButton(QDesignerFormEditorInterface *core, const QPalette &palette, QWidget *parent)
   : QToolButton(parent), m_palette(palette)
{
   m_core = core;
   setFocusPolicy(Qt::NoFocus);
   setText(tr("Change Palette"));
   setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

   connect(this, &QAbstractButton::clicked, this, &PaletteEditorButton::showPaletteEditor);
}

PaletteEditorButton::~PaletteEditorButton()
{
}

void PaletteEditorButton::setPalette(const QPalette &palette)
{
   m_palette = palette;
}

void PaletteEditorButton::setSuperPalette(const QPalette &palette)
{
   m_superPalette = palette;
}

void PaletteEditorButton::showPaletteEditor()
{
   int result;
   QPalette p = QPalette();
   QPalette pal = PaletteEditor::getPalette(m_core, nullptr, m_palette, m_superPalette, &result);

   if (result == QDialog::Accepted) {
      m_palette = pal;
      emit paletteChanged(m_palette);
   }
}

