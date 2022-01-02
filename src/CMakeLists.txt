find_package(PkgConfig REQUIRED)
pkg_check_modules (MYLIBS REQUIRED jemalloc)

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${mister_SOURCE_DIR}/include/mister/*.h")

add_library(
    mister SHARED
    connect.c packet.c util.c will.c
    packet_internal.h connect_internal.h util.h ${HEADER_LIST}
)

target_include_directories(mister PUBLIC ../include)
target_link_libraries(mister PRIVATE mrzlog ${MYLIBS_LIBRARIES})
