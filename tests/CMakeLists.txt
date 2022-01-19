find_package(Catch2 REQUIRED)

include_directories(mister PUBLIC ${mister_SOURCE_DIR}/include)

# build a library which includes the Catch2 main(), Catch2::Catch2, and testing utilities
add_library(testlib CatchMain.cpp util.c util.h)
target_compile_features(testlib PRIVATE cxx_std_17)

# tests must be executables
add_executable(test-000-default_connect test-000-default_connect.cpp)
add_executable(test-001-will_connect test-001-will_connect.cpp)

target_compile_features(test-000-default_connect PRIVATE cxx_std_17)
target_compile_features(test-001-will_connect PRIVATE cxx_std_17)

# link to 'mister' as well as our test library
target_link_libraries(test-000-default_connect PRIVATE mister testlib)
target_link_libraries(test-001-will_connect PRIVATE mister testlib)

 # COMMAND can be a target
add_test(NAME test-000-default_connect COMMAND test-000-default_connect)
add_test(NAME test-001-will_connect COMMAND test-001-will_connect)

# copy the fixture files to the build directory
configure_file("fixtures/default_connect_mdata_dump.txt" "fixtures/default_connect_mdata_dump.txt" COPYONLY)
configure_file("fixtures/default_connect_packet.bin" "fixtures/default_connect_packet.bin" COPYONLY)
configure_file("fixtures/will_connect_mdata_dump.txt" "fixtures/will_connect_mdata_dump.txt" COPYONLY)
configure_file("fixtures/will_connect_packet.bin" "fixtures/will_connect_packet.bin" COPYONLY)

# add the cmocka test directory
add_subdirectory(unittests)
