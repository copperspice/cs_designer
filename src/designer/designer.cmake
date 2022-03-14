list(APPEND DESIGNER_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_toolwindow.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_formwindow.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_workbench.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_settings.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_actions.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_server.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_appearanceoptions.h
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_components.h
)

list(APPEND DESIGNER_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_toolwindow.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_formwindow.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_workbench.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_settings.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_server.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_actions.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_appearanceoptions.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/designer/qdesigner_components.cpp
)
