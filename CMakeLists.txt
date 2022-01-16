cmake_minimum_required(VERSION 3.22)

# Specify search path for CMake modules to be loaded by include()
# and find_package()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

project(
    mister VERSION 1.0
    DESCRIPTION "MQTT for Redis"
    LANGUAGES C
)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)

if(EXISTS "${LOC_PATH}")
    message(
        FATAL_ERROR "You cannot build in a source directory "
        "(or any directory with a CMakeLists.txt file). "
        "Please make a build subdirectory and use that. "
        "Remove the CMakeCache.txt and CMakeFiles/ just created."
    )
endif()

option(UNIT_TESTING "Build with unit tests" ON)
option(MODULE_TESTING "Build with module tests" OFF) # not yet implmented

if (UNIT_TESTING OR MODULE_TESTING)
  set(BUILD_STATIC_LIB ON)
endif()

if (UNIT_TESTING)
    find_package(CMocka REQUIRED)
endif ()

if (UNIT_TESTING)
    include(AddCMockaTest)
    add_subdirectory(tests)
endif ()

add_subdirectory(src)

add_subdirectory(apps)

add_subdirectory(module)

add_subdirectory(zlog)

add_subdirectory(scratch)

message(STATUS "********************************************")
message(STATUS "********** ${PROJECT_NAME} build options : **********")

message(STATUS "Unit testing: ${UNIT_TESTING}")
message(STATUS "Module code testing: ${MODULE_TESTING}")

message(STATUS "********************************************")
