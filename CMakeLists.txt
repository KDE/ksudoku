project(ksudoku)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.3.0")
set (KF5_MIN_VERSION "5.31.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Qml Quick QuickWidgets Svg Test OpenGL)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    GuiAddons
    I18n
    ItemModels
    KDELibs4Support
    KIO
    NewStuff
    NotifyConfig
    TextWidgets
    WidgetsAddons
    WindowSystem
    XmlGui
)

find_package(KF5KDEGames 4.9.0 REQUIRED)

find_package(OpenGL)
set_package_properties(OpenGL PROPERTIES DESCRIPTION "API for developing portable, interactive 2D and 3Dgraphics applications" TYPE REQUIRED PURPOSE "Kubrick will not be built and KSudoku will not have Roxdoku support without OpenGL.")

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

if (OPENGL_FOUND)
     set(OPENGL_SUPPORT true)
endif()


##options
#OPTION(DEBUG
#	"Build the project using debugging code"
#	ON  #default on, make default off in stable branch
#	CMAKE_BUILD_TYPE "DEBUG"
#	#if CMAKE_BUILD_TYPE is not defined, FindKDE3.make
#	#adds -O2 to CMAKE_CXX_FLAGS
#)

IF(DEBUG)
	ADD_DEFINITIONS(-DDEBUG -ggdb -O)
ENDIF(DEBUG)

# enable code depending on OPENGL-support
if(OPENGL_SUPPORT)
	add_definitions(-DOPENGL_SUPPORT)
endif(OPENGL_SUPPORT)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

add_subdirectory(src)
add_subdirectory(doc)

#message(STATUS "ksudoku: DEBUG: ${DEBUG}")
#message(STATUS "ksudoku: install prefix: ${CMAKE_INSTALL_PREFIX}")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
