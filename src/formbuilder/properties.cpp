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

#include <abstract_formbuilder.h>
#include <formbuilderextra.h>
#include <properties.h>
#include <resourcebuilder.h>
#include <ui4.h>
#include <utils.h>

#include <QAbstractScrollArea>
#include <QDateTime>
#include <QFont>
#include <QFrame>
#include <QIcon>
#include <QPixmap>
#include <QUrl>

static inline void fixEnum(QString &s)
{
   int qualifierIndex = s.lastIndexOf(QLatin1Char(':'));

   if (qualifierIndex == -1) {
      qualifierIndex = s.lastIndexOf(QLatin1Char('.'));
   }

   if (qualifierIndex != -1) {
      s.remove(0, qualifierIndex + 1);
   }
}

// Convert complex DOM types with the help of  QAbstractFormBuilder
QVariant domPropertyToVariant(QAbstractFormBuilder *afb, const QMetaObject &meta, const  DomProperty *p)
{
   // Complex types that need functions from QAbstractFormBuilder
   switch (p->kind()) {

      case DomProperty::String: {
         const int index = meta.indexOfProperty(p->attributeName());

         if (index != -1 && meta.property(index).type() == QVariant::KeySequence) {
            return QVariant::fromValue(QKeySequence(p->elementString()->text()));
         }
      }
      break;

      case DomProperty::Palette: {
         const DomPalette *dom = p->elementPalette();
         QPalette palette;

         if (dom->elementActive()) {
            afb->setupColorGroup(palette, QPalette::Active, dom->elementActive());
         }

         if (dom->elementInactive()) {
            afb->setupColorGroup(palette, QPalette::Inactive, dom->elementInactive());
         }

         if (dom->elementDisabled()) {
            afb->setupColorGroup(palette, QPalette::Disabled, dom->elementDisabled());
         }

         palette.setCurrentColorGroup(QPalette::Active);
         return QVariant::fromValue(palette);
      }

      case DomProperty::Set: {
         const QString pname = p->attributeName();
         const int index = meta.indexOfProperty(pname);

         if (index == -1) {
            csWarning(QCoreApplication::translate("QFormBuilder",
                  "Flag Property %1 could not be read").formatArg(pname));

            return QVariant();
         }

         const QMetaEnum e = meta.property(index).enumerator();
         Q_ASSERT(e.isFlag() == true);

         return QVariant(e.keysToValue(p->elementSet()));
      }

      case DomProperty::Enum: {
         const QString pname = p->attributeName();
         const int index = meta.indexOfProperty(pname);

         QString enumValue = p->elementEnum();

         // Triggers in case of objects in Designer like Spacer/Line for which properties
         // are serialized using language introspection. On preview these objects are
         // emulated by a work around in the formbuilder (size policy/orientation)

         fixEnum(enumValue);

         if (index == -1) {
            // ### special-casing for Line (QFrame)

            if (meta.className() == "QFrame" && (pname == "orientation")) {
               return QVariant(enumValue == QFormBuilderStrings::instance().horizontalPostFix ? QFrame::HLine : QFrame::VLine);

            } else {
               csWarning(QCoreApplication::translate("QFormBuilder",
                     "The enumeration-type property %1 could not be read.").formatArg(p->attributeName()));
               return QVariant();
            }
         }

         const QMetaEnum e = meta.property(index).enumerator();
         return QVariant(e.keyToValue(enumValue));
      }

      case DomProperty::Brush:
         return QVariant::fromValue(afb->setupBrush(p->elementBrush()));

      default:
         if (afb->resourceBuilder()->isResourceProperty(p)) {
            return afb->resourceBuilder()->loadResource(afb->workingDirectory(), p);
         }

         break;
   }

   // simple type
   return domPropertyToVariant(p);
}

