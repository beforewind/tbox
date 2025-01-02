
cmake_minimum_required(VERSION 3.16)

project(cli_test LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/../../install)
# 设置静态库文件目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 动态库文件目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 可执行文件目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#SET(CMAKE_PREFIX_PATH $ENV{QTDIR}/lib/cmake)
#SET(CMAKE_PREFIX_PATH $ENV{QTDIR}/5.9.7/msvc2017_64/lib/cmake)
#SET(CMAKE_PREFIX_PATH $ENV{QTDIR}/5.9.7/msvc2017_64)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# auto compile
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTORCC ON)

# find Qt5 libs
#FIND_PACKAGE(Qt5 COMPONENTS Core Widgets Network REQUIRED)

#add_definitions(-std=c++14 -Wall -Werror)
#add_definitions(-std=c++14 -Wall)  # -Wall many warning! disable it
#add_definitions(-std=c++14)
#add_definitions(-DMELO_USE_COUT)
add_definitions(-D__STDC_LIMIT_MACROS)
set(CMAKE_CXX_STANDARD 17)


set(PACKAGE_DEPENDENCIES
#  soem
)

include_directories(
  ${CMAKE_SOURCE_DIR}/../../src
  ${CMAKE_SOURCE_DIR}/../../src/ectbox
  ${CMAKE_SOURCE_DIR}/../../src/utils
  ${CMAKE_SOURCE_DIR}/../../deps/spdlog-1.11.0/include
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/osal
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/osal/win32
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/oshw/win32
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/oshw/win32/wpcap/include
)

link_directories(
  ${PROJECT_NAME}
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/oshw/win32/wpcap/Lib/x64
)

SET(MAIN_SOURCES
  ${CMAKE_SOURCE_DIR}/../../src/example/main_cli_test.cpp
  ${CMAKE_SOURCE_DIR}/../../src/example/tbox_helper.cpp
)

SET(SOURCES
  ${CMAKE_SOURCE_DIR}/../../src/ectbox/ectbox.cpp
  ${CMAKE_SOURCE_DIR}/../../src/ectbox/ectbus.cpp
  ${CMAKE_SOURCE_DIR}/../../src/ectbox/slave.cpp
  ${CMAKE_SOURCE_DIR}/../../src/utils/tlogger.cpp
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/osal/win32/osal.c
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/oshw/win32/nicdrv.c
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/oshw/win32/oshw.c
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatbase.c
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatcoe.c
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatconfig.c
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatdc.c
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatfoe.c
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatmain.c
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatprint.c
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatsoe.c
)

SET(INTERFACE_HEADERS
  ${CMAKE_SOURCE_DIR}/../../src/example/tbox_helper.h
  ${CMAKE_SOURCE_DIR}/../../src/include/tbox.h
)

# add headers
SET(HEADERS
  ${CMAKE_SOURCE_DIR}/../../src/utils/tlogger.h
  ${CMAKE_SOURCE_DIR}/../../src/ectbox/ectbox.h
  ${CMAKE_SOURCE_DIR}/../../src/ectbox/ectbus.h
  ${CMAKE_SOURCE_DIR}/../../src/ectbox/slave.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/osal/osal.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/osal/win32/osal_defs.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/oshw/win32/nicdrv.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/oshw/win32/oshw.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercat.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatbase.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatcoe.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatconfig.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatconfiglist.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatdc.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatfoe.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatmain.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatprint.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercatsoe.h
  ${CMAKE_SOURCE_DIR}/../../deps/SOEM-1.4.0/soem/ethercattype.h
)

# add build to includes
INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
#    ../
#    ../common
    )

  add_library(
    EcTbox SHARED  # STATIC  # SHARED
    ${INTERFACE_HEADERS}
    ${HEADERS}
    ${SOURCES}
  )

#add_library(
#  EcTboxStatic STATIC
#  ${INTERFACE_HEADERS}
#  ${HEADERS}
#  ${SOURCES}
#)

# 连接Tbox库所需的库
target_link_libraries(
    EcTbox
    wpcap
    Packet
    winmm
    Ws2_32
)

# 测试程序，应该只用连接Tbox.dll动态库及其头文件
add_executable(
  ${PROJECT_NAME}
#  ${HEADERS}
#  ${SOURCES}
  ${INTERFACE_HEADERS}
  ${MAIN_SOURCES}
)

target_link_libraries(
    ${PROJECT_NAME}
    wpcap
    Packet
    winmm
    Ws2_32
#    EcTbox  # need?
)


#target_link_libraries(
#  ${PROJECT_NAME}
#  ${PROJECT_NAME}
##  ${catkin_LIBRARIES}
#)

install(TARGETS EcTbox EcTbox
    ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/../../install/lib
    LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/../../install/lib
    RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/../../install/bin
)

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/../../install/lib
    LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/../../install/lib
    RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/../../install/bin
)
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CMAKE_SOURCE_DIR}/../../install/include/${PROJECT_NAME}
)
