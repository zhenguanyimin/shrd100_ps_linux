#!/bin/bash

BIN_NAME=app.tgz
TMP_DIR=app_tmp
BIN_DIR=${TMP_DIR}/bin
LIB_DIR=${TMP_DIR}/lib
INSTALL_DIR=$(cd $(dirname $0); pwd)
LIBEV_DIR=$(dirname $INSTALL_DIR)/sysroot/usr/lib
ROOTFS_PACK_DIR=$(dirname $INSTALL_DIR)/kernel/sdk_tracer_petalinux20222/project-spec/meta-user/recipes-apps/app.rc/files/bin
ROOTFS_PACK_BASEDIR=$(dirname $INSTALL_DIR)/kernel/sdk_tracer_petalinux20222/project-spec/meta-user/recipes-apps/app.rc/files

set -e
pushd ${INSTALL_DIR}
rm -rf ${TMP_DIR}
mkdir -p ${BIN_DIR} ${LIB_DIR}

cp build/src/linux_app.elf ${BIN_DIR}
cp build/src/libeap.so ${LIB_DIR}

cp -a ${LIBEV_DIR}/libev.so* ${LIB_DIR}

cp -a ${LIBEV_DIR}/libunwind*.so* ${LIB_DIR}

cp -a ${LIBEV_DIR}/libprotobuf*.so* ${LIB_DIR}

#cp cfg files
cp -a cfg/product_hw_cfg.* ${ROOTFS_PACK_BASEDIR}/etc/
cp -a cfg/product_sw_cfg.* ${ROOTFS_PACK_BASEDIR}/etc/
cp -a cfg/product_debug_cfg.json ${ROOTFS_PACK_BASEDIR}/etc/


pushd ${TMP_DIR}
tar -czf ${INSTALL_DIR}/${BIN_NAME} *
popd

rm -f ${ROOTFS_PACK_DIR}/${BIN_NAME}
cp ${INSTALL_DIR}/${BIN_NAME} ${ROOTFS_PACK_DIR}

rm -rf ${INSTALL_DIR}/${BIN_NAME} ${TMP_DIR}

popd
