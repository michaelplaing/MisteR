add_executable(
    mrapp 
    mrapp.c 
    ../include/mister/redismodule.h 
    ../include/mister/mister.h 
    ../include/mister/connect.h
)

target_link_libraries(mrapp PRIVATE mister ev hiredis)
