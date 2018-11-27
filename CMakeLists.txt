cmake_minimum_required(VERSION 3.10)
project(llforth)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VERBOSE_MAKEFILE on)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs core)

add_library(lib STATIC lib.cpp)

add_executable(llforthc compiler.cpp)
target_link_libraries(llforthc ${llvm_libs})

add_custom_target(test-compiler
        COMMAND lit -a --path ${LLVM_TOOLS_BINARY_DIR} --path $<TARGET_FILE_DIR:llforthc> --param lib=$<TARGET_LINKER_FILE:lib> ../test/compiler
        DEPENDS llforthc test/compiler/*.fs lib
)

add_executable(llforth llforth.o)
target_link_libraries(llforth lib)
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
