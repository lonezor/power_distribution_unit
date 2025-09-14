add_library(rc_relay_32 STATIC)
add_library(relay_controller::relay_32 ALIAS rc_relay_32)

target_sources(rc_relay_32
    PRIVATE
    src/relay.c
)

set_property(TARGET rc_relay_32 PROPERTY C_STANDARD 99)

set_property(TARGET rc_relay_32 PROPERTY POSITION_INDEPENDENT_CODE TRUE)

target_include_directories(rc_relay_32
   PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
    TARGETS
    rc_relay_32
)
