if( POLICY CMP0048 )
  cmake_policy( SET CMP0048 NEW ) # CMake 3.0
endif()

project( lc3-vm VERSION 0.1 )
cmake_minimum_required( VERSION 2.6 )
set( CMAKE_C_STANDARD 99 )
add_library( read_image OBJECT lc3.h read_image.c )
add_library( exec_inst OBJECT lc3.h exec_inst.c )
add_executable( ${PROJECT_NAME} $<TARGET_OBJECTS:read_image> $<TARGET_OBJECTS:exec_inst> main.c )

add_executable( ${PROJECT_NAME}-test $<TARGET_OBJECTS:read_image> $<TARGET_OBJECTS:exec_inst> test.c )
enable_testing()
add_test(NAME ${PROJECT_NAME}-test COMMAND ./${PROJECT_NAME}-test )
SET_TESTS_PROPERTIES( ${PROJECT_NAME}-test PROPERTIES 
  PASS_REGULAR_EXPRESSION "All test passed!"
  FAIL_REGULAR_EXPRESSION "Test [\\d]+ fail!"
)