find_library(JEMALLOC jemalloc REQUIRED)
find_library(ZLOG zlog REQUIRED)

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${mister_SOURCE_DIR}/include/mister/*.h")

add_library(
    mister SHARED
    init.c connect.c connack.c publish.c puback.c subscribe.c suback.c packet.c util.c memory.c
    mister_internal.h ${HEADER_LIST}
)

target_include_directories(mister PUBLIC ../include)
target_link_libraries(mister PUBLIC zlog jemalloc)
