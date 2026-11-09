cmake_minimum_required(VERSION 3.14)
project(ImGuiColorTextEdit VERSION 1.93.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_GEOGRAM "Enable standalone Geogram compatibility features" OFF)
option(ENABLE_BALAZS "Enable unique features from BalazsJako's historical editor" OFF)
option(BUILD_TESTING "Build editor regression tests" OFF)

# Reuse an application's ImGui target, or the installed package. Both sides
# must use the same ImGui build configuration (notably IMGUI_USE_WCHAR32).
if(TARGET imgui::imgui)
    set(_imgui_target imgui::imgui)
elseif(TARGET imgui)
    set(_imgui_target imgui)
else()
    find_package(imgui CONFIG REQUIRED)
    if(TARGET imgui::imgui)
        set(_imgui_target imgui::imgui)
    elseif(TARGET imgui)
        set(_imgui_target imgui)
    else()
        add_library(imgui::imgui INTERFACE IMPORTED)
        set_target_properties(imgui::imgui PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${imgui_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${imgui_LIBRARIES}")
        set(_imgui_target imgui::imgui)
    endif()
endif()

add_library(ImGuiColorTextEdit TextEditor.cpp TextDiff.cpp)
add_library(ImGuiColorTextEdit::ImGuiColorTextEdit ALIAS ImGuiColorTextEdit)
target_compile_features(ImGuiColorTextEdit PUBLIC cxx_std_17)
target_include_directories(ImGuiColorTextEdit PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ImGuiColorTextEdit>)
target_link_libraries(ImGuiColorTextEdit PUBLIC
    $<BUILD_INTERFACE:${_imgui_target}>
    $<INSTALL_INTERFACE:imgui::imgui>)
set_target_properties(ImGuiColorTextEdit PROPERTIES
    VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
if(BUILD_SHARED_LIBS)
    target_compile_definitions(ImGuiColorTextEdit PUBLIC TEXTEDITOR_SHARED)
    target_compile_definitions(ImGuiColorTextEdit PRIVATE TEXTEDITOR_BUILD)
endif()
if(ENABLE_GEOGRAM)
    target_compile_definitions(ImGuiColorTextEdit PUBLIC __GEOGRAM__)
endif()
if(ENABLE_BALAZS)
    target_compile_definitions(ImGuiColorTextEdit PUBLIC __BALAZS__)
endif()

configure_package_config_file(cmake/ImGuiColorTextEditConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/ImGuiColorTextEditConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ImGuiColorTextEdit")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ImGuiColorTextEditConfigVersion.cmake"
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(TARGETS ImGuiColorTextEdit EXPORT ImGuiColorTextEdit-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES TextEditor.h TextDiff.h TextEditorLanguages.h dtl.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ImGuiColorTextEdit)
install(DIRECTORY languages DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ImGuiColorTextEdit
    FILES_MATCHING PATTERN "*.h")
install(EXPORT ImGuiColorTextEdit-targets NAMESPACE ImGuiColorTextEdit::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ImGuiColorTextEdit)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ImGuiColorTextEditConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ImGuiColorTextEditConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ImGuiColorTextEdit)

if(BUILD_TESTING)
    enable_testing()
    add_executable(editor-regression tests/editor-regression.cpp)
    target_link_libraries(editor-regression PRIVATE ImGuiColorTextEdit)
    add_test(NAME editor-regression COMMAND editor-regression)
endif()
