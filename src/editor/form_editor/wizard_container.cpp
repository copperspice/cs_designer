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

#include <abstract_formeditor.h>
#include <extension.h>
#include <wizard_container.h>

#include <QWizard>
#include <QWizardPage>

typedef QList<int> IdList;
typedef QList<QWizardPage *> WizardPageList;

namespace qdesigner_internal {

QWizardContainer::QWizardContainer(QWizard *widget, QObject *parent)
   : QObject(parent), m_wizard(widget)
{
}

int QWizardContainer::count() const
{
   return m_wizard->pageIds().size();
}

QWidget *QWizardContainer::widget(int index) const
{
   QWidget *rc = nullptr;
   if (index >= 0) {
      const IdList idList = m_wizard->pageIds();
      if (index < idList.size()) {
         rc = m_wizard->page(idList.at(index));
      }
   }
   return rc;
}

int QWizardContainer::currentIndex() const
{
   const IdList idList = m_wizard->pageIds();
   const int currentId = m_wizard->currentId();
   const int rc = idList.empty() ? -1 : idList.indexOf(currentId);
   return rc;
}

void QWizardContainer::setCurrentIndex(int index)
{
   if (index < 0 || m_wizard->pageIds().empty()) {
      return;
   }

   int currentIdx = currentIndex();

   if (currentIdx == -1) {
      m_wizard->restart();
      currentIdx = currentIndex();
   }

   if (currentIdx == index) {
      return;
   }

   const int d = qAbs(index - currentIdx);
   if (index > currentIdx) {
      for (int i = 0; i < d; i++) {
         m_wizard->next();
      }

   } else {
      for (int i = 0; i < d; i++) {
         m_wizard->back();
      }
   }
}

static const QString msgWrongType = "** WARNING Attempt to add object which is not of class WizardPage to a QWizard";

void QWizardContainer::addWidget(QWidget *widget)
{
   QWizardPage *page = dynamic_cast<QWizardPage *>(widget);

   if (! page) {
      qWarning("%s", csPrintable(msgWrongType));
      return;
   }

   m_wizard->addPage(page);

   // Might be -1 after adding the first page
   setCurrentIndex(m_wizard->pageIds().size() - 1);
}

void QWizardContainer::insertWidget(int index, QWidget *widget)
{
   constexpr const int DELTA = 5;

   QWizardPage *newPage = dynamic_cast<QWizardPage *>(widget);

   if (! newPage) {
      qWarning("%s", csPrintable(msgWrongType));
      return;
   }

   const IdList pageIdList = m_wizard->pageIds();
   const int pageCount     = pageIdList.size();

   if (index >= pageCount) {
      addWidget(widget);
      return;
   }

   // Insert before, reshuffle ids if required
   int oldId = pageIdList.at(index);
   int newId = oldId - 1;

   const bool needsShuffle = (index == 0 && newId < 0) || (index > 0 && pageIdList.at(index - 1) == newId);

   if (needsShuffle) {
      // create a gap by shuffling pages
      WizardPageList pageList;
      pageList.push_back(newPage);

      for (int i = index; i < pageCount; i++) {
         pageList.push_back(m_wizard->page(pageIdList.at(i)));
         m_wizard->removePage(pageIdList.at(i));
      }

      newId = oldId + DELTA;
      auto wcend = pageList.constEnd();

      for (auto it = pageList.constBegin(); it != wcend; ++it) {
         m_wizard->setPage(newId, *it);
         newId += DELTA;
      }

   } else {
      // gap found, just insert
      m_wizard->setPage(newId, newPage);
   }

   // might be at -1 after adding the first page
   setCurrentIndex(index);
}

void QWizardContainer::remove(int index)
{
   if (index < 0) {
      return;
   }

   const IdList idList = m_wizard->pageIds();
   if (index >= idList.size()) {
      return;
   }

   m_wizard->removePage(idList.at(index));

   // goto next page, preferably
   const int newSize = idList.size() - 1;

   if (index < newSize) {
      setCurrentIndex(index);
   } else {
      if (newSize > 0) {
         setCurrentIndex(newSize - 1);
      }
   }
}

// ---------------- QWizardPagePropertySheet
const QString QWizardPagePropertySheet::pageIdProperty = "pageId";

QWizardPagePropertySheet::QWizardPagePropertySheet(QWizardPage *object, QObject *parent)
   : QDesignerPropertySheet(object, parent), m_pageIdIndex(createFakeProperty(pageIdProperty, QString()))
{
   setAttribute(m_pageIdIndex, true);
}

bool QWizardPagePropertySheet::reset(int index)
{
   if (index == m_pageIdIndex) {
      setProperty(index, QString());
      return true;
   }
   return QDesignerPropertySheet::reset(index);
}

// ---------------- QWizardPropertySheet
QWizardPropertySheet::QWizardPropertySheet(QWizard *object, QObject *parent)
   : QDesignerPropertySheet(object, parent), m_startId("startId")
{
}

bool QWizardPropertySheet::isVisible(int index) const
{
   if (propertyName(index) == m_startId) {
      return false;
   }
   return QDesignerPropertySheet::isVisible(index);
}

}   // end namespace

