cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_BUILD_TYPE "Debug")

project(tinyweb)
enable_testing()

# Download and unpack googletest at configure time
configure_file(third-party/CMakeLists.txt.in googletest-download/CMakeLists.txt)
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

if (CMAKE_VERSION VERSION_LESS 2.8.11)
	include_directories("${gtest_SOURCE_DIR}/include")
endif()

find_package(glog 0.4.0 REQUIRED)

include_directories(header)
aux_source_directory(src SRCS)
add_executable(${CMAKE_PROJECT_NAME} ${SRCS})
target_link_libraries(${CMAKE_PROJECT_NAME} glog::glog)

#add_executable(db_test ./test/test.cc)
#target_link_libraries(db_test ${CMAKE_PROJECT_NAME})
#target_link_libraries(db_test gtest_main)
#add_test(NAME db_test COMMAND db_test)

#add_executable(skiptable_test ./test/skiplist_test.cc)
#target_link_libraries(skiptable_test ${CMAKE_PROJECT_NAME})
#target_link_libraries(skiptable_test gtest_main glog::glog)
#add_test(NAME skiptable_test COMMAND skiptable_test)