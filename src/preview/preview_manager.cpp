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
#include <designer_formbuilder.h>
#include <designer_settings.h>
#include <preview_manager.h>
#include <widgetfactory.h>

//  #include <deviceskin.h>

#include <formwindowbase_p.h>
#include <formwindowbase_p.h>
#include <shared_settings_p.h>
#include <shared_settings_p.h>
#include <zoomwidget_p.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>
#include <QDialog>
#include <QDockWidget>
#include <QEvent>
#include <QMainWindow>
#include <QMap>
#include <QMatrix>
#include <QMenu>
#include <QPixmap>
#include <QSharedData>
#include <QVBoxLayout>
#include <QWidget>

static inline int compare(const qdesigner_internal::PreviewConfiguration &pc1, const qdesigner_internal::PreviewConfiguration &pc2)
{
   int rc = pc1.style().compare(pc2.style());

   if (rc) {
      return rc;
   }

   rc = pc1.applicationStyleSheet().compare(pc2.applicationStyleSheet());

   if (rc) {
      return rc;
   }

   return pc1.deviceSkin().compare(pc2.deviceSkin());
}

namespace {

struct PreviewData {
   PreviewData(const QPointer<QWidget> &widget, const  QDesignerFormWindowInterface *formWindow,
      const qdesigner_internal::PreviewConfiguration &pc);

   QPointer<QWidget> m_widget;
   const QDesignerFormWindowInterface *m_formWindow;
   qdesigner_internal::PreviewConfiguration m_configuration;
};

PreviewData::PreviewData(const QPointer<QWidget> &widget, const QDesignerFormWindowInterface *formWindow,
      const qdesigner_internal::PreviewConfiguration &pc)
   : m_widget(widget), m_formWindow(formWindow), m_configuration(pc)
{
}
}

namespace qdesigner_internal {

/* In designer, we have the situation that laid-out maincontainers have
 * a geometry set (which might differ from their sizeHint()). The QGraphicsItem
 * should return that in its size hint, else such cases won't work */

class DesignerZoomProxyWidget : public ZoomProxyWidget
{
   Q_DISABLE_COPY(DesignerZoomProxyWidget)

 public:
   DesignerZoomProxyWidget(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = Qt::EmptyFlag);

 protected:
   QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF() ) const override;
};

DesignerZoomProxyWidget::DesignerZoomProxyWidget(QGraphicsItem *parent, Qt::WindowFlags wFlags) :
   ZoomProxyWidget(parent, wFlags)
{
}

QSizeF DesignerZoomProxyWidget::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
   if (const QWidget *w = widget()) {
      return QSizeF(w->size());
   }

   return ZoomProxyWidget::sizeHint(which, constraint);
}

// DesignerZoomWidget which returns DesignerZoomProxyWidget in its factory function
class DesignerZoomWidget : public ZoomWidget
{
   Q_DISABLE_COPY(DesignerZoomWidget)

 public:
   DesignerZoomWidget(QWidget *parent = nullptr);

 private:
   QGraphicsProxyWidget *createProxyWidget(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = Qt::EmptyFlag) const override;
};

DesignerZoomWidget::DesignerZoomWidget(QWidget *parent) :
   ZoomWidget(parent)
{
}

QGraphicsProxyWidget *DesignerZoomWidget::createProxyWidget(QGraphicsItem *parent, Qt::WindowFlags wFlags) const
{
   return new DesignerZoomProxyWidget(parent, wFlags);
}

// PreviewDeviceSkin: Forwards the key events to the window and
// provides context menu with rotation options. Derived class
// can apply additional transformations to the skin.

