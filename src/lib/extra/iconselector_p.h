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

#ifndef ICONSELECTOR_H
#define ICONSELECTOR_H

#include <designer_utils.h>

class QtResourceModel;
class QDesignerFormEditorInterface;
class QDesignerDialogGuiInterface;
class QDesignerResourceBrowserInterface;

#include <QWidget>
#include <QDialog>
#include <QScopedPointer>

namespace qdesigner_internal {

class DesignerIconCache;
class DesignerPixmapCache;
class PropertySheetIconValue;

struct IconThemeEditorPrivate;

// Resource Dialog that embeds the language-dependent resource widget as returned by the language extension
class LanguageResourceDialog : public QDialog
{
   CS_OBJECT(LanguageResourceDialog)

   explicit LanguageResourceDialog(QDesignerResourceBrowserInterface *rb, QWidget *parent = nullptr);

 public:
   virtual ~LanguageResourceDialog();

   // Factory: Returns 0 if the language extension does not provide a resource browser.
   static LanguageResourceDialog *create(QDesignerFormEditorInterface *core, QWidget *parent);

   void setCurrentPath(const QString &filePath);
   QString currentPath() const;

 private:
   QScopedPointer<class LanguageResourceDialogPrivate> d_ptr;
   Q_DECLARE_PRIVATE(LanguageResourceDialog)
   Q_DISABLE_COPY(LanguageResourceDialog)

   // slot methods
   void slotAccepted();
   void slotPathChanged(const QString &path);
};

class IconSelector : public QWidget
{
   CS_OBJECT(IconSelector)

 public:
   IconSelector(QWidget *parent = nullptr);
   virtual ~IconSelector();

   void setFormEditor(QDesignerFormEditorInterface *core); // required for dialog gui.
   void setIconCache(DesignerIconCache *iconCache);
   void setPixmapCache(DesignerPixmapCache *pixmapCache);

   void setIcon(const PropertySheetIconValue &icon);
   PropertySheetIconValue icon() const;

   // Check whether a pixmap may be read
   enum CheckMode { CheckFast, CheckFully };
   static bool checkPixmap(const QString &fileName, CheckMode cm = CheckFully, QString *errorMessage = nullptr);

   // Choose a pixmap from file
   static QString choosePixmapFile(const QString &directory, QDesignerDialogGuiInterface *dlgGui, QWidget *parent);

   // Choose a pixmap from resource; use language-dependent resource browser if present
   static QString choosePixmapResource(QDesignerFormEditorInterface *core, QtResourceModel *resourceModel, const QString &oldPath,
      QWidget *parent);

   CS_SIGNAL_1(Public, void iconChanged(const PropertySheetIconValue &icon))
   CS_SIGNAL_2(iconChanged, icon)

 private:
   QScopedPointer<class IconSelectorPrivate> d_ptr;

   Q_DECLARE_PRIVATE(IconSelector)
   Q_DISABLE_COPY(IconSelector)

   CS_SLOT_1(Private, void slotStateActivated())
   CS_SLOT_2(slotStateActivated)

   CS_SLOT_1(Private, void slotSetActivated())
   CS_SLOT_2(slotSetActivated)

   CS_SLOT_1(Private, void slotSetResourceActivated())
   CS_SLOT_2(slotSetResourceActivated)

   CS_SLOT_1(Private, void slotSetFileActivated())
   CS_SLOT_2(slotSetFileActivated)

   CS_SLOT_1(Private, void slotResetActivated())
   CS_SLOT_2(slotResetActivated)

   CS_SLOT_1(Private, void slotResetAllActivated())
   CS_SLOT_2(slotResetAllActivated)

   CS_SLOT_1(Private, void slotUpdate())
   CS_SLOT_2(slotUpdate)
};

// IconThemeEditor: Let's the user input theme icon names and shows a preview label.
class IconThemeEditor : public QWidget
{
   CS_OBJECT(IconThemeEditor)

   CS_PROPERTY_READ(theme, theme)
   CS_PROPERTY_WRITE(theme, setTheme)
   CS_PROPERTY_DESIGNABLE(theme, true)

 public:
   explicit IconThemeEditor(QWidget *parent = nullptr, bool wantResetButton = true);
   virtual ~IconThemeEditor();

   QString theme() const;
   void setTheme(const QString &theme);

   CS_SIGNAL_1(Public, void edited(const QString &un_named_arg1))
   CS_SIGNAL_2(edited, un_named_arg1)

   CS_SLOT_1(Public, void reset())
   CS_SLOT_2(reset)

 private:
   CS_SLOT_1(Private, void slotChanged(const QString &un_named_arg1))
   CS_SLOT_2(slotChanged)

   void updatePreview(const QString &);

   QScopedPointer<IconThemeEditorPrivate> d;
};

} // namespace qdesigner_internal

#endif

