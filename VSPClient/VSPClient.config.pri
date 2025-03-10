#
# VSPClient build configuration included by .pro
#

# By default the framework name
TARGET = VSPClientUI

# Defaults
CONFIG += c++20
CONFIG += sdk_no_version_check
CONFIG += global_init_link_order
CONFIG += incremental
CONFIG -= create_prl
#CONFIG += nostrip
CONFIG += debug_and_release
CONFIG += force_debug_info
CONFIG += separate_debug_info
CONFIG += lrelease

# Build lib framework or app
#CONFIG += vsp_library
CONFIG += vsp_framework
#CONFIG += vsp_app

vsp_app {
	TARGET = VSPClient
}

# disables all the APIs deprecated before Qt 6.0.0
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
DEFINES += VSP_TARGET_NAME=$$shell_quote('$$TARGET')

INCLUDEPATH += $$PWD/../VSPController
INCLUDEPATH += $$PWD/../VSPSetup

# for QT Creator and Visual Studio project.
QMAKE_PROJECT_NAME = $${TARGET}

QMAKE_CFLAGS += -mmacosx-version-min=12.2
QMAKE_CXXFLAGS += -mmacosx-version-min=12.2
QMAKE_CXXFLAGS += -fno-omit-frame-pointer
QMAKE_CXXFLAGS += -funwind-tables

release {
	#QMAKE_LFLAGS += -s
}
debug {
	QMAKE_CXXFLAGS += -ggdb3
}

#otool -L
LIBS += -dead_strip
LIBS += -liconv

LIBS += -F$$OUT_PWD/../VSPController -framework VSPController
LIBS += -F$$OUT_PWD/../VSPSetup      -framework VSPSetup

# Generate application bundle
vsp_app {
	TEMPLATE = app
	CONFIG += app_bundle
	CONFIG += embed_libraries
	CONFIG += embed_translations

	QMAKE_MACOSX_DEPLOYMENT_TARGET = 12.2

	# Important for the App with embedded frameworks and libs
	QMAKE_RPATHDIR += @executable_path/../Frameworks
	QMAKE_RPATHDIR += @executable_path/lib

	LICENSE.files = $$PWD/LICENSE
	LICENSE.path = Contents/Resources
	QMAKE_BUNDLE_DATA += LICENSE

	translations_en.files = \
		$$PWD/assets/en.lproj/InfoPlist.strings \
		$$PWD/vspui_en_US.qm
	translations_en.path = Contents/Resources/en.lproj
	QMAKE_BUNDLE_DATA += translations_en

	translations_de.files = \
		$$PWD/assets/de.lproj/InfoPlist.strings \
		$$PWD/vspui_de_DE.qm
	translations_de.path = Contents/Resources/de.lproj
	QMAKE_BUNDLE_DATA += translations_de

	icons.files = \
		$$PWD/assets/icns/vspclient.icns \
		$$PWD/assets/png/vspclient_512x512.png
	icons.path = Contents/Resources
	QMAKE_BUNDLE_DATA += icons

	vspdext.files = \
		$$PWD/../../VSPDriver/build/Debug-driverkit/org.eof.tools.VSPDriver.dext
	#vspdext.files = \
	#	$$PWD/../../VSPDriver/build/Release-driverkit/org.eof.tools.VSPDriver.dext
	vspdext.path = Contents/Library/SystemExtensions
	QMAKE_BUNDLE_DATA += vspdext

	frameworks.files = \
		$$OUT_PWD/../VSPController/VSPController.framework \
		$$OUT_PWD/../VSPSetup/VSPSetup.framework
	frameworks.path = Contents/Frameworks
	QMAKE_BUNDLE_DATA += frameworks

	QMAKE_INFO_PLIST += \
		$$PWD/Info.plist

	#QMAKE_PRE_LINK +=

	QMAKE_POST_LINK += \
		$$PWD/qt-bundle-bugfix.sh $${TARGET} app VSPController && \
		$$PWD/qt-bundle-bugfix.sh $${TARGET} app VSPSetup

	QMAKE_CODE_SIGN_ENTITLEMENTS=$$PWD/VSPClient.entitlements
	QMAKE_CODE_SIGN_IDENTITY='Mac Developer'
}

# Generate framework bundle
vsp_framework {
	TEMPLATE = lib
	DEFINES	+= VSPCLIENT_LIBRARY
	CONFIG += lib_bundle
	CONFIG += embed_libraries
	CONFIG -= create_prl

	QMAKE_FRAMEWORK_BUNDLE_NAME = $${TARGET}
	QMAKE_FRAMEWORK_VERSION = A
	QMAKE_BUNDLE_EXTENSION = .framework
	#QMAKE_INFO_PLIST = $$PWD/Info.plist

	# Important for the App with embedded framework
	QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/

	LICENSE.version = Versions
	LICENSE.files = $$PWD/LICENSE
	LICENSE.path = Resources
	QMAKE_BUNDLE_DATA += LICENSE

	translations_en.version = Versions
	translations_en.files = \
		$$PWD/assets/en.lproj/InfoPlist.strings \
		$$PWD/vspui_en_US.qm
	translations_en.path = Resources/en.lproj
	QMAKE_BUNDLE_DATA += translations_en

	translations_de.version = Versions
	translations_de.files = \
		$$PWD/assets/de.lproj/InfoPlist.strings \
		$$PWD/vspui_de_DE.qm
	translations_de.path = Resources/de.lproj
	QMAKE_BUNDLE_DATA += translations_de

	icons.version = Versions
	icons.files = \
		$$PWD/assets/icns/vspclient.icns \
		$$PWD/assets/png/vspclient_512x512.png
	icons.path = Resources
	QMAKE_BUNDLE_DATA += icons
}

# Generate genric library
vsp_library {
	TEMPLATE = lib
	DEFINES	+= VSPCLIENT_LIBRARY
	CONFIG += create_prl
	CONFIG += embed_translations
	CONFIG += lib_version_first

	# Important for the App with embedded framework
	QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../lib/
}
