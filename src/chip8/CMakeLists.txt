add_library(chip8)

target_sources(chip8
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/chip8.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/chip8.cpp
)

target_include_directories(chip8
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(chip8
    PRIVATE
        project-options
)

if (VKCHIP8_BUILD_TESTS)
    add_executable(chip8_test)

    target_sources(chip8_test
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/test/chip8.t.cpp
    )

    target_link_libraries(chip8_test
        PRIVATE
            Catch2::Catch2WithMain
            project-options
    )

    if (NOT CMAKE_CROSSCOMPILING)
        include(Catch)
        catch_discover_tests(chip8_test)
    endif()
endif()

source_group("Header Files"
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/chip8.hpp
)

source_group("Source Files"
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/chip8.cpp
)
