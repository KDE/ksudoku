cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

set (RELEASE_SERVICE_VERSION_MAJOR "24")
set (RELEASE_SERVICE_VERSION_MINOR "11")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

project(ksudoku VERSION "1.4.${RELEASE_SERVICE_COMPACT_VERSION}")

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "6.0.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMDeprecationSettings)

include(InternalMacros)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Widgets
    PrintSupport
    Svg
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    DocTools
    GuiAddons
    I18n
    KIO
    JobWidgets
    WidgetsAddons
    XmlGui
)

find_package(KDEGames6 6.0.0 REQUIRED)


find_package(Qt6OpenGLWidgets ${QT_MIN_VERSION})

find_package(OpenGL)
set_package_properties(OpenGL PROPERTIES DESCRIPTION "API for developing portable, interactive 2D and 3Dgraphics applications" TYPE OPTIONAL PURPOSE "KSudoku will not have Roxdoku support without OpenGL.")


if (OPENGL_FOUND AND OPENGL_GLU_FOUND)
    set(OPENGL_SUPPORT true)
    if (OPENGL_FOUND AND OPENGL_GLU_FOUND)
        set(OPENGL_SUPPORT true)
    endif()
endif()

ecm_set_disabled_deprecation_versions(
    QT 6.7
    KF 6.4
    KDEGAMES 6.0
)

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
