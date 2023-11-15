#!/bin/bash

set -e

function log {
    echo "$(date) [CI_LOG] $1"
}

function build {
    log "start to build app."
    docker run --rm \
        -e CI=true -e CI_COMMIT_TAG="$1-${compileTag}" \
        -e CI_BUILD_REF="$(git rev-parse HEAD)" \
        -v "$PWD":"$PWD" -w "$PWD" \
        10.240.34.2:10000/cibuilder/tracer-app-builder:v1 \
        bash linux_app/build_ci.sh

    log "build app complete, start to build kernel."
    docker run --rm \
        -e CI=true \
        -v /data/gitlab-runner/volumes/xilinx/petalinux/cache:/home/xilinx/2022.2 \
        -v "$PWD":"$PWD" -w "$PWD" \
        10.240.34.2:10000/cibuilder/xilinx-petalinux:v1 \
        bash kernel/sdk_tracer_petalinux20222/build_ci.sh

    log "build kernel complete."

    mkdir -p package/"$1-${compileTag}"
    cp -a linux_app/build/src/linux_app.elf package/"$1-${compileTag}"
    cp -a linux_app/build/src/libeap.so package/"$1-${compileTag}"
    cp -ar kernel/sdk_tracer_petalinux20222/images/* package/"$1-${compileTag}"
}

function build_or_skip {
    local regex="(^|.*,)$1(,.*|$)"
    if  [[ "$compileType" =~ $regex ]];then
        log "build $1"
        build "$1"
    else
        log "skip $1"
    fi
}

function clean {
    log "clean "
    rm -rf kernel/sdk_tracer_petalinux20222/build/tmp/stamps # 删除整个build目录则为不使用缓存
    rm -rf kernel/sdk_tracer_petalinux20222/images
}

rm -rf package

# 传入的compileType变量，应该以逗号为分隔符，类似于: SHRD101L,SHRD102L,SHRD104L
# 脚本会遍历下面列表，出现在compileType变量的才会进行编译
for type in SHRD101L SHRD103L SHRD102L SHRD104L
do
    clean
    build_or_skip $type
done
