#
# VSPClient build configuration included by .pro
#

# major.minor.patch.build
VERSION = 1.0.4.14

##
TARGET = VSPClient

# for QT Creator and Visual Studio project files only.
QMAKE_PROJECT_NAME = $${TARGET}

# Defaults
CONFIG += c++20
CONFIG += sdk_no_version_check
CONFIG += global_init_link_order
CONFIG += incremental

CONFIG += nostrip
CONFIG += debug
#CONFIG += lrelease

# Build lib framework or app
#CONFIG += vsp_library
CONFIG += vsp_framework
#CONFIG += vsp_app

# Generate a normal application bundle
vsp_app {
	TEMPLATE = app
	CONFIG += app_bundle
	CONFIG += embed_libraries
	CONFIG += embed_translations
	DEFINES += VSP_APP
}

# Generate a genric library
vsp_library {
	TEMPLATE = lib
	DEFINES	+= VSPCLIENT_LIBRARY
	CONFIG += create_prl
	CONFIG += embed_translations
	CONFIG += lib_version_first
	DEFINES += VSP_LIB
}

# Generate a framework bundle
vsp_framework {
	TEMPLATE = lib
	DEFINES	+= VSPCLIENT_LIBRARY
	CONFIG += lib_bundle
	CONFIG += create_prl
	CONFIG += embed_libraries
	CONFIG += embed_translations
	DEFINES += VSP_FRAMEWORK
}

# disables all the APIs deprecated before Qt 6.0.0
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
DEFINES += VSP_DEBUG

QMAKE_LFLAGS         = -Wl,-rpath,@executable_path/../Frameworks/
QMAKE_LFLAGS_SONAME	 = -Wl,-install_name,@executable_path/../Frameworks/

QMAKE_CXXFLAGS      += -fno-omit-frame-pointer
QMAKE_CXXFLAGS      += -funwind-tables
QMAKE_CXXFLAGS      += -ggdb3

#otool -L
LIBS += -dead_strip
LIBS += -liconv

LIBS += -F$$OUT_PWD/../VSPController -framework VSPController
LIBS += -F$$OUT_PWD/../VSPSetup      -framework VSPSetup

INCLUDEPATH += $$OUT_PWD/../../VSPController
INCLUDEPATH += $$OUT_PWD/../../VSPSetup

# ----------------------------------------

mac {
	QMAKE_MACOSX_DEPLOYMENT_TARGET = 12.2

	#QMAKE_PROVISIONING_PROFILE
	#QMAKE_OSX_ARCHITECTURES="x86_64;arm64"

	#QMAKE_CXXFLAGS     += -arch x86_64 -arch arm64
	QMAKE_CFLAGS        += -mmacosx-version-min=12.2
	QMAKE_CXXFLAGS      += -mmacosx-version-min=12.2

	#AppEntitlements.files = $$PWD/VSPClient.entitlements
	#AppEntitlements.path = Contents
	#QMAKE_BUNDLE_DATA += AppEntitlement

	translations_en.files = $$PWD/assets/en.lproj/InfoPlist.strings
	translations_en.path = Contents/Resources/en.lproj
	translations_de.files = $$PWD/assets/de.lproj/InfoPlist.strings
	translations_de.path = Contents/Resources/de.lproj
	QMAKE_BUNDLE_DATA += translations_en translations_de

	LICENSE.clear
	LICENSE.files = $$PWD/LICENSE
	LICENSE.path = Contents/Resources
	QMAKE_BUNDLE_DATA += LICENSE

	icons.files = $$PWD/assets/icns/vspclient.icns
	icons.path = Contents/Resources
	QMAKE_BUNDLE_DATA += icons
}

mac:vsp_framework {
	QMAKE_FRAMEWORK_VERSION = 2.1
	QMAKE_BUNDLE_EXTENSION = .framework
	QMAKE_FRAMEWORK_BUNDLE_NAME = $${TARGET}

	QMAKE_INFO_PLIST += \
		$$PWD/Info.plist \
		$$PWD/VSPClient.entitlements
}

mac:vsp_library {
#-
}

mac:vsp_app {
	QMAKE_INFO_PLIST += \
		$$PWD/Info.plist \
		$$PWD/VSPClient.entitlements

	QMAKE_POST_LINK += \
		$$PWD/qt-bundle-bugfix.sh $${TARGET} app VSPController && \
		$$PWD/qt-bundle-bugfix.sh $${TARGET} app VSPSetup

	#vspdext.files = \
	#	$$PWD/../../VSPDriver_FMWK/build/Debug-driverkit/org.eof.tools.VSPDriver.dext
	#vspdext.files = \
	#	$$PWD/../../VSPDriver_FMWK/build/Release-driverkit/org.eof.tools.VSPDriver.dext
	#vspdext.path = Contents/Library/SystemExtensions
	#QMAKE_BUNDLE_DATA += vspdext

	frameworks.files = \
		$$OUT_PWD/../VSPController/VSPController.framework \
		$$OUT_PWD/../VSPSetup/VSPSetup.framework
	frameworks.path = Contents/Frameworks
	QMAKE_BUNDLE_DATA += frameworks

	CODE_SIGN_ENTITLEMENTS=$$PWD/VSPClient.entitlements
	CODE_SIGN_IDENTITY='Mac Developer'
}
