#!/bin/bash

if [[ -n "$VERSION" ]];then
    bash linux_app/app_version_make.sh "$VERSION"
fi

bash linux_app/app_build.sh

bash kernel/sdk_tracer_petalinux20222/build.sh
