# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)
# 项目信息
project (cpp_restful)

add_definitions(-std=c++11)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/linux/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/linux/lib)

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src DIR_SRCS)

# 指定生成目标
add_library(cpp_restful ${DIR_SRCS})
target_link_libraries(cpp_restful event event_pthreads)