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

#include <designer_utils.h>
#include <designer_widgetbox.h>
#include <ui4.h>

#include <QRegularExpression>
#include <QSharedData>
#include <QXmlStreamReader>

class QDesignerWidgetBoxWidgetData : public QSharedData
{
 public:
   QDesignerWidgetBoxWidgetData(const QString &aname, const QString &xml,
      const QString &icon_name, QDesignerWidgetBoxInterface::Widget::Type atype);

   QString m_name;
   QString m_xml;
   QString m_icon_name;
   QDesignerWidgetBoxInterface::Widget::Type m_type;
};

QDesignerWidgetBoxWidgetData::QDesignerWidgetBoxWidgetData(const QString &aname, const QString &xml,
   const QString &icon_name, QDesignerWidgetBoxInterface::Widget::Type atype)
   : m_name(aname), m_xml(xml), m_icon_name(icon_name), m_type(atype)
{
}

QDesignerWidgetBoxInterface::Widget::Widget(const QString &aname, const QString &xml,
   const QString &icon_name, Type atype) :
   m_data(new QDesignerWidgetBoxWidgetData(aname, xml, icon_name, atype))
{
}

QDesignerWidgetBoxInterface::Widget::~Widget()
{
}

QDesignerWidgetBoxInterface::Widget::Widget(const Widget &w) :
   m_data(w.m_data)
{
}

QDesignerWidgetBoxInterface::Widget &QDesignerWidgetBoxInterface::Widget::operator=(const Widget &rhs)
{
   if (this != &rhs) {
      m_data = rhs.m_data;
   }
   return *this;
}

QString QDesignerWidgetBoxInterface::Widget::name() const
{
   return m_data->m_name;
}

void QDesignerWidgetBoxInterface::Widget::setName(const QString &aname)
{
   m_data->m_name = aname;
}

QString QDesignerWidgetBoxInterface::Widget::domXml() const
{
   return m_data->m_xml;
}

void QDesignerWidgetBoxInterface::Widget::setDomXml(const QString &xml)
{
   m_data->m_xml = xml;
}

QString QDesignerWidgetBoxInterface::Widget::iconName() const
{
   return m_data->m_icon_name;
}

void QDesignerWidgetBoxInterface::Widget::setIconName(const QString &icon_name)
{
   m_data->m_icon_name = icon_name;
}

QDesignerWidgetBoxInterface::Widget::Type QDesignerWidgetBoxInterface::Widget::type() const
{
   return m_data->m_type;
}

void QDesignerWidgetBoxInterface::Widget::setType(Type atype)
{
   m_data->m_type = atype;
}

bool QDesignerWidgetBoxInterface::Widget::isNull() const
{
   return m_data->m_name.isEmpty();
}

namespace qdesigner_internal {
QDesignerWidgetBox::QDesignerWidgetBox(QWidget *parent, Qt::WindowFlags flags)
   : QDesignerWidgetBoxInterface(parent, flags),
     m_loadMode(LoadMerge)
{

}

QDesignerWidgetBox::LoadMode QDesignerWidgetBox::loadMode() const
{
   return m_loadMode;
}

void QDesignerWidgetBox::setLoadMode(LoadMode lm)
{
   m_loadMode = lm;
}

// Convenience to find a widget by class name
bool QDesignerWidgetBox::findWidget(const QDesignerWidgetBoxInterface *wbox,
   const QString &className,
   const QString &category,
   Widget *widgetData)
{
   // Note that entry names do not necessarily match the class name
   // (at least, not for the standard widgets), so,
   // look in the XML for the class name of the first widget to appear
   const QString widgetTag = "<widget";

   QString pattern = "^<widget\\s+class\\s*=\\s*\"";
   pattern += className;
   pattern += "\".*$";

   QRegularExpression regex(pattern, QPatternOption::ExactMatchOption);
   Q_ASSERT(regex.isValid());

   const int catCount = wbox->categoryCount();

   for (int c = 0; c < catCount; ++c) {
      const Category cat = wbox->category(c);

      if (category.isEmpty() || cat.name() == category) {
         const int widgetCount =  cat.widgetCount();

         for (int w = 0; w < widgetCount; w++) {
            const Widget widget = cat.widget(w);
            QString xml = widget.domXml(); // Erase the <ui> tag that can be present starting from 4.4

            const int widgetTagIndex = xml.indexOf(widgetTag);

            if (widgetTagIndex != -1) {
               xml.remove(0, widgetTagIndex);

               auto match = regex.match(xml);

               if (match.hasMatch()) {
                  *widgetData = widget;

                  return true;
               }
            }
         }
      }
   }
   return false;
}

// Convenience to create a Dom Widget from widget box xml code.
DomUI *QDesignerWidgetBox::xmlToUi(const QString &name, const QString &xml, bool insertFakeTopLevel,
   QString *errorMessage)
{
   QXmlStreamReader reader(xml);
   DomUI *ui = nullptr;

   // The xml description must either contain a root element "ui" with a child element "widget"
   // or "widget" as the root element (4.3 legacy)
   const QString widgetTag = "widget";

   while (! reader.atEnd()) {
      if (reader.readNext() == QXmlStreamReader::StartElement) {
         const QString readerName = reader.name();

         if (ui) {
            reader.raiseError(tr("Unexpected element <%1>").formatArg(readerName));
            continue;
         }

         if (readerName.compare("widget", Qt::CaseInsensitive) == 0) {
            // 4.3 legacy, wrap into DomUI
            ui = new DomUI;
            DomWidget *widget = new DomWidget;
            widget->read(reader);
            ui->setElementWidget(widget);

         } else if (readerName.compare("ui", Qt::CaseInsensitive) == 0) {
            // 4.4
            ui = new DomUI;
            ui->read(reader);

         } else {
            reader.raiseError(tr("Unexpected element <%1>").formatArg(readerName));
         }
      }
   }

   if (reader.hasError()) {
      delete ui;

      *errorMessage = tr("A parse error occurred at line %1, column %2 of the XML code "
            "specified for the widget %3: %4\n%5")
            .formatArg(reader.lineNumber()).formatArg(reader.columnNumber()).formatArg(name)
            .formatArg(reader.errorString()).formatArg(xml);

      return nullptr;
   }

   if (! ui || ! ui->elementWidget()) {
      delete ui;
      *errorMessage = tr("The XML code specified for the widget %1 does not contain "
            "any widget elements.\n%2").formatArg(name).formatArg(xml);

      return nullptr;
   }

   if (insertFakeTopLevel)  {
      DomWidget *fakeTopLevel = new DomWidget;
      fakeTopLevel->setAttributeClass("QWidget");

      QList<DomWidget *> children;
      children.push_back(ui->takeElementWidget());
      fakeTopLevel->setElementWidget(children);
      ui->setElementWidget(fakeTopLevel);
   }

   return ui;
}

// Convenience to create a Dom Widget from widget box xml code.
DomUI *QDesignerWidgetBox::xmlToUi(const QString &name, const QString &xml, bool insertFakeTopLevel)
{
   QString errorMessage;
   DomUI *rc = xmlToUi(name, xml, insertFakeTopLevel, &errorMessage);
   if (!rc) {
      csWarning(errorMessage);
   }
   return rc;
}

}  // namespace qdesigner_internal


