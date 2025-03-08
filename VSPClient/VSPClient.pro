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
	$$PWD/vspui_en_US.ts \
	$$PWD/vspui_de_DE.ts

DISTFILES += \
	$$PWD/LICENSE \
	$$PWD/README.md \
	$$PWD/qt-bundle-bugfix.sh \
	$$PWD/VSPClient.entitlements \
	$$PWD/makelocales.sh

# Update translation files
#LOC_QM_FILES.depends = $$TRANSLATIONS
#LOC_QM_FILES.files = $$TRANSLATIONS
#LOC_QM_FILES.depends = FORCE
#LOC_QM_FILES.commands = $$PWD/makelocales.sh $$shell_quote($$PWD)
#LOC_QM_FILES.target = \
#	$$PWD/vspui_en_US.qm \
#	$$PWD/vspui_de_DE.qm
#message($${LOC_QM_FILES.files})
#PRE_TARGETDEPS	    += $${LOC_QM_FILES.files}
#QMAKE_EXTRA_TARGETS += LOC_QM_FILES   #'.NOTPARALLEL'

# Default rules for deployment.
target.path = /Applications
INSTALLS += target

message("Build: $${TARGET}")
