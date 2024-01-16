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

#include <iconloader_p.h>

#include <QFile>
#include <QIcon>
#include <QPixmap>

namespace qdesigner_internal {

QIcon createIconSet(const QString &name)
{
   static const QStringList candidates = {
      ":/resources/form_editor/images/",

#ifdef Q_OS_DARWIN
      ":/resources/form_editor/images/mac/",
#else
      ":/resources/form_editor/images/win/",
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

QIcon emptyIcon()
{
   return QIcon(":/resources/form_editor/images/emptyicon.png");
}

static QIcon buildIcon(const QString &prefix, const int *sizes, size_t sizeCount)
{
   QIcon result;

   for (size_t i = 0; i < sizeCount; ++i) {
      const QString size = QString::number(sizes[i]);
      const QPixmap pixmap(prefix + size + ".png");

      Q_ASSERT(! pixmap.size().isEmpty());
      result.addPixmap(pixmap);
   }

   return result;
}

QIcon csLogoIcon()
{
   static const int sizes[] = {32, 64};
   static const QIcon result = buildIcon(":/resources/form_editor/images/cslogo-", sizes, sizeof(sizes) / sizeof(sizes[0]));

   return result;
}

}   // end namespace - qdesigner_internal



