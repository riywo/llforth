cmake_minimum_required(VERSION 3.10)
project(llvm-forth)

set(CMAKE_CXX_STANDARD 14)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs core)

add_executable(llforthc compiler.cpp)
target_link_libraries(llforthc ${llvm_libs})

add_executable(llforth llforth.o)
target_link_libraries(llforth ${llvm_libs})
add_custom_command(
        OUTPUT llfoth.ll llforth.o
        DEPENDS llforthc interpreter.forth
        COMMAND cat ../interpreter.forth | $<TARGET_FILE:llforthc> > llforth.ll
        COMMAND cat llforth.ll
        COMMAND ${LLVM_TOOLS_BINARY_DIR}/llc -filetype=obj llforth.ll
)
