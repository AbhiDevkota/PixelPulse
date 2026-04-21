cmake_minimum_required(VERSION 3.28)
project(CoreZone LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)

# -----------------------
# Glob sources and headers
# -----------------------
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

if(NOT SRC_FILES)
    message(WARNING "No source files found in ${CMAKE_CURRENT_SOURCE_DIR}/src")
endif()

add_executable(CoreZone
    ${SRC_FILES}
    ${HEADER_FILES}
)

if (MSVC)
    target_compile_options(CoreZone PRIVATE /W4 /WX)
else()
    target_compile_options(CoreZone PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

target_include_directories(CoreZone PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(CoreZone PRIVATE cxx_std_20)

target_link_libraries(CoreZone PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio SFML::Network)

# -----------------------
# Copy assets
# -----------------------
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
else()
    message(WARNING "assets folder not found")
endif()

# Copy fonts next to executable after every build
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/fonts")
    add_custom_command(TARGET CoreZone POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        "$<TARGET_FILE_DIR:CoreZone>/fonts"
        COMMENT "Copying fonts to output directory"
    )
else()
    message(WARNING "fonts folder not found at ${CMAKE_CURRENT_SOURCE_DIR}/fonts")
endif()

# Copy audios next to executable after every build
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/audios")
    add_custom_command(TARGET CoreZone POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/audios"
        "$<TARGET_FILE_DIR:CoreZone>/audios"
        COMMENT "Copying audios to output directory"
    )
else()
    message(WARNING "audios folder not found at ${CMAKE_CURRENT_SOURCE_DIR}/audios")
endif()