add_executable(rc_ctrl_32
    src/main.c
)

set_property(TARGET rc_ctrl_32 PROPERTY C_STANDARD 99)

set_property(TARGET rc_ctrl_32 PROPERTY POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(rc_ctrl_32
    PRIVATE
    relay_controller::relay_32
)

install(
    TARGETS
    rc_ctrl_32
)
