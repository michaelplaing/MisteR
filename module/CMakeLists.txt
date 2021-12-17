file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${mister_SOURCE_DIR}/include/mister/*.h")

add_library(
    mrmo MODULE
    mrmo.c
    ${HEADER_LIST}
)

target_include_directories(mrmo PRIVATE ../include)
target_link_libraries(mrapp PRIVATE mister)