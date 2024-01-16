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

#ifndef ABSTRACT_NEWFORMWIDGET_H
#define ABSTRACT_NEWFORMWIDGET_H

#include <QWidget>

class QDesignerFormEditorInterface;

class QDesignerNewFormWidgetInterface : public QWidget
{
   CS_OBJECT(QDesignerNewFormWidgetInterface)

 public:
   explicit QDesignerNewFormWidgetInterface(QWidget *parent = nullptr);
   virtual ~QDesignerNewFormWidgetInterface();

   virtual bool hasCurrentTemplate() const = 0;
   virtual QString currentTemplate(QString *errorMessage = nullptr) = 0;

   static QDesignerNewFormWidgetInterface *createNewFormWidget(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);

   CS_SIGNAL_1(Public, void templateActivated())
   CS_SIGNAL_2(templateActivated)

   CS_SIGNAL_1(Public, void currentTemplateChanged(bool templateSelected))
   CS_SIGNAL_2(currentTemplateChanged, templateSelected)

 private:
   Q_DISABLE_COPY(QDesignerNewFormWidgetInterface)
};


#endif // ABSTRACTNEWFORMWIDGET_H
