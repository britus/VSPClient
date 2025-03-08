#!/bin/bash

BUILD_DIR=${1}

echo "++++ Generate .qm files from translations"
lprodump ${BUILD_DIR}/../VSPClient.pro  -out ${BUILD_DIR}/VSPClient.json
lrelease -removeidentical -compress -project ${BUILD_DIR}/VSPClient.json
