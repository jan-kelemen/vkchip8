add_executable(vkchip8)

target_sources(vkchip8
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/global_data.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/global_data.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_impl_sdl2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_impl_sdl2.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_impl_vulkan.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_impl_vulkan.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pc_speaker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pc_speaker.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/screen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/screen.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_context.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_device.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_device.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_pipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_pipeline.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_renderer.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_swap_chain.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_swap_chain.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_utility.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_utility.hpp
        
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vkchip8.m.cpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert
        ${CMAKE_CURRENT_BINARY_DIR}/vert.spv
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag
        ${CMAKE_CURRENT_BINARY_DIR}/frag.spv
)

target_include_directories(vkchip8
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(vkchip8
    PRIVATE
        fmt::fmt
        glm::glm
        imgui::imgui
        vulkan-headers::vulkan-headers
        Vulkan::Loader
        SDL2::SDL2main
        spdlog::spdlog
        chip8
        project-options
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vert.spv
    COMMAND 
        ${GLSLC_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert -o ${CMAKE_CURRENT_BINARY_DIR}/vert.spv
    DEPENDS 
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/frag.spv
    COMMAND 
        ${GLSLC_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag -o ${CMAKE_CURRENT_BINARY_DIR}/frag.spv
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag
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

source_group("Header Files"
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/global_data.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_impl_sdl2.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_impl_vulkan.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pc_speaker.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/screen.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_context.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_device.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_pipeline.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_renderer.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_swap_chain.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_utility.hpp
)

source_group("Source Files"
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/global_data.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_impl_sdl2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_impl_vulkan.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pc_speaker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/screen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_device.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_pipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_swap_chain.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_utility.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vkchip8.m.cpp
)

source_group("Shader Files"
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag
)

set_property(TARGET vkchip8 
    PROPERTY 
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
