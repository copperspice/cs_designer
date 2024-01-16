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

#ifndef FORMEDITOR_OPTIONSPAGE_H
#define FORMEDITOR_OPTIONSPAGE_H

#include <abstract_optionspage.h>
#include <QPointer>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class PreviewConfigurationWidget;
class GridPanel;
class ZoomSettingsWidget;

class FormEditorOptionsPage : public QDesignerOptionsPageInterface
{
 public:
   explicit FormEditorOptionsPage(QDesignerFormEditorInterface *core);

   QString name() const;
   QWidget *createPage(QWidget *parent);
   virtual void apply();
   virtual void finish();

 private:
   QDesignerFormEditorInterface *m_core;
   QPointer<PreviewConfigurationWidget> m_previewConf;
   QPointer<GridPanel> m_defaultGridConf;
   QPointer<ZoomSettingsWidget> m_zoomSettingsWidget;
};

}   // end namespace - qdesigner_internal

#endif