/*  emerald - hold skins

class PreviewDeviceSkin : public DeviceSkin
{
    CS_OBJECT(PreviewDeviceSkin)

public:
    enum Direction { DirectionUp, DirectionLeft,  DirectionRight };

    explicit PreviewDeviceSkin(const DeviceSkinParameters &parameters, QWidget *parent);
    virtual void setPreview(QWidget *w);
    QSize screenSize() const { return  m_screenSize; }

protected:
    virtual void populateContextMenu(QMenu *) {}

    // Fit the widget in case the orientation changes (transposing screensize)
    virtual void fitWidget(const QSize &size);

    //  Calculate the complete transformation for the skin
    // (base class implementation provides rotation).
    virtual QMatrix skinTransform() const;

private:
    CS_SLOT_1(Private, void slotSkinKeyPressEvent(int code,const QString & text,bool autorep))
    CS_SLOT_2(slotSkinKeyPressEvent)

    CS_SLOT_1(Private, void slotSkinKeyReleaseEvent(int code,const QString & text,bool autorep))
    CS_SLOT_2(slotSkinKeyReleaseEvent)

    CS_SLOT_1(Private, void slotPopupMenu())
    CS_SLOT_2(slotPopupMenu)

    CS_SLOT_1(Private, void slotDirection(QAction * un_named_arg1))
    CS_SLOT_2(slotDirection)

    const QSize m_screenSize;
    Direction m_direction;

    QAction *m_directionUpAction;
    QAction *m_directionLeftAction;
    QAction *m_directionRightAction;
    QAction *m_closeAction;
};

PreviewDeviceSkin::PreviewDeviceSkin(const DeviceSkinParameters &parameters, QWidget *parent) :
    DeviceSkin(parameters, parent),
    m_screenSize(parameters.screenSize()),
    m_direction(DirectionUp),
    m_directionUpAction(0),
    m_directionLeftAction(0),
    m_directionRightAction(0),
    m_closeAction(0)
{
    connect(this, &PreviewDeviceSkin::skinKeyPressEvent,
            this, &PreviewDeviceSkin::slotSkinKeyPressEvent);
    connect(this, &PreviewDeviceSkin::skinKeyReleaseEvent,
            this, &PreviewDeviceSkin::slotSkinKeyReleaseEvent);
    connect(this, &PreviewDeviceSkin::popupMenu, this, &PreviewDeviceSkin::slotPopupMenu);
}

void PreviewDeviceSkin::setPreview(QWidget *formWidget)
{
    formWidget->setFixedSize(m_screenSize);
    formWidget->setParent(this, Qt::SubWindow);
    formWidget->setAutoFillBackground(true);
    setView(formWidget);
}

void PreviewDeviceSkin::slotSkinKeyPressEvent(int code, const QString& text, bool autorep)
{
    if (QWidget *focusWidget =  QApplication::focusWidget()) {
        QKeyEvent e(QEvent::KeyPress,code,0,text,autorep);
        QApplication::sendEvent(focusWidget, &e);
    }
}

void PreviewDeviceSkin::slotSkinKeyReleaseEvent(int code, const QString& text, bool autorep)
{
    if (QWidget *focusWidget =  QApplication::focusWidget()) {
        QKeyEvent e(QEvent::KeyRelease,code,0,text,autorep);
        QApplication::sendEvent(focusWidget, &e);
    }
}


// Create a checkable action with integer data and
// set it checked if it matches the currentState.
static inline QAction *createCheckableActionIntData(const QString &label, int actionValue, int currentState,
      QActionGroup *ag, QObject *parent)
{
    QAction *a = new QAction(label, parent);
    a->setData(actionValue);
    a->setCheckable(true);
    if (actionValue == currentState)
        a->setChecked(true);
    ag->addAction(a);
    return a;
}

void PreviewDeviceSkin::slotPopupMenu()
{
    QMenu menu(this);
    // Create actions
    if (!m_directionUpAction) {
        QActionGroup *directionGroup = new QActionGroup(this);
        connect(directionGroup, &QActionGroup::triggered, this, &PreviewDeviceSkin::slotDirection);
        directionGroup->setExclusive(true);
        m_directionUpAction = createCheckableActionIntData(tr("&Portrait"), DirectionUp, m_direction, directionGroup, this);
        //: Rotate form preview counter-clockwise
        m_directionLeftAction = createCheckableActionIntData(tr("Landscape (&CCW)"), DirectionLeft, m_direction, directionGroup, this);
        //: Rotate form preview clockwise
        m_directionRightAction = createCheckableActionIntData(tr("&Landscape (CW)"), DirectionRight, m_direction, directionGroup, this);
        m_closeAction = new QAction(tr("&Close"), this);
        connect(m_closeAction, &QAction::triggered, parentWidget(), &QWidget::close);
    }
    menu.addAction(m_directionUpAction);
    menu.addAction(m_directionLeftAction);
    menu.addAction(m_directionRightAction);
    menu.addSeparator();
    populateContextMenu(&menu);
    menu.addAction(m_closeAction);
    menu.exec(QCursor::pos());
}

void PreviewDeviceSkin::slotDirection(QAction *a)
{
    const Direction newDirection = static_cast<Direction>(a->data().toInt());
    if (m_direction == newDirection)
        return;
    const Qt::Orientation newOrientation = newDirection == DirectionUp ? Qt::Vertical : Qt::Horizontal;
    const Qt::Orientation oldOrientation = m_direction  == DirectionUp ? Qt::Vertical : Qt::Horizontal;
    m_direction = newDirection;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (oldOrientation != newOrientation) {
        QSize size = screenSize();
        if (newOrientation == Qt::Horizontal)
            size.transpose();
        fitWidget(size);
    }
    setTransform(skinTransform());
    QApplication::restoreOverrideCursor();
}

void PreviewDeviceSkin::fitWidget(const QSize &size)
{
    view()->setFixedSize(size);
}

QMatrix PreviewDeviceSkin::skinTransform() const
{
    QMatrix newTransform;
    switch (m_direction)  {
        case DirectionUp:
            break;
        case DirectionLeft:
            newTransform.rotate(270.0);
            break;
        case DirectionRight:
            newTransform.rotate(90.0);
            break;
    }
    return newTransform;
}

*/

