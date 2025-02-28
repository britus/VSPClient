
cd build
echo "QT Mac Deploy"
macdeployqt VSPClient/VSPClientUI.app -verbose=1 \
	-always-overwrite -no-strip \
	-appstore-compliant  \
	-sign-for-notarization=83NQU389JN \
	-codesign=83NQU389JN

cd ..
