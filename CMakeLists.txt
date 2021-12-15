cmake_minimum_required(VERSION 3.22)

project(
    mister VERSION 1.0
    DESCRIPTION "MQTT for Redis"
    LANGUAGES C
)

add_subdirectory(src)

add_subdirectory(apps)

add_subdirectory(module)
