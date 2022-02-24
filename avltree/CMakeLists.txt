#file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${mister_SOURCE_DIR}/include/mister/*.h")

add_executable(
    testavl
    testavl.c avltree.c
    avltree.h
)

# target_link_libraries(scr PRIVATE mister)
