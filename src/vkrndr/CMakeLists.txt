add_library(vkrndr)

target_sources(vkrndr
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/global_data.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/sdl_window.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_context.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_device.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_pipeline.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_render_target.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_renderer.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_swap_chain.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_utility.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan_window.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/global_data.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sdl_window.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_device.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_pipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_render_target.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_swap_chain.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan_utility.cpp
)

target_include_directories(vkrndr
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(vkrndr
    PUBLIC
        imgui::imgui
        vulkan-headers::vulkan-headers
        SDL2::SDL2
        imgui_impl
    PRIVATE
        fmt::fmt
        glm::glm
        Vulkan::Loader
        spdlog::spdlog
        project-options
)

