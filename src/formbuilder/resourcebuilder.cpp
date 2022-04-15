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

#include <resourcebuilder.h>
#include <ui4.h>

#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QPixmap>
#include <QIcon>
#include <QVariant>

constexpr const int DEBUG_THEME = 0;

QResourceBuilder::QResourceBuilder()
{

}

QResourceBuilder::~QResourceBuilder()
{

}

int QResourceBuilder::iconStateFlags(const DomResourceIcon *dpi)
{
   int rc = 0;
   if (dpi->hasElementNormalOff()) {
      rc |= NormalOff;
   }
   if (dpi->hasElementNormalOn()) {
      rc |= NormalOn;
   }
   if (dpi->hasElementDisabledOff()) {
      rc |= DisabledOff;
   }
   if (dpi->hasElementDisabledOn()) {
      rc |= DisabledOn;
   }
   if (dpi->hasElementActiveOff()) {
      rc |= ActiveOff;
   }
   if (dpi->hasElementActiveOn()) {
      rc |= ActiveOn;
   }
   if (dpi->hasElementSelectedOff()) {
      rc |= SelectedOff;
   }
   if (dpi->hasElementSelectedOn()) {
      rc |= SelectedOn;
   }

   return rc;
}

QVariant QResourceBuilder::loadResource(const QDir &workingDirectory, const DomProperty *property) const
{
   switch (property->kind()) {
      case DomProperty::Pixmap: {
         const DomResourcePixmap *dpx = property->elementPixmap();
         QPixmap pixmap(QFileInfo(workingDirectory, dpx->text()).absoluteFilePath());
         return QVariant::fromValue(pixmap);
      }

      case DomProperty::IconSet: {
         const DomResourceIcon *dpi = property->elementIconSet();

         if (!dpi->attributeTheme().isEmpty()) {
            const QString theme = dpi->attributeTheme();
            const bool known = QIcon::hasThemeIcon(theme);

            if (DEBUG_THEME) {
               qDebug("Theme %s known %d", csPrintable(theme), known);
            }

            if (known) {
               return QVariant::fromValue(QIcon::fromTheme(dpi->attributeTheme()));
            }

         } // non-empty theme
         if (const int flags = iconStateFlags(dpi)) { // new, post 4.4 format
            QIcon icon;
            if (flags & NormalOff) {
               icon.addFile(QFileInfo(workingDirectory, dpi->elementNormalOff()->text()).absoluteFilePath(), QSize(), QIcon::Normal, QIcon::Off);
            }
            if (flags & NormalOn) {
               icon.addFile(QFileInfo(workingDirectory, dpi->elementNormalOn()->text()).absoluteFilePath(), QSize(), QIcon::Normal, QIcon::On);
            }
            if (flags & DisabledOff) {
               icon.addFile(QFileInfo(workingDirectory, dpi->elementDisabledOff()->text()).absoluteFilePath(), QSize(), QIcon::Disabled, QIcon::Off);
            }
            if (flags & DisabledOn) {
               icon.addFile(QFileInfo(workingDirectory, dpi->elementDisabledOn()->text()).absoluteFilePath(), QSize(), QIcon::Disabled, QIcon::On);
            }
            if (flags & ActiveOff) {
               icon.addFile(QFileInfo(workingDirectory, dpi->elementActiveOff()->text()).absoluteFilePath(), QSize(), QIcon::Active, QIcon::Off);
            }
            if (flags & ActiveOn) {
               icon.addFile(QFileInfo(workingDirectory, dpi->elementActiveOn()->text()).absoluteFilePath(), QSize(), QIcon::Active, QIcon::On);
            }
            if (flags & SelectedOff) {
               icon.addFile(QFileInfo(workingDirectory, dpi->elementSelectedOff()->text()).absoluteFilePath(), QSize(), QIcon::Selected, QIcon::Off);
            }
            if (flags & SelectedOn) {
               icon.addFile(QFileInfo(workingDirectory, dpi->elementSelectedOn()->text()).absoluteFilePath(), QSize(), QIcon::Selected, QIcon::On);
            }
            return QVariant::fromValue(icon);
         } else { // 4.3 legacy
            const QIcon icon(QFileInfo(workingDirectory, dpi->text()).absoluteFilePath());
            return QVariant::fromValue(icon);
         }
      }
      break;
      default:
         break;
   }
   return QVariant();
}

QVariant QResourceBuilder::toNativeValue(const QVariant &value) const
{
   return value;
}

DomProperty *QResourceBuilder::saveResource(const QDir &workingDirectory, const QVariant &value) const
{
   Q_UNUSED(workingDirectory)
   Q_UNUSED(value)
   return nullptr;
}

bool QResourceBuilder::isResourceProperty(const DomProperty *p) const
{
   switch (p->kind()) {
      case DomProperty::Pixmap:
      case DomProperty::IconSet:
         return true;
      default:
         break;
   }
   return false;
}

bool QResourceBuilder::isResourceType(const QVariant &value) const
{
   switch (value.type()) {
      case QVariant::Pixmap:
      case QVariant::Icon:
         return true;
      default:
         break;
   }
   return false;
}

