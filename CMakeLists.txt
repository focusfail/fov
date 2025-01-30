cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(DEFAULT_BUILD_TYPE "Release")
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")
add_subdirectory(third_party/cglm)
add_subdirectory(third_party/glfw)

# glad
add_library(glad STATIC
    third_party/glad/src/glad.c
)
target_include_directories(glad PUBLIC third_party/glad/include)

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE third_party/stb)

# logc
add_library(logc STATIC
    third_party/log.c/src/log.c
)
target_include_directories(logc PUBLIC third_party/log.c/src)
target_compile_definitions(logc PUBLIC LOG_USE_COLOR)

project(fov)
file(GLOB_RECURSE fov_src app/source/*.c)
add_executable(fov)
target_sources(fov PRIVATE ${fov_src})
target_link_libraries(fov PRIVATE glfw stb glad cglm logc UxTheme Dwmapi)
target_include_directories(fov PRIVATE app/include)
target_compile_definitions(fov PRIVATE 
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    $<$<CONFIG:Release>:RELEASE_BUILD>
)
