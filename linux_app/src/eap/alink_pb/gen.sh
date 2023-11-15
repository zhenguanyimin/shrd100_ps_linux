#!/bin/sh

cd $(dirname $0)

protoc --cpp_out=./  *.proto
