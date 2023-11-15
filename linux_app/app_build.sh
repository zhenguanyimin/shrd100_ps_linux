#!/bin/sh

set -e

pushd $(dirname $0)

bash src/eap/alink_pb/gen.sh

rm -rf build
mkdir -p build

pushd build

cmake -DPRODUCT=1 .. "$@"
make

popd

popd

