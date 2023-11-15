# cmake file for xilinx 7ev
# by chenyili

if(NOT PRODUCT)
    message("\n[UnitTest Begin]\n")

    cmake_minimum_required(VERSION 3.0.0)
    project(UnitTest)
    message("CMAKE_SOURCE_DIR:${CMAKE_SOURCE_DIR}")
    set(ROOT_DIR ${CMAKE_SOURCE_DIR})

    if (NOT WIN32)
    set(CMAKE_C_FLAGS "--coverage")
    set(CMAKE_CXX_FLAGS "--coverage")
    endif()

    add_definitions(-D__UNITTEST__)
    add_definitions(-DECANCELED=1)
    add_definitions(-DEINVAL=0)
    add_definitions(-D__ARM_ARCH=8)
    add_definitions(-D__ARM_NEON__)
    add_definitions(-D__ARM_NEON)
    # add_definitions(-D__aarch64__)

    include_directories(${ROOT_DIR}/unittest/include)

    add_subdirectory(${ROOT_DIR}/src)
    add_subdirectory(${ROOT_DIR}/unittest/main)
    message("\n[UnitTest End]\n")

else()
    cmake_minimum_required(VERSION 3.10)
    project(linux_app)

    # 设置使用C++标准版本
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
    add_definitions("-Wall -g")

    message("\n[Tracer Target Build Begin]\n")
    message("CMAKE_SOURCE_DIR:${CMAKE_SOURCE_DIR}")

    set(ROOT_DIR ${CMAKE_SOURCE_DIR})
    set(PRJ_ROOT ${ROOT_DIR}/..)
    set(PRJ_SYSROOT ${PRJ_ROOT}/sysroot)

    # set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_BUILD_TYPE "debug")

    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR arm)

#外部传入版本号
if (VERSION)
    set(CMAKE_C_FLAGS "-DVERSION='\"${VERSION}\"'")
endif()

    add_definitions(-O2)
    add_definitions(-Wformat -Wformat-security -Werror=format-security -Wdate-time)

    #add our project sysroot path
    include_directories(${PRJ_SYSROOT}/usr/include)
    link_directories(${PRJ_SYSROOT}/usr/lib)

    if(NO_ADC_MODULE)
        add_definitions(-DNO_ADC_MODULE=1)
    endif()

    # add source file directories
    add_subdirectory(${ROOT_DIR}/src)

endif()

