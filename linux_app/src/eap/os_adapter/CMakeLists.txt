#add_subdirectory(base)
add_subdirectory(linux)

#set(OS_ADAPTER_LIBS ${OS_ADAPTER_BASE_LIBS} ${LINUX_ADAPTER_LIBS} PARENT_SCOPE)

# 获取多个嵌套目录的源文件
file(GLOB_RECURSE LIB_SOURCES_OS_ADAPTER "linux/*.cpp")
# 将源文件返回给父目录
set(LIB_SOURCES_OS_ADAPTER ${LIB_SOURCES_OS_ADAPTER} PARENT_SCOPE)

