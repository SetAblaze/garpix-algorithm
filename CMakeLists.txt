cmake_minimum_required(VERSION 3.14)

if (WIN32)
    project(GARPIX_PROJECT LANGUAGES CXX)
elseif(UNIX)
    project(GARPIX_PROJECT)
endif()

set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

#=================== INCLUSION OF Project Files ====================#
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories(${SOURCE_DIR})

file(GLOB_RECURSE SOURCES
    "${SOURCE_DIR}/*.cpp"
)

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)

# Add the executable
# switch win32 for exe if needed
if(WIN32)
    add_executable(GARPIX_PROJECT #[[WIN32]] ${SOURCES})
elseif(UNIX)
    add_executable(GARPIX_PROJECT ${SOURCES})
endif()

# Add the target includes for GARPIX_PROJECT
target_include_directories(GARPIX_PROJECT PRIVATE ${SOURCE_DIR})

#===================== LINKING LIBRARIES =======================#
