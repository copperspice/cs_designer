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
#include <abstract_formwindow.h>
#include <abstract_formwindowcursor.h>
#include <abstract_integration.h>
#include <abstract_integration.h>
#include <abstract_propertyeditor.h>
#include <abstract_widgetdatabase.h>
#include <designer_object_inspector.h>
#include <designer_property_command.h>
#include <designer_property_editor.h>
#include <designer_propertysheet.h>
#include <designer_propertysheet.h>
#include <designer_utils.h>
#include <dynamicpropertysheet.h>
#include <dynamicpropertysheet.h>
#include <extension.h>

#include <spacer_widget_p.h>

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QLayout>
#include <QPushButton>
#include <QSize>
#include <QTextStream>
#include <QWidget>

namespace  {

/* currently not used

// Debug resolve mask of font
QString fontMask(unsigned m)
{
   QString rc;
   if (m & QFont::FamilyResolved) {
      rc += QString("Family");
   }

   if (m & QFont::SizeResolved) {
      rc += QString("Size ");
   }

   if (m & QFont::WeightResolved) {
      rc += QString("Bold ");
   }

   if (m & QFont::StyleResolved) {
      rc += QString("Style ");
   }
   if (m & QFont::UnderlineResolved) {
      rc += QString("Underline ");
   }

   if (m & QFont::StrikeOutResolved) {
      rc += QString("StrikeOut ");
   }

   if (m & QFont::KerningResolved) {
      rc += QString("Kerning ");
   }

   if (m & QFont::StyleStrategyResolved) {
      rc += QString("StyleStrategy");
   }

   return rc;
}

// Debug font
QString fontString(const QFont &f)
{
   QString rc;
   {
      const QChar comma = QLatin1Char(',');
      QTextStream str(&rc);

      str << QString("QFont(\"") <<  f.family() << comma << f.pointSize();

      if (f.bold()) {
         str << comma <<  QString("bold");
      }
      if (f.italic()) {
         str << comma <<  QString("italic");
      }
      if (f.underline()) {
         str << comma <<  QString("underline");
      }
      if (f.strikeOut()) {
         str << comma <<  QString("strikeOut");
      }
      if (f.kerning()) {
         str << comma << QString("kerning");
      }

      str <<  comma << f.styleStrategy() << QString(" resolve: ")
         << fontMask(f.resolve()) << QLatin1Char(')');
   }

   return rc;
}

*/

QSize checkSize(const QSize &size)
{
   return size.boundedTo(QSize(0xFFFFFF, 0xFFFFFF));
}

QSize diffSize(QDesignerFormWindowInterface *fw)
{
   const QWidget *container = fw->core()->integration()->containerWindow(fw);
   if (!container) {
      return QSize();
   }

   const QSize diff = container->size() - fw->size(); // decoration offset of container window
   return diff;
}

void checkSizes(QDesignerFormWindowInterface *fw, const QSize &size, QSize *formSize, QSize *containerSize)
{
   const QWidget *container = fw->core()->integration()->containerWindow(fw);
   if (!container) {
      return;
   }

   const  QSize diff = diffSize(fw); // decoration offset of container window

   QSize newFormSize = checkSize(size).expandedTo(
         fw->mainContainer()->minimumSizeHint()); // don't try to resize to smaller size than minimumSizeHint
   QSize newContainerSize = newFormSize + diff;

   newContainerSize = newContainerSize.expandedTo(container->minimumSizeHint());
   newContainerSize = newContainerSize.expandedTo(container->minimumSize());

   newFormSize = newContainerSize - diff;

   newContainerSize = checkSize(newContainerSize);

   if (formSize) {
      *formSize = newFormSize;
   }

   if (containerSize) {
      *containerSize = newContainerSize;
   }
}

/* SubProperties: When applying a changed property to a multiselection, it sometimes makes
 * sense to apply only parts (subproperties) of the property.
 * For example, if someone changes the x-value of a geometry in the property editor
 * and applies it to a multi-selection, y should not be applied as this would cause all
 * the widgets to overlap.
 * The following routines can be used to find out the changed subproperties of a property,
 * which are represented as a mask, and to apply them while leaving the others intact. */

enum RectSubPropertyMask {
   SubPropertyX      = 1,
   SubPropertyY      = 2,
   SubPropertyWidth  = 4,
   SubPropertyHeight = 8
};

enum SizePolicySubPropertyMask {
   SubPropertyHSizePolicy = 1,
   SubPropertyHStretch    = 2,
   SubPropertyVSizePolicy = 4,
   SubPropertyVStretch    = 8
};

enum AlignmentSubPropertyMask {
   SubPropertyHorizontalAlignment = 1,
   SubPropertyVerticalAlignment   = 2
};

enum StringSubPropertyMask {
   SubPropertyStringValue          = 1,
   SubPropertyStringComment        = 2,
   SubPropertyStringTranslatable   = 4,
   SubPropertyStringDisambiguation = 8
};

enum StringListSubPropertyMask {
   SubPropertyStringListValue          = 1,
   SubPropertyStringListComment        = 2,
   SubPropertyStringListTranslatable   = 4,
   SubPropertyStringListDisambiguation = 8
 };

enum KeySequenceSubPropertyMask {
   SubPropertyKeySequenceValue          = 1,
   SubPropertyKeySequenceComment        = 2,
   SubPropertyKeySequenceTranslatable   = 4,
   SubPropertyKeySequenceDisambiguation = 8
};

enum CommonSubPropertyMask {
   SubPropertyAll = 0xFFFFFFFF
};

// Set the mask flag in mask if the properties do not match.
#define COMPARE_SUBPROPERTY(object1, object2, getter, mask, maskFlag) if (object1.getter() != object2.getter()) mask |= maskFlag;

// find changed subproperties of a rectangle
unsigned compareSubProperties(const QRect &r1, const QRect &r2)
{
   unsigned rc = 0;
   COMPARE_SUBPROPERTY(r1, r2, x, rc, SubPropertyX)
   COMPARE_SUBPROPERTY(r1, r2, y, rc, SubPropertyY)
   COMPARE_SUBPROPERTY(r1, r2, width, rc, SubPropertyWidth)
   COMPARE_SUBPROPERTY(r1, r2, height, rc, SubPropertyHeight)
   return rc;
}

// find changed subproperties of a QSize
unsigned compareSubProperties(const QSize &r1, const QSize &r2)
{
   unsigned rc = 0;
   COMPARE_SUBPROPERTY(r1, r2, width,  rc, SubPropertyWidth)
   COMPARE_SUBPROPERTY(r1, r2, height, rc, SubPropertyHeight)
   return rc;
}
// find changed subproperties of a QSizePolicy
unsigned compareSubProperties(const QSizePolicy &sp1, const QSizePolicy &sp2)
{
   unsigned rc = 0;
   COMPARE_SUBPROPERTY(sp1, sp2, horizontalPolicy,  rc, SubPropertyHSizePolicy)
   COMPARE_SUBPROPERTY(sp1, sp2, horizontalStretch, rc, SubPropertyHStretch)
   COMPARE_SUBPROPERTY(sp1, sp2, verticalPolicy,    rc, SubPropertyVSizePolicy)
   COMPARE_SUBPROPERTY(sp1, sp2, verticalStretch,   rc, SubPropertyVStretch)
   return rc;
}
// find changed subproperties of qdesigner_internal::PropertySheetStringValue
unsigned compareSubProperties(const qdesigner_internal::PropertySheetStringValue &str1,
   const qdesigner_internal::PropertySheetStringValue &str2)
{
   unsigned rc = 0;
   COMPARE_SUBPROPERTY(str1, str2, value,          rc, SubPropertyStringValue)
   COMPARE_SUBPROPERTY(str1, str2, comment,        rc, SubPropertyStringComment)
   COMPARE_SUBPROPERTY(str1, str2, translatable,   rc, SubPropertyStringTranslatable)
   COMPARE_SUBPROPERTY(str1, str2, disambiguation, rc, SubPropertyStringDisambiguation)
   return rc;
}
// find changed subproperties of qdesigner_internal::PropertySheetStringListValue
unsigned compareSubProperties(const qdesigner_internal::PropertySheetStringListValue &str1,
   const qdesigner_internal::PropertySheetStringListValue &str2)
{
   unsigned rc = 0;
   COMPARE_SUBPROPERTY(str1, str2, value,          rc, SubPropertyStringListValue)
   COMPARE_SUBPROPERTY(str1, str2, comment,        rc, SubPropertyStringListComment)
   COMPARE_SUBPROPERTY(str1, str2, translatable,   rc, SubPropertyStringListTranslatable)
   COMPARE_SUBPROPERTY(str1, str2, disambiguation, rc, SubPropertyStringListDisambiguation)
   return rc;
}
// find changed subproperties of qdesigner_internal::PropertySheetKeySequenceValue
unsigned compareSubProperties(const qdesigner_internal::PropertySheetKeySequenceValue &str1,
   const qdesigner_internal::PropertySheetKeySequenceValue &str2)
{
   unsigned rc = 0;
   COMPARE_SUBPROPERTY(str1, str2, value,          rc, SubPropertyKeySequenceValue)
   COMPARE_SUBPROPERTY(str1, str2, comment,        rc, SubPropertyKeySequenceComment)
   COMPARE_SUBPROPERTY(str1, str2, translatable,   rc, SubPropertyKeySequenceTranslatable)
   COMPARE_SUBPROPERTY(str1, str2, disambiguation, rc, SubPropertyKeySequenceDisambiguation)
   return rc;
}

// Compare font-subproperties taking the [undocumented] resolve flag into account
template <class Property>
void compareFontSubProperty(const QFont &f1, const QFont &f2, Property (QFont::*getter) () const,
      unsigned maskBit, unsigned &mask)
{
   const bool f1Changed = f1.resolve() & maskBit;
   const bool f2Changed = f2.resolve() & maskBit;

   // Role has been set/reset in editor
   if (f1Changed != f2Changed) {
      mask |= maskBit;

   } else {
      // Was modified in both palettes: Compare values.
      if (f1Changed && f2Changed && (f1.*getter)() != (f2.*getter)()) {
         mask |= maskBit;
      }
   }
}

// find changed subproperties of a QFont
unsigned compareSubProperties(const QFont &f1, const QFont &f2)
{
   unsigned rc = 0;
   compareFontSubProperty(f1, f2, &QFont::family,        QFont::FamilyResolved, rc);
   compareFontSubProperty(f1, f2, &QFont::pointSize,     QFont::SizeResolved, rc);
   compareFontSubProperty(f1, f2, &QFont::bold,          QFont::WeightResolved, rc);
   compareFontSubProperty(f1, f2, &QFont::italic,        QFont::StyleResolved, rc);
   compareFontSubProperty(f1, f2, &QFont::underline,     QFont::UnderlineResolved, rc);
   compareFontSubProperty(f1, f2, &QFont::strikeOut,     QFont::StrikeOutResolved, rc);
   compareFontSubProperty(f1, f2, &QFont::kerning,       QFont::KerningResolved, rc);
   compareFontSubProperty(f1, f2, &QFont::styleStrategy, QFont::StyleStrategyResolved, rc);

   return rc;
}

// Compare colors of a role
bool roleColorChanged(const QPalette &p1, const QPalette &p2, QPalette::ColorRole role)
{
   for (int group = QPalette::Active; group < QPalette::NColorGroups;  group++) {
      const QPalette::ColorGroup pgroup = static_cast<QPalette::ColorGroup>(group);

      if (p1.color(pgroup, role) != p2.color(pgroup, role)) {
         return true;
      }
   }

   return false;
}

// find changed subproperties of a QPalette taking the [undocumented] resolve flags into account
unsigned compareSubProperties(const QPalette &p1, const QPalette &p2)
{
   unsigned rc = 0;
   unsigned maskBit = 1u;
   // generate a mask for each role
   const unsigned p1Changed = p1.resolve();
   const unsigned p2Changed = p2.resolve();

   for (int role = QPalette::WindowText;  role < QPalette::NColorRoles; role++, maskBit <<= 1u) {
      const bool p1RoleChanged = p1Changed & maskBit;
      const bool p2RoleChanged = p2Changed & maskBit;
      // Role has been set/reset in editor
      if (p1RoleChanged != p2RoleChanged) {
         rc |= maskBit;
      } else {
         // Was modified in both palettes: Compare values.
         if (p1RoleChanged && p2RoleChanged && roleColorChanged(p1, p2, static_cast<QPalette::ColorRole>(role))) {
            rc |= maskBit;
         }
      }
   }
   return rc;
}

// find changed subproperties of a QAlignment which is a flag combination of vertical and horizontal

unsigned compareSubProperties(Qt::Alignment a1, Qt::Alignment a2)
{
   unsigned rc = 0;
   if ((a1 & Qt::AlignHorizontal_Mask) != (a2 & Qt::AlignHorizontal_Mask)) {
      rc |= SubPropertyHorizontalAlignment;
   }

   if ((a1 & Qt::AlignVertical_Mask) != (a2 & Qt::AlignVertical_Mask)) {
      rc |= SubPropertyVerticalAlignment;
   }

   return rc;
}

Qt::Alignment variantToAlignment(const QVariant &q)
{
   return Qt::Alignment(qdesigner_internal::Utils::valueOf(q));
}

// find changed subproperties of a variant
unsigned compareSubProperties(const QVariant &q1, const QVariant &q2, qdesigner_internal::SpecialProperty specialProperty)
{
   // Do not clobber new value in the comparison function in
   // case someone sets a QString on a PropertySheetStringValue.
   if (q1.type() != q2.type()) {
      return SubPropertyAll;
   }

   switch (q1.type()) {
      case QVariant::Rect:
         return compareSubProperties(q1.toRect(), q2.toRect());

      case QVariant::Size:
         return compareSubProperties(q1.toSize(), q2.toSize());

      case QVariant::SizePolicy:
         return compareSubProperties(q1.value<QSizePolicy>(), q2.value<QSizePolicy>());

      case QVariant::Font:
         return compareSubProperties(q1.value<QFont>(), q2.value<QFont>());

      case QVariant::Palette:
         return compareSubProperties(q1.value<QPalette>(), q2.value<QPalette>());

      default:
         if (q1.userType() == QVariant::typeToTypeId<qdesigner_internal::PropertySheetIconValue>()) {
            return q1.value<qdesigner_internal::PropertySheetIconValue>()
                  .compare(q2.value<qdesigner_internal::PropertySheetIconValue>());

         } else if (q1.userType() == QVariant::typeToTypeId<qdesigner_internal::PropertySheetStringValue>()) {
            return compareSubProperties(q1.value<qdesigner_internal::PropertySheetStringValue>(),
                  q2.value<qdesigner_internal::PropertySheetStringValue>());

         } else if (q1.userType() == QVariant::typeToTypeId<qdesigner_internal::PropertySheetStringListValue>()) {
            return compareSubProperties(q1.value<qdesigner_internal::PropertySheetStringListValue>(),
                  q2.value<qdesigner_internal::PropertySheetStringListValue>());

         } else if (q1.userType() == QVariant::typeToTypeId<qdesigner_internal::PropertySheetKeySequenceValue>()) {
            return compareSubProperties(q1.value<qdesigner_internal::PropertySheetKeySequenceValue>(),
                  q2.value<qdesigner_internal::PropertySheetKeySequenceValue>());
         }

         // Enumerations, flags
         switch (specialProperty) {
            case qdesigner_internal::SP_Alignment:
               return compareSubProperties(variantToAlignment(q1), variantToAlignment(q2));
            default:
               break;
         }
         break;
   }
   return SubPropertyAll;
}

// Apply  the sub property if mask flag is set in mask
#define SET_SUBPROPERTY(rc, newValue, getter, setter, mask, maskFlag) if (mask & maskFlag) rc.setter(newValue.getter());

// apply changed subproperties to a rectangle
QRect applyRectSubProperty(const QRect &oldValue, const QRect &newValue, unsigned mask)
{
   QRect rc = oldValue;
   SET_SUBPROPERTY(rc, newValue, x,      moveLeft,  mask, SubPropertyX)
   SET_SUBPROPERTY(rc, newValue, y,      moveTop,   mask, SubPropertyY)
   SET_SUBPROPERTY(rc, newValue, width,  setWidth,  mask, SubPropertyWidth)
   SET_SUBPROPERTY(rc, newValue, height, setHeight, mask, SubPropertyHeight)
   return rc;
}


// apply changed subproperties to a rectangle QSize
QSize applySizeSubProperty(const QSize &oldValue, const QSize &newValue, unsigned mask)
{
   QSize rc = oldValue;
   SET_SUBPROPERTY(rc, newValue, width,  setWidth,  mask, SubPropertyWidth)
   SET_SUBPROPERTY(rc, newValue, height, setHeight, mask, SubPropertyHeight)
   return rc;
}


// apply changed subproperties to a SizePolicy
QSizePolicy applySizePolicySubProperty(const QSizePolicy &oldValue, const QSizePolicy &newValue, unsigned mask)
{
   QSizePolicy rc = oldValue;
   SET_SUBPROPERTY(rc, newValue, horizontalPolicy,  setHorizontalPolicy,  mask, SubPropertyHSizePolicy)
   SET_SUBPROPERTY(rc, newValue, horizontalStretch, setHorizontalStretch, mask, SubPropertyHStretch)
   SET_SUBPROPERTY(rc, newValue, verticalPolicy,    setVerticalPolicy,    mask, SubPropertyVSizePolicy)
   SET_SUBPROPERTY(rc, newValue, verticalStretch,   setVerticalStretch,   mask, SubPropertyVStretch)
   return rc;
}

// apply changed subproperties to a qdesigner_internal::PropertySheetStringValue
qdesigner_internal::PropertySheetStringValue applyStringSubProperty(
      const qdesigner_internal::PropertySheetStringValue &oldValue,
      const qdesigner_internal::PropertySheetStringValue &newValue, unsigned mask)
{
   qdesigner_internal::PropertySheetStringValue rc = oldValue;
   SET_SUBPROPERTY(rc, newValue, value, setValue, mask, SubPropertyStringValue)
   SET_SUBPROPERTY(rc, newValue, comment, setComment, mask, SubPropertyStringComment)
   SET_SUBPROPERTY(rc, newValue, translatable, setTranslatable, mask, SubPropertyStringTranslatable)
   SET_SUBPROPERTY(rc, newValue, disambiguation, setDisambiguation, mask, SubPropertyStringDisambiguation)

   return rc;
}

// apply changed subproperties to a qdesigner_internal::PropertySheetStringListValue
qdesigner_internal::PropertySheetStringListValue applyStringListSubProperty(
      const qdesigner_internal::PropertySheetStringListValue &oldValue,
      const qdesigner_internal::PropertySheetStringListValue &newValue, unsigned mask)
{
   qdesigner_internal::PropertySheetStringListValue rc = oldValue;
   SET_SUBPROPERTY(rc, newValue, value, setValue, mask, SubPropertyStringListValue)
   SET_SUBPROPERTY(rc, newValue, comment, setComment, mask, SubPropertyStringListComment)
   SET_SUBPROPERTY(rc, newValue, translatable, setTranslatable, mask, SubPropertyStringListTranslatable)
   SET_SUBPROPERTY(rc, newValue, disambiguation, setDisambiguation, mask, SubPropertyStringListDisambiguation)
   return rc;
}

// apply changed subproperties to a qdesigner_internal::PropertySheetKeySequenceValue
qdesigner_internal::PropertySheetKeySequenceValue applyKeySequenceSubProperty(
      const qdesigner_internal::PropertySheetKeySequenceValue &oldValue,
      const qdesigner_internal::PropertySheetKeySequenceValue &newValue, unsigned mask)
{
   qdesigner_internal::PropertySheetKeySequenceValue rc = oldValue;
   SET_SUBPROPERTY(rc, newValue, value, setValue, mask, SubPropertyKeySequenceValue)
   SET_SUBPROPERTY(rc, newValue, comment, setComment, mask, SubPropertyKeySequenceComment)
   SET_SUBPROPERTY(rc, newValue, translatable, setTranslatable, mask, SubPropertyKeySequenceTranslatable)
   SET_SUBPROPERTY(rc, newValue, disambiguation, setDisambiguation, mask, SubPropertyKeySequenceDisambiguation)
   return rc;
}

// Apply the font-subproperties keeping the [undocumented]
// resolve flag in sync (note that PropertySetterType might be something like const T&).
template <class PropertyReturnType, class PropertySetterType>
inline void setFontSubProperty(unsigned mask, const QFont &newValue, unsigned maskBit,
      PropertyReturnType (QFont::*getter) () const, void (QFont::*setter) (PropertySetterType), QFont &value)
{
   if (mask & maskBit) {
      (value.*setter)((newValue.*getter)());
      // Set the resolve bit from NewValue in return value
      uint r = value.resolve();
      const bool origFlag = newValue.resolve() & maskBit;

      if (origFlag) {
         r |= maskBit;
      } else {
         r &= ~maskBit;
      }

      value.resolve(r);
   }
}

// apply changed subproperties to a QFont
QFont applyFontSubProperty(const QFont &oldValue, const QFont &newValue, unsigned mask)
{
   QFont  rc = oldValue;
   setFontSubProperty(mask, newValue, QFont::FamilyResolved,        &QFont::family,        &QFont::setFamily, rc);
   setFontSubProperty(mask, newValue, QFont::SizeResolved,          &QFont::pointSize,     &QFont::setPointSize, rc);
   setFontSubProperty(mask, newValue, QFont::WeightResolved,        &QFont::bold,          &QFont::setBold, rc);
   setFontSubProperty(mask, newValue, QFont::StyleResolved,         &QFont::italic,        &QFont::setItalic, rc);
   setFontSubProperty(mask, newValue, QFont::UnderlineResolved,     &QFont::underline,     &QFont::setUnderline, rc);
   setFontSubProperty(mask, newValue, QFont::StrikeOutResolved,     &QFont::strikeOut,     &QFont::setStrikeOut, rc);
   setFontSubProperty(mask, newValue, QFont::KerningResolved,       &QFont::kerning,       &QFont::setKerning, rc);
   setFontSubProperty(mask, newValue, QFont::StyleStrategyResolved, &QFont::styleStrategy, &QFont::setStyleStrategy, rc);

   return rc;
}

// apply changed subproperties to a QPalette
QPalette applyPaletteSubProperty(const QPalette &oldValue, const QPalette &newValue, unsigned mask)
{
   QPalette rc = oldValue;
   // apply a mask for each role
   unsigned maskBit = 1u;
   for (int role = QPalette::WindowText;  role < QPalette::NColorRoles; role++, maskBit <<= 1u) {
      if (mask & maskBit) {
         for (int group = QPalette::Active; group < QPalette::NColorGroups;  group++) {
            const QPalette::ColorGroup pgroup = static_cast<QPalette::ColorGroup>(group);
            const QPalette::ColorRole prole =  static_cast<QPalette::ColorRole>(role);
            rc.setColor(pgroup, prole, newValue.color(pgroup, prole));
         }
         // Set the resolve bit from NewValue in return value
         uint r = rc.resolve();
         const bool origFlag = newValue.resolve() & maskBit;
         if (origFlag) {
            r |= maskBit;
         } else {
            r &= ~maskBit;
         }
         rc.resolve(r);
      }
   }
   return rc;
}

// apply changed subproperties to  a QAlignment which is a flag combination of vertical and horizontal
Qt::Alignment applyAlignmentSubProperty(Qt::Alignment oldValue, Qt::Alignment newValue, unsigned mask)
{
   // easy: both changed.
   if (mask == (SubPropertyHorizontalAlignment | SubPropertyVerticalAlignment)) {
      return newValue;
   }
   // Change subprop
   const Qt::Alignment changeMask   = (mask & SubPropertyHorizontalAlignment) ? Qt::AlignHorizontal_Mask : Qt::AlignVertical_Mask;
   const Qt::Alignment takeOverMask = (mask & SubPropertyHorizontalAlignment) ? Qt::AlignVertical_Mask   : Qt::AlignHorizontal_Mask;
   return (oldValue & takeOverMask) | (newValue & changeMask);
}

}