// ------------ PreviewConfigurationPrivate
class PreviewConfigurationData : public QSharedData
{
 public:
   PreviewConfigurationData() {}
   explicit PreviewConfigurationData(const QString &style, const QString &applicationStyleSheet, const QString &deviceSkin);

   QString m_style;

   // Style sheet to prepend (to simulate the effect od QApplication::setSyleSheet()).
   QString m_applicationStyleSheet;
   QString m_deviceSkin;
};

PreviewConfigurationData::PreviewConfigurationData(const QString &style, const QString &applicationStyleSheet,
   const QString &deviceSkin) :
   m_style(style),
   m_applicationStyleSheet(applicationStyleSheet),
   m_deviceSkin(deviceSkin)
{
}

/* emerald - hold skins

class ZoomablePreviewDeviceSkin : public PreviewDeviceSkin
{
    CS_OBJECT(ZoomablePreviewDeviceSkin)

public:
    explicit ZoomablePreviewDeviceSkin(const DeviceSkinParameters &parameters, QWidget *parent);
    void setPreview(QWidget *w) override;

    int zoomPercent() const; // Device Skins have a double 'zoom' property

    CS_SLOT_1(Public, void setZoomPercent(int un_named_arg1))
    CS_SLOT_2(setZoomPercent)

    CS_SIGNAL_1(Public, void zoomPercentChanged(int un_named_arg1))
    CS_SIGNAL_2(zoomPercentChanged,un_named_arg1)

protected:
    void populateContextMenu(QMenu *m) override;
    QMatrix skinTransform() const override;
    void fitWidget(const QSize &size) override;

private:
    ZoomMenu *m_zoomMenu;
    QAction *m_zoomSubMenuAction;
    ZoomWidget *m_zoomWidget;
};

ZoomablePreviewDeviceSkin::ZoomablePreviewDeviceSkin(const DeviceSkinParameters &parameters, QWidget *parent) :
    PreviewDeviceSkin(parameters, parent),
    m_zoomMenu(new ZoomMenu(this)),
    m_zoomSubMenuAction(0),
    m_zoomWidget(new DesignerZoomWidget)
{
    connect(m_zoomMenu, &ZoomMenu::zoomChanged, this, &ZoomablePreviewDeviceSkin::setZoomPercent);
    connect(m_zoomMenu, &ZoomMenu::zoomChanged, this, &ZoomablePreviewDeviceSkin::zoomPercentChanged);
    m_zoomWidget->setZoomContextMenuEnabled(false);
    m_zoomWidget->setWidgetZoomContextMenuEnabled(false);
    m_zoomWidget->resize(screenSize());
    m_zoomWidget->setParent(this, Qt::SubWindow);
    m_zoomWidget->setAutoFillBackground(true);
    setView(m_zoomWidget);
}

static inline qreal zoomFactor(int percent)
{
    return qreal(percent) / 100.0;
}

static inline QSize scaleSize(int zoomPercent, const QSize &size)
{
    return zoomPercent == 100 ? size : (QSizeF(size) * zoomFactor(zoomPercent)).toSize();
}

void ZoomablePreviewDeviceSkin::setPreview(QWidget *formWidget)
{
    m_zoomWidget->setWidget(formWidget);
    m_zoomWidget->resize(scaleSize(zoomPercent(), screenSize()));
}

int ZoomablePreviewDeviceSkin::zoomPercent() const
{
    return m_zoomWidget->zoom();
}

void ZoomablePreviewDeviceSkin::setZoomPercent(int zp)
{
    if (zp == zoomPercent())
        return;

    // If not triggered by the menu itself: Update it
    if (m_zoomMenu->zoom() != zp)
        m_zoomMenu->setZoom(zp);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_zoomWidget->setZoom(zp);
    setTransform(skinTransform());
    QApplication::restoreOverrideCursor();
}

void ZoomablePreviewDeviceSkin::populateContextMenu(QMenu *menu)
{
    if (!m_zoomSubMenuAction) {
        m_zoomSubMenuAction = new QAction(tr("&Zoom"), this);
        QMenu *zoomSubMenu = new QMenu;
        m_zoomSubMenuAction->setMenu(zoomSubMenu);
        m_zoomMenu->addActions(zoomSubMenu);
    }
    menu->addAction(m_zoomSubMenuAction);
    menu->addSeparator();
}

QMatrix ZoomablePreviewDeviceSkin::skinTransform() const
{
    // Complete transformation consisting of base class rotation and zoom.
    QMatrix rc = PreviewDeviceSkin::skinTransform();
    const int zp = zoomPercent();
    if (zp != 100) {
        const qreal factor = zoomFactor(zp);
        rc.scale(factor, factor);
    }
    return rc;
}

void ZoomablePreviewDeviceSkin::fitWidget(const QSize &size)
{
    m_zoomWidget->resize(scaleSize(zoomPercent(), size));
}

*/