// Convert simple DOM types
QVariant domPropertyToVariant(const DomProperty *p)
{
   switch (p->kind()) {
      case DomProperty::Bool:
         return QVariant(p->elementBool() == QFormBuilderStrings::instance().trueValue);

      case DomProperty::Cstring:
         return QVariant(p->elementCstring().toUtf8());

      case DomProperty::Point: {
         const DomPoint *point = p->elementPoint();
         return QVariant(QPoint(point->elementX(), point->elementY()));
      }

      case DomProperty::PointF: {
         const DomPointF *pointf = p->elementPointF();
         return QVariant(QPointF(pointf->elementX(), pointf->elementY()));
      }

      case DomProperty::Size: {
         const DomSize *size = p->elementSize();
         return QVariant(QSize(size->elementWidth(), size->elementHeight()));
      }

      case DomProperty::SizeF: {
         const DomSizeF *sizef = p->elementSizeF();
         return QVariant(QSizeF(sizef->elementWidth(), sizef->elementHeight()));
      }

      case DomProperty::Rect: {
         const DomRect *rc = p->elementRect();
         const QRect g(rc->elementX(), rc->elementY(), rc->elementWidth(), rc->elementHeight());
         return QVariant(g);
      }

      case DomProperty::RectF: {
         const DomRectF *rcf = p->elementRectF();
         const QRectF g(rcf->elementX(), rcf->elementY(), rcf->elementWidth(), rcf->elementHeight());
         return QVariant(g);
      }

      case DomProperty::String:
         return QVariant(p->elementString()->text());

      case DomProperty::Number:
         return QVariant(p->elementNumber());

      case DomProperty::UInt:
         return QVariant(p->elementUInt());

      case DomProperty::LongLong:
         return QVariant(p->elementLongLong());

      case DomProperty::ULongLong:
         return QVariant(p->elementULongLong());

      case DomProperty::Double:
         return QVariant(p->elementDouble());

      case DomProperty::Char: {
         const DomChar *character = p->elementChar();
         const QChar c(character->elementUnicode());
         return QVariant::fromValue(c);
      }

      case DomProperty::Color: {
         const DomColor *color = p->elementColor();
         QColor c(color->elementRed(), color->elementGreen(), color->elementBlue());
         if (color->hasAttributeAlpha()) {
            c.setAlpha(color->attributeAlpha());
         }
         return QVariant::fromValue(c);
      }

      case DomProperty::Font: {
         const DomFont *font = p->elementFont();

         QFont f;
         if (font->hasElementFamily() && !font->elementFamily().isEmpty()) {
            f.setFamily(font->elementFamily());
         }
         if (font->hasElementPointSize() && font->elementPointSize() > 0) {
            f.setPointSize(font->elementPointSize());
         }
         if (font->hasElementWeight() && font->elementWeight() > 0) {
            f.setWeight(font->elementWeight());
         }
         if (font->hasElementItalic()) {
            f.setItalic(font->elementItalic());
         }
         if (font->hasElementBold()) {
            f.setBold(font->elementBold());
         }
         if (font->hasElementUnderline()) {
            f.setUnderline(font->elementUnderline());
         }
         if (font->hasElementStrikeOut()) {
            f.setStrikeOut(font->elementStrikeOut());
         }
         if (font->hasElementKerning()) {
            f.setKerning(font->elementKerning());
         }
         if (font->hasElementAntialiasing()) {
            f.setStyleStrategy(font->elementAntialiasing() ? QFont::PreferDefault : QFont::NoAntialias);
         }
         if (font->hasElementStyleStrategy()) {
            f.setStyleStrategy(enumKeyOfObjectToValue<QAbstractFormBuilderGadget, QFont::StyleStrategy>("styleStrategy",
                  font->elementStyleStrategy().toLatin1()));
         }
         return QVariant::fromValue(f);
      }

      case DomProperty::Date: {
         const DomDate *date = p->elementDate();
         return QVariant(QDate(date->elementYear(), date->elementMonth(), date->elementDay()));
      }

      case DomProperty::Time: {
         const DomTime *t = p->elementTime();
         return QVariant(QTime(t->elementHour(), t->elementMinute(), t->elementSecond()));
      }

      case DomProperty::DateTime: {
         const DomDateTime *dateTime = p->elementDateTime();
         const QDate d(dateTime->elementYear(), dateTime->elementMonth(), dateTime->elementDay());
         const QTime tm(dateTime->elementHour(), dateTime->elementMinute(), dateTime->elementSecond());
         return QVariant(QDateTime(d, tm));
      }

      case DomProperty::Url: {
         const DomUrl *url = p->elementUrl();
         return QVariant(QUrl(url->elementString()->text()));
      }

      case DomProperty::Cursor:
         return QVariant::fromValue(QCursor(static_cast<Qt::CursorShape>(p->elementCursor())));

      case DomProperty::CursorShape:
         return QVariant::fromValue(QCursor(enumKeyOfObjectToValue<QAbstractFormBuilderGadget, Qt::CursorShape>("cursorShape",
                     p->elementCursorShape().toLatin1())));

      case DomProperty::Locale: {
         const DomLocale *locale = p->elementLocale();
         return QVariant::fromValue(QLocale(enumKeyOfObjectToValue<QAbstractFormBuilderGadget, QLocale::Language>("language",
                     locale->attributeLanguage().toLatin1()),
                  enumKeyOfObjectToValue<QAbstractFormBuilderGadget, QLocale::Country>("country", locale->attributeCountry().toLatin1())));
      }

      case DomProperty::SizePolicy: {
         const DomSizePolicy *sizep = p->elementSizePolicy();

         QSizePolicy sizePolicy;
         sizePolicy.setHorizontalStretch(sizep->elementHorStretch());
         sizePolicy.setVerticalStretch(sizep->elementVerStretch());

         const QMetaEnum sizeType_enum = metaEnum<QAbstractFormBuilderGadget>("sizeType");

         if (sizep->hasElementHSizeType()) {
            sizePolicy.setHorizontalPolicy((QSizePolicy::Policy) sizep->elementHSizeType());

         } else if (sizep->hasAttributeHSizeType()) {
            const QSizePolicy::Policy sp = enumKeyToValue<QSizePolicy::Policy>(sizeType_enum, sizep->attributeHSizeType().toLatin1());
            sizePolicy.setHorizontalPolicy(sp);
         }

         if (sizep->hasElementVSizeType()) {
            sizePolicy.setVerticalPolicy((QSizePolicy::Policy) sizep->elementVSizeType());

         } else if (sizep->hasAttributeVSizeType()) {
            const  QSizePolicy::Policy sp = enumKeyToValue<QSizePolicy::Policy>(sizeType_enum, sizep->attributeVSizeType().toLatin1());
            sizePolicy.setVerticalPolicy(sp);
         }

         return QVariant::fromValue(sizePolicy);
      }

      case DomProperty::StringList:
         return QVariant(p->elementStringList()->elementString());

      default:
         csWarning(QCoreApplication::translate("QFormBuilder",
               "Reading properties of the type %1 is not supported yet.").formatArg(p->kind()));
         break;
   }

   return QVariant();
}

