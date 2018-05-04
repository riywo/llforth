cmake_minimum_required(VERSION 3.10)
project(llvm-forth)

set(CMAKE_CXX_STANDARD 14)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(compiler compiler.cpp)
llvm_map_components_to_libnames(llvm_libs core)
target_link_libraries(compiler ${llvm_libs})

add_custom_command(OUTPUT compiler.ll compiler.o
        DEPENDS compiler
        COMMAND $<TARGET_FILE:compiler> > compiler.ll
        COMMAND ${LLVM_TOOLS_BINARY_DIR}/llc -filetype=obj compiler.ll)

add_executable(llvm-forth compiler.o)
target_link_libraries(llvm-forth ${llvm_libs}) # TODO

