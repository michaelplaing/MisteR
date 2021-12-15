add_library(
    mrmo MODULE
    mrmo.c
    ../include/mister/mister.h 
    ../include/mister/redismodule.h
)
target_include_directories(mrmo PRIVATE ../include)
