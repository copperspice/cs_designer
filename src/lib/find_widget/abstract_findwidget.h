/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#ifndef ABSTRACT_FINDWIDGET_H
#define ABSTRACT_FINDWIDGET_H

#include <QIcon>
#include <QWidget>

class QCheckBox;
class QEvent;
class QKeyEvent;
class QLabel;
class QLineEdit;
class QObject;
class QToolButton;

class AbstractFindWidget : public QWidget
{
   CS_OBJECT(AbstractFindWidget)

 public:
   enum FindFlag {
      /// Use a layout that is roughly half as wide and twice as high as the regular one.
      NarrowLayout = 1,
      /// Do not show the "Whole words" checkbox.
      NoWholeWords = 2,
      /// Do not show the "Case sensitive" checkbox.
      NoCaseSensitive = 4
   };
   using FindFlags = QFlags<FindFlag>;
   explicit AbstractFindWidget(FindFlags flags = FindFlags(), QWidget *parent = nullptr);
   virtual ~AbstractFindWidget();

   bool eventFilter(QObject *object, QEvent *e) override;

   static QIcon findIconSet();

   CS_SLOT_1(Public, void activate())
   CS_SLOT_2(activate)

   CS_SLOT_1(Public, virtual void deactivate())
   CS_SLOT_2(deactivate)

   CS_SLOT_1(Public, void findNext())
   CS_SLOT_2(findNext)

   CS_SLOT_1(Public, void findPrevious())
   CS_SLOT_2(findPrevious)

   CS_SLOT_1(Public, void findCurrentText())
   CS_SLOT_2(findCurrentText)

 protected:
   void keyPressEvent(QKeyEvent *event) override;

   virtual void find(const QString &textToFind, bool skipCurrent, bool backward, bool *found, bool *wrapped) = 0;

   bool caseSensitive() const;
   bool wholeWords() const;

 private:
   CS_SLOT_1(Private, void updateButtons())
   CS_SLOT_2(updateButtons)

   void findInternal(const QString &textToFind, bool skipCurrent, bool backward);

   QLineEdit *m_editFind;
   QLabel *m_labelWrapped;
   QToolButton *m_toolNext;
   QToolButton *m_toolClose;
   QToolButton *m_toolPrevious;
   QCheckBox *m_checkCase;
   QCheckBox *m_checkWholeWords;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractFindWidget::FindFlags)

#endif