namespace qdesigner_internal {

// apply changed subproperties to a variant
PropertyHelper::Value applySubProperty(const QVariant &oldValue, const QVariant &newValue,
   qdesigner_internal::SpecialProperty specialProperty, unsigned mask, bool changed)
{
   if (mask == SubPropertyAll) {
      return PropertyHelper::Value(newValue, changed);
   }

   switch (oldValue.type()) {
      case QVariant::Rect:
         return PropertyHelper::Value(applyRectSubProperty(oldValue.toRect(), newValue.toRect(), mask), changed);

      case QVariant::Size:
         return PropertyHelper::Value(applySizeSubProperty(oldValue.toSize(), newValue.toSize(), mask), changed);

      case QVariant::SizePolicy:
         return PropertyHelper::Value(QVariant::fromValue(applySizePolicySubProperty(oldValue.value<QSizePolicy>(),
               newValue.value<QSizePolicy>(), mask)), changed);

      case QVariant::Font: {
         // Changed flag in case of font and palette depends on resolve mask only, not on the passed "changed" value.

         // The first case: the user changed bold subproperty and then pressed reset button for this subproperty (not for
         // the whole font property). We instantiate SetPropertyCommand passing changed=true. But in this case no
         // subproperty is changed and the whole property should be marked an unchanged.

         // The second case: there are 2 pushbuttons, for 1st the user set bold and italic subproperties,
         // for the 2nd he set bold only. He does multiselection so that the current widget is the 2nd one.
         // He press reset next to bold subproperty. In result the 2nd widget should have the whole
         // font property marked as unchanged and the 1st widget should have the font property
         // marked as changed and only italic subproperty should be marked as changed (the bold should be reset).

         // The third case: there are 2 pushbuttons, for 1st the user set bold and italic subproperties,
         // for the 2nd he set bold only. He does multiselection so that the current widget is the 2nd one.
         // He press reset button for the whole font property. In result whole font properties for both
         // widgets should be marked as unchanged.

         QFont font = applyFontSubProperty(oldValue.value<QFont>(), newValue.value<QFont>(), mask);
         return PropertyHelper::Value(QVariant::fromValue(font), font.resolve());
      }

      case QVariant::Palette: {
         QPalette palette = applyPaletteSubProperty(oldValue.value<QPalette>(), newValue.value<QPalette>(), mask);
         return PropertyHelper::Value(QVariant::fromValue(palette), palette.resolve());
      }

      default:
         if (oldValue.userType() == QVariant::typeToTypeId<qdesigner_internal::PropertySheetIconValue>()) {
            PropertySheetIconValue icon = oldValue.value<qdesigner_internal::PropertySheetIconValue>();
            icon.assign(newValue.value<qdesigner_internal::PropertySheetIconValue>(), mask);

            return PropertyHelper::Value(QVariant::fromValue(icon), icon.mask());

         } else if (oldValue.userType() == QVariant::typeToTypeId<qdesigner_internal::PropertySheetStringValue>()) {
            qdesigner_internal::PropertySheetStringValue str = applyStringSubProperty(
                  oldValue.value<qdesigner_internal::PropertySheetStringValue>(),
                  newValue.value<qdesigner_internal::PropertySheetStringValue>(), mask);

            return PropertyHelper::Value(QVariant::fromValue(str), changed);

         } else if (oldValue.userType() == QVariant::typeToTypeId<qdesigner_internal::PropertySheetStringListValue>()) {
            qdesigner_internal::PropertySheetStringListValue str = applyStringListSubProperty(
                  oldValue.value<qdesigner_internal::PropertySheetStringListValue>(),
                  newValue.value<qdesigner_internal::PropertySheetStringListValue>(), mask);

            return PropertyHelper::Value(QVariant::fromValue(str), changed);

         } else if (oldValue.userType() == QVariant::typeToTypeId<qdesigner_internal::PropertySheetKeySequenceValue>()) {
            qdesigner_internal::PropertySheetKeySequenceValue key = applyKeySequenceSubProperty(
                  oldValue.value<qdesigner_internal::PropertySheetKeySequenceValue>(),
                  newValue.value<qdesigner_internal::PropertySheetKeySequenceValue>(), mask);

            return PropertyHelper::Value(QVariant::fromValue(key), changed);
         }

         // Enumerations, flags
         switch (specialProperty) {
            case qdesigner_internal::SP_Alignment: {
               qdesigner_internal::PropertySheetFlagValue f = oldValue.value<qdesigner_internal::PropertySheetFlagValue>();
               f.value = applyAlignmentSubProperty(variantToAlignment(oldValue), variantToAlignment(newValue), mask);

               QVariant v;
               v.setValue(f);
               return PropertyHelper::Value(v, changed);
            }

            default:
               break;
         }

         break;
   }

   return PropertyHelper::Value(newValue, changed);
}

// figure out special property
enum SpecialProperty getSpecialProperty(const QString &propertyName)
{
   if (propertyName == QString("objectName")) {
      return SP_ObjectName;
   }

