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

#ifndef PROPERTYBROWSER_UTILS_H
#define PROPERTYBROWSER_UTILS_H

#include <QMap>
#include <QIcon>
#include <QWidget>
#include <QStringList>

class QMouseEvent;
class QCheckBox;
class QLineEdit;

class QtCursorDatabase
{
 public:
   QtCursorDatabase();
   void clear();

   QStringList cursorShapeNames() const;
   QMap<int, QIcon> cursorShapeIcons() const;
   QString cursorToShapeName(const QCursor &cursor) const;
   QIcon cursorToShapeIcon(const QCursor &cursor) const;
   int cursorToValue(const QCursor &cursor) const;

   QCursor valueToCursor(int value) const;

 private:
   void appendCursor(Qt::CursorShape shape, const QString &name, const QIcon &icon);
   QStringList m_cursorNames;
   QMap<int, QIcon> m_cursorIcons;
   QMap<int, Qt::CursorShape> m_valueToCursorShape;
   QMap<Qt::CursorShape, int> m_cursorShapeToValue;
};

class QtPropertyBrowserUtils
{
 public:
   static QPixmap brushValuePixmap(const QBrush &b);
   static QIcon brushValueIcon(const QBrush &b);
   static QString colorValueText(const QColor &c);
   static QPixmap fontValuePixmap(const QFont &f);
   static QIcon fontValueIcon(const QFont &f);
   static QString fontValueText(const QFont &f);
   static QString dateFormat();
   static QString timeFormat();
   static QString dateTimeFormat();
};

class QtBoolEdit : public QWidget
{
   CS_OBJECT(QtBoolEdit)

 public:
   QtBoolEdit(QWidget *parent = nullptr);

   bool textVisible() const {
      return m_textVisible;
   }
   void setTextVisible(bool textVisible);

   Qt::CheckState checkState() const;
   void setCheckState(Qt::CheckState state);

   bool isChecked() const;
   void setChecked(bool c);

   bool blockCheckBoxSignals(bool block);

   CS_SIGNAL_1(Public, void toggled(bool un_named_arg1))
   CS_SIGNAL_2(toggled, un_named_arg1)

 protected:
   void mousePressEvent(QMouseEvent *event) override;

 private:
   QCheckBox *m_checkBox;
   bool m_textVisible;
};

#endif
