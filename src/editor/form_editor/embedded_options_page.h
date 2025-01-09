/***********************************************************************
*
* Copyright (c) 2021-2025 Barbara Geller
* Copyright (c) 2021-2025 Ansel Sermersheim
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

#ifndef EMBEDDEDOPTIONSPAGE_H
#define EMBEDDEDOPTIONSPAGE_H

#include <abstract_formeditor.h>
#include <abstract_optionspage.h>

#include <QPointer>
#include <QWidget>

namespace qdesigner_internal {

class EmbeddedOptionsControlPrivate;

class EmbeddedOptionsControl : public QWidget
{
   CS_OBJECT(EmbeddedOptionsControl)

 public:
   explicit EmbeddedOptionsControl(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
   ~EmbeddedOptionsControl();

   bool isDirty() const;

   CS_SLOT_1(Public, void loadSettings())
   CS_SLOT_2(loadSettings)

   CS_SLOT_1(Public, void saveSettings())
   CS_SLOT_2(saveSettings)

 private:
   EmbeddedOptionsControl(const EmbeddedOptionsControl &) = delete;
   EmbeddedOptionsControl &operator=(const EmbeddedOptionsControl &) = delete;

   CS_SLOT_1(Private, void slotAdd())
   CS_SLOT_2(slotAdd)

   CS_SLOT_1(Private, void slotEdit())
   CS_SLOT_2(slotEdit)

   CS_SLOT_1(Private, void slotDelete())
   CS_SLOT_2(slotDelete)

   CS_SLOT_1(Private, void slotProfileIndexChanged(int un_named_arg1))
   CS_SLOT_2(slotProfileIndexChanged)

   EmbeddedOptionsControlPrivate *m_d;

   friend class EmbeddedOptionsControlPrivate;
};

class EmbeddedOptionsPage : public QDesignerOptionsPageInterface
{
 public:
   explicit EmbeddedOptionsPage(QDesignerFormEditorInterface *core);

   QString name() const  override;
   QWidget *createPage(QWidget *parent) override;
   void finish() override;
   void apply() override;

 private:
   EmbeddedOptionsPage(const EmbeddedOptionsPage &) = delete;
   EmbeddedOptionsPage &operator=(const EmbeddedOptionsPage &) = delete;

   QDesignerFormEditorInterface *m_core;
   QPointer<EmbeddedOptionsControl> m_embeddedOptionsControl;
};

}   // end namespace

#endif
