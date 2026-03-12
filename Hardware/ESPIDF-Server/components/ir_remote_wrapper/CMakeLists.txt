if(CMAKE_BUILD_EARLY_EXPANSION)
    idf_component_register(REQUIRES "arduino-esp32")
    return()
endif()

include(FetchContent)

if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

FetchContent_Declare(
        irremote_repo
        GIT_REPOSITORY https://github.com/crankyoldgit/IRremoteESP8266.git
        GIT_TAG        master
)

FetchContent_GetProperties(irremote_repo)
if(NOT irremote_repo_POPULATED)
    FetchContent_Populate(irremote_repo)
endif()

# COMPILE THE COMPONENT
# Find all the .cpp files in the newly downloaded src/ folder
file(GLOB_RECURSE library_sources "${irremote_repo_SOURCE_DIR}/src/*.cpp")

# Register the target so ESP-IDF can compile it
idf_component_register(
        SRCS ${library_sources}
        INCLUDE_DIRS "${irremote_repo_SOURCE_DIR}/src"
        REQUIRES "arduino-esp32"
)