cmake_minimum_required(VERSION 3.10)
project(LicenseSystem)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(LicenseSystem main.cpp)