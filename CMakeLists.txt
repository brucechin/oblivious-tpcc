cmake_minimum_required (VERSION 3.0)
project (emp-ot C CXX)
set(NAME "emp-ot")
set (CMAKE_CXX_STANDARD 11) 

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/common.cmake)
include(${CMAKE_FOLDER}/cmake/source_of_randomness.cmake)
include(${CMAKE_FOLDER}/cmake/threading.cmake)

find_package(emp-tool REQUIRED)
# Requirements for lattice
if (LATTICEOT)
  message(STATUS "Building with support for lattice OT")
  find_package(Eigen3 3.3 REQUIRED NO_MODULE)
  find_package(Boost 1.58 REQUIRED COMPONENTS random)
  include_directories(${EMP-TOOL_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
else (LATTICEOT)
  include_directories(${EMP-TOOL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
endif (LATTICEOT)



# Installation
install(FILES cmake/emp-ot-config.cmake DESTINATION cmake/)
install(DIRECTORY emp-ot DESTINATION include/)


set(SRC "${CMAKE_CURRENT_BINARY_DIR}/test")
add_library(generator ${SRC}/randomgenerator.cpp)
add_library(clock ${SRC}/clock.cpp)
add_library(tpccdb ${SRC}/tpccdb.cpp)
add_library(tpccclient ${SRC}/tpccclient.cpp)

add_executable(dummy ${SRC}/dummy.cpp)
add_executable(tpcc ${SRC}/tpcc.cpp)
target_link_libraries(tpcc generator clock tpccdb tpccclient ${EMP-TOOL_LIBRARIES})
target_link_libraries(dummy ${EMP-TOOL_LIBRARIES})