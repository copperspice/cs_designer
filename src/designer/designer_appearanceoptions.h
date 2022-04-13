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

#ifndef QDESIGNER_APPEARANCEOPTIONS_H
#define QDESIGNER_APPEARANCEOPTIONS_H

#include <abstract_optionspage.h>
#include <designer_enums.h>
#include <designer_toolwindow.h>

#include <QObject>
#include <QPointer>
#include <QWidget>

class QDesignerFormEditorInterface;
class QDesignerSettings;

namespace Ui {
class AppearanceOptionsWidget;
}

/* AppearanceOptions data */
struct AppearanceOptions {
   AppearanceOptions();
   bool equals(const AppearanceOptions &) const;
   void toSettings(QDesignerSettings &) const;
   void fromSettings(const QDesignerSettings &);

   UIMode uiMode;
   ToolWindowFontSettings toolWindowFontSettings;
};

inline bool operator==(const AppearanceOptions &ao1, const AppearanceOptions &ao2)
{
   return ao1.equals(ao2);
}

inline bool operator!=(const AppearanceOptions &ao1, const AppearanceOptions &ao2)
{
   return !ao1.equals(ao2);
}

/* QDesignerAppearanceOptionsWidget: Let the user edit AppearanceOptions */
class QDesignerAppearanceOptionsWidget : public QWidget
{
   CS_OBJECT(QDesignerAppearanceOptionsWidget)

 public:
   explicit QDesignerAppearanceOptionsWidget(QWidget *parent = nullptr);
   ~QDesignerAppearanceOptionsWidget();

   AppearanceOptions appearanceOptions() const;
   void setAppearanceOptions(const AppearanceOptions &ao);

   CS_SIGNAL_1(Public, void uiModeChanged(bool modified))
   CS_SIGNAL_2(uiModeChanged, modified)

 private:
   CS_SLOT_1(Private, void slotUiModeComboChanged())
   CS_SLOT_2(slotUiModeComboChanged)

   UIMode uiMode() const;

   Ui::AppearanceOptionsWidget *m_ui;
   UIMode m_initialUIMode;
};

/* The options page for appearance options. */

class QDesignerAppearanceOptionsPage : public QObject, public QDesignerOptionsPageInterface
{
   CS_OBJECT(QDesignerAppearanceOptionsPage)

 public:
   QDesignerAppearanceOptionsPage(QDesignerFormEditorInterface *core);

   QString name() const override;
   QWidget *createPage(QWidget *parent) override;

   void apply() override;
   void finish() override;

   CS_SIGNAL_1(Public, void settingsChanged())
   CS_SIGNAL_2(settingsChanged)

 private:
   QDesignerFormEditorInterface *m_core;
   QPointer<QDesignerAppearanceOptionsWidget> m_widget;
   AppearanceOptions m_initialOptions;
};

#endif
