list(APPEND DESIGNER_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/abstract_formbuilder.h
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/formbuilder.h
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/properties.h
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/formbuilderextra.h
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/resourcebuilder.h
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/textbuilder.h
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/ui4.h
)

list(APPEND DESIGNER_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/abstract_formbuilder.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/formbuilder.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/ui4.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/properties.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/formbuilderextra.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/resourcebuilder.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder/textbuilder.cpp
)
