cmake_minimum_required(VERSION 3.14.0-rc2)

project(Tudou)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -lpthread")
add_executable(Tudou test_httpserver.cc ../event.cc ../util.cc ../channel.cc )