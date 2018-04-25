cmake_minimum_required(VERSION 3.10)
project(llvm-forth)

set(CMAKE_CXX_STANDARD 14)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(frontend frontend.cpp)
llvm_map_components_to_libnames(llvm_libs core)
target_link_libraries(frontend ${llvm_libs})

add_custom_command(OUTPUT foo.ll foo.o
        COMMAND $<TARGET_FILE:frontend> > foo.ll
        COMMAND ${LLVM_TOOLS_BINARY_DIR}/llc -filetype=obj -o foo.o foo.ll)

add_executable(foo foo.o)
target_link_libraries(foo ${llvm_libs}) # TODO