// ------------- PreviewConfiguration

static const QString appStyleSheetKey = "AppStyleSheet";
static const QString skinKey          = "Skin";
static const QString styleKey         = "Style";

PreviewConfiguration::PreviewConfiguration() :
   m_d(new PreviewConfigurationData)
{
}

PreviewConfiguration::PreviewConfiguration(const QString &sty, const QString &applicationSheet, const QString &skin) :
   m_d(new PreviewConfigurationData(sty, applicationSheet, skin))
{
}

PreviewConfiguration::PreviewConfiguration(const PreviewConfiguration &o) :
   m_d(o.m_d)
{
}

PreviewConfiguration &PreviewConfiguration::operator=(const PreviewConfiguration &o)
{
   m_d.operator = (o.m_d);
   return *this;
}

PreviewConfiguration::~PreviewConfiguration()
{
}

void PreviewConfiguration::clear()
{
   PreviewConfigurationData &d = *m_d;
   d.m_style.clear();
   d.m_applicationStyleSheet.clear();
   d.m_deviceSkin.clear();
}

QString PreviewConfiguration::style() const
{
   return m_d->m_style;
}

void PreviewConfiguration::setStyle(const QString &s)
{
   m_d->m_style = s;
}

// Style sheet to prepend (to simulate the effect od QApplication::setSyleSheet()).
QString PreviewConfiguration::applicationStyleSheet() const
{
   return m_d->m_applicationStyleSheet;
}

void PreviewConfiguration::setApplicationStyleSheet(const QString &as)
{
   m_d->m_applicationStyleSheet = as;
}

QString PreviewConfiguration::deviceSkin() const
{
   return m_d->m_deviceSkin;
}

void PreviewConfiguration::setDeviceSkin(const QString &s)
{
   m_d->m_deviceSkin = s;
}

