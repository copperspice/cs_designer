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

#include <property_view_utils.h>

#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLocale>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

QtCursorDatabase::QtCursorDatabase()
{
   appendCursor(Qt::ArrowCursor, QCoreApplication::translate("QtCursorDatabase", "Arrow"),
      QIcon(":/resources/property_browser/images/cursor-arrow.png"));

   appendCursor(Qt::UpArrowCursor, QCoreApplication::translate("QtCursorDatabase", "Up Arrow"),
      QIcon(":/resources/property_browser/images/cursor-uparrow.png"));

   appendCursor(Qt::CrossCursor, QCoreApplication::translate("QtCursorDatabase", "Cross"),
      QIcon(":/resources/property_browser/images/cursor-cross.png"));

   appendCursor(Qt::WaitCursor, QCoreApplication::translate("QtCursorDatabase", "Wait"),
      QIcon(":/resources/property_browser/images/cursor-wait.png"));

   appendCursor(Qt::IBeamCursor, QCoreApplication::translate("QtCursorDatabase", "IBeam"),
      QIcon(":/resources/property_browser/images/cursor-ibeam.png"));

   appendCursor(Qt::SizeVerCursor, QCoreApplication::translate("QtCursorDatabase", "Size Vertical"),
      QIcon(":/resources/property_browser/images/cursor-sizev.png"));

   appendCursor(Qt::SizeHorCursor, QCoreApplication::translate("QtCursorDatabase", "Size Horizontal"),
      QIcon(":/resources/property_browser/images/cursor-sizeh.png"));

   appendCursor(Qt::SizeFDiagCursor, QCoreApplication::translate("QtCursorDatabase", "Size Backslash"),
      QIcon(":/resources/property_browser/images/cursor-sizef.png"));

   appendCursor(Qt::SizeBDiagCursor, QCoreApplication::translate("QtCursorDatabase", "Size Slash"),
      QIcon(":/resources/property_browser/images/cursor-sizeb.png"));

   appendCursor(Qt::SizeAllCursor, QCoreApplication::translate("QtCursorDatabase", "Size All"),
      QIcon(":/resources/property_browser/images/cursor-sizeall.png"));

   appendCursor(Qt::BlankCursor, QCoreApplication::translate("QtCursorDatabase", "Blank"),
      QIcon());

   appendCursor(Qt::SplitVCursor, QCoreApplication::translate("QtCursorDatabase", "Split Vertical"),
      QIcon(":/resources/property_browser/images/cursor-vsplit.png"));

   appendCursor(Qt::SplitHCursor, QCoreApplication::translate("QtCursorDatabase", "Split Horizontal"),
      QIcon(":/resources/property_browser/images/cursor-hsplit.png"));

   appendCursor(Qt::PointingHandCursor, QCoreApplication::translate("QtCursorDatabase", "Pointing Hand"),
      QIcon(":/resources/property_browser/images/cursor-hand.png"));

   appendCursor(Qt::ForbiddenCursor, QCoreApplication::translate("QtCursorDatabase", "Forbidden"),
      QIcon(":/resources/property_browser/images/cursor-forbidden.png"));

   appendCursor(Qt::OpenHandCursor, QCoreApplication::translate("QtCursorDatabase", "Open Hand"),
      QIcon(":/resources/property_browser/images/cursor-openhand.png"));

   appendCursor(Qt::ClosedHandCursor, QCoreApplication::translate("QtCursorDatabase", "Closed Hand"),
      QIcon(":/resources/property_browser/images/cursor-closedhand.png"));

   appendCursor(Qt::WhatsThisCursor, QCoreApplication::translate("QtCursorDatabase", "What's This"),
      QIcon(":/resources/property_browser/images/cursor-whatsthis.png"));

   appendCursor(Qt::BusyCursor, QCoreApplication::translate("QtCursorDatabase", "Busy"),
      QIcon(":/resources/property_browser/images/cursor-busy.png"));
}

void QtCursorDatabase::clear()
{
   m_cursorNames.clear();
   m_cursorIcons.clear();
   m_valueToCursorShape.clear();
   m_cursorShapeToValue.clear();
}

void QtCursorDatabase::appendCursor(Qt::CursorShape shape, const QString &name, const QIcon &icon)
{
   if (m_cursorShapeToValue.contains(shape)) {
      return;
   }
   const int value = m_cursorNames.count();
   m_cursorNames.append(name);
   m_cursorIcons.insert(value, icon);
   m_valueToCursorShape.insert(value, shape);
   m_cursorShapeToValue.insert(shape, value);
}

QStringList QtCursorDatabase::cursorShapeNames() const
{
   return m_cursorNames;
}

QMap<int, QIcon> QtCursorDatabase::cursorShapeIcons() const
{
   return m_cursorIcons;
}

QString QtCursorDatabase::cursorToShapeName(const QCursor &cursor) const
{
   int val = cursorToValue(cursor);
   if (val >= 0) {
      return m_cursorNames.at(val);
   }
   return QString();
}

QIcon QtCursorDatabase::cursorToShapeIcon(const QCursor &cursor) const
{
   int val = cursorToValue(cursor);
   return m_cursorIcons.value(val);
}

