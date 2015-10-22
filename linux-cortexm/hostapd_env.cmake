set(ENV{CC} ${CMAKE_C_COMPILER})
set(ENV{CFLAGS}  ${CMAKE_C_FLAGS})
set(ENV{LDFLAGS} ${CMAKE_C_FLAGS})
execute_process(
    COMMAND make V=1
    WORKING_DIRECTORY ${HOSTAPD_SOURCE_DIR}
    OUTPUT_VARIABLE _RES
    ERROR_VARIABLE _RES
)
execute_process(COMMAND cmake -E echo ${_RES})