void PreviewConfiguration::toSettings(const QString &prefix, QDesignerSettingsInterface *settings) const
{
   const PreviewConfigurationData &d = *m_d;
   settings->beginGroup(prefix);

   settings->setValue(styleKey,  d.m_style);
   settings->setValue(appStyleSheetKey, d.m_applicationStyleSheet);
   settings->setValue(skinKey,  d.m_deviceSkin);

   settings->endGroup();
}

void PreviewConfiguration::fromSettings(const QString &prefix, const QDesignerSettingsInterface *settings)
{
   clear();
   QString key = prefix;
   key += QLatin1Char('/');
   const int prefixSize = key.size();

   PreviewConfigurationData &d = *m_d;

   const QVariant emptyString = QVariant(QString());

   key += styleKey;
   d.m_style = settings->value(key, emptyString).toString();

   key.replace(prefixSize, key.size() - prefixSize, appStyleSheetKey);
   d.m_applicationStyleSheet = settings->value(key, emptyString).toString();

   key.replace(prefixSize, key.size() - prefixSize, skinKey);
   d.m_deviceSkin = settings->value(key, emptyString).toString();
}


bool operator<(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2)
{
   return compare(pc1, pc2) < 0;
}

bool operator==(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2)
{
   return compare(pc1, pc2) == 0;
}

bool operator!=(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2)
{
   return compare(pc1, pc2) != 0;
}

// ------------- PreviewManagerPrivate
class PreviewManagerPrivate
{
 public:
   PreviewManagerPrivate(PreviewManager::PreviewMode mode);

   const PreviewManager::PreviewMode m_mode;

   QPointer<QWidget> m_activePreview;

   typedef QList<PreviewData> PreviewDataList;
   PreviewDataList m_previews;

   //    typedef QMap<QString, DeviceSkinParameters> DeviceSkinConfigCache;
   //    DeviceSkinConfigCache m_deviceSkinConfigCache;

   QDesignerFormEditorInterface *m_core;
   bool m_updateBlocked;
};

PreviewManagerPrivate::PreviewManagerPrivate(PreviewManager::PreviewMode mode)
   : m_mode(mode), m_core(nullptr), m_updateBlocked(false)
{
}

// ------------- PreviewManager

PreviewManager::PreviewManager(PreviewMode mode, QObject *parent)
   : QObject(parent), d(new PreviewManagerPrivate(mode))
{
}

PreviewManager:: ~PreviewManager()
{
   delete d;
}


Qt::WindowFlags PreviewManager::previewWindowFlags(const QWidget *widget) const
{
#ifdef Q_OS_WIN
   Qt::WindowFlags windowFlags = (widget->windowType() == Qt::Window) ?
      Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint :
      Qt::WindowFlags(Qt::Dialog);
#else
   (void) widget;

   // Only Dialogs have close buttons on Mac.
   // On Linux, we don't want an additional task bar item and we don't want a minimize button;
   // we want the preview to be on top.
   Qt::WindowFlags windowFlags = Qt::Dialog;
#endif
   return windowFlags;
}

QWidget *PreviewManager::createDeviceSkinContainer(const QDesignerFormWindowInterface *fw) const
{
   return new QDialog(fw->window());
}

// Some widgets might require fake containers
static QWidget *fakeContainer(QWidget *w)
{
   // Prevent a dock widget from trying to dock to Designer's main window
   // (which can be found in the parent hierarchy in MDI mode) by  providing a fake mainwindow

   if (QDockWidget *dock = dynamic_cast<QDockWidget *>(w)) {
      // Reparent: Clear modality, propagate title and resize outer container
      const QSize size = w->size();
      w->setWindowModality(Qt::NonModal);

      dock->setFeatures(dock->features() & ~(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable |
            QDockWidget::DockWidgetClosable));
      dock->setAllowedAreas(Qt::LeftDockWidgetArea);

      QMainWindow *mw = new QMainWindow;
      int leftMargin, topMargin, rightMargin, bottomMargin;
      mw->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
      mw->addDockWidget(Qt::LeftDockWidgetArea, dock);
      mw->resize(size + QSize(leftMargin + rightMargin, topMargin + bottomMargin));
      return mw;
   }

   return w;
}

