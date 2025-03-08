#!/bin/bash

PWDIR=`pwd`
COPYDIR=${1}

mkdir -p ${PWDIR}/build

# clean binary
rm -fR ${PWDIR}/build/VSPController/VSPController.framework
rm -fR ${PWDIR}/build/VSPController/Makefile

rm -fR ${PWDIR}/build/VSPSetup/VSPSetup.framework
rm -fR ${PWDIR}/build/VSPSetup/Makefile

rm -fR ${PWDIR}/build/VSPClient/VSPClient*
rm -fR ${PWDIR}/build/VSPClient/Makefile

# generate localization profile
lprodump -pro VSPClient.pro -out VSPClient.json

cd ${PWDIR}/build

${PWDIR}/VSPClient/makelocales.sh ${PWDIR}/build
${QTDIR}/bin/qmake -recursive ../VSPClient.pro

cd VSPController && make -j8 && cd ..
cd VSPSetup && make -j8 && cd ..
cd VSPClient && make -j8 && cd ..

if [ "x${COPYDIR}" != "x" ] ; then
	mkdir -vp ${COPYDIR}

	## ------------ Framework --------------
	if [ -d ${PWDIR}/build/VSPClient/VSPClientUI.framework ] ; then
		rm -fR ${COPYDIR}/VSPClientUI.framework && \
			cp -Rv ${PWDIR}/build/VSPClient/VSPClientUI.framework ${COPYDIR}/
	fi
	if [ -d ${PWDIR}/build/VSPController/VSPController.framework ] ; then
		rm -fR ${COPYDIR}/VSPController.framework && \
			cp -Rv ${PWDIR}/build/VSPController/VSPController.framework ${COPYDIR}/
	fi
	if [ -d ${PWDIR}/build/VSPSetup/VSPSetup.framework ] ; then
		rm -fR ${COPYDIR}/VSPSetup.framework && \
			cp -Rv ${PWDIR}/build/VSPSetup/VSPSetup.framework ${COPYDIR}/
	fi

	## ------------ Libraries --------------
	if [ -e ${PWDIR}/build/VSPClient/libVSPClient.dylib ] ; then
		rm -fR ${COPYDIR}/libVSPClient* && \
			cp -Rv ${PWDIR}/build/VSPClient/libVSPClient* ${COPYDIR}/
	fi
	if [ -e ${PWDIR}/build/VSPController/libVSPController.dylib ] ; then
		rm -fR ${COPYDIR}/libVSPController* && \
			cp -Rv ${PWDIR}/build/VSPController/libVSPController* ${COPYDIR}/
	fi
	if [ -e ${PWDIR}/build/VSPSetup/libVSPSetup.dylib ] ; then
		rm -fR ${COPYDIR}/libVSPSetup* && \
			cp -Rv ${PWDIR}/build/VSPSetup/libVSPSetup* ${COPYDIR}/
	fi
fi

cd ${PWDIR}
