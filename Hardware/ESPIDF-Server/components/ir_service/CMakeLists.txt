file(GLOB_RECURSE ir_service_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

idf_component_register(
        SRCS ${ir_service_sources}
        INCLUDE_DIRS "include"
        REQUIRES ir_remote_wrapper
)
