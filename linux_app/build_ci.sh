#!/bin/bash

set -e

cd $(dirname $0)

if [[ -n "$CI_COMMIT_TAG" ]];then 
    VERSION="$CI_COMMIT_TAG";
elif [[ -z "$VERSION" ]];then 
    VERSION="SHRD101L-V99.99.99";
fi

if [[ "$CI" == "true" ]];then
    source /data/petalinux/2022.2/environment-setup-cortexa72-cortexa53-xilinx-linux
fi

bash app_version_make.sh "${VERSION}"
bash app_build.sh
