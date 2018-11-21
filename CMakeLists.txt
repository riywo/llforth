cmake_minimum_required(VERSION 3.10)
project(llforth)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VERBOSE_MAKEFILE on)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs core)

add_library(lib STATIC lib.cpp)
add_executable(lib_test test/lib_test.cpp)
target_link_libraries(lib_test lib gtest_main)

add_executable(llforthc compiler.cpp)
target_link_libraries(llforthc ${llvm_libs})

add_custom_target(test-compiler
        COMMAND lit -a --path ${LLVM_TOOLS_BINARY_DIR} --path $<TARGET_FILE_DIR:llforthc> --param lib=$<TARGET_LINKER_FILE:lib> ../test/compiler
        DEPENDS llforthc test/compiler/*.fs lib
)

add_executable(llforth llforth.o)
target_link_libraries(llforth lib)
set_target_properties(llforth PROPERTIES LINKER_LANGUAGE CXX)
add_custom_command(
        OUTPUT llforth.ll
        DEPENDS llforthc interpreter.fs test-compiler
        COMMAND $<TARGET_FILE:llforthc> ../interpreter.fs > llforth.ll
)
add_custom_command(
        OUTPUT llforth.o
        DEPENDS llforth.ll lib
        COMMAND ${LLVM_TOOLS_BINARY_DIR}/llc -filetype=obj llforth.ll
)


add_custom_target(test-interpreter
        COMMAND lit --path ${LLVM_TOOLS_BINARY_DIR} --path $<TARGET_FILE_DIR:llforthc> ../test/interpreter
        DEPENDS llforth
)

################################################################################

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)
