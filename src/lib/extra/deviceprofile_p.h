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

#ifndef DEVICEPROFILE_H
#define DEVICEPROFILE_H

#include <QSharedDataPointer>
#include <QString>
#include <QWidget>

class QDesignerFormEditorInterface;

class QStyle;

namespace qdesigner_internal {

class DeviceProfileData;

/* DeviceProfile for embedded design. They influence
 * default properties (for example, fonts), dpi and
 * style of the form. This class represents a device
 * profile. */

class DeviceProfile
{
 public:
   DeviceProfile();

   DeviceProfile(const DeviceProfile &);
   DeviceProfile &operator=(const DeviceProfile &);
   ~DeviceProfile();

   void clear();

   // Device name
   QString name() const;
   void setName(const QString &);

   // System settings active
   bool isEmpty() const;

   // Default font family of the embedded system
   QString fontFamily() const;
   void setFontFamily(const QString &);

   // Default font size of the embedded system
   int fontPointSize() const;
   void setFontPointSize(int p);

   // Display resolution of the embedded system
   int dpiX() const;
   void setDpiX(int d);
   int dpiY() const;
   void setDpiY(int d);

   // Style
   QString style() const;
   void setStyle(const QString &);

   // Initialize from desktop system
   void fromSystem();

   static void systemResolution(int *dpiX, int *dpiY);
   static void widgetResolution(const QWidget *w, int *dpiX, int *dpiY);

   bool equals(const DeviceProfile &rhs) const;

   // Apply to form/preview (using font inheritance)
   enum ApplyMode {
      /* Pre-Apply to parent widget of form being edited: Apply font
       * and make use of property inheritance to be able to modify the
       * font property freely. */
      ApplyFormParent,
      /* Post-Apply to preview widget: Change only inherited font
       * sub properties. */
      ApplyPreview
   };
   void apply(const QDesignerFormEditorInterface *core, QWidget *widget, ApplyMode am) const;

   static void applyDPI(int dpiX, int dpiY, QWidget *widget);

   QString toString() const;

   QString toXml() const;
   bool fromXml(const QString &xml, QString *errorMessage);

 private:
   QSharedDataPointer<DeviceProfileData> m_d;
};

inline bool operator==(const DeviceProfile &s1, const DeviceProfile &s2)
{
   return s1.equals(s2);
}

inline bool operator!=(const DeviceProfile &s1, const DeviceProfile &s2)
{
   return !s1.equals(s2);
}

}

#endif // DEVICEPROFILE_H