   if (propertyName == QString("layoutName")) {
      return SP_LayoutName;
   }

   if (propertyName == "spacerName") {
      return SP_SpacerName;
   }

   if (propertyName == QString("icon")) {
      return SP_Icon;
   }

   if (propertyName == QString("currentTabName")) {
      return SP_CurrentTabName;
   }

   if (propertyName == QString("currentItemName")) {
      return SP_CurrentItemName;
   }

   if (propertyName == QString("currentPageName")) {
      return SP_CurrentPageName;
   }

   if (propertyName == QString("geometry")) {
      return SP_Geometry;
   }

   if (propertyName == QString("windowTitle")) {
      return SP_WindowTitle;
   }

   if (propertyName == QString("minimumSize")) {
      return SP_MinimumSize;
   }

   if (propertyName == QString("maximumSize")) {
      return SP_MaximumSize;
   }

   if (propertyName == QString("alignment")) {
      return SP_Alignment;
   }

   if (propertyName == QString("autoDefault")) {
      return SP_AutoDefault;
   }

   if (propertyName == QString("shortcut")) {
      return SP_Shortcut;
   }

   if (propertyName == QString("orientation")) {
      return SP_Orientation;
   }

   return SP_None;
}

PropertyHelper::PropertyHelper(QObject *object, SpecialProperty specialProperty,
      QDesignerPropertySheetExtension *sheet, int index)
   : m_specialProperty(specialProperty), m_object(object), m_objectType(OT_Object),
     m_propertySheet(sheet), m_index(index),
     m_oldValue(m_propertySheet->property(m_index), m_propertySheet->isChanged(m_index))
{
   if (object->isWidgetType()) {
      m_parentWidget = (dynamic_cast<QWidget *>(object))->parentWidget();
      m_objectType = OT_Widget;

   } else {
      if (const QAction *action = dynamic_cast<const QAction *>(m_object.data())) {
         m_objectType = action->associatedWidgets().empty() ? OT_FreeAction : OT_AssociatedAction;
      }
   }
}

QDesignerIntegration *PropertyHelper::integration(QDesignerFormWindowInterface *fw) const
{
   return dynamic_cast<QDesignerIntegration *>(fw->core()->integration());
}

// Set widget value, apply corrections and checks in case of main window.
void PropertyHelper::checkApplyWidgetValue(QDesignerFormWindowInterface *fw, QWidget *w,
   SpecialProperty specialProperty, QVariant &value)
{

   bool isMainContainer = false;
   if (QDesignerFormWindowCursorInterface *cursor = fw->cursor()) {
      if (cursor->isWidgetSelected(w)) {
         if (cursor->isWidgetSelected(fw->mainContainer())) {
            isMainContainer = true;
         }
      }
   }
   if (!isMainContainer) {
      return;
   }

   QWidget *container = fw->core()->integration()->containerWindow(fw);
   if (!container) {
      return;
   }


   switch (specialProperty) {
      case SP_MinimumSize: {
         const QSize size = checkSize(value.toSize());
         value.setValue(size);
      }

      break;
      case SP_MaximumSize: {
         QSize fs, cs;
         checkSizes(fw, value.toSize(), &fs, &cs);
         container->setMaximumSize(cs);
         fw->mainContainer()->setMaximumSize(fs);
         value.setValue(fs);

      }
      break;
      case SP_Geometry: {
         QRect r = value.toRect();
         QSize fs, cs;
         checkSizes(fw, r.size(), &fs, &cs);
         container->resize(cs);
         r.setSize(fs);
         value.setValue(r);
      }
      break;

      default:
         break;
   }
}

unsigned PropertyHelper::updateMask() const
{
   unsigned rc = 0;

   switch (m_specialProperty) {
      case SP_ObjectName:
      case SP_LayoutName:
      case SP_SpacerName:
      case SP_CurrentTabName:
      case SP_CurrentItemName:
      case SP_CurrentPageName:
         if (m_objectType != OT_FreeAction) {
            rc |= UpdateObjectInspector;
         }
         break;

      case SP_Icon:
         if (m_objectType == OT_AssociatedAction) {
            rc |= UpdateObjectInspector;
         }
         break;

      case SP_Orientation: // for updating splitter icon
         rc |= UpdateObjectInspector;
         break;

      default:
         break;

   }

   return rc;
}

bool PropertyHelper::canMerge(const PropertyHelper &other) const
{
   return m_object == other.m_object &&  m_index == other.m_index;
}

void PropertyHelper::triggerActionChanged(QAction *a)
{
   a->setData(QVariant(true));    // triggers signal "changed" in QAction
   a->setData(QVariant(false));
}

// Update the object to reflect the changes
void PropertyHelper::updateObject(QDesignerFormWindowInterface *fw, const QVariant &oldValue, const QVariant &newValue)
{
   switch (m_objectType) {
      case OT_Widget: {
         switch (m_specialProperty) {
            case SP_ObjectName: {
               const QString oldName = oldValue.value<PropertySheetStringValue>().value();
               const QString newName = newValue.value<PropertySheetStringValue>().value();
               QDesignerFormWindowCommand::updateBuddies(fw, oldName, newName);
            }
            break;

            default:
               break;
         }
      }
      break;

      case OT_AssociatedAction:
      case OT_FreeAction:
         // SP_Shortcut is a fake property, so, QAction::changed does not trigger.
         if (m_specialProperty == SP_ObjectName || m_specialProperty == SP_Shortcut) {
            triggerActionChanged(dynamic_cast<QAction *>(m_object.data()));
         }
         break;

      default:
         break;
   }

   switch (m_specialProperty) {
      case SP_ObjectName:
      case SP_LayoutName:
      case SP_SpacerName:
         if (QDesignerIntegration *objI = integration(fw)) {
            const QString oldName = oldValue.value<PropertySheetStringValue>().value();
            const QString newName = newValue.value<PropertySheetStringValue>().value();

            objI->emitObjectNameChanged(fw, m_object, newName, oldName);
         }
         break;

      default:
         break;
   }
}

void PropertyHelper::ensureUniqueObjectName(QDesignerFormWindowInterface *fw, QObject *object) const
{
   switch (m_specialProperty) {
      case SP_SpacerName:
         if (object->isWidgetType()) {
            if (Spacer *sp = dynamic_cast<Spacer *>(object)) {
               fw->ensureUniqueObjectName(sp);
               return;
            }
         }
         fw->ensureUniqueObjectName(object);
         break;

      case SP_LayoutName:
         // Layout name is invoked on the parent widget

         if (object->isWidgetType()) {
            const QWidget *w = dynamic_cast<const QWidget *>(object);
            if (QLayout *wlayout = w->layout()) {
               fw->ensureUniqueObjectName(wlayout);
               return;
            }
         }
         fw->ensureUniqueObjectName(object);
         break;

      case SP_ObjectName:
         fw->ensureUniqueObjectName(object);
         break;

      default:
         break;
   }
}

PropertyHelper::Value PropertyHelper::setValue(QDesignerFormWindowInterface *fw, const QVariant &value, bool changed,
   unsigned subPropertyMask)
{
   // Set new whole value
   if (subPropertyMask == SubPropertyAll) {
      return applyValue(fw, m_oldValue.first, Value(value, changed));
   }

   // apply subproperties
   const PropertyHelper::Value maskedNewValue = applySubProperty(m_oldValue.first, value,
         m_specialProperty, subPropertyMask, changed);

   return applyValue(fw, m_oldValue.first, maskedNewValue);
}

// Apply the value and update, returns corrected value
PropertyHelper::Value PropertyHelper::applyValue(QDesignerFormWindowInterface *fw,
      const QVariant &oldValue, Value newValue)
{
   if (m_objectType ==  OT_Widget) {
      checkApplyWidgetValue(fw, dynamic_cast<QWidget *>(m_object.data()), m_specialProperty, newValue.first);
   }

   m_propertySheet->setProperty(m_index, newValue.first);
   m_propertySheet->setChanged(m_index, newValue.second);

   switch (m_specialProperty) {
      case SP_LayoutName:
      case SP_ObjectName:
      case SP_SpacerName:
         ensureUniqueObjectName(fw, m_object);
         newValue.first = m_propertySheet->property(m_index);
         break;

      default:
         break;
   }

   updateObject(fw, oldValue, newValue.first);
   return newValue;
}

PropertyHelper::Value PropertyHelper::restoreOldValue(QDesignerFormWindowInterface *fw)
{
   return applyValue(fw, m_propertySheet->property(m_index), m_oldValue);
}

// find the default value in widget DB in case PropertySheet::reset fails
QVariant PropertyHelper::findDefaultValue(QDesignerFormWindowInterface *fw) const
{
   if (m_specialProperty == SP_AutoDefault && dynamic_cast<const QPushButton *>(m_object.data())) {
      // AutoDefault defaults to true on dialogs
      const bool isDialog = dynamic_cast<const QDialog *>(fw->mainContainer());
      return QVariant(isDialog);
   }

   const int item_idx = fw->core()->widgetDataBase()->indexOfObject(m_object);
   if (item_idx == -1) {
      return  m_oldValue.first;   // simply do not know the value
   }

   const QDesignerWidgetDataBaseItemInterface *item = fw->core()->widgetDataBase()->item(item_idx);
   const QList<QVariant> default_prop_values = item->defaultPropertyValues();

   if (m_index < default_prop_values.size()) {
      return default_prop_values.at(m_index);
   }

   if (m_oldValue.first.type() == QVariant::Color) {
      return QColor();
   }

   return m_oldValue.first; // Again, we just do not know
}

PropertyHelper::Value PropertyHelper::restoreDefaultValue(QDesignerFormWindowInterface *fw)
{
   Value defaultValue = qMakePair(QVariant(), false);
   const QVariant currentValue = m_propertySheet->property(m_index);

   // try to reset sheet, else try to find default
   if (m_propertySheet->reset(m_index)) {
      defaultValue.first = m_propertySheet->property(m_index);

   } else {
      defaultValue.first = findDefaultValue(fw);
      m_propertySheet->setProperty(m_index, defaultValue.first);
   }

   m_propertySheet->setChanged(m_index, defaultValue.second);

   if (m_objectType == OT_Widget) {
      checkApplyWidgetValue(fw, dynamic_cast<QWidget *>(m_object.data()), m_specialProperty, defaultValue.first);
   }

   switch (m_specialProperty) {
      case SP_LayoutName:
      case SP_ObjectName:
      case SP_SpacerName:
         ensureUniqueObjectName(fw, m_object);
         defaultValue.first = m_propertySheet->property(m_index);
         break;

      default:
         break;
   }

   updateObject(fw, currentValue, defaultValue.first);

   return defaultValue;
}

PropertyListCommand::PropertyDescription::PropertyDescription(const QString &propertyName,
      QDesignerPropertySheetExtension *propertySheet, int index)
   : m_propertyName(propertyName), m_propertyGroup(propertySheet->propertyGroup(index)),
     m_propertyType(propertySheet->property(index).type()),
     m_specialProperty(getSpecialProperty(propertyName))
{
}

PropertyListCommand::PropertyDescription::PropertyDescription()
   : m_propertyType(QVariant::Invalid), m_specialProperty(SP_None)
{
}

bool PropertyListCommand::PropertyDescription::equals(const PropertyDescription &p) const
{
   return m_propertyType == p.m_propertyType && m_specialProperty == p.m_specialProperty &&
      m_propertyName == p.m_propertyName && m_propertyGroup   == p.m_propertyGroup;
}

PropertyListCommand::PropertyListCommand(QDesignerFormWindowInterface *formWindow, QUndoCommand *parent)
   : QDesignerFormWindowCommand(QString(), formWindow, parent)
{
}

const QString PropertyListCommand::propertyName() const
{
   return m_propertyDescription.m_propertyName;
}

SpecialProperty PropertyListCommand::specialProperty() const
{
   return m_propertyDescription.m_specialProperty;
}

// add an object
bool PropertyListCommand::add(QObject *object, const QString &propertyName)
{
   QDesignerPropertySheetExtension *sheet = propertySheet(object);
   Q_ASSERT(sheet);

   const int index = sheet->indexOf(propertyName);
   if (index == -1) {
      return false;
   }

   if (!sheet->isEnabled(index)) {
      return false;
   }

   const PropertyDescription description(propertyName, sheet, index);

   if (m_propertyHelperList.empty()) {
      // first entry
      m_propertyDescription = description;
   } else {
      // checks: mismatch or only one object in case of name
      const bool match = m_propertyDescription.equals(description);
      if (!match || m_propertyDescription.m_specialProperty == SP_ObjectName) {
         return false;
      }
   }

   const PropertyHelperPtr ph(createPropertyHelper(object, m_propertyDescription.m_specialProperty, sheet, index));
   m_propertyHelperList.push_back(ph);
   return true;
}

PropertyHelper *PropertyListCommand::createPropertyHelper(QObject *object, SpecialProperty sp,
   QDesignerPropertySheetExtension *sheet, int sheetIndex) const
{
   return new PropertyHelper(object, sp, sheet, sheetIndex);
}

// Init from a list and make sure referenceObject is added first to obtain the right property group
bool PropertyListCommand::initList(const ObjectList &list, const QString &apropertyName, QObject *referenceObject)
{
   propertyHelperList().clear();

   // Ensure the referenceObject (property editor) is first, so the right property group is chosen.
   if (referenceObject) {
      if (!add(referenceObject, apropertyName)) {
         return false;
      }
   }

   for (QObject *o : list) {
      if (o != referenceObject) {
         add(o, apropertyName);
      }
   }

   return !propertyHelperList().empty();
}


QObject *PropertyListCommand::object(int index) const
{
   Q_ASSERT(index < m_propertyHelperList.size());
   return m_propertyHelperList.at(index)->object();
}

QVariant PropertyListCommand::oldValue(int index) const
{
   Q_ASSERT(index < m_propertyHelperList.size());
   return m_propertyHelperList.at(index)->oldValue();
}

void PropertyListCommand::setOldValue(const QVariant &oldValue, int index)
{
   Q_ASSERT(index < m_propertyHelperList.size());
   m_propertyHelperList.at(index)->setOldValue(oldValue);
}

// Set a new value when applied to a PropertyHelper.
class SetValueFunction
{
 public:
   SetValueFunction(QDesignerFormWindowInterface *formWindow, const PropertyHelper::Value &newValue, unsigned subPropertyMask);