static PreviewConfiguration configurationFromSettings(QDesignerFormEditorInterface *core, const QString &style)
{
   qdesigner_internal::PreviewConfiguration pc;
   const QDesignerSharedSettings settings(core);

   if (settings.isCustomPreviewConfigurationEnabled()) {
      pc = settings.customPreviewConfiguration();
   }

   if (!style.isEmpty()) {
      pc.setStyle(style);
   }

   return pc;
}

QWidget *PreviewManager::showPreview(const QDesignerFormWindowInterface *fw, const QString &style, int deviceProfileIndex,
   QString *errorMessage)
{
   return showPreview(fw, configurationFromSettings(fw->core(), style), deviceProfileIndex, errorMessage);
}

QWidget *PreviewManager::showPreview(const QDesignerFormWindowInterface *fw, const QString &style, QString *errorMessage)
{
   return showPreview(fw, style, -1, errorMessage);
}

QWidget *PreviewManager::createPreview(const QDesignerFormWindowInterface *fw,
   const PreviewConfiguration &pc,
   int deviceProfileIndex,
   QString *errorMessage,
   int initialZoom)
{
   if (!d->m_core) {
      d->m_core = fw->core();
   }

   const bool zoomable = initialZoom > 0;
   // Figure out which profile to apply
   DeviceProfile deviceProfile;
   if (deviceProfileIndex >= 0) {
      deviceProfile = QDesignerSharedSettings(fw->core()).deviceProfileAt(deviceProfileIndex);
   } else {
      if (const FormWindowBase *fwb = dynamic_cast<const FormWindowBase *>(fw)) {
         deviceProfile = fwb->deviceProfile();
      }
   }
   // Create
   QWidget *formWidget = QDesignerFormBuilder::createPreview(fw, pc.style(), pc.applicationStyleSheet(), deviceProfile, errorMessage);
   if (!formWidget) {
      return nullptr;
   }

   const QString title = tr("%1 - [Preview]").formatArg(formWidget->windowTitle());
   formWidget = fakeContainer(formWidget);
   formWidget->setWindowTitle(title);

   // Clear any modality settings, child widget modalities must not be higher than parent's
   formWidget->setWindowModality(Qt::NonModal);
   // No skin
   const QString deviceSkin = pc.deviceSkin();
   if (deviceSkin.isEmpty()) {
      if (zoomable) { // Embed into ZoomWidget
         ZoomWidget *zw = new DesignerZoomWidget;
         connect(zw->zoomMenu(), &ZoomMenu::zoomChanged, this, &PreviewManager::slotZoomChanged);
         zw->setWindowTitle(title);
         zw->setWidget(formWidget);
         // Keep any widgets' context menus working, do not use global menu
         zw->setWidgetZoomContextMenuEnabled(true);
         zw->setParent(fw->window(), previewWindowFlags(formWidget));
         // Make preview close when Widget closes (Dialog/accept, etc)
         formWidget->setAttribute(Qt::WA_DeleteOnClose, true);
         connect(formWidget, &QObject::destroyed, zw, &QWidget::close);
         zw->setZoom(initialZoom);
         zw->setProperty(WidgetFactory::disableStyleCustomPaintingPropertyC, QVariant(true));
         return zw;
      }
      formWidget->setParent(fw->window(), previewWindowFlags(formWidget));
      formWidget->setProperty(WidgetFactory::disableStyleCustomPaintingPropertyC, QVariant(true));
      return formWidget;
   }

   /*  emerald - hold skins

       // Embed into skin. find config in cache
       PreviewManagerPrivate::DeviceSkinConfigCache::iterator it = d->m_deviceSkinConfigCache.find(deviceSkin);

       if (it == d->m_deviceSkinConfigCache.end()) {
           DeviceSkinParameters parameters;
           if (!parameters.read(deviceSkin, DeviceSkinParameters::ReadAll, errorMessage)) {
               formWidget->deleteLater();
               return nullptr;
             }
           it = d->m_deviceSkinConfigCache.insert(deviceSkin, parameters);
       }

   */

   QWidget *skinContainer = createDeviceSkinContainer(fw);

   /*  emerald - hold skins

       PreviewDeviceSkin *skin = 0;

       if (zoomable) {
           ZoomablePreviewDeviceSkin *zds = new ZoomablePreviewDeviceSkin(it.value(), skinContainer);
           zds->setZoomPercent(initialZoom);
           connect(zds, &ZoomablePreviewDeviceSkin::zoomPercentChanged, this, &PreviewManager::slotZoomChanged);
           skin = zds;
       }  else {
           skin = new PreviewDeviceSkin(it.value(), skinContainer);
       }

       skin->setPreview(formWidget);
   */

   // Make preview close when Widget closes (Dialog/accept, etc)
   formWidget->setAttribute(Qt::WA_DeleteOnClose, true);

   connect(formWidget, &QObject::destroyed, skinContainer, &QWidget::close);

   skinContainer->setWindowTitle(title);
   skinContainer->setProperty(WidgetFactory::disableStyleCustomPaintingPropertyC, QVariant(true));

   return skinContainer;
}