int QtCursorDatabase::cursorToValue(const QCursor &cursor) const
{

   Qt::CursorShape shape = cursor.shape();
   if (m_cursorShapeToValue.contains(shape)) {
      return m_cursorShapeToValue[shape];
   }

   return -1;
}

QCursor QtCursorDatabase::valueToCursor(int value) const
{
   if (m_valueToCursorShape.contains(value)) {
      return QCursor(m_valueToCursorShape[value]);
   }
   return QCursor();
}


QPixmap QtPropertyBrowserUtils::brushValuePixmap(const QBrush &b)
{
   QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
   img.fill(0);

   QPainter painter(&img);
   painter.setCompositionMode(QPainter::CompositionMode_Source);
   painter.fillRect(0, 0, img.width(), img.height(), b);
   QColor color = b.color();
   if (color.alpha() != 255) { // indicate alpha by an inset
      QBrush  opaqueBrush = b;
      color.setAlpha(255);
      opaqueBrush.setColor(color);
      painter.fillRect(img.width() / 4, img.height() / 4,
         img.width() / 2, img.height() / 2, opaqueBrush);
   }
   painter.end();
   return QPixmap::fromImage(img);
}

QIcon QtPropertyBrowserUtils::brushValueIcon(const QBrush &b)
{
   return QIcon(brushValuePixmap(b));
}

QString QtPropertyBrowserUtils::colorValueText(const QColor &c)
{
   return QCoreApplication::translate("QtPropertyBrowserUtils", "[%1, %2, %3] (%4)")
      .formatArg(c.red()).formatArg(c.green()).formatArg(c.blue()).formatArg(c.alpha());
}

QPixmap QtPropertyBrowserUtils::fontValuePixmap(const QFont &font)
{
   QFont f = font;
   QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
   img.fill(0);
   QPainter p(&img);
   p.setRenderHint(QPainter::TextAntialiasing, true);
   p.setRenderHint(QPainter::Antialiasing, true);
   f.setPointSize(13);
   p.setFont(f);
   QTextOption t;
   t.setAlignment(Qt::AlignCenter);
   p.drawText(QRect(0, 0, 16, 16), QString(QLatin1Char('A')), t);
   return QPixmap::fromImage(img);
}

QIcon QtPropertyBrowserUtils::fontValueIcon(const QFont &f)
{
   return QIcon(fontValuePixmap(f));
}

QString QtPropertyBrowserUtils::fontValueText(const QFont &f)
{
   return QCoreApplication::translate("QtPropertyBrowserUtils", "[%1, %2]")
      .formatArg(f.family()).formatArg(f.pointSize());
}

QString QtPropertyBrowserUtils::dateFormat()
{
   QLocale loc;
   return loc.dateFormat(QLocale::ShortFormat);
}

QString QtPropertyBrowserUtils::timeFormat()
{
   QLocale loc;
   // ShortFormat is missing seconds on UNIX.
   return loc.timeFormat(QLocale::LongFormat);
}

QString QtPropertyBrowserUtils::dateTimeFormat()
{
   QString format = dateFormat();
   format += QLatin1Char(' ');
   format += timeFormat();
   return format;
}

QtBoolEdit::QtBoolEdit(QWidget *parent)
   : QWidget(parent), m_checkBox(new QCheckBox(this)), m_textVisible(true)
{
   QHBoxLayout *lt = new QHBoxLayout;

   if (QApplication::layoutDirection() == Qt::LeftToRight) {
      lt->setContentsMargins(4, 0, 0, 0);
   } else {
      lt->setContentsMargins(0, 0, 4, 0);
   }

   lt->addWidget(m_checkBox);
   setLayout(lt);

   connect(m_checkBox, &QCheckBox::toggled, this, &QtBoolEdit::toggled);

   setFocusProxy(m_checkBox);
   m_checkBox->setText(tr("True"));
}

void QtBoolEdit::setTextVisible(bool textVisible)
{
   if (m_textVisible == textVisible) {
      return;
   }

   m_textVisible = textVisible;
   if (m_textVisible) {
      m_checkBox->setText(isChecked() ? tr("True") : tr("False"));
   } else {
      m_checkBox->setText(QString());
   }
}

Qt::CheckState QtBoolEdit::checkState() const
{
   return m_checkBox->checkState();
}

void QtBoolEdit::setCheckState(Qt::CheckState state)
{
   m_checkBox->setCheckState(state);
}

bool QtBoolEdit::isChecked() const
{
   return m_checkBox->isChecked();
}

void QtBoolEdit::setChecked(bool c)
{
   m_checkBox->setChecked(c);
   if (!m_textVisible) {
      return;
   }
   m_checkBox->setText(isChecked() ? tr("True") : tr("False"));
}

bool QtBoolEdit::blockCheckBoxSignals(bool block)
{
   return m_checkBox->blockSignals(block);
}

void QtBoolEdit::mousePressEvent(QMouseEvent *event)
{
   if (event->buttons() == Qt::LeftButton) {
      m_checkBox->click();
      event->accept();
   } else {
      QWidget::mousePressEvent(event);
   }
}


