#!/bin/bash

QTDIR=${HOME}/Qt/6.8.2/macos
COPYDIR=${1}

mkdir -p build

cd build
${QTDIR}/bin/qmake -recursive ../VSPClient.pro
make -j8
cd ..

if [ "x${COPYDIR}" != "x" ] ; then
    mkdir -p ${COPYDIR}

    ## ------------ Framework --------------
    if [ -d build/VSPClient/VSPClientUI.framework ] ; then
        rm -fR ${COPYDIR}/VSPClientUI.framework && \
            cp -Rv build/VSPClient/VSPClientUI.framework ${COPYDIR}/
    fi
    if [ -d build/VSPController/VSPController.framework ] ; then
        rm -fR ${COPYDIR}/VSPController.framework && \
            cp -Rv build/VSPController/VSPController.framework ${COPYDIR}/
    fi
    if [ -d build/VSPSetup/VSPSetup.framework ] ; then
        rm -fR ${COPYDIR}/VSPSetup.framework && \
            cp -Rv build/VSPSetup/VSPSetup.framework ${COPYDIR}/
    fi

    ## ------------ Libraries --------------
    if [ -e build/VSPClient/libVSPClient.dylib ] ; then
        rm -fR ${COPYDIR}/libVSPClient* && \
            cp -Rv build/VSPClient/libVSPClient* ${COPYDIR}/
    fi
    if [ -e build/VSPController/libVSPController.dylib ] ; then
        rm -fR ${COPYDIR}/libVSPController* && \
            cp -Rv build/VSPController/libVSPController* ${COPYDIR}/
    fi
    if [ -e build/VSPSetup/libVSPSetup.dylib ] ; then
        rm -fR ${COPYDIR}/libVSPSetup* && \
            cp -Rv build/VSPSetup/libVSPSetup* ${COPYDIR}/
    fi
fi
