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

#ifndef STYLESHEET_EDITOR_H
#define STYLESHEET_EDITOR_H

#include <ui_edit_stylesheet.h>

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;

#include <QDialog>

class StyleSheetEditorDialog : public QDialog
{
   CS_OBJECT(StyleSheetEditorDialog)

 public:
   enum Mode {
      ModeGlobal,    // resources are disabled
      ModePerForm    // resources are available
   };

   StyleSheetEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent,
         QWidget *applyWidget = nullptr, Mode mode = ModePerForm);

   ~StyleSheetEditorDialog();

   QString text() const;
   void setText(const QString &text);

 private:
   // slots
   void slotAddResource(const QString &property);
   void slotAddGradient(const QString &property);
   void slotAddColor(const QString &property);
   void slotAddFont();

   void showResourceMenu();
   void showGradientMenu();
   void showColorMenu();
   void editorContextMenu(const QPoint &pos);
   void applyStyleSheet();

   void insertCssProperty(const QString &name, const QString &value);

   Ui::StyleSheetEditor *m_ui;

   QDesignerFormEditorInterface *m_core;
   QWidget *m_applyWidget;

   QMenu *m_resourceMenu;
   QMenu *m_gradientMenu;
   QMenu *m_colorMenu;
};

#endif
