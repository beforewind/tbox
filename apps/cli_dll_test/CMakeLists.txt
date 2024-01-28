
cmake_minimum_required(VERSION 3.16)

project(cli_dll_test LANGUAGES CXX C)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/../../install)
# 设置静态库文件目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 动态库文件目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 可执行文件目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-D__STDC_LIMIT_MACROS)
set(CMAKE_CXX_STANDARD 17)

include_directories(
  ${CMAKE_SOURCE_DIR}/../../src
)

#link_directories(
#  ${PROJECT_NAME}
#  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/oshw/win32/wpcap/Lib/x64
#)

SET(MAIN_SOURCES
  ${CMAKE_SOURCE_DIR}/../../src/example/main_cli_test.cpp
  ${CMAKE_SOURCE_DIR}/../../src/example/tbox_helper.cpp
)

SET(INTERFACE_HEADERS
  ${CMAKE_SOURCE_DIR}/../../src/include/tbox.h
  ${CMAKE_SOURCE_DIR}/../../src/example/tbox_helper.h
)

# 测试程序，应该只用连接Tbox.dll动态库及其头文件
add_executable(
  ${PROJECT_NAME}
  ${INTERFACE_HEADERS}
  ${MAIN_SOURCES}
)

target_link_libraries(
    ${PROJECT_NAME}
#    EcTbox
#    wpcap
#    Packet
#    winmm
#    Ws2_32
)

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/../../../install/lib
    LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/../../../install/lib
    RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/../../../install/bin
)
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CMAKE_SOURCE_DIR}/../../../install/include/${PROJECT_NAME}
)
