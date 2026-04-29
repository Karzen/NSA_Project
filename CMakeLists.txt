cmake_minimum_required(VERSION 3.25)
project(NSA_Project CXX)

enable_language(ASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(payload_dll SHARED payload/library.cpp)

add_executable(injector_exe injector/src/main.cpp injector/src/HellsGate/syscalls.s injector/src/HellsGate/HellsGate.cpp injector/include/HellsGate/HellsGate.h injector/src/InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryDefault.cpp injector/include/InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryDefault.h injector/src/InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryUndocumented.cpp injector/include/InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryUndocumented.h injector/src/InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibrarySyscall.cpp injector/include/InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibrarySyscall.h injector/src/ProcUtils.cpp injector/include/ProcUtils.h)

target_include_directories(injector_exe PRIVATE injector/include)

target_link_libraries(injector_exe PRIVATE user32 kernel32)
target_link_libraries(payload_dll PRIVATE user32 kernel32)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)