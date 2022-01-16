find_package(PkgConfig REQUIRED)
pkg_check_modules (TESTLIBS REQUIRED cmocka)

include_directories(mister PUBLIC ${mister_SOURCE_DIR}/include)

set(
    MISTER_UNIT_TESTS
    test_mr_init_connect_packet
)

foreach(_UNIT_TEST ${MISTER_UNIT_TESTS})
    add_cmocka_test(
        ${_UNIT_TEST}
        SOURCES ${_UNIT_TEST}.c
        LINK_LIBRARIES mister ${TESTLIBS_LIBRARIES}
    )
endforeach()
