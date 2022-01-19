find_library(HIREDIS hiredis REQUIRED)
find_library(LIBUV uv REQUIRED)

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${mister_SOURCE_DIR}/include/mister/*.h")

add_executable(
    mrapp
    mrapp.c
    ${HEADER_LIST}
)

target_link_libraries(mrapp PRIVATE mister mrzlog hiredis uv)
