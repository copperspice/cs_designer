list(APPEND DESIGNER_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor/buddyeditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor/buddyeditor_plugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor/buddyeditor_tool.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qdesigner_resource.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qdesignerundostack.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindow.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindow_widgetstack.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindow_dnditem.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindowcursor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/widgetselection.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindowmanager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formeditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qlayoutwidget_propertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/layout_propertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/spacer_propertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/line_propertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/default_container.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/default_actionprovider.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qmainwindow_container.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qmdiarea_container.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qwizard_container.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/default_layoutdecoration.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/tool_widgeteditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formeditor_optionspage.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/embeddedoptionspage.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindowsettings.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/deviceprofiledialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/dpi_chooser.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/previewactiongroup.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/itemview_propertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/templateoptionspage.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtcolorbutton.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientstopsmodel.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientstopswidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientstopscontroller.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientwidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradienteditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientdialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtcolorbutton.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtcolorline.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientview.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_select.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientmanager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientutils.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/propertyeditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/designerpropertymanager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/paletteeditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/paletteeditorbutton.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/stringlisteditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/stringlisteditorbutton.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/previewwidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/previewframe.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/newdynamicpropertydialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/brushpropertymanager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/fontpropertymanager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/qlonglongvalidator.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalslot_utils_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/connectdialog_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor_tool.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor_plugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditorwindow.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor/tabordereditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor/tabordereditor_plugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor/tabordereditor_tool.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taskmenu_editor/inplace_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taskmenu_editor/itemlist_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taskmenu_editor/listwidget_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taskmenu_editor/treewidget_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taskmenu_editor/tablewidget_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taskmenu_editor/inplace_widget_helper.h
)

list(APPEND DESIGNER_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor/buddyeditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor/buddyeditor_tool.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor/buddyeditor_plugin.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qdesigner_resource.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qdesignerundostack.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindow.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindow_widgetstack.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindow_dnditem.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindowcursor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/widgetselection.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindowmanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formeditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qlayoutwidget_propertysheet.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/layout_propertysheet.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/spacer_propertysheet.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/line_propertysheet.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qmainwindow_container.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qmdiarea_container.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qwizard_container.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/default_container.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/default_layoutdecoration.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/default_actionprovider.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/tool_widgeteditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formeditor_optionspage.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/embeddedoptionspage.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindowsettings.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/deviceprofiledialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/dpi_chooser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/previewactiongroup.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/itemview_propertysheet.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/templateoptionspage.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtcolorbutton.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientstopsmodel.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientstopswidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientstopscontroller.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientwidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradienteditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientdialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtcolorbutton.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtcolorline.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientview.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_select.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientmanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/qtgradientutils.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/propertyeditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/designerpropertymanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/paletteeditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/paletteeditorbutton.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/stringlisteditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/stringlisteditorbutton.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/previewwidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/previewframe.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/newdynamicpropertydialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/brushpropertymanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/fontpropertymanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/qlonglongvalidator.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalslot_utils.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/connectdialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor_tool.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor_plugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditorwindow.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor/tabordereditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor/tabordereditor_tool.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor/tabordereditor_plugin.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taskmenu_editor/itemlist_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taskmenu_editor/listwidget_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taskmenu_editor/treewidget_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taskmenu_editor/tablewidget_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taskmenu_editor/inplace_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taskmenu_editor/inplace_widget_helper.cpp
)

