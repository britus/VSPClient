TEMPLATE = lib
DEFINES += VSPCONTROLLER_LIBRARY

CONFIG -= qt
CONFIG += c++17
CONFIG += sdk_no_version_check
CONFIG += lib_bundle
CONFIG += shared
CONFIG += nostrip
CONFIG += debug
#CONFIG += lrelease
CONFIG += embed_translations
CONFIG += embed_libraries
CONFIG += create_prl
CONFIG += incremental
CONFIG += global_init_link_order
CONFIG += lib_version_first

# disables all the APIs deprecated before Qt 6.0.0
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
DEFINES += VSP_DEBUG

SOURCES += \
    vspcontroller.cpp

HEADERS += \
    vspcontroller.hpp \
    vspcontroller_global.h \
    vspcontrollerpriv.hpp

DISTFILES += \
    Info.plist \
    LICENSE

QMAKE_CFLAGS   += -mmacosx-version-min=12.2
QMAKE_CXXFLAGS += -mmacosx-version-min=12.2
QMAKE_CXXFLAGS += -fno-omit-frame-pointer
QMAKE_CXXFLAGS += -funwind-tables
QMAKE_CXXFLAGS += -ggdb3

QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/

QMAKE_PROJECT_NAME = VSPController
QMAKE_FRAMEWORK_BUNDLE_NAME = VSPController
QMAKE_FRAMEWORK_VERSION = 1.0
QMAKE_BUNDLE_EXTENSION = .framework
QMAKE_INFO_PLIST = $$PWD/Info.plist

FRAMEWORK_HEADERS.version = Versions
FRAMEWORK_HEADERS.files = $${HEADERS}
FRAMEWORK_HEADERS.path = Headers
QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS

LICENSE.version = Versions
LICENSE.files = $$PWD/LICENSE
LICENSE.path = Resources
QMAKE_BUNDLE_DATA += LICENSE

#otool -L
LIBS += -dead_strip
LIBS += -framework CoreFoundation
LIBS += -framework IOKit
LIBS += -liconv

message("Build: $${TARGET}")
