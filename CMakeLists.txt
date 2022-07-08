cmake_minimum_required(VERSION 3.14)

if (WIN32)
    project(GARPIX_PROJECT LANGUAGES CXX)
elseif(UNIX)
    project(GARPIX_PROJECT)
endif()

set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

#===================== INCLUDING LIBRARIES =======================#

#=================== INCLUSION OF Project Files ====================#
include(ExternalProject)

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

include("${CMAKE_SOURCE_DIR}/vendor/rapidjson.cmake")


include_directories(
    ${SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${RAPIDJSON_INCLUDE_DIR}
    )

file(GLOB_RECURSE SOURCES
    "${SOURCE_DIR}/*.cpp"
)

# Add the executable
# switch win32 for exe if needed
if(WIN32)
    add_executable(GARPIX_PROJECT #[[WIN32]] ${SOURCES})
elseif(UNIX)
    add_executable(GARPIX_PROJECT ${SOURCES})
endif()

add_dependencies(GARPIX_PROJECT rapidjson)

# Add the target includes for GARPIX_PROJECT
target_include_directories(GARPIX_PROJECT PRIVATE ${SOURCE_DIR})

#===================== LINKING LIBRARIES =======================#