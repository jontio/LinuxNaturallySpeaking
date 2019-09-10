cmake_minimum_required (VERSION 3.0 FATAL_ERROR)
project(ktexteditor_textview)

# we need some parts of the ECM CMake helpers
find_package (ECM ${KF5_DEP_VERSION} REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(ECMAddAppIcon)
include(GenerateExportHeader)

include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include (CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(Qt5 REQUIRED COMPONENTS
    Core
    Widgets
)

find_package(KF5 REQUIRED COMPONENTS
  Config
  Crash
  I18n
  JobWidgets
  KIO
  Parts
  TextEditor
  WindowSystem
  XmlGui
  IconThemes
)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Widgets_INCLUDE_DIRS})

set(ktexteditor_textview_PART_SRCS textview.cpp )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

qt5_add_resources(ktexteditor_textview_PART_SRCS)

add_library (ktexteditor_textview MODULE ${ktexteditor_textview_PART_SRCS})

kcoreaddons_desktop_to_json (ktexteditor_textview ktexteditor_textview.desktop)

target_link_libraries(ktexteditor_textview
    KF5::TextEditor
    KF5::Parts
    KF5::IconThemes
    KF5::I18n
    KF5::Service
)

