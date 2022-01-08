find_package(PkgConfig REQUIRED)
pkg_check_modules (MYLIBS REQUIRED jemalloc)

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${mister_SOURCE_DIR}/include/mister/*.h")

add_library(
    mister SHARED
    init.c connect.c connack.c packet.c util.c will.c memory.c
    packet_internal.h connect_internal.h connack_internal.h will_internal.h util_internal.h ${HEADER_LIST}
)

target_include_directories(mister PUBLIC ../include)
target_link_libraries(mister PRIVATE mrzlog ${MYLIBS_LIBRARIES})
