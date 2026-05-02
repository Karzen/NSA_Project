cmake_minimum_required(VERSION 3.25)
project(NSA_Project CXX)

enable_language(ASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Drop the symbol table
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -s")

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

# Remove dead code
#============================================================================
if(WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffunction-sections -fdata-sections")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
endif()
#============================================================================


add_library(payload_dll SHARED payload/library.cpp)
add_executable(injector_exe injector/src/main.cpp "${CMAKE_CURRENT_BINARY_DIR}/library_data.h" injector/src/HellsGate/syscalls.s injector/src/HellsGate/HellsGate.cpp injector/include/HellsGate/HellsGate.h injector/src/InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryDefault.cpp injector/include/InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryDefault.h injector/src/InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryUndocumented.cpp injector/include/InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryUndocumented.h injector/src/InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibrarySyscall.cpp injector/include/InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibrarySyscall.h injector/src/Utils/ProcUtils.cpp injector/include/Utils/ProcUtils.h injector/src/Utils/FilesystemUtils.cpp injector/include/Utils/FilesystemUtils.h injector/src/InjectorStrategy/InjectorManualMap/InjectorManualMap.cpp injector/include/InjectorStrategy/InjectorManualMap/InjectorManualMap.h injector/src/HellsGate/SyscallManager.cpp injector/include/HellsGate/SyscallManager.h)

# Strip dll metadata
set_target_properties(payload_dll PROPERTIES LINK_FLAGS "-s")

target_include_directories(injector_exe PRIVATE injector/include)
target_include_directories(injector_exe PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(injector_exe PRIVATE user32 kernel32)
target_link_libraries(payload_dll PRIVATE user32 kernel32)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Embeddidng the DLL into the executable as a hex array
# ============================================================================
add_dependencies(injector_exe payload_dll)

add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/library_data.h"
        COMMAND ${CMAKE_COMMAND} -DINPUT_FILE=$<TARGET_FILE:payload_dll>
        -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/library_data.h
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BinToHeader.cmake"
        DEPENDS payload_dll
        COMMENT "Converting DLL to header..."
)

target_include_directories(injector_exe PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
#============================================================================


