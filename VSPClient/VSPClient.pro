QT += core
QT += gui
QT += widgets
QT += network
QT += concurrent
QT += serialbus
QT += serialport
QT += xml

TARGET = VSPClient

# major.minor.patch.build
VERSION = 1.0.3.14

###
CONFIG += c++17
CONFIG += sdk_no_version_check
CONFIG += nostrip
CONFIG += debug
#CONFIG += lrelease
CONFIG += embed_translations
CONFIG += create_prl
CONFIG += app_bundle
CONFIG += embed_translations
CONFIG += embed_libraries
CONFIG += incremental
CONFIG += global_init_link_order
CONFIG += lib_version_first

# disables all the APIs deprecated before Qt 6.0.0
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
DEFINES += VSP_DEBUG

include(driver/driver.pri)
include(model/model.pri)
include(serialio/serialio.pri)
include(ui/ui.pri)

SOURCES += \
    main.cpp \

RESOURCES += \
    vspui.qrc

TRANSLATIONS += \
    vspui_en_US.ts

DISTFILES += \
    LICENSE \
    README.md \
    VSPClient.entitlements \
    qt-bundle-bugfix.sh

INCLUDEPATH += $$PWD/../VSPController
DEPENDPATH += $$PWD/../VSPController

INCLUDEPATH += $$PWD/../VSPSetup
DEPENDPATH += $$PWD/../VSPSetup

PRE_TARGETDEPS += $$DEPENDPATH

# Bugfix QT bundle build
QMAKE_POST_LINK += \
    $${PWD}/qt-bundle-bugfix.sh $${TARGET} VSPController && \
    $${PWD}/qt-bundle-bugfix.sh $${TARGET} VSPSetup

QMAKE_PROJECT_NAME = VSPClient
QMAKE_MACOSX_DEPLOYMENT_TARGET = 12.2
#QMAKE_PROVISIONING_PROFILE
QMAKE_INFO_PLIST += \
    $$PWD/Info.plist \
    $$PWD/VSPClient.entitlements

#AppEntitlements.files = $$PWD/VSPClient.entitlements
#AppEntitlements.path = Contents
#QMAKE_BUNDLE_DATA += AppEntitlement

#translations_en.files = $$PWD/en.lproj/InfoPlist.strings
#translations_en.path = en.lproj
#translations_de.files = $$PWD/de.lproj/InfoPlist.strings
#translations_de.path = de.lproj
#QMAKE_BUNDLE_DATA += translations_en translations_de

icons.files = $$PWD/assets/icns/vspclient.icns
icons.path = Contents/Resources
QMAKE_BUNDLE_DATA += icons

LICENSE.files = $$PWD/LICENSE
LICENSE.path = Contents/Resources
QMAKE_BUNDLE_DATA += LICENSE

frameworks.files = \
    $$OUT_PWD/../vspcontroller/VSPController.framework \
    $$OUT_PWD/../vspsetup/VSPSetup.framework
frameworks.path = Contents/Frameworks
QMAKE_BUNDLE_DATA += frameworks

vspdext.files = \
    $$PWD/Library/SystemExtensions/org.eof.tools.VSPDriver.dext
vspdext.path = Contents/Library/SystemExtensions
QMAKE_BUNDLE_DATA += vspdext

QMAKE_LFLAGS         = -Wl,-rpath,@executable_path/../Frameworks/
QMAKE_LFLAGS_SONAME	 = -Wl,-install_name,@executable_path/../Frameworks/

QMAKE_CFLAGS   += -mmacosx-version-min=12.2
QMAKE_CXXFLAGS += -mmacosx-version-min=12.2
QMAKE_CXXFLAGS += -fno-omit-frame-pointer
QMAKE_CXXFLAGS += -funwind-tables
QMAKE_CXXFLAGS += -ggdb3

#otool -L
LIBS += -dead_strip
LIBS += -F$$OUT_PWD/../vspcontroller -framework VSPController
LIBS += -F$$OUT_PWD/../vspsetup      -framework VSPSetup
LIBS += -liconv

# Default rules for deployment.
target.path = /Applications
INSTALLS += target

message("Build: $${TARGET}")
