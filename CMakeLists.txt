cmake_minimum_required(VERSION 3.10)
project(llforth)

set(CMAKE_CXX_STANDARD 14)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs core)

add_executable(llforthc compiler.cpp)
target_link_libraries(llforthc ${llvm_libs})

add_custom_target(test-compiler
        COMMAND lit -a --path ${LLVM_TOOLS_BINARY_DIR} --path $<TARGET_FILE_DIR:llforthc> ../test/compiler
        DEPENDS llforthc
)

add_executable(llforth llforth.o)
target_link_libraries(llforth ${llvm_libs})
add_custom_command(
        OUTPUT llfoth.ll llforth.o
        DEPENDS llforthc interpreter.forth test-compiler
        COMMAND $<TARGET_FILE:llforthc> ../interpreter.forth > llforth.ll
        COMMAND ${LLVM_TOOLS_BINARY_DIR}/llc -filetype=obj llforth.ll
)

add_custom_target(test-interpreter
        COMMAND lit --path ${LLVM_TOOLS_BINARY_DIR} --path $<TARGET_FILE_DIR:llforthc> ../test/interpreter
        DEPENDS llforth
)
