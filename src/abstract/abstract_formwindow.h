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

#ifndef ABSTRACTFORMWINDOW_H
#define ABSTRACTFORMWINDOW_H

class QDesignerFormEditorInterface;
class QDesignerFormWindowCursorInterface;
class QDesignerFormWindowToolInterface;
class QtResourceSet;

#include <QWidget>

class QUndoStack;
class QDir;

class QDesignerFormWindowInterface: public QWidget
{
   CS_OBJECT(QDesignerFormWindowInterface)

 public:
   enum FeatureFlag {
      EditFeature = 0x01,
      GridFeature = 0x02,
      TabOrderFeature = 0x04,
      DefaultFeature = EditFeature | GridFeature
   };
   using Feature = QFlags<FeatureFlag>;

   enum ResourceFileSaveMode {
      SaveAllResourceFiles,
      SaveOnlyUsedResourceFiles,
      DontSaveResourceFiles
   };

   explicit QDesignerFormWindowInterface(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
   virtual ~QDesignerFormWindowInterface();

   virtual QString fileName() const = 0;
   virtual QDir absoluteDir() const = 0;

   virtual QString contents() const = 0;
   virtual QStringList checkContents() const = 0;
   virtual bool setContents(QIODevice *dev, QString *errorMessage = nullptr) = 0;

   virtual Feature features() const = 0;
   virtual bool hasFeature(Feature f) const = 0;

   virtual QString author() const = 0;
   virtual void setAuthor(const QString &author) = 0;

   virtual QString comment() const = 0;
   virtual void setComment(const QString &comment) = 0;

   virtual void layoutDefault(int *margin, int *spacing) = 0;
   virtual void setLayoutDefault(int margin, int spacing) = 0;

   virtual void layoutFunction(QString *margin, QString *spacing) = 0;
   virtual void setLayoutFunction(const QString &margin, const QString &spacing) = 0;

   virtual QString pixmapFunction() const = 0;
   virtual void setPixmapFunction(const QString &pixmapFunction) = 0;

   virtual QString exportMacro() const = 0;
   virtual void setExportMacro(const QString &exportMacro) = 0;

   virtual QStringList includeHints() const = 0;
   virtual void setIncludeHints(const QStringList &includeHints) = 0;

   virtual ResourceFileSaveMode resourceFileSaveMode() const = 0;
   virtual void setResourceFileSaveMode(ResourceFileSaveMode behaviour) = 0;

   virtual QtResourceSet *resourceSet() const =  0;
   virtual void setResourceSet(QtResourceSet *resourceSet) = 0;

   QStringList activeResourceFilePaths() const;

   virtual QDesignerFormEditorInterface *core() const;
   virtual QDesignerFormWindowCursorInterface *cursor() const = 0;

   virtual int toolCount() const = 0;

   virtual int currentTool() const = 0;
   virtual void setCurrentTool(int index) = 0;

   virtual QDesignerFormWindowToolInterface *tool(int index) const = 0;
   virtual void registerTool(QDesignerFormWindowToolInterface *tool) = 0;

   virtual QPoint grid() const = 0;

   virtual QWidget *mainContainer() const = 0;
   virtual void setMainContainer(QWidget *mainContainer) = 0;
   virtual QWidget *formContainer() const = 0;

   virtual bool isManaged(QWidget *widget) const = 0;

   virtual bool isDirty() const = 0;

   static QDesignerFormWindowInterface *findFormWindow(QWidget *w);
   static QDesignerFormWindowInterface *findFormWindow(QObject *obj);

   virtual QUndoStack *commandHistory() const = 0;
   virtual void beginCommand(const QString &description) = 0;
   virtual void endCommand() = 0;

   virtual void simplifySelection(QList<QWidget *> *widgets) const = 0;

   // notifications
   virtual void emitSelectionChanged() = 0;

   virtual QStringList resourceFiles() const = 0;
   virtual void addResourceFile(const QString &path) = 0;
   virtual void removeResourceFile(const QString &path) = 0;

   virtual void ensureUniqueObjectName(QObject *object) = 0;

   CS_SIGNAL_1(Public, void mainContainerChanged(QWidget *mainContainer))
   CS_SIGNAL_2(mainContainerChanged, mainContainer)

   CS_SIGNAL_1(Public, void toolChanged(int toolIndex))
   CS_SIGNAL_2(toolChanged, toolIndex)

   CS_SIGNAL_1(Public, void fileNameChanged(const QString &fileName))
   CS_SIGNAL_2(fileNameChanged, fileName)

   CS_SIGNAL_1(Public, void featureChanged(Feature f))
   CS_SIGNAL_2(featureChanged, f)

   CS_SIGNAL_1(Public, void selectionChanged())
   CS_SIGNAL_2(selectionChanged)

   CS_SIGNAL_1(Public, void geometryChanged())
   CS_SIGNAL_2(geometryChanged)

   CS_SIGNAL_1(Public, void resourceFilesChanged())
   CS_SIGNAL_2(resourceFilesChanged)

   CS_SIGNAL_1(Public, void widgetManaged(QWidget *widget))
   CS_SIGNAL_2(widgetManaged, widget)

   CS_SIGNAL_1(Public, void widgetUnmanaged(QWidget *widget))
   CS_SIGNAL_2(widgetUnmanaged, widget)

   CS_SIGNAL_1(Public, void aboutToUnmanageWidget(QWidget *widget))
   CS_SIGNAL_2(aboutToUnmanageWidget, widget)

   CS_SIGNAL_1(Public, void activated(QWidget *widget))
   CS_SIGNAL_2(activated, widget)

   CS_SIGNAL_1(Public, void changed())
   CS_SIGNAL_2(changed)

   CS_SIGNAL_1(Public, void widgetRemoved(QWidget *w))
   CS_SIGNAL_2(widgetRemoved, w)

   CS_SIGNAL_1(Public, void objectRemoved(QObject *o))
   CS_SIGNAL_2(objectRemoved, o)

   CS_SLOT_1(Public, virtual void manageWidget(QWidget *widget) = 0)
   CS_SLOT_2(manageWidget)

   CS_SLOT_1(Public, virtual void unmanageWidget(QWidget *widget) = 0)
   CS_SLOT_2(unmanageWidget)

   CS_SLOT_1(Public, virtual void setFeatures(Feature f) = 0)
   CS_SLOT_2(setFeatures)

   CS_SLOT_1(Public, virtual void setDirty(bool dirty) = 0)
   CS_SLOT_2(setDirty)

   CS_SLOT_1(Public, virtual void clearSelection(bool changePropertyDisplay = true) = 0)
   CS_SLOT_2(clearSelection)

   CS_SLOT_1(Public, virtual void selectWidget(QWidget *w, bool select = true) = 0)
   CS_SLOT_2(selectWidget)

   CS_SLOT_1(Public, virtual void setGrid(const QPoint &grid) = 0)
   CS_SLOT_2(setGrid)

   CS_SLOT_1(Public, virtual void setFileName(const QString &fileName) = 0)
   CS_SLOT_2(setFileName)

   virtual bool setContents(const QString &contents) = 0;

   CS_SLOT_1(Public, virtual void editWidgets() = 0)
   CS_SLOT_2(editWidgets)

   CS_SLOT_1(Public, void activateResourceFilePaths(const QStringList &paths, int *errorCount = nullptr,
         QString *errorMessages = nullptr))
   CS_SLOT_2(activateResourceFilePaths)
};

#endif
