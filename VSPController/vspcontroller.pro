TEMPLATE = lib

TARGET = VSPController

CONFIG -= qt
CONFIG += c++17
CONFIG += sdk_no_version_check
CONFIG += shared
CONFIG += nostrip
CONFIG += debug
#CONFIG += lrelease
CONFIG += embed_translations
CONFIG += create_prl
CONFIG += incremental
CONFIG += global_init_link_order
CONFIG += lib_version_first
CONFIG += vsp_framework

# disables all the APIs deprecated before Qt 6.0.0
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
DEFINES += VSPCONTROLLER_LIBRARY

SOURCES += \
    vspcontroller.cpp

HEADERS += \
    vspcontroller.hpp \
    vspcontroller_global.h \
    vspcontrollerpriv.hpp

DISTFILES += \
    LICENSE

QMAKE_PROJECT_NAME = $${TAGET}

QMAKE_CFLAGS   += -mmacosx-version-min=12.2
QMAKE_CXXFLAGS += -mmacosx-version-min=12.2
QMAKE_CXXFLAGS += -fno-omit-frame-pointer
QMAKE_CXXFLAGS += -funwind-tables
QMAKE_CXXFLAGS += -ggdb3

QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/

vsp_framework {
    CONFIG += embed_libraries
    CONFIG += lib_bundle

    QMAKE_FRAMEWORK_BUNDLE_NAME = $${TARGET}
    QMAKE_FRAMEWORK_VERSION = A
    QMAKE_BUNDLE_EXTENSION = .framework
    #QMAKE_INFO_PLIST = $$PWD/Info.plist

    FRAMEWORK_HEADERS.version = Versions
    FRAMEWORK_HEADERS.files = $${HEADERS}
    FRAMEWORK_HEADERS.path = Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS

    LICENSE.version = Versions
    LICENSE.files = $$PWD/LICENSE
    LICENSE.path = Resources
    QMAKE_BUNDLE_DATA += LICENSE

    icons.version = Versions
    icons.files = $$PWD/vspcontroller.icns
    icons.path = Resources
    QMAKE_BUNDLE_DATA += icons
}

#otool -L
LIBS += -dead_strip
LIBS += -framework CoreFoundation
LIBS += -framework IOKit
LIBS += -liconv

message("Build: $${TARGET}")
