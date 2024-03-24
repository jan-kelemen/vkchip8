add_executable(vkchip8)

target_sources(vkchip8
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vkchip8.m.cpp
)

target_link_libraries(vkchip8
    PRIVATE
        fmt::fmt
        project-options
)

if (VKCHIP8_BUILD_TESTS)
    add_executable(vkchip8_test)

    target_sources(vkchip8_test
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/test/vkchip8.t.cpp
    )

    target_link_libraries(vkchip8_test
        PRIVATE
            Catch2::Catch2WithMain
            project-options
    )

    if (NOT CMAKE_CROSSCOMPILING)
        include(Catch)
        catch_discover_tests(vkchip8_test)
    endif()
endif()
