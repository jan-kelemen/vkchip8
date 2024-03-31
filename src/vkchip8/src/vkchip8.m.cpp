#include <screen.hpp>
#include <vulkan_context.hpp>
#include <vulkan_device.hpp>
#include <vulkan_renderer.hpp>
#include <vulkan_swap_chain.hpp>

#include <chip8.hpp>
#include <pc_speaker.hpp>

#include "imgui.h"
#include "imgui_impl_sdl2.hpp"
#include "imgui_impl_vulkan.hpp"
#include <SDL.h>

#include <SDL_vulkan.h>
#include <stdio.h> // printf, fprintf
#include <stdlib.h> // abort
#include <vulkan/vulkan.h>

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <map>
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

    std::map<SDL_Keycode, vkchip8::key_code> key_map{
        {SDLK_1, vkchip8::key_code::k1},
        {SDLK_2, vkchip8::key_code::k2},
        {SDLK_3, vkchip8::key_code::k3},
        {SDLK_4, vkchip8::key_code::k4},
        {SDLK_q, vkchip8::key_code::k4},
        {SDLK_w, vkchip8::key_code::k5},
        {SDLK_e, vkchip8::key_code::k6},
        {SDLK_r, vkchip8::key_code::kC},
        {SDLK_a, vkchip8::key_code::k7},
        {SDLK_s, vkchip8::key_code::k8},
        {SDLK_d, vkchip8::key_code::k9},
        {SDLK_f, vkchip8::key_code::kE},
        {SDLK_z, vkchip8::key_code::kA},
        {SDLK_x, vkchip8::key_code::k0},
        {SDLK_c, vkchip8::key_code::kB},
        {SDLK_v, vkchip8::key_code::kF},
    };
} // namespace

// Main code
int main([[maybe_unused]] int argc, char** argv)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
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
    SDL_Window* window = SDL_CreateWindow("vkchip8",
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

    vkchip8::pc_speaker speaker;

    auto code{read_file(argv[1])};
    vkchip8::chip8 emulator{vkchip8::chip8::memory_size,
        [&speaker]() { speaker.beep(); }};

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

        vkchip8::screen screen_renderer{&emulator};
        screen_renderer.attach_renderer(&device,
            renderer.descriptor_pool(),
            swap_chain.image_format(),
            swap_chain.image_count());

        // Main loop
        bool done = false;
        while (!done)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                {
                    done = true;
                }
                if (event.type == SDL_WINDOWEVENT &&
                    event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID(window))
                {
                    done = true;
                }

                if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
                {
                    if (auto it{key_map.find(event.key.keysym.sym)};
                        it != key_map.cend())
                    {
                        emulator.key_event(event.type == SDL_KEYDOWN
                                ? vkchip8::key_event_type::pressed
                                : vkchip8::key_event_type::released,
                            it->second);
                    }
                    else
                    {
                        spdlog::error("Unrecogrnized key code: {}",
                            event.key.keysym.sym);
                    }
                }
            }

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            // ImGui::ShowDemoWindow();
            ImGui::ShowMetricsWindow();

            ImGui::Begin("Registers");
            for (size_t i{}; i != emulator.data_registers_.size(); ++i)
            {
                auto const label{"V" + std::to_string(i)};
                ImGui::LabelText(label.c_str(),
                    "%x",
                    emulator.data_registers_[i]);
            }
            ImGui::LabelText("delay", "%x", emulator.delay_timer_);
            ImGui::End();

            for (auto i{0}; i != 16; ++i)
            {
                emulator.tick();
            }

            emulator.tick_timers();
            speaker.tick();

            renderer.draw(screen_renderer);
        }
        vkDeviceWaitIdle(device.logical());
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
