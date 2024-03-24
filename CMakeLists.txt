cmake_minimum_required(VERSION 3.27)

project(vkchip8 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(VKCHIP8_ENABLE_CLANG_FORMAT "Enable clang-format in build" OFF)
option(VKCHIP8_ENABLE_CLANG_TIDY "Enable clang-tidy in build" OFF)
option(VKCHIP8_ENABLE_COMPILER_STATIC_ANALYSIS "Enable static analysis provided by compiler in build" OFF)
option(VKCHIP8_ENABLE_CPPCHECK "Enable cppcheck in build" OFF)
option(VKCHIP8_ENABLE_IWYU "Enable include-what-you-use in build" OFF)

find_package(fmt REQUIRED)
find_package(imgui REQUIRED)
find_package(glm REQUIRED)
find_package(SDL2)
find_package(spdlog REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(VulkanLoader REQUIRED)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
    if (BUILD_TESTING)
        find_package(Catch2)
        set(VKCHIP8_BUILD_TESTS ON)
    endif()

    include(${PROJECT_SOURCE_DIR}/cmake/clang-tidy.cmake)
    include(${PROJECT_SOURCE_DIR}/cmake/cppcheck.cmake)
    include(${PROJECT_SOURCE_DIR}/cmake/iwyu.cmake)

    include(${PROJECT_SOURCE_DIR}/cmake/project-options.cmake)
endif()

add_subdirectory(src)
