cmake_minimum_required(VERSION 3.25)
project(NSA_Project CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(payload_dll SHARED payload/library.cpp)

add_executable(injector_exe injector/main.cpp)

target_link_libraries(injector_exe PRIVATE user32 kernel32)
target_link_libraries(payload_dll PRIVATE user32 kernel32)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)