QWidget *PreviewManager::showPreview(const QDesignerFormWindowInterface *fw,
      const PreviewConfiguration &pc, int deviceProfileIndex, QString *errorMessage)
{
   constexpr const int DEFAULT_SPACING = 10;

   if (QWidget *existingPreviewWidget = raise(fw, pc)) {
      return existingPreviewWidget;
   }

   const QDesignerSharedSettings settings(fw->core());
   const int initialZoom = settings.zoomEnabled() ? settings.zoom() : -1;

   QWidget *widget = createPreview(fw, pc, deviceProfileIndex, errorMessage, initialZoom);
   if (! widget) {
      return nullptr;
   }

   // Install filter for Escape key
   widget->setAttribute(Qt::WA_DeleteOnClose, true);
   widget->installEventFilter(this);

   switch (d->m_mode) {
      case ApplicationModalPreview:
         // Cannot do this on the Mac as the dialog would have no close button
         widget->setWindowModality(Qt::ApplicationModal);
         break;

      case SingleFormNonModalPreview:
      case MultipleFormNonModalPreview:
         widget->setWindowModality(Qt::NonModal);

         connect(fw, &QDesignerFormWindowInterface::changed, widget, &QWidget::close);
         connect(fw, &QObject::destroyed, widget, &QWidget::close);

         if (d->m_mode == SingleFormNonModalPreview) {
            connect(fw->core()->formWindowManager(), &QDesignerFormWindowManagerInterface::activeFormWindowChanged,
               widget, &QWidget::close);
         }
         break;
   }

   // Semi-smart algorithm to position previews:
   // If its the first one, position relative to form.
   // 2nd, attempt to tile right (for comparing styles) or cascade

   const QSize size = widget->size();
   const bool firstPreview = d->m_previews.empty();

   if (firstPreview) {
      widget->move(fw->mapToGlobal(QPoint(DEFAULT_SPACING, DEFAULT_SPACING)));

   } else {
      if (QWidget *lastPreview = d->m_previews.back().m_widget) {
         QDesktopWidget *desktop = qApp->desktop();

         const QRect lastPreviewGeometry = lastPreview->frameGeometry();
         const QRect availGeometry = desktop->availableGeometry(desktop->screenNumber(lastPreview));
         const QPoint newPos = lastPreviewGeometry.topRight() + QPoint(DEFAULT_SPACING, 0);

         if (newPos.x() +  size.width() < availGeometry.right()) {
            widget->move(newPos);
         } else {
            widget->move(lastPreviewGeometry.topLeft() + QPoint(DEFAULT_SPACING, DEFAULT_SPACING));
         }
      }

   }

   d->m_previews.push_back(PreviewData(QPointer<QWidget>(widget), fw, pc));
   widget->show();

   if (firstPreview) {
      emit firstPreviewOpened();
   }

   return widget;
}

QWidget *PreviewManager::raise(const QDesignerFormWindowInterface *fw, const PreviewConfiguration &pc)
{
   typedef PreviewManagerPrivate::PreviewDataList PreviewDataList;
   if (d->m_previews.empty()) {
      return nullptr;
   }

   // find matching window
   const PreviewDataList::const_iterator cend =  d->m_previews.constEnd();
   for (PreviewDataList::const_iterator it = d->m_previews.constBegin(); it !=  cend ; ++it) {
      QWidget *w = it->m_widget;
      if (w && it->m_formWindow == fw && it->m_configuration == pc) {
         w->raise();
         w->activateWindow();
         return w;
      }
   }
   return nullptr;
}