   PropertyHelper::Value operator()(PropertyHelper &);

 private:
   QDesignerFormWindowInterface *m_formWindow;
   const PropertyHelper::Value m_newValue;
   const unsigned m_subPropertyMask;
};

SetValueFunction::SetValueFunction(QDesignerFormWindowInterface *formWindow, const PropertyHelper::Value &newValue,
      unsigned subPropertyMask)
   : m_formWindow(formWindow), m_newValue(newValue), m_subPropertyMask(subPropertyMask)
{
}

PropertyHelper::Value SetValueFunction::operator()(PropertyHelper &ph)
{
   return ph.setValue(m_formWindow, m_newValue.first, m_newValue.second, m_subPropertyMask);
}

// Restore old value when applied to a PropertyHelper.
class UndoSetValueFunction
{
 public:
   UndoSetValueFunction(QDesignerFormWindowInterface *formWindow) : m_formWindow(formWindow) {}
   PropertyHelper::Value operator()(PropertyHelper &ph) {
      return ph.restoreOldValue(m_formWindow);
   }
 private:
   QDesignerFormWindowInterface *m_formWindow;
};

// ----- RestoreDefaultFunction: Restore default value when applied to a PropertyHelper.
class RestoreDefaultFunction
{
 public:
   RestoreDefaultFunction(QDesignerFormWindowInterface *formWindow)
      : m_formWindow(formWindow)
   { }

