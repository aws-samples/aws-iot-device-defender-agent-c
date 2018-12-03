cmake_minimum_required(VERSION 3.0)
project(AWSIotDeviceDefenderAgent C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")
enable_testing()
include(ExternalProject)

## Tiny CBOR
# Here we are using the EXTERNALPROJECT_ADD feature of cmake to download a tarball of the library automaticallys
find_program(MAKE_EXE NAMES gmake nmake make)
EXTERNALPROJECT_ADD(project_tinycbor
        URL https://github.com/intel/tinycbor/archive/v0.5.2.tar.gz
        BUILD_COMMAND ${MAKE_EXE}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ""
        INSTALL_COMMAND ""
        )

ExternalProject_Get_Property(project_tinycbor SOURCE_DIR)
# This creates a linkable CMAKE target for tinycbor with a dependency on the external project
# If you try to link directly to the external project, you'll get a "can't link to a untility target" error
add_library(tinycbor STATIC IMPORTED)
set_property(TARGET tinycbor PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/lib/libtinycbor.a)
add_dependencies(tinycbor project_tinycbor)

# Agent
# add agent.c to executable here for older versions of CMake
add_executable(agent src/agent.c)
target_include_directories(agent PRIVATE
        include/
        external_libs/cjson
        external_libs/aws-iot-device-sdk-embedded-C/include
        external_libs/aws-iot-device-sdk-embedded-C/external_libs/
        ${SOURCE_DIR}/src)

target_sources(agent PRIVATE
        src/agent_config.h
        src/collector.c
        src/metrics.c
        src/jobsHandler.c
        external_libs/cjson/cJSON.c)

target_link_libraries(agent
        iotsdk
        tinycbor
       )

# Dependencies
## IoT SDK -  Build with Cmake
add_subdirectory(external_libs/aws-iot-device-sdk-embedded-C)

# Tests
## Test Collector
add_executable(test_collector EXCLUDE_FROM_ALL test/test_collector.c)
target_include_directories(test_collector PRIVATE
        external_libs/unity
        external_libs/cjson
        ${SOURCE_DIR}/src
        src/)
target_compile_definitions(test_collector PUBLIC COLLECTOR_TEST)
target_sources(test_collector PRIVATE
        src/collector.c
        src/metrics.c
        external_libs/unity/unity.c
        external_libs/cjson/cJSON.c)
target_link_libraries(test_collector PRIVATE tinycbor)
add_test(test_collector test_collector)

## Test Metrics
add_executable(test_metrics EXCLUDE_FROM_ALL test/test_metrics.c)
target_include_directories(test_metrics PRIVATE
        external_libs/unity
        external_libs/cjson
        ${SOURCE_DIR}/src
        src/)
target_compile_definitions(test_metrics PUBLIC COLLECTOR_TEST)
target_sources(test_metrics PRIVATE
        src/collector.c
        src/metrics.c
        external_libs/unity/unity.c
        external_libs/cjson/cJSON.c)
target_link_libraries(test_metrics PRIVATE
        tinycbor)
add_test(test_metrics test_metrics)