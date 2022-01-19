find_package(PkgConfig REQUIRED)
pkg_check_modules (Catch2 REQUIRED catch2>=2.13.8)
find_package(Catch2 REQUIRED)

include_directories(mister PUBLIC ${mister_SOURCE_DIR}/include)

# build a library which includes the Catch2 main(), Catch2::Catch2, and testing utilities
add_library(testlib CatchMain.cpp util.c util.h)
target_compile_features(testlib PRIVATE cxx_std_17)

# tests must be executables
add_executable(test-000-default_connect test-000-default_connect.cpp)

# Use C++17
target_compile_features(test-000-default_connect PRIVATE cxx_std_17)

# link to 'mister' as well as our test library
target_link_libraries(test-000-default_connect PRIVATE mister testlib)

add_test(NAME test-000-default_connect COMMAND test-000-default_connect) # Command can be a target

# copy the fixture files to the build directory
configure_file("fixtures/init_mdata_dump.txt" "fixtures/init_mdata_dump.txt" COPYONLY)
configure_file("fixtures/init_pack_connect.bin" "fixtures/init_pack_connect.bin" COPYONLY)

# add the cmocka test directory
add_subdirectory(unittests)
