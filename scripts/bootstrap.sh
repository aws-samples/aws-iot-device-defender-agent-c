#!/usr/bin/env bash

PROJECT_ROOT=$(dirname `pwd`)
TEMP_DOWNLOAD_DIR=$PROJECT_ROOT/scripts/downloads
EXT_LIBS_ROOT=$PROJECT_ROOT/external_libs

IOT_SDK_SRC_ARCHIVE='https://github.com/aws/aws-iot-device-sdk-embedded-C/archive/v3.0.1.tar.gz'
IOT_SDK_SRC_ROOT=$EXT_LIBS_ROOT/aws-iot-device-sdk-embedded-C

MBEDTLS_SRC_ARCHIVE='https://github.com/ARMmbed/mbedtls/archive/mbedtls-2.13.0.tar.gz'
CPPUTest_SRC_ARCHIVE='https://github.com/cpputest/cpputest/releases/download/v3.8/cpputest-3.8.tar.gz'

CBOR_SRC_ARCHIVE='https://github.com/aws/amazon-freertos/archive/v1.4.2.tar.gz'
CBOR_SRC_ROOT=$EXT_LIBS_ROOT/cbor

#Check for cURL
if ! command -v curl; then
    echo "cUrl required to download dependencies, exiting"
    exit 1
fi

if ! command -v tar; then
    echo "tar required to download dependencies, exiting"
    exit 1
fi

mkdir -p $TEMP_DOWNLOAD_DIR
cd $TEMP_DOWNLOAD_DIR

#Get AWS IoT Device SDK for Embedded C
mkdir -p $IOT_SDK_SRC_ROOT
curl -L -O $IOT_SDK_SRC_ARCHIVE
tar -xvf 'v3.0.1.tar.gz' --strip-components 1 -C $IOT_SDK_SRC_ROOT

#Since the SDK doesn't provide a CMakeLists.txt file, we'll provide a simple one
cp $EXT_LIBS_ROOT/SDK_CMakeLists.txt $IOT_SDK_SRC_ROOT/CMakeLists.txt

#Get the SDK's MBEDTLS dependency
curl -L -O $MBEDTLS_SRC_ARCHIVE
tar -xvf 'mbedtls-2.13.0.tar.gz' --strip-components 1 -C $IOT_SDK_SRC_ROOT/external_libs/mbedTLS

#Get CppUTest
#curl -L -O $CPPUTest_SRC_ARCHIVE
#tar -xvf 'cpputest-3.8.tar.gz' --strip-components 1 -C $IOT_SDK_SRC_ROOT/external_libs/CppUTest

#Get Unity

#Get cJSON


#cleanup downloads
cd ..
rm -rf $TEMP_DOWNLOAD_DIR