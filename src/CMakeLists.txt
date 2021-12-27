find_package(PkgConfig REQUIRED)
pkg_check_modules (MYLIBS REQUIRED jemalloc)

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${mister_SOURCE_DIR}/include/mister/*.h")

add_library(
    mister SHARED
    connect.c packet.c validate.c
    packet_internal.h connect_internal.h validate.h ${HEADER_LIST}
)

target_include_directories(mister PUBLIC ../include)
target_link_libraries(mister PRIVATE ${MYLIBS_LIBRARIES})
