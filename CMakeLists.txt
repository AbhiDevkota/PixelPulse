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
    message(FATAL_ERROR "No .cpp files found under ${CMAKE_CURRENT_SOURCE_DIR}/src")
endif()

set(COREZONE_SOURCES
    ${SRC_FILES}
    ${HEADER_FILES}
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CoreZone.rc")
    list(APPEND COREZONE_SOURCES CoreZone.rc)
endif()

add_executable(CoreZone
    ${COREZONE_SOURCES}
 "include/Menu.h" "include/Settings.h")

if (MSVC)
    target_compile_options(CoreZone PRIVATE /W4)   # removed /WX so warnings don't kill build
else()
    target_compile_options(CoreZone PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(CoreZone PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(CoreZone PRIVATE cxx_std_20)

target_link_libraries(CoreZone PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio SFML::Network)

# -----------------------
# ASSET COPYING - FIXED & MORE RELIABLE
# -----------------------

# Copy at configure time (helps Visual Studio)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/fonts")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/fonts" DESTINATION "${CMAKE_BINARY_DIR}/bin")
    message(STATUS "✅ Fonts copied to build/bin/fonts/")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/audios")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/audios" DESTINATION "${CMAKE_BINARY_DIR}/bin")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Icons")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Icons" DESTINATION "${CMAKE_BINARY_DIR}/bin")
    message(STATUS "✅ Icons copied to build/bin/Icons/")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_BINARY_DIR}/bin")
endif()

# Also copy after every build (backup)
add_custom_command(TARGET CoreZone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CoreZone>/fonts"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CoreZone>/audios"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CoreZone>/Icons"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CoreZone>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        "$<TARGET_FILE_DIR:CoreZone>/fonts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/audios"
        "$<TARGET_FILE_DIR:CoreZone>/audios"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Icons"
        "$<TARGET_FILE_DIR:CoreZone>/Icons"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:CoreZone>/assets"
    COMMENT "Copying fonts, audios, icons & assets to executable directory"
    VERBATIM
)