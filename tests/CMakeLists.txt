find_package(Catch2 REQUIRED)

include_directories(mister PUBLIC ${mister_SOURCE_DIR}/include)

# build a library which includes the Catch2 main(), Catch2::Catch2, and testing utilities
add_library(testlib CatchMain.cpp test_util.c test_util.h)
target_compile_features(testlib PRIVATE cxx_std_17)

set(
    TESTLIST
    test-000-connect
    test-001-connack
    test-002-publish
    test-003-puback
)

message(STATUS Tests:)
list(APPEND CMAKE_MESSAGE_INDENT "    ")
foreach(TESTNAME ${TESTLIST})
    message(STATUS ${TESTNAME})
    add_executable(${TESTNAME} ${TESTNAME}.cpp) # tests must be executables
    target_compile_features(${TESTNAME} PRIVATE cxx_std_17)
    target_link_libraries(${TESTNAME} PRIVATE mister testlib) # link to 'mister'
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME}) # COMMAND can be a target
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)

set(
    FIXTUREFILES
    fixtures/default_connect_printable.txt
    fixtures/default_connect_packet.bin
    fixtures/will_connect_printable.txt
    fixtures/will_connect_packet.bin
    fixtures/complex_connect_printable.txt
    fixtures/complex_connect_packet.bin
    fixtures/default_connack_printable.txt
    fixtures/default_connack_packet.bin
    fixtures/complex_connack_printable.txt
    fixtures/complex_connack_packet.bin
    fixtures/default_publish_printable.txt
    fixtures/default_publish_packet.bin
    fixtures/complex_publish_printable.txt
    fixtures/complex_publish_packet.bin
    fixtures/default_puback_printable.txt
    fixtures/default_puback_packet.bin
    fixtures/complex_puback_printable.txt
    fixtures/complex_puback_packet.bin
)

message(STATUS Fixture Files:)
list(APPEND CMAKE_MESSAGE_INDENT "  ")
foreach(FILENAME ${FIXTUREFILES})
    message(STATUS ${FILENAME})
    configure_file(${FILENAME} ${FILENAME} COPYONLY)
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)