// Apply a simple variant type to a DOM property
static bool applySimpleProperty(const QVariant &v, bool translateString, DomProperty *dom_prop)
{
   switch (v.type()) {
      case QVariant::String: {
         DomString *str = new DomString();
         str->setText(v.toString());
         if (!translateString) {
            str->setAttributeNotr(QString("true"));
         }
         dom_prop->setElementString(str);
      }
      return true;

      case QVariant::ByteArray:
         dom_prop->setElementCstring(QString::fromUtf8(v.toByteArray()));
         return true;

      case QVariant::Int:
         dom_prop->setElementNumber(v.toInt());
         return true;

      case QVariant::UInt:
         dom_prop->setElementUInt(v.toUInt());
         return true;

      case QVariant::LongLong:
         dom_prop->setElementLongLong(v.toLongLong());
         return true;

      case QVariant::ULongLong:
         dom_prop->setElementULongLong(v.toULongLong());
         return true;

      case QVariant::Double:
         dom_prop->setElementDouble(v.toDouble());
         return true;

      case QVariant::Bool:
         dom_prop->setElementBool(v.toBool() ? QFormBuilderStrings::instance().trueValue : QFormBuilderStrings::instance().falseValue);
         return true;

      case QVariant::Char: {
         DomChar *ch = new DomChar();
         const QChar character = v.toChar();
         ch->setElementUnicode(character.unicode());
         dom_prop->setElementChar(ch);
      }
      return true;

      case QVariant::Point: {
         DomPoint *pt = new DomPoint();
         const QPoint point = v.toPoint();
         pt->setElementX(point.x());
         pt->setElementY(point.y());
         dom_prop->setElementPoint(pt);
      }
      return true;

      case QVariant::PointF: {
         DomPointF *ptf = new DomPointF();
         const QPointF pointf = v.toPointF();
         ptf->setElementX(pointf.x());
         ptf->setElementY(pointf.y());
         dom_prop->setElementPointF(ptf);
      }
      return true;

      case QVariant::Color: {
         DomColor *clr = new DomColor();
         const QColor color = v.value<QColor>();
         clr->setElementRed(color.red());
         clr->setElementGreen(color.green());
         clr->setElementBlue(color.blue());

         const int alphaChannel = color.alpha();

         if (alphaChannel != 255) {
            clr->setAttributeAlpha(alphaChannel);
         }

         dom_prop->setElementColor(clr);
      }
      return true;

      case QVariant::Size: {
         DomSize *sz = new DomSize();
         const QSize size = v.toSize();
         sz->setElementWidth(size.width());
         sz->setElementHeight(size.height());
         dom_prop->setElementSize(sz);
      }
      return true;

      case QVariant::SizeF: {
         DomSizeF *szf = new DomSizeF();
         const QSizeF sizef = v.toSizeF();
         szf->setElementWidth(sizef.width());
         szf->setElementHeight(sizef.height());
         dom_prop->setElementSizeF(szf);
      }
      return true;

      case QVariant::Rect: {
         DomRect *rc = new DomRect();
         const QRect rect = v.toRect();
         rc->setElementX(rect.x());
         rc->setElementY(rect.y());
         rc->setElementWidth(rect.width());
         rc->setElementHeight(rect.height());
         dom_prop->setElementRect(rc);
      }
      return true;

      case QVariant::RectF: {
         DomRectF *rcf = new DomRectF();

         const QRectF rectf = v.toRectF();
         rcf->setElementX(rectf.x());
         rcf->setElementY(rectf.y());
         rcf->setElementWidth(rectf.width());
         rcf->setElementHeight(rectf.height());
         dom_prop->setElementRectF(rcf);
      }
      return true;

      case QVariant::Font: {
         DomFont *fnt = new DomFont();

         const QFont font = v.value<QFont>();
         const uint mask = font.resolve();

         if (mask & QFont::WeightResolved) {
            fnt->setElementBold(font.bold());
            fnt->setElementWeight(font.weight());
         }

         if (mask & QFont::FamilyResolved) {
            fnt->setElementFamily(font.family());
         }
         if (mask & QFont::StyleResolved) {
            fnt->setElementItalic(font.italic());
         }
         if (mask & QFont::SizeResolved) {
            fnt->setElementPointSize(font.pointSize());
         }
         if (mask & QFont::StrikeOutResolved) {
            fnt->setElementStrikeOut(font.strikeOut());
         }
         if (mask & QFont::UnderlineResolved) {
            fnt->setElementUnderline(font.underline());
         }
         if (mask & QFont::KerningResolved) {
            fnt->setElementKerning(font.kerning());
         }
         if (mask & QFont::StyleStrategyResolved) {
            const QMetaEnum styleStrategy_enum = metaEnum<QAbstractFormBuilderGadget>("styleStrategy");
            fnt->setElementStyleStrategy(styleStrategy_enum.valueToKey(font.styleStrategy()));
         }
         dom_prop->setElementFont(fnt);
      }

      return true;

      case QVariant::Cursor: {
         const QMetaEnum cursorShape_enum = metaEnum<QAbstractFormBuilderGadget>("cursorShape");
         dom_prop->setElementCursorShape(cursorShape_enum.valueToKey(v.value<QCursor>().shape()));
      }

      return true;

      case QVariant::KeySequence: {
         DomString *s = new DomString();
         s->setText(v.value<QKeySequence>().toString(QKeySequence::PortableText));
         dom_prop->setElementString(s);
      }

      return true;

      case QVariant::Locale: {
         DomLocale *dom = new DomLocale();
         const QLocale locale = v.value<QLocale>();

         const QMetaEnum language_enum = metaEnum<QAbstractFormBuilderGadget>("language");
         const QMetaEnum country_enum  = metaEnum<QAbstractFormBuilderGadget>("country");

         dom->setAttributeLanguage(language_enum.valueToKey(locale.language()));
         dom->setAttributeCountry(country_enum.valueToKey(locale.country()));

         dom_prop->setElementLocale(dom);
      }
      return true;

      case QVariant::SizePolicy: {
         DomSizePolicy *dom = new DomSizePolicy();
         const QSizePolicy sizePolicy = v.value<QSizePolicy>();

         dom->setElementHorStretch(sizePolicy.horizontalStretch());
         dom->setElementVerStretch(sizePolicy.verticalStretch());

         const QMetaEnum sizeType_enum = metaEnum<QAbstractFormBuilderGadget>("sizeType");

         dom->setAttributeHSizeType(sizeType_enum.valueToKey(sizePolicy.horizontalPolicy()));
         dom->setAttributeVSizeType(sizeType_enum.valueToKey(sizePolicy.verticalPolicy()));

         dom_prop->setElementSizePolicy(dom);
      }
      return true;

      case QVariant::Date: {
         DomDate *dom = new DomDate();
         const QDate date = v.value<QDate>();

         dom->setElementYear(date.year());
         dom->setElementMonth(date.month());
         dom->setElementDay(date.day());

         dom_prop->setElementDate(dom);
      }
      return true;

      case QVariant::Time: {
         DomTime *dom = new DomTime();
         const QTime time = v.value<QTime>();

         dom->setElementHour(time.hour());
         dom->setElementMinute(time.minute());
         dom->setElementSecond(time.second());

         dom_prop->setElementTime(dom);
      }
      return true;

      case QVariant::DateTime: {
         DomDateTime *dom = new DomDateTime();
         const QDateTime dateTime = v.value<QDateTime>();

         dom->setElementHour(dateTime.time().hour());
         dom->setElementMinute(dateTime.time().minute());
         dom->setElementSecond(dateTime.time().second());
         dom->setElementYear(dateTime.date().year());
         dom->setElementMonth(dateTime.date().month());
         dom->setElementDay(dateTime.date().day());

         dom_prop->setElementDateTime(dom);
      }
      return true;

      case QVariant::Url: {
         DomUrl *dom = new DomUrl();
         const QUrl url = v.toUrl();

         DomString *str = new DomString();
         str->setText(url.toString());
         dom->setElementString(str);

         dom_prop->setElementUrl(dom);
      }
      return true;

      case QVariant::StringList: {
         DomStringList *sl = new DomStringList;
         sl->setElementString(v.value<QStringList>());
         dom_prop->setElementStringList(sl);
      }
      return true;

      default:
         break;
   }

   return false;
}
static QString msgCannotWriteProperty(const QString &pname, const QVariant &v)
{
   return QCoreApplication::translate("QFormBuilder", "The property %1 could not be written. The type %2 is not supported yet.")
      .formatArg(pname).formatArg(v.typeName());
}

