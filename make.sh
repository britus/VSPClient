#!/bin/bash

COPYDIR=${1}
PWD_DIR=`pwd`

mkdir -p build

# clean binary
rm -fR build/VSPController/VSPController.framework
rm -fR build/VSPController/Makefile

rm -fR build/VSPSetup/VSPSetup.framework
rm -fR build/VSPSetup/Makefile

rm -fR build/VSPClient/VSPClient*
rm -fR build/VSPClient/Makefile


cd build
${QTDIR}/bin/qmake -recursive ../VSPClient.pro

cd VSPController && make -j8 && cd ..
cd VSPSetup && make -j8 && cd ..
cd VSPClient && make -j8 && cd ..

if [ "x${COPYDIR}" != "x" ] ; then
	mkdir -vp ${COPYDIR}

	## ------------ Framework --------------
	if [ -d ${PWD_DIR}/build/VSPClient/VSPClientUI.framework ] ; then
		rm -fR ${COPYDIR}/VSPClientUI.framework && \
			cp -Rv ${PWD_DIR}/build/VSPClient/VSPClientUI.framework ${COPYDIR}/
	fi
	if [ -d ${PWD_DIR}/build/VSPController/VSPController.framework ] ; then
		rm -fR ${COPYDIR}/VSPController.framework && \
			cp -Rv ${PWD_DIR}/build/VSPController/VSPController.framework ${COPYDIR}/
	fi
	if [ -d ${PWD_DIR}/build/VSPSetup/VSPSetup.framework ] ; then
		rm -fR ${COPYDIR}/VSPSetup.framework && \
			cp -Rv ${PWD_DIR}/build/VSPSetup/VSPSetup.framework ${COPYDIR}/
	fi

	## ------------ Libraries --------------
	if [ -e ${PWD_DIR}/build/VSPClient/libVSPClient.dylib ] ; then
		rm -fR ${COPYDIR}/libVSPClient* && \
			cp -Rv ${PWD_DIR}/build/VSPClient/libVSPClient* ${COPYDIR}/
	fi
	if [ -e ${PWD_DIR}/build/VSPController/libVSPController.dylib ] ; then
		rm -fR ${COPYDIR}/libVSPController* && \
			cp -Rv ${PWD_DIR}/build/VSPController/libVSPController* ${COPYDIR}/
	fi
	if [ -e ${PWD_DIR}/build/VSPSetup/libVSPSetup.dylib ] ; then
		rm -fR ${COPYDIR}/libVSPSetup* && \
			cp -Rv ${PWD_DIR}/build/VSPSetup/libVSPSetup* ${COPYDIR}/
	fi
fi