   PropertyHelper::Value operator()(PropertyHelper &ph) {
      return ph.restoreDefaultValue(m_formWindow);
   }

 private:
   QDesignerFormWindowInterface *m_formWindow;
};

// Iterates over a sequence of PropertyHelpers and applies a function to them.
// The function returns the corrected value which is then set in the property editor.
// Returns a combination of update flags.

template <class PropertyListIterator, class Function>
unsigned changePropertyList(QDesignerFormEditorInterface *core, const QString &propertyName,
      PropertyListIterator begin, PropertyListIterator end, Function function)
{
   unsigned updateMask = 0;

   QDesignerPropertyEditorInterface *propertyEditor = core->propertyEditor();
   bool updatedPropertyEditor = false;

   for (PropertyListIterator iter = begin; iter != end; ++iter) {
      PropertyHelper *ph = iter->data();

      if (QObject *object = ph->object()) {
         // Might have been deleted in the meantime
         const PropertyHelper::Value newValue = function(*ph);
         updateMask |= ph->updateMask();

         // Update property editor if it is the current object
         if (! updatedPropertyEditor && propertyEditor && object == propertyEditor->object()) {
            propertyEditor->setPropertyValue(propertyName, newValue.first, newValue.second);
            updatedPropertyEditor = true;
         }
      }
   }

   if (! updatedPropertyEditor) {
      updateMask |=  PropertyHelper::UpdatePropertyEditor;
   }

   return updateMask;
}

// set a new value, return update mask
unsigned PropertyListCommand::setValue(QVariant value, bool changed, unsigned subPropertyMask)
{
   return changePropertyList(formWindow()->core(),
         m_propertyDescription.m_propertyName,
         m_propertyHelperList.begin(), m_propertyHelperList.end(),
         SetValueFunction(formWindow(), PropertyHelper::Value(value, changed), subPropertyMask));
}

// restore old value, return update mask
unsigned PropertyListCommand::restoreOldValue()
{
   return changePropertyList(formWindow()->core(),
         m_propertyDescription.m_propertyName, m_propertyHelperList.begin(), m_propertyHelperList.end(),
         UndoSetValueFunction(formWindow()));
}

// set default value and return update mask
unsigned PropertyListCommand::restoreDefaultValue()
{
   return changePropertyList(formWindow()->core(),
         m_propertyDescription.m_propertyName, m_propertyHelperList.begin(), m_propertyHelperList.end(),
         RestoreDefaultFunction(formWindow()));
}

void PropertyListCommand::update(unsigned updateMask)
{
   if (updateMask & PropertyHelper::UpdateObjectInspector) {
      if (QDesignerObjectInspectorInterface *oi = formWindow()->core()->objectInspector()) {
         oi->setFormWindow(formWindow());
      }
   }

   if (updateMask & PropertyHelper::UpdatePropertyEditor) {
      if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
         propertyEditor->setObject(propertyEditor->object());
      }
   }
}

