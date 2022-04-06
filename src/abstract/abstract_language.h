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

#ifndef QDESIGNER_ABTRACT_LANGUAGE_H
#define QDESIGNER_ABTRACT_LANGUAGE_H

#include <extension.h>

class QDialog;
class QWidget;
class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;
class QDesignerResourceBrowserInterface;

class QDesignerLanguageExtension
{
 public:
   virtual ~QDesignerLanguageExtension() {}

   virtual QString name() const = 0;

   virtual QDialog *createFormWindowSettingsDialog(QDesignerFormWindowInterface *formWindow, QWidget *parentWidget) = 0;
   virtual QDesignerResourceBrowserInterface *createResourceBrowser(QWidget *parentWidget) = 0;

   virtual QDialog *createPromotionDialog(QDesignerFormEditorInterface *formEditor, QWidget *parentWidget = nullptr) = 0;

   virtual QDialog *createPromotionDialog(QDesignerFormEditorInterface *formEditor,
      const QString &promotableWidgetClassName,
      QString *promoteToClassName,
      QWidget *parentWidget = nullptr) = 0;

   virtual bool isLanguageResource(const QString &path) const = 0;

   virtual QString classNameOf(QObject *object) const = 0;

   virtual bool signalMatchesSlot(const QString &signal, const QString &slot) const = 0;

   virtual QString widgetBoxContents() const = 0;

   virtual QString uiExtension() const = 0;
};

CS_DECLARE_EXTENSION_INTERFACE(QDesignerLanguageExtension, "copperspice.com.CS.Designer.Language.3")

#endif
