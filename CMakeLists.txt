cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

set (RELEASE_SERVICE_VERSION_MAJOR "21")
set (RELEASE_SERVICE_VERSION_MINOR "07")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

project(ksudoku VERSION "1.4.${RELEASE_SERVICE_COMPACT_VERSION}")

set (QT_MIN_VERSION "5.15.0")
set (KF5_MIN_VERSION "5.83.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Qml Quick QuickWidgets PrintSupport Svg Test)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DocTools
    GuiAddons
    I18n
    IconThemes
    KIO
    JobWidgets
    WidgetsAddons
    XmlGui
)

find_package(KF5KDEGames 4.9.0 REQUIRED)

find_package(Qt5OpenGL ${QT_MIN_VERSION})
find_package(OpenGL)
set_package_properties(OpenGL PROPERTIES DESCRIPTION "API for developing portable, interactive 2D and 3Dgraphics applications" TYPE OPTIONAL PURPOSE "KSudoku will not have Roxdoku support without OpenGL.")

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

if (Qt5OpenGL_FOUND AND OPENGL_FOUND AND OPENGL_GLU_FOUND)
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
if (${KF5Config_VERSION} STRGREATER "5.56.0")
        add_definitions(-DQT_NO_FOREACH)
        MESSAGE(STATUS "compile without foreach")
endif()
add_subdirectory(src)
add_subdirectory(doc)

#message(STATUS "ksudoku: DEBUG: ${DEBUG}")
#message(STATUS "ksudoku: install prefix: ${CMAKE_INSTALL_PREFIX}")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
