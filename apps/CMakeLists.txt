find_package(PkgConfig REQUIRED)
pkg_check_modules (MYLIBS REQUIRED hiredis libuv)

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${mister_SOURCE_DIR}/include/mister/*.h")

add_executable(
    mrapp 
    mrapp.c 
    ${HEADER_LIST}
)

target_link_libraries(mrapp PRIVATE mister ${MYLIBS_LIBRARIES})
