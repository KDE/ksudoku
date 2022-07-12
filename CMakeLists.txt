cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

set (RELEASE_SERVICE_VERSION_MAJOR "22")
set (RELEASE_SERVICE_VERSION_MINOR "11")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

project(ksudoku VERSION "1.4.${RELEASE_SERVICE_COMPACT_VERSION}")

set (QT_MIN_VERSION "5.15.0")
set (KF5_MIN_VERSION "5.90.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMInstallIcons)
include(ECMSetupVersion)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS
    Widgets
    PrintSupport
    Svg
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DocTools
    GuiAddons
    I18n
    KIO
    JobWidgets
    WidgetsAddons
    XmlGui
)

find_package(KF5KDEGames 4.9.0 REQUIRED)

find_package(Qt${QT_MAJOR_VERSION}OpenGL ${QT_MIN_VERSION})
find_package(OpenGL)
set_package_properties(OpenGL PROPERTIES DESCRIPTION "API for developing portable, interactive 2D and 3Dgraphics applications" TYPE OPTIONAL PURPOSE "KSudoku will not have Roxdoku support without OpenGL.")


add_definitions(
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)
if (Qt${QT_MAJOR_VERSION}OpenGL_FOUND AND OPENGL_FOUND AND OPENGL_GLU_FOUND)
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

add_subdirectory(src)
add_subdirectory(doc)

ki18n_install(po)
kdoctools_install(po)

#message(STATUS "ksudoku: DEBUG: ${DEBUG}")
#message(STATUS "ksudoku: install prefix: ${CMAKE_INSTALL_PREFIX}")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
