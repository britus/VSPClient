#!/bin/bash

if [ "x" == "x${1}" ] ; then
	echo "1: Target name expected."
	exit 1
fi
if [ "x" == "x${2}" ] ; then
	echo "2: Framework name expected."
	exit 1
fi

OUT_PWD=`pwd`
TARGET=${1}
FRAMEWORK=${2}

echo "Link: ${FRAMEWORK} into ${TARGET} framework path" && \

cd ${OUT_PWD}/${TARGET}.app/Contents/Frameworks/${FRAMEWORK}.framework

if [ -e ${FRAMEWORK} ]; then
	echo "${FRAMEWORK} link exist, OK."
	exit 0
fi

mkdir -p ${OUT_PWD}/${TARGET}.app/Contents/Frameworks

cp -Rpv ${OUT_PWD}/../${FRAMEWORK}/${FRAMEWORK}.framework \
		${OUT_PWD}/${TARGET}.app/Contents/Frameworks/

ln -vsf Versions/Current/${FRAMEWORK} ${FRAMEWORK}

if [ ! -L ${FRAMEWORK} ]; then
	echo "Bundle QT-BUGIFX ${TARGET}: ${FRAMEWORK} failed."
	exit 1
fi

echo "${FRAMEWORK} QT-BUNDLE BUGFIX OKAY!"
