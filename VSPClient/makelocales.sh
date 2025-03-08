#!/bin/bash

BASE_DIR=${1}

echo "++++ Generate .qm files from translations"
lprodump ${BASE_DIR}/../VSPClient.pro -out ${BASE_DIR}/../VSPClient.json
lrelease -removeidentical -compress -project ${BASE_DIR}/../VSPClient.json