static bool isOfType(const QMetaObject &metaObj, const QMetaObject &metaParent)
{
   const QMetaObject *metaChild = &metaObj;

   while (metaChild != nullptr) {
      if (compareMetaObject(*metaChild, metaParent)) {
         return true;
      }

      metaChild = metaChild->superClass();
   }

   return false;
}

static bool isTranslatable(const QString &pname, const QVariant &v, const QMetaObject &meta)
{
   const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

   if (pname == strings.objectNameProperty) {
      return false;
   }

   if (pname == strings.styleSheetProperty && v.type() == QVariant::String && isOfType(meta, QWidget::staticMetaObject())) {
      return false;
   }

   return true;
}

// Convert complex variant types to DOM properties with the help of  QAbstractFormBuilder
// Does not perform a check using  QAbstractFormBuilder::checkProperty().
DomProperty *variantToDomProperty(QAbstractFormBuilder *afb, const QMetaObject &meta,
   const QString &pname, const QVariant &v)
{
   const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

   DomProperty *dom_prop = new DomProperty();
   dom_prop->setAttributeName(pname);

   const int pindex = meta.indexOfProperty(pname.toLatin1());

   if (pindex != -1) {
      QMetaProperty meta_property = meta.property(pindex);

      if ((v.type() == QVariant::Int || v.type() == QVariant::UInt) && meta_property.isEnumType()) {
         const QMetaEnum e = meta_property.enumerator();

         if (e.isFlag()) {
            dom_prop->setElementSet(e.valueToKeys(v.toInt()));
         } else {
            dom_prop->setElementEnum(e.valueToKey(v.toInt()));
         }

         return dom_prop;
      }

      if (! meta_property.hasStdCppSet() || (isOfType(meta, QAbstractScrollArea::staticMetaObject()) &&
            pname == strings.cursorProperty)) {
         dom_prop->setAttributeStdset(0);
      }
   }

   // Try simple properties
   if (applySimpleProperty(v, isTranslatable(pname, v, meta), dom_prop)) {
      return dom_prop;
   }

   // Complex properties
   switch (v.type()) {
      case QVariant::Palette: {
         DomPalette *dom = new DomPalette();
         QPalette palette = v.value<QPalette>();

         palette.setCurrentColorGroup(QPalette::Active);
         dom->setElementActive(afb->saveColorGroup(palette));

         palette.setCurrentColorGroup(QPalette::Inactive);
         dom->setElementInactive(afb->saveColorGroup(palette));

         palette.setCurrentColorGroup(QPalette::Disabled);
         dom->setElementDisabled(afb->saveColorGroup(palette));

         dom_prop->setElementPalette(dom);
      }
      break;

      case QVariant::Brush:
         dom_prop->setElementBrush(afb->saveBrush(v.value<QBrush>()));
         break;

      default: {
         const bool hadAttributeStdset = dom_prop->hasAttributeStdset();
         const bool attributeStdset    = dom_prop->attributeStdset();
         delete dom_prop;

         if (afb->resourceBuilder()->isResourceType(v)) {
            dom_prop = afb->resourceBuilder()->saveResource(afb->workingDirectory(), v);
            if (dom_prop) {
               dom_prop->setAttributeName(pname);
               if (hadAttributeStdset) {
                  dom_prop->setAttributeStdset(attributeStdset);
               }
            }
            break;
         }

         csWarning(msgCannotWriteProperty(pname, v));
      }
      return nullptr;
   }

   return dom_prop;
}
