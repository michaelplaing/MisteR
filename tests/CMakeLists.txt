find_package(PkgConfig REQUIRED)
pkg_check_modules (Catch2 REQUIRED catch2>=2.13.8)
find_package(Catch2 REQUIRED)

include_directories(mister PUBLIC ${mister_SOURCE_DIR}/include)

# Tests need to be added as executables first - include CatchMain
add_executable(default_connect test-000-default_connect.cpp CatchMain.cpp util.c util.h)

# Use C++17 in the test
target_compile_features(default_connect PRIVATE cxx_std_17)

# link to 'mister' as well as the Catch2 testing library
target_link_libraries(default_connect PRIVATE mister Catch2::Catch2)

add_test(NAME default_connect COMMAND default_connect) # Command can be a target
configure_file("fixtures/init_mdata_dump.txt" "fixtures/init_mdata_dump.txt" COPYONLY)
configure_file("fixtures/init_pack_connect.bin" "fixtures/init_pack_connect.bin" COPYONLY)

add_subdirectory(unittests)
