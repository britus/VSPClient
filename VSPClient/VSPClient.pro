QT += core
QT += gui
QT += widgets
QT += network
QT += concurrent
QT += serialbus
QT += serialport
QT += xml

## all build config
include(VSPClient.config.pri)

## sources
include(driver/driver.pri)
include(model/model.pri)
include(serialio/serialio.pri)
include(ui/ui.pri)

SOURCES += \
    $$PWD/main.cpp \

RESOURCES += \
    $$PWD/vspui.qrc

TRANSLATIONS += \
    $$PWD/vspui_en_US.ts

DISTFILES += \
    $$PWD/LICENSE \
    $$PWD/README.md \
    $$PWD/qt-bundle-bugfix.sh \
    $$PWD/Info-App.plist

# Default rules for deployment.
target.path = /Applications
INSTALLS += target

message("Build: $${TARGET}")
