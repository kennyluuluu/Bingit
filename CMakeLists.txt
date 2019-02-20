cmake_minimum_required(VERSION 3.10.0)
project(bingit-assignment-2)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# https://stackoverflow.com/questions/24311402/copy-all-files-with-given-extension-to-output-directory-using-cmake
#Copy all contents of a folder based on regex path
#copies all folders in static_roots into the build directory
FILE(GLOB MY_STATIC_ROOTS
  "./static_roots/*"
)
FILE(COPY ${MY_STATIC_ROOTS} DESTINATION ${CMAKE_BINARY_DIR})

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log_setup log date_time filesystem thread regex chrono atomic)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Add libraries
add_library(server_lib src/server.cc src/session.cc src/config_parser.cc src/config_params.cc src/handler_manager.cc)
add_library(config_parser src/config_parser.cc)
add_library(handler_lib src/echo_handler.cc src/static_handler.cc src/bad_request_handler.cc src/status_handler src/reply.cc src/request.cc)

# Add executable and link libraries
add_executable(server src/server_main.cc)
target_link_libraries(server server_lib handler_lib ${Boost_LIBRARIES})

# Add test executable and link libraries
add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test config_parser gtest_main)

add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test server_lib handler_lib gtest_main server_lib ${Boost_LIBRARIES})

add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test server_lib handler_lib gtest_main server_lib ${Boost_LIBRARIES})

add_executable(handler_test tests/handler_test.cc)
target_link_libraries(handler_test server_lib gtest_main server_lib ${Boost_LIBRARIES})

add_executable(status_handler_test tests/status_handler_test.cc)
target_link_libraries(status_handler_test server_lib handler_lib gtest_main ${Boost_LIBRARIES})

# integration test
# https://stackoverflow.com/questions/25627336/integrate-bash-test-scripts-in-cmake
find_program(BASH_PROGRAM bash)
if (BASH_PROGRAM)
  add_test(integration_test ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/integration.sh)
endif (BASH_PROGRAM)

# get gtest to work with our tests
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(status_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# generate coverage report
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS server config_parser server_lib TESTS config_parser_test server_test session_test handler_test)