void PropertyListCommand::undo()
{
   update(restoreOldValue());
   QDesignerPropertyEditor *designerPropertyEditor = dynamic_cast<QDesignerPropertyEditor *>(core()->propertyEditor());

   if (designerPropertyEditor) {
      designerPropertyEditor->updatePropertySheet();
   }
}

// check if lists are aequivalent for command merging (same widgets and props)
bool PropertyListCommand::canMergeLists(const PropertyHelperList &other) const
{
   if (m_propertyHelperList.size() != other.size()) {
      return false;
   }

   for (int i = 0; i < m_propertyHelperList.size(); i++) {
      if (!m_propertyHelperList.at(i)->canMerge(*other.at(i))) {
         return false;
      }
   }

   return true;
}

// ---- SetPropertyCommand ----
SetPropertyCommand::SetPropertyCommand(QDesignerFormWindowInterface *formWindow,
   QUndoCommand *parent)
   :  PropertyListCommand(formWindow, parent), m_subPropertyMask(SubPropertyAll)
{
}

bool SetPropertyCommand::init(QObject *object, const QString &apropertyName, const QVariant &newValue)
{
   Q_ASSERT(object != nullptr);

   m_newValue = newValue;
   propertyHelperList().clear();

   if (! add(object, apropertyName)) {
      return false;
   }

   setDescription();

   return true;
}

