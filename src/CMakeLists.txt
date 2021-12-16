file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${mister_SOURCE_DIR}/include/mister/*.h")

add_library(
    mister SHARED
    connect.c pack.c 
    pack_internal.h connect_internal.h ${HEADER_LIST}
)
target_include_directories(mister PUBLIC ../include)
target_link_libraries(mister PRIVATE ev hiredis)

