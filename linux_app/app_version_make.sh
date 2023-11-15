#!/bin/bash

cd $(dirname $0)

VER_H=src/inc/app_version.h

APP_VERSION="$1"

if [[ $(which git) ]];then
    GIT_VERSION=$(git rev-parse HEAD)
    sed -i "s/#define GIT_VERSION.*/#define GIT_VERSION \"${GIT_VERSION}\"/" ${VER_H}
elif [[ -n "$CI_BUILD_REF" ]];then
    sed -i "s/#define GIT_VERSION.*/#define GIT_VERSION \"${CI_BUILD_REF}\"/" ${VER_H}
fi
sed -i "s/#define APP_VERSION.*/#define APP_VERSION \"${APP_VERSION}\"/" ${VER_H}
