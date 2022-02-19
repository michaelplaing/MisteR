find_package(Catch2 REQUIRED)

include_directories(mister PUBLIC ${mister_SOURCE_DIR}/include)

# build a library which includes the Catch2 main(), Catch2::Catch2, and testing utilities
add_library(testlib CatchMain.cpp test_util.c test_util.h)
target_compile_features(testlib PRIVATE cxx_std_17)

# tests must be executables
add_executable(test-000-connect test-000-connect.cpp)
target_compile_features(test-000-connect PRIVATE cxx_std_17)

add_executable(test-001-connack test-001-connack.cpp)
target_compile_features(test-001-connack PRIVATE cxx_std_17)

add_executable(test-002-publish test-002-publish.cpp)
target_compile_features(test-002-publish PRIVATE cxx_std_17)

# link to 'mister' and as well as our test library
target_link_libraries(test-000-connect PRIVATE mister testlib)
target_link_libraries(test-001-connack PRIVATE mister testlib)
target_link_libraries(test-002-publish PRIVATE mister testlib)

 # COMMAND can be a target
 add_test(NAME test-000-connect COMMAND test-000-connect)
 add_test(NAME test-001-connack COMMAND test-001-connack)
 add_test(NAME test-002-publish COMMAND test-002-publish)

# set up fixture files - configure_file recopies if a file changes
configure_file("fixtures/default_connect_printable.txt" "fixtures/default_connect_printable.txt" COPYONLY)
configure_file("fixtures/default_connect_packet.bin" "fixtures/default_connect_packet.bin" COPYONLY)
configure_file("fixtures/will_connect_printable.txt" "fixtures/will_connect_printable.txt" COPYONLY)
configure_file("fixtures/will_connect_packet.bin" "fixtures/will_connect_packet.bin" COPYONLY)
configure_file("fixtures/complex_connect_printable.txt" "fixtures/complex_connect_printable.txt" COPYONLY)
configure_file("fixtures/complex_connect_packet.bin" "fixtures/complex_connect_packet.bin" COPYONLY)

configure_file("fixtures/default_connack_printable.txt" "fixtures/default_connack_printable.txt" COPYONLY)
configure_file("fixtures/default_connack_packet.bin" "fixtures/default_connack_packet.bin" COPYONLY)
configure_file("fixtures/complex_connack_printable.txt" "fixtures/complex_connack_printable.txt" COPYONLY)
configure_file("fixtures/complex_connack_packet.bin" "fixtures/complex_connack_packet.bin" COPYONLY)
