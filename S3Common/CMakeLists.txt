cmake_minimum_required(VERSION 3.18)

set(LIBRARY_NAME S3Common)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(SOURCE_FILES
	JsonValidation.cpp
	RedisClient.cpp
)

set (HEADER_FILES
	JsonValidation.h
	RedisClient.h
)

add_library(${LIBRARY_NAME} ${HEADER_FILES} ${SOURCE_FILES})

find_package(Boost REQUIRED COMPONENTS json)
find_package(redis++ CONFIG REQUIRED)
find_package(hiredis CONFIG REQUIRED)

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${LIBRARY_NAME} PUBLIC Boost::json redis++::redis++ hiredis::hiredis Utilities)
