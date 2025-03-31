cmake_minimum_required(VERSION 3.18)

set(LIBRARY_NAME S3FileStorage)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(SOURCE_FILES
	FileStorage.cpp
	Bucket.cpp
)

set (HEADER_FILES
	FileStorage.h
	Bucket.h
)

project(${LIBRARY_NAME} VERSION 1.0.0.0)

add_library(${LIBRARY_NAME} ${HEADER_FILES} ${SOURCE_FILES})

find_package(Boost REQUIRED COMPONENTS json)

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${LIBRARY_NAME} PUBLIC Boost::json Utilities Thread Network)