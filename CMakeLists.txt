cmake_minimum_required(VERSION 2.8) 
project(euth)

set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -g")

find_package(Boost 1.55.0 REQUIRED COMPONENTS system thread log)
if(Boost_FOUND)
	message(STATUS "Boost found")
	message(STATUS "Boost libraries: " ${Boost_LIBRARIES})
    add_definitions(-DBOOST_ALL_DYN_LINK)
endif()

message(STATUS "c++ flag : " ${CMAKE_CXX_FLAGS}) 
message(STATUS "this is source dir " ${PROJECT_SOURCE_DIR})
message(STATUS "this is binary dir " ${PROJECT_BINARY_DIR})

include_directories(./)
include_directories(Boost_INCLUDE_DIRS)

#AUX_SOURCE_DIRECTORY(./ source)

add_executable(eTester UnitTest_PacketInTester.cpp PacketInTester.cpp  Session.cpp FlowGen.cpp utility.cpp)
add_executable(eproxy eproxy.cpp Session.cpp utility.cpp)
if(UNIX)
	target_link_libraries(
        eTester 
        ${Boost_LIBRARIES}
		pthread
        pcap
		)
    target_link_libraries(
        eproxy
        ${Boost_LIBRARIES}
        pthread
        )
elseif(WIN32)
elseif(APPLE)
endif(UNIX)

