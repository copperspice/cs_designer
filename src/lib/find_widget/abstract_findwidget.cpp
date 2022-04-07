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

#include <abstract_findwidget.h>

#include <QEvent>
#include <QFile>
#include <QTimer>

#include <QCheckBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSpacerItem>
#include <QToolButton>

static QIcon createIconSet(const QString &name)
{
   static const QStringList candidates = {
      ":/resources/findwidget/images/",

#ifdef Q_OS_DARWIN
      ":/resources/findwidget/images/mac/",
#else
      ":/resources/findwidget/images/win/",
#endif

   };

   for (const QString &fname : candidates) {
      QString tmp = fname + name;

      if (QFile::exists(tmp)) {
         return QIcon(tmp);
      }
   }

   return QIcon();
}

AbstractFindWidget::AbstractFindWidget(FindFlags flags, QWidget *parent)
   : QWidget(parent)
{
   QBoxLayout *topLayOut;
   QBoxLayout *layOut;

   if (flags & NarrowLayout) {
      topLayOut = new QVBoxLayout(this);
      layOut = new QHBoxLayout;
      topLayOut->addLayout(layOut);
   } else {
      topLayOut = layOut = new QHBoxLayout(this);
   }

#ifndef Q_OS_DARWIN
   topLayOut->setSpacing(6);
   topLayOut->setMargin(0);
#endif

   m_toolClose = new QToolButton(this);
   m_toolClose->setIcon(createIconSet("closetab.png"));
   m_toolClose->setAutoRaise(true);
   layOut->addWidget(m_toolClose);

   connect(m_toolClose, &QToolButton::clicked, this, &AbstractFindWidget::deactivate);

   m_editFind = new QLineEdit(this);
   layOut->addWidget(m_editFind);

   connect(m_editFind, &QLineEdit::returnPressed, this, &AbstractFindWidget::findNext);
   connect(m_editFind, &QLineEdit::textChanged,   this, &AbstractFindWidget::findCurrentText);
   connect(m_editFind, &QLineEdit::textChanged,   this, &AbstractFindWidget::updateButtons);

   m_toolPrevious = new QToolButton(this);
   m_toolPrevious->setAutoRaise(true);
   m_toolPrevious->setText(tr("&Previous"));
   m_toolPrevious->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
   m_toolPrevious->setIcon(createIconSet("previous.png"));
   layOut->addWidget(m_toolPrevious);

   connect(m_toolPrevious, &QToolButton::clicked, this, &AbstractFindWidget::findPrevious);

   m_toolNext = new QToolButton(this);
   m_toolNext->setAutoRaise(true);
   m_toolNext->setText(tr("&Next"));
   m_toolNext->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
   m_toolNext->setIcon(createIconSet("next.png"));
   layOut->addWidget(m_toolNext);

   connect(m_toolNext, &QToolButton::clicked, this, &AbstractFindWidget::findNext);

   if (flags & NarrowLayout) {
      QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Fixed);
      m_toolPrevious->setSizePolicy(sp);
      m_toolPrevious->setMinimumWidth(m_toolPrevious->minimumSizeHint().height());
      m_toolNext->setSizePolicy(sp);
      m_toolNext->setMinimumWidth(m_toolNext->minimumSizeHint().height());

      QSpacerItem *spacerItem = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum);
      layOut->addItem(spacerItem);

      layOut = new QHBoxLayout;
      topLayOut->addLayout(layOut);

   } else {
      m_editFind->setMinimumWidth(150);
   }

   if (! (flags & NoCaseSensitive)) {
      m_checkCase = new QCheckBox(tr("&Case sensitive"), this);
      layOut->addWidget(m_checkCase);

      connect(m_checkCase, &QCheckBox::toggled, this, &AbstractFindWidget::findCurrentText);

   } else {
      m_checkCase = nullptr;
   }

   if (!(flags & NoWholeWords)) {
      m_checkWholeWords = new QCheckBox(tr("Whole &words"), this);
      layOut->addWidget(m_checkWholeWords);

      connect(m_checkWholeWords, &QCheckBox::toggled, this, &AbstractFindWidget::findCurrentText);

   } else {
      m_checkWholeWords = nullptr;
   }

   m_labelWrapped = new QLabel(this);
   m_labelWrapped->setTextFormat(Qt::RichText);
   m_labelWrapped->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);
   m_labelWrapped->setText(tr("Search wrapped"));
   m_labelWrapped->hide();

   layOut->addWidget(m_labelWrapped);

   QSpacerItem *spacerItem = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum);
   layOut->addItem(spacerItem);

   setMinimumWidth(minimumSizeHint().width());

   updateButtons();
   hide();
}

AbstractFindWidget::~AbstractFindWidget()
{
}

QIcon AbstractFindWidget::findIconSet()
{
   return createIconSet("searchfind.png");
}

void AbstractFindWidget::activate()
{
   show();
   m_editFind->selectAll();
   m_editFind->setFocus(Qt::ShortcutFocusReason);
}

/*!
    Deactivates the find widget, making it invisible and handing focus to any
    associated QTextEdit.
 */
void AbstractFindWidget::deactivate()
{
   hide();
}

void AbstractFindWidget::findNext()
{
   findInternal(m_editFind->text(), true, false);
}

void AbstractFindWidget::findPrevious()
{
   findInternal(m_editFind->text(), true, true);
}

void AbstractFindWidget::findCurrentText()
{
   findInternal(m_editFind->text(), false, false);
}

void AbstractFindWidget::keyPressEvent(QKeyEvent *event)
{
   if (event->key() == Qt::Key_Escape) {
      deactivate();
      return;
   }

   QWidget::keyPressEvent(event);
}

void AbstractFindWidget::updateButtons()
{
   const bool en = !m_editFind->text().isEmpty();
   m_toolPrevious->setEnabled(en);
   m_toolNext->setEnabled(en);
}

void AbstractFindWidget::findInternal(const QString &ttf, bool skipCurrent, bool backward)
{
   bool found = false;
   bool wrapped = false;
   find(ttf, skipCurrent, backward, &found, &wrapped);
   QPalette p;
   p.setColor(QPalette::Active, QPalette::Base, found ? Qt::white : QColor(255, 102, 102));
   m_editFind->setPalette(p);
   m_labelWrapped->setVisible(wrapped);
}

bool AbstractFindWidget::caseSensitive() const
{
   return m_checkCase && m_checkCase->isChecked();
}

bool AbstractFindWidget::wholeWords() const
{
   return m_checkWholeWords && m_checkWholeWords->isChecked();
}

bool AbstractFindWidget::eventFilter(QObject *object, QEvent *e)
{
   if (isVisible() && e->type() == QEvent::KeyPress) {
      QKeyEvent *ke = static_cast<QKeyEvent *>(e);
      if (ke->key() == Qt::Key_Escape) {
         hide();
         return true;
      }
   }

   return QWidget::eventFilter(object, e);
}


