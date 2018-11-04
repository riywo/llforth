cmake_minimum_required(VERSION 3.10)
project(llvm-forth)

set(CMAKE_CXX_STANDARD 14)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs core)

add_executable(main main.cpp)
target_link_libraries(main ${llvm_libs})

add_executable(llvm-forth main.o)
target_link_libraries(llvm-forth ${llvm_libs})
add_custom_command(
        OUTPUT main.ll main.o
        DEPENDS main
        COMMAND echo 'foo bye' | $<TARGET_FILE:main> > main.ll
        COMMAND cat main.ll
        COMMAND ${LLVM_TOOLS_BINARY_DIR}/llc -filetype=obj main.ll
)


add_executable(compiler compiler.cpp)
target_link_libraries(compiler ${llvm_libs})
#add_custom_command(
#        OUTPUT out.ll out.o
#        DEPENDS compiler test.forth
#        COMMAND cat ../test.forth | $<TARGET_FILE:compiler> > out.ll
#        COMMAND cat out.ll
#        COMMAND ${LLVM_TOOLS_BINARY_DIR}/llc -filetype=obj out.ll
#)