bool SetPropertyCommand::init(const ObjectList &list, const QString &apropertyName, const QVariant &newValue,
   QObject *referenceObject, bool enableSubPropertyHandling)
{
   if (! initList(list, apropertyName, referenceObject)) {
      return false;
   }

   m_newValue = newValue;

   setDescription();

   if (enableSubPropertyHandling) {
      m_subPropertyMask = subPropertyMask(newValue, referenceObject);
   }

   return true;
}

unsigned SetPropertyCommand::subPropertyMask(const QVariant &newValue, QObject *referenceObject)
{
   // figure out the mask of changed sub properties when comparing newValue to the current value of the reference object.
   if (!referenceObject) {
      return SubPropertyAll;
   }

   QDesignerPropertySheetExtension *sheet = propertySheet(referenceObject);
   Q_ASSERT(sheet);

   const int index = sheet->indexOf(propertyName());
   if (index == -1 || !sheet->isVisible(index)) {
      return SubPropertyAll;
   }

   return compareSubProperties(sheet->property(index), newValue, specialProperty());
}

void SetPropertyCommand::setDescription()
{
   if (propertyHelperList().size() == 1) {
      setText(QApplication::translate("Command",
            "Changed '%1' of '%2'").formatArg(propertyName())
            .formatArg(propertyHelperList().at(0)->object()->objectName()));

   } else {
      int count = propertyHelperList().size();
      setText(QCoreApplication::translate("Command", "Changed '%1' of %n objects", "", count)
            .formatArg(propertyName()));
   }
}

void SetPropertyCommand::redo()
{
   update(setValue(m_newValue, true, m_subPropertyMask));

   QDesignerPropertyEditor *designerPropertyEditor =
         dynamic_cast<QDesignerPropertyEditor *>(core()->propertyEditor());

   if (designerPropertyEditor) {
      designerPropertyEditor->updatePropertySheet();
   }
}

int SetPropertyCommand::id() const
{
   return 1976;
}

QVariant SetPropertyCommand::mergeValue(const QVariant &newValue)
{
   return newValue;
}

bool SetPropertyCommand::mergeWith(const QUndoCommand *other)
{
   if (id() != other->id() || !formWindow()->isDirty()) {
      return false;
   }

   // Merging: When  for example when the user types ahead in an inplace-editor,
   // it makes sense to merge all the generated commands containing the one-character changes.
   // In the case of subproperties, if the user changes the font size from 10 to 30 via 20
   // and then changes to bold, it makes sense to merge the font size commands only.
   // This is why the m_subPropertyMask is checked.

   const SetPropertyCommand *cmd = static_cast<const SetPropertyCommand *>(other);
   if (! propertyDescription().equals(cmd->propertyDescription()) ||
      m_subPropertyMask  != cmd->m_subPropertyMask || ! canMergeLists(cmd->propertyHelperList())) {

      return false;
   }

   const QVariant newValue = mergeValue(cmd->newValue());
   if (! newValue.isValid()) {
      return false;
   }

   m_newValue = newValue;
   m_subPropertyMask |= cmd->m_subPropertyMask;

   return true;
}

ResetPropertyCommand::ResetPropertyCommand(QDesignerFormWindowInterface *formWindow)
   : PropertyListCommand(formWindow)
{
}

bool ResetPropertyCommand::init(QObject *object, const QString &apropertyName)
{
   Q_ASSERT(object);

   propertyHelperList().clear();

   if (! add(object, apropertyName)) {
      return false;
   }

   setDescription();

   return true;
}

