#!/bin/sh

set -e

pushd $(dirname $0)

rm -rf build
mkdir -p build

pushd build

cmake -DPRODUCT=1 -DNO_ADC_MODULE=1 ..
make

popd

popd