void PreviewManager::closeAllPreviews()
{
   typedef PreviewManagerPrivate::PreviewDataList PreviewDataList;

   if (! d->m_previews.empty()) {
      d->m_updateBlocked = true;
      d->m_activePreview = nullptr;

      const PreviewDataList::iterator cend =  d->m_previews.end();

      for (PreviewDataList::iterator it = d->m_previews.begin(); it !=  cend ; ++it) {
         if (it->m_widget) {
            it->m_widget->close();
         }
      }

      d->m_previews.clear();
      d->m_updateBlocked = false;

      emit lastPreviewClosed();
   }
}

void PreviewManager::updatePreviewClosed(QWidget *w)
{
   typedef PreviewManagerPrivate::PreviewDataList PreviewDataList;
   if (d->m_updateBlocked) {
      return;
   }

   // Purge out all nullptrs or widgets to be deleted
   for (PreviewDataList::iterator it = d->m_previews.begin(); it != d->m_previews.end() ; ) {
      QWidget *iw = it->m_widget;

      if (iw == nullptr || iw == w) {
         it = d->m_previews.erase(it);
      } else {
         ++it;
      }
   }

   if (d->m_previews.empty()) {
      emit lastPreviewClosed();
   }
}

bool PreviewManager::eventFilter(QObject *watched, QEvent *event)
{
   do {
      if (! watched->isWidgetType()) {
         break;
      }

      QWidget *previewWindow = dynamic_cast<QWidget *>(watched);

      if (!previewWindow || !previewWindow->isWindow()) {
         break;
      }

      switch (event->type()) {
         case QEvent::KeyPress:
         case QEvent::ShortcutOverride:        {
            const  QKeyEvent *keyEvent = static_cast<const QKeyEvent *>(event);
            const int key = keyEvent->key();

            if ((key == Qt::Key_Escape
#ifdef Q_OS_DARWIN
                  || (keyEvent->modifiers() == Qt::ControlModifier && key == Qt::Key_Period)
#endif
               )) {
               previewWindow->close();
               return true;
            }
         }
         break;

         case QEvent::WindowActivate:
            d->m_activePreview = previewWindow;
            break;

         case  QEvent::Destroy: // We don't get QEvent::Close if someone accepts a QDialog.
            updatePreviewClosed(previewWindow);
            break;

         case  QEvent::Close:
            updatePreviewClosed(previewWindow);
            previewWindow->removeEventFilter (this);
            break;
         default:
            break;
      }
   } while (false);

   return QObject::eventFilter(watched, event);
}

int PreviewManager::previewCount() const
{
   return  d->m_previews.size();
}

QPixmap PreviewManager::createPreviewPixmap(const QDesignerFormWindowInterface *fw, const QString &style,
      int deviceProfileIndex, QString *errorMessage)
{
   return createPreviewPixmap(fw, configurationFromSettings(fw->core(), style), deviceProfileIndex, errorMessage);
}

QPixmap PreviewManager::createPreviewPixmap(const QDesignerFormWindowInterface *fw, const QString &style, QString *errorMessage)
{
   return createPreviewPixmap(fw, style, -1, errorMessage);
}

QPixmap PreviewManager::createPreviewPixmap(const QDesignerFormWindowInterface *fw,
      const PreviewConfiguration &pc, int deviceProfileIndex, QString *errorMessage)
{
   QWidget *widget = createPreview(fw, pc, deviceProfileIndex, errorMessage);

   if (! widget) {
      return QPixmap();
   }

   const QPixmap rc = widget->grab(QRect(0, 0, -1, -1));
   widget->deleteLater();
   return rc;
}

void PreviewManager::slotZoomChanged(int z)
{
   if (d->m_core) {
      // Save the last zoom chosen by the user

      QDesignerSharedSettings settings(d->m_core);
      settings.setZoom(z);
   }
}
}