bool ResetPropertyCommand::init(const ObjectList &list, const QString &apropertyName, QObject *referenceObject)
{
   ObjectList modifiedList = list; // filter out modified properties

   for (ObjectList::iterator it = modifiedList.begin(); it != modifiedList.end() ; ) {
      QDesignerPropertySheetExtension *sheet = propertySheet(*it);

      Q_ASSERT(sheet);

      const int index = sheet->indexOf(apropertyName);

      if (index == -1 || !sheet->isChanged(index)) {
         it = modifiedList.erase(it);
      } else {
         ++it;
      }
   }

   if (!modifiedList.contains(referenceObject)) {
      referenceObject = nullptr;
   }

   if (modifiedList.isEmpty() || !initList(modifiedList, apropertyName, referenceObject)) {
      return false;
   }

   setDescription();

   return true;
}

void ResetPropertyCommand::setDescription()
{
   if (propertyHelperList().size() == 1) {
      setText(QCoreApplication::translate("Command",
            "Reset '%1' of '%2'").formatArg(propertyName()).formatArg(propertyHelperList().at(0)->object()->objectName()));
   } else {
      int count = propertyHelperList().size();
      setText(QCoreApplication::translate("Command", "Reset '%1' of %n objects", "", count).formatArg(propertyName()));
   }
}

void ResetPropertyCommand::redo()
{
   update(restoreDefaultValue());
   QDesignerPropertyEditor *designerPropertyEditor = dynamic_cast<QDesignerPropertyEditor *>(core()->propertyEditor());

   if (designerPropertyEditor) {
      designerPropertyEditor->updatePropertySheet();
   }
}

AddDynamicPropertyCommand::AddDynamicPropertyCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QString(), formWindow)
{
}

bool AddDynamicPropertyCommand::init(const QList<QObject *> &selection, QObject *current,
   const QString &propertyName, const QVariant &value)
{
   Q_ASSERT(current);
   m_propertyName = propertyName;

   QDesignerFormEditorInterface *core = formWindow()->core();
   QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension *>(core->extensionManager(),
         current);
   Q_ASSERT(dynamicSheet);

   m_selection.clear();

   if (!value.isValid()) {
      return false;
   }

   if (!dynamicSheet->canAddDynamicProperty(m_propertyName)) {
      return false;
   }

   m_selection.append(current);

   m_value = value;

   QListIterator<QObject *> it(selection);
   while (it.hasNext()) {
      QObject *obj = it.next();
      if (m_selection.contains(obj)) {
         continue;
      }
      dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension *>(core->extensionManager(), obj);
      Q_ASSERT(dynamicSheet);
      if (dynamicSheet->canAddDynamicProperty(m_propertyName)) {
         m_selection.append(obj);
      }
   }

   setDescription();
   return true;
}

void AddDynamicPropertyCommand::redo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   QListIterator<QObject *> it(m_selection);

   while (it.hasNext()) {
      QObject *obj = it.next();
      QDesignerDynamicPropertySheetExtension *dynamicSheet =
            qt_extension<QDesignerDynamicPropertySheetExtension *>(core->extensionManager(), obj);

      dynamicSheet->addDynamicProperty(m_propertyName, m_value);

      if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
         if (propertyEditor->object() == obj) {
            propertyEditor->setObject(obj);
         }
      }
   }
}

void AddDynamicPropertyCommand::undo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   QListIterator<QObject *> it(m_selection);

   while (it.hasNext()) {
      QObject *obj = it.next();
      QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), obj);

      QDesignerDynamicPropertySheetExtension *dynamicSheet =
               qt_extension<QDesignerDynamicPropertySheetExtension *>(core->extensionManager(), obj);

      dynamicSheet->removeDynamicProperty(sheet->indexOf(m_propertyName));

      if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
         if (propertyEditor->object() == obj) {
            propertyEditor->setObject(obj);
         }
      }
   }
}

void AddDynamicPropertyCommand::setDescription()
{
   if (m_selection.size() == 1) {
      setText(QApplication::translate("Command",
            "Add dynamic property '%1' to '%2'").formatArg(m_propertyName).formatArg(m_selection.first()->objectName()));
   } else {
      int count = m_selection.size();
      setText(QCoreApplication::translate("Command", "Add dynamic property '%1' to %n objects", "", count).formatArg(m_propertyName));
   }
}

RemoveDynamicPropertyCommand::RemoveDynamicPropertyCommand(QDesignerFormWindowInterface *formWindow)
   : QDesignerFormWindowCommand(QString(), formWindow)
{
}

bool RemoveDynamicPropertyCommand::init(const QList<QObject *> &selection, QObject *current,
   const QString &propertyName)
{
   Q_ASSERT(current);
   m_propertyName = propertyName;

   QDesignerFormEditorInterface *core = formWindow()->core();
   QDesignerPropertySheetExtension *propertySheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), current);
   Q_ASSERT(propertySheet);

   QDesignerDynamicPropertySheetExtension *dynamicSheet =
            qt_extension<QDesignerDynamicPropertySheetExtension *>(core->extensionManager(), current);
   Q_ASSERT(dynamicSheet);

   m_objectToValueAndChanged.clear();

   const int index = propertySheet->indexOf(m_propertyName);
   if (! dynamicSheet->isDynamicProperty(index)) {
      return false;
   }

   m_objectToValueAndChanged[current] = qMakePair(propertySheet->property(index), propertySheet->isChanged(index));

   QListIterator<QObject *> it(selection);
   while (it.hasNext()) {
      QObject *obj = it.next();
      if (m_objectToValueAndChanged.contains(obj)) {
         continue;
      }

      propertySheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), obj);
      dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension *>(core->extensionManager(), obj);
      const int idx = propertySheet->indexOf(m_propertyName);

      if (dynamicSheet->isDynamicProperty(idx)) {
         m_objectToValueAndChanged[obj] = qMakePair(propertySheet->property(idx), propertySheet->isChanged(idx));
      }
   }

   setDescription();
   return true;
}

void RemoveDynamicPropertyCommand::redo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   auto it = m_objectToValueAndChanged.constBegin();

   while (it != m_objectToValueAndChanged.constEnd()) {
      QObject *obj = it.key();
      QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension *>(core->extensionManager(),
            obj);
      QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), obj);
      dynamicSheet->removeDynamicProperty(sheet->indexOf(m_propertyName));
      if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
         if (propertyEditor->object() == obj) {
            propertyEditor->setObject(obj);
         }
      }
      ++it;
   }
}

void RemoveDynamicPropertyCommand::undo()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   auto it = m_objectToValueAndChanged.constBegin();

   while (it != m_objectToValueAndChanged.constEnd()) {
      QObject *obj = it.key();
      QDesignerPropertySheetExtension *propertySheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), obj);
      QDesignerDynamicPropertySheetExtension *dynamicSheet = qt_extension<QDesignerDynamicPropertySheetExtension *>(core->extensionManager(),
            obj);
      const int index = dynamicSheet->addDynamicProperty(m_propertyName, it.value().first);
      propertySheet->setChanged(index, it.value().second);
      if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
         if (propertyEditor->object() == obj) {
            propertyEditor->setObject(obj);
         }
      }
      ++it;
   }
}

void RemoveDynamicPropertyCommand::setDescription()
{
   if (m_objectToValueAndChanged.size() == 1) {
      setText(QApplication::translate("Command",
            "Remove dynamic property '%1' from '%2'").formatArg(m_propertyName).formatArg(
            m_objectToValueAndChanged.constBegin().key()->objectName()));
   } else {
      int count = m_objectToValueAndChanged.size();
      setText(QApplication::translate("Command", "Remove dynamic property '%1' from %n objects", "", count).formatArg(m_propertyName));
   }
}

} // namespace qdesigner_internal

