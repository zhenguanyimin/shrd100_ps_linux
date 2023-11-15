## How to build

### (1) make a build dir
```bash
mkdir build
```
### (2) do cmake
```bash
cd build
cmake -DPRODUCT=1 ..
```
### (3) make
```bash
make
```
### (4)the bin file are in build/src/linux_app.elf

## or
```bash
bash app_build.sh
```

## Unittest

### 1) build

```bash
mkdir buildtest
cd buildtest
cmake ..
make
```

### 2) run test
```bash
bin/main
```






1. 编译pb描述文件
(1)设置交叉编译工具链，以提供相应版本的pb编译工具protoc
source /opt/tracer/petalinux/2022.2/environment-setup-cortexa72-cortexa53-xilinx-linux

(2)重新编译proto文件
cd linux_app/src/eap/alink_pb
./gen.sh

2. 编译linux_app
省略
请参考linux_app/README

或调用app_build.sh
3. 打包配置到rootfs
cd linux_app
./app_pack.sh

此脚本，会自动将linux_app/cfg/* 拷到kernel/sdk_tracer_petalinux20222/project-spec/meta-user/recipes-apps/app.rc/files/etc

在rootfs里，配置文件在/etc目录下



