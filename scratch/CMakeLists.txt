add_executable(
    scr
    scr.c
)

target_link_libraries(scr PRIVATE jemalloc)
