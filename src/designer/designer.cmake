list(APPEND DESIGNER_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_toolwindow.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_formwindow.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_workbench.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_settings.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_actions.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_server.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_appearanceoptions.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_components.h

   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_resource.h
)

list(APPEND DESIGNER_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_toolwindow.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_formwindow.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_workbench.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_settings.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_server.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_actions.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_appearanceoptions.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_components.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/designer/designer_resource.cpp
)
