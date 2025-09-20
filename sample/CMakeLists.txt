cmake_minimum_required(VERSION 3.14)
project(myproj)

# 1. 递归收集所有源码
file(GLOB_RECURSE SRCS src/*.cpp)

# 2. 生成可执行文件 EchoServer
add_executable(EchoServer ${SRCS})

# 3. 头文件搜索路径
target_include_directories(EchoServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 4. 动态库路径 + 库名（去掉前缀 lib 和后缀 .so）
target_link_directories(EchoServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)
target_link_libraries(EchoServer PRIVATE
    mymuduo      # 对应 libmymuduo.so
)

# 5. 运行时自动找到 libmymuo.so
set_target_properties(EchoServer PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    BUILD_WITH_INSTALL_RPATH TRUE
)
if(APPLE)
    set_target_properties(EchoServer PROPERTIES
        INSTALL_RPATH "@executable_path/../lib"
    )
elseif(UNIX)
    set_target_properties(EchoServer PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
    )
endif()