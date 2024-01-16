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

#ifndef ABSTRACTWIDGETBOX_H
#define ABSTRACTWIDGETBOX_H

#include <QSharedDataPointer>
#include <QWidget>
#include <QIcon>

class DomUI;
class QDesignerDnDItemInterface;
class QDesignerWidgetBoxWidgetData;

class QDesignerWidgetBoxInterface : public QWidget
{
   CS_OBJECT(QDesignerWidgetBoxInterface)

 public:
   class Widget
   {
    public:
      enum Type { Default, Custom };
      Widget(const QString &aname = QString(), const QString &xml = QString(),
         const QString &icon_name = QString(), Type atype = Default);
      ~Widget();

      Widget(const Widget &w);
      Widget &operator=(const Widget &w);

      QString name() const;
      void setName(const QString &aname);
      QString domXml() const;
      void setDomXml(const QString &xml);
      QString iconName() const;
      void setIconName(const QString &icon_name);
      Type type() const;
      void setType(Type atype);

      bool isNull() const;

    private:
      QSharedDataPointer<QDesignerWidgetBoxWidgetData> m_data;
   };

   typedef QList<Widget> WidgetList;

   class Category
   {
    public:
      enum Type { Default, Scratchpad };

      Category(const QString &aname = QString(), Type atype = Default)
         : m_name(aname), m_type(atype) {}

      QString name() const {
         return m_name;
      }
      void setName(const QString &aname) {
         m_name = aname;
      }
      int widgetCount() const {
         return m_widget_list.size();
      }
      Widget widget(int idx) const {
         return m_widget_list.at(idx);
      }
      void removeWidget(int idx) {
         m_widget_list.removeAt(idx);
      }
      void addWidget(const Widget &awidget) {
         m_widget_list.append(awidget);
      }
      Type type() const {
         return m_type;
      }
      void setType(Type atype) {
         m_type = atype;
      }

      bool isNull() const {
         return m_name.isEmpty();
      }

    private:
      QString m_name;
      Type m_type;
      QList<Widget> m_widget_list;
   };
   typedef QList<Category> CategoryList;

   explicit QDesignerWidgetBoxInterface(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
   virtual ~QDesignerWidgetBoxInterface();

   virtual int categoryCount() const = 0;
   virtual Category category(int cat_idx) const = 0;
   virtual void addCategory(const Category &cat) = 0;
   virtual void removeCategory(int cat_idx) = 0;

   virtual int widgetCount(int cat_idx) const = 0;
   virtual Widget widget(int cat_idx, int wgt_idx) const = 0;
   virtual void addWidget(int cat_idx, const Widget &wgt) = 0;
   virtual void removeWidget(int cat_idx, int wgt_idx) = 0;

   int findOrInsertCategory(const QString &categoryName);

   virtual void dropWidgets(const QList<QDesignerDnDItemInterface *> &item_list,
      const QPoint &global_mouse_pos) = 0;

   virtual void setFileName(const QString &file_name) = 0;
   virtual QString fileName() const = 0;
   virtual bool load() = 0;
   virtual bool save() = 0;
};

CS_DECLARE_METATYPE(QDesignerWidgetBoxInterface::Widget)

#endif
