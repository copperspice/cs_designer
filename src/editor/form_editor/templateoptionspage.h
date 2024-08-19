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

#ifndef QDESIGNER_TEMPLATEOPTIONS_H
#define QDESIGNER_TEMPLATEOPTIONS_H

#include <abstract_optionspage.h>

#include <QPointer>
#include <QStringList>
#include <QWidget>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

namespace Ui {
class TemplateOptionsWidget;
}

/* Present the user with a list of form template paths to save form templates. */
class TemplateOptionsWidget : public QWidget
{
   CS_OBJECT(TemplateOptionsWidget)

 public:
   explicit TemplateOptionsWidget(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
   ~TemplateOptionsWidget();

   QStringList templatePaths() const;
   void setTemplatePaths(const QStringList &l);

   static QString chooseTemplatePath(QDesignerFormEditorInterface *core, QWidget *parent);

 private:
   Q_DISABLE_COPY(TemplateOptionsWidget)

   CS_SLOT_1(Private, void addTemplatePath())
   CS_SLOT_2(addTemplatePath)

   CS_SLOT_1(Private, void removeTemplatePath())
   CS_SLOT_2(removeTemplatePath)

   CS_SLOT_1(Private, void templatePathSelectionChanged())
   CS_SLOT_2(templatePathSelectionChanged)

   QDesignerFormEditorInterface *m_core;
   Ui::TemplateOptionsWidget *m_ui;
};

class TemplateOptionsPage : public QDesignerOptionsPageInterface
{
 public:
   explicit TemplateOptionsPage(QDesignerFormEditorInterface *core);

   QString name() const override;
   QWidget *createPage(QWidget *parent) override;
   void apply() override;
   void finish() override;

 private:
   Q_DISABLE_COPY(TemplateOptionsPage)

   QDesignerFormEditorInterface *m_core;
   QStringList m_initialTemplatePaths;
   QPointer<TemplateOptionsWidget> m_widget;
};

}

#endif // QDESIGNER_TEMPLATEOPTIONS_H
