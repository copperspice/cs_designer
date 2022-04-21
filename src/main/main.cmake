list(APPEND DESIGNER_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/main/mainwindow.h
)

list(APPEND DESIGNER_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/main/mainwindow.cpp

   qrc_designer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../designer.rc
)
