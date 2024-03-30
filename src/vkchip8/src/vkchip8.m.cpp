#include <vulkan_context.hpp>
#include <vulkan_device.hpp>
#include <vulkan_renderer.hpp>
#include <vulkan_swap_chain.hpp>

#include <chip8.hpp>

#include "imgui.h"
#include "imgui_impl_sdl2.hpp"
#include "imgui_impl_vulkan.hpp"
#include <SDL.h>
#include <SDL_vulkan.h>
#include <stdio.h> // printf, fprintf
#include <stdlib.h> // abort
#include <vulkan/vulkan.h>

#include <filesystem>
#include <fstream>
#include <vector>

namespace
{
    [[nodiscard]] std::vector<char> read_file(std::filesystem::path const& file)
    {
        std::ifstream stream{file, std::ios::ate | std::ios::binary};

        if (!stream.is_open())
        {
            throw std::runtime_error{"failed to open file!"};
        }

        auto const eof{stream.tellg()};

        std::vector<char> buffer(static_cast<size_t>(eof));
        stream.seekg(0);

        stream.read(buffer.data(), eof);

        return buffer;
    }
} // namespace

namespace
{
#ifdef NDEBUG
    constexpr bool enable_validation_layers{false};
#else
    constexpr bool enable_validation_layers{true};
#endif
} // namespace

// Main code
int main([[maybe_unused]] int argc, char** argv)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
        0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with Vulkan graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags) (SDL_WINDOW_VULKAN |
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+Vulkan example",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }

    auto code{read_file(argv[1])};
    vkchip8::chip8 emulator;
    emulator.load(
        std::span{reinterpret_cast<std::byte*>(code.data()), code.size()});

    {
        auto context{vkchip8::create_context(window, enable_validation_layers)};
        auto device{vkchip8::create_device(context)};
        vkchip8::vulkan_swap_chain swap_chain{window, &context, &device};
        vkchip8::vulkan_renderer renderer{window,
            &context,
            &device,
            &swap_chain};

        // Main loop
        bool done = false;
        while (!done)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                    done = true;
                if (event.type == SDL_WINDOWEVENT &&
                    event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID(window))
                    done = true;
            }

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            ImGui::ShowDemoWindow();
            ImGui::ShowMetricsWindow();

            ImGui::Begin("Screen");
            for (auto const& screen_row : emulator.screen_data())
            {
                std::string pixels;
                for (size_t i{}; i != screen_row.size(); ++i)
                {
                    pixels += screen_row.test(i) ? 'X' : ' ';
                }
                ImGui::Text(pixels.c_str());
            }
            ImGui::End();

            emulator.tick();

            renderer.draw();
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
