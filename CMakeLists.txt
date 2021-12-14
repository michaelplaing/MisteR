cmake_minimum_required(VERSION 3.22)

project(
    MisteR VERSION 1.0
    DESCRIPTION "MQTT for Redis"
    LANGUAGES C
)

add_executable(
    mr
    apps/mr.c apps/connect.c apps/pack.c
    include/mister/mister.h
    include/mister/connect.h
    include/mister/redismodule.h
    include/mister/pack.h
    include/mister/pack_internal.h
)
target_include_directories(mr PRIVATE include)

add_library(
    mister MODULE
    module/mister.c
    include/mister/mister.h 
    include/mister/redismodule.h
)
target_include_directories(mister PRIVATE include)
