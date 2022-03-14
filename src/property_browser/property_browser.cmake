list(APPEND DESIGNER_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/editorfactory.h
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/variantproperty.h
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/tree_propertybrowser.h
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/button_propertybrowser.h
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/groupbox_propertybrowser.h

   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/propertybrowser.h
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/propertymanager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/property_browser_utils.h
)

list(APPEND DESIGNER_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/editorfactory.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/variantproperty.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/tree_propertybrowser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/button_propertybrowser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/groupbox_propertybrowser.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/propertybrowser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/propertymanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/property_browser/propertybrowser_utils.cpp
)
