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

#ifndef STYLESHEETEDITOR_H
#define STYLESHEETEDITOR_H

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;

#include <QTextEdit>
#include <QDialog>
#include <QLabel>

class QDialogButtonBox;

namespace qdesigner_internal {

class StyleSheetEditor : public QTextEdit
{
   CS_OBJECT(StyleSheetEditor)

 public:
   StyleSheetEditor(QWidget *parent = nullptr);
};

// Edit a style sheet.
class StyleSheetEditorDialog : public QDialog
{
   CS_OBJECT(StyleSheetEditorDialog)

 public:
   enum Mode {
      ModeGlobal, // resources are disabled (we don't have current resource set loaded), used e.g. in configuration dialog context
      ModePerForm // resources are available
   };

   StyleSheetEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent, Mode mode = ModePerForm);
   ~StyleSheetEditorDialog();
   QString text() const;
   void setText(const QString &t);

 protected:
   QDialogButtonBox *buttonBox() const;
   void setOkButtonEnabled(bool v);

 private:
   CS_SLOT_1(Private, void slotContextMenuRequested(const QPoint &pos))
   CS_SLOT_2(slotContextMenuRequested)

   CS_SLOT_1(Private, void slotAddResource(const QString &property))
   CS_SLOT_2(slotAddResource)

   CS_SLOT_1(Private, void slotAddGradient(const QString &property))
   CS_SLOT_2(slotAddGradient)

   CS_SLOT_1(Private, void slotAddColor(const QString &property))
   CS_SLOT_2(slotAddColor)

   CS_SLOT_1(Private, void slotAddFont())
   CS_SLOT_2(slotAddFont)

   void insertCssProperty(const QString &name, const QString &value);

   QDialogButtonBox *m_buttonBox;
   StyleSheetEditor *m_editor;

   QDesignerFormEditorInterface *m_core;
   QAction *m_addResourceAction;
   QAction *m_addGradientAction;
   QAction *m_addColorAction;
   QAction *m_addFontAction;
};

// Edit the style sheet property of the designer selection.
// Provides an "Apply" button.

class StyleSheetPropertyEditorDialog : public StyleSheetEditorDialog
{
   CS_OBJECT(StyleSheetPropertyEditorDialog)

 public:
   StyleSheetPropertyEditorDialog(QWidget *parent, QDesignerFormWindowInterface *fw, QWidget *widget);

 private:
   CS_SLOT_1(Private, void applyStyleSheet())
   CS_SLOT_2(applyStyleSheet)

   QDesignerFormWindowInterface *m_fw;
   QWidget *m_widget;
};

} // namespace qdesigner_internal

#endif
