list(APPEND DESIGNER_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_formeditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_introspection.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_settings.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_formeditorplugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_resourcebrowser.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_integration.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_propertyeditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_formwindow.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_formwindowtool.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_formwindowcursor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_formwindowmanager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_widgetdatabase.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_metadatabase.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_widgetfactory.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_objectinspector.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_actioneditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_language.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_optionspage.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_widgetbox.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_dnditem.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_promotioninterface.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_dialoggui.h
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_newformwidget.h
)

list(APPEND DESIGNER_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_formeditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_introspection.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_formeditorplugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_resourcebrowser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_integration.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_propertyeditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_formwindow.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_formwindowtool.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_formwindowcursor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_formwindowmanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_widgetdatabase.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_metadatabase.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_widgetfactory.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_objectinspector.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_actioneditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_widgetbox.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_promotioninterface.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_dialoggui.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract/abstract_newformwidget.cpp

)
