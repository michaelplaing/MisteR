file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${mister_SOURCE_DIR}/zlog/src/*.h")

add_library(
    mrzlog SHARED
    buf.c
    category.c
    category_table.c
    conf.c
    event.c
    format.c
    level.c
    level_list.c
    mdc.c
    record.c
    record_table.c
    rotater.c
    rule.c
    spec.c
    thread.c
    zc_arraylist.c
    zc_hashtable.c
    zc_profile.c
    zc_util.c
    zlog-chk-conf.c
    zlog.c
    ${HEADER_LIST}
)
