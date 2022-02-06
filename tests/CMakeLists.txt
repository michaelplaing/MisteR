find_package(Catch2 REQUIRED)

include_directories(mister PUBLIC ${mister_SOURCE_DIR}/include)

# build a library which includes the Catch2 main(), Catch2::Catch2, and testing utilities
add_library(testlib CatchMain.cpp util.c util.h)
target_compile_features(testlib PRIVATE cxx_std_17)

# tests must be executables
add_executable(test-000-connect_packet test-000-connect_packet.cpp)
target_compile_features(test-000-connect_packet PRIVATE cxx_std_17)

# link to 'mister' and as well as our test library
target_link_libraries(test-000-connect_packet PRIVATE mister testlib)

 # COMMAND can be a target
add_test(NAME test-000-connect_packet COMMAND test-000-connect_packet)

# copy the fixture files to the build directory - configure_file watches for changes and will recopy
configure_file("fixtures/default_connect_printable_mdata.txt" "fixtures/default_connect_printable_mdata.txt" COPYONLY)
configure_file("fixtures/default_connect_packet.bin" "fixtures/default_connect_packet.bin" COPYONLY)
configure_file("fixtures/will_connect_printable_mdata.txt" "fixtures/will_connect_printable_mdata.txt" COPYONLY)
configure_file("fixtures/will_connect_packet.bin" "fixtures/will_connect_packet.bin" COPYONLY)
configure_file("fixtures/complex_connect_printable_mdata.txt" "fixtures/complex_connect_printable_mdata.txt" COPYONLY)
configure_file("fixtures/complex_connect_packet.bin" "fixtures/complex_connect_packet.bin" COPYONLY)
