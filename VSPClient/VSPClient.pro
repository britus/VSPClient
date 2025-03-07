QT += core
QT += gui
QT += widgets
QT += network
QT += concurrent
QT += serialbus
QT += serialport
QT += xml

lessThan(QT_MAJOR_VERSION, 6) {
QT += macextras
}

## all build config
include(VSPClient.config.pri)

## sources
include(driver/driver.pri)
include(model/model.pri)
include(serialio/serialio.pri)
include(themes/themes.pri)
include(session/session.pri)
include(ui/ui.pri)

SOURCES += \
	$$PWD/main.cpp

RESOURCES += \
	$$PWD/vspui.qrc

TRANSLATIONS += \
	$$PWD/i18n/vspui_en_US.ts \
	$$PWD/i18n/vspui_de_DE.ts

DISTFILES += \
	$$PWD/LICENSE \
	$$PWD/README.md \
	$$PWD/qt-bundle-bugfix.sh \
	$$PWD/VSPClient.entitlements \
	$$PWD/makelocales.sh

# Update translation files
localegen.files = $$TRANSLATIONS
localegen.commands = $$PWD/makelocales.sh $$shell_quote($$PWD)
QMAKE_EXTRA_TARGETS += localegen    #'.NOTPARALLEL'
PRE_TARGETDEPS += localegen

# Default rules for deployment.
target.path = /Applications
INSTALLS += target

message("Build: $${TARGET}")
