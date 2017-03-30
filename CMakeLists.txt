#cmake_minimum_required(VERSION 3.5)
#project(spaul5_bytecode)
#
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#
#set(SOURCE_FILES
#    src/loader.c
#    src/loader.h
#    src/vm.c
#    src/vm.h
#    src/vm_strings.c
#    src/vm_strings.h
#    src/wrun.c
#    test/samples/fib.bytecode
#    test/samples/hello.bytecode
#    test/samples/printarg.bytecode
#    test/c_unit.c
#    test/c_unit.h
#    test/test_core.c
#    test/test_funcs.c
#    CMakeLists.txt
#    LICENSE)
#
#add_executable(spaul5_bytecode ${SOURCE_FILES})

cmake_minimum_required(VERSION 3.0)
project(bytecode)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c99 -DMARK_AND_COMPACT -Wall")

set(SOURCE src/vm.c src/loader.c src/vm_strings.c)

add_library(vm ${SOURCE})
target_include_directories(vm PUBLIC src)

include_directories(src)

add_library(c_unit test/c_unit.c)
target_include_directories (c_unit PUBLIC test)

add_executable(wrun src/wrun.c)
target_link_libraries(wrun vm)

include(CTest)

add_executable(test_core test/test_core.c)
target_link_libraries(test_core LINK_PUBLIC vm c_unit)
add_test(NAME test_core
        COMMAND    valgrind --error-exitcode=1 --tool=memcheck --leak-check=full ./test_core)

add_executable(test_funcs test/test_funcs.c)
target_link_libraries(test_funcs LINK_PUBLIC vm c_unit)
add_test(NAME test_funcs
        COMMAND    valgrind --error-exitcode=1 --tool=memcheck --leak-check=full ./test_funcs)