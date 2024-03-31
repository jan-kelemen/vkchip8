#ifndef VKCHIP8_SDL_WINDOW_INCLUDED
#define VKCHIP8_SDL_WINDOW_INCLUDED

#include <vulkan_window.hpp>

#include <SDL.h>
#include <vulkan/vulkan_core.h>

#include <string_view>

namespace vkchip8
{
    class [[nodiscard]] sdl_guard final
    {
    public: // Construction
        sdl_guard(uint32_t flags);

        sdl_guard(sdl_guard const&) = delete;

        sdl_guard(sdl_guard&&) noexcept = delete;

    public: // Destruction
        ~sdl_guard();

    public: // Operators
        sdl_guard& operator=(sdl_guard const&) = delete;

        sdl_guard& operator=(sdl_guard&&) = delete;
    };

    class [[nodiscard]] sdl_window final : public vulkan_window
    {
    public: // Construction
        sdl_window(std::string_view title,
            SDL_WindowFlags window_flags,
            bool centered,
            int width,
            int height);

        sdl_window(sdl_window const&) = delete;

        sdl_window(sdl_window&&) noexcept = delete;

    public: // Destruction
        ~sdl_window() override;

    public: // Interface
        [[nodiscard]] constexpr SDL_Window* native_handle() const noexcept;

    public: // vulkan_window implementation
        std::vector<char const*> required_extensions() const override;

        bool create_surface(VkInstance instance,
            VkSurfaceKHR& surface) const override;

        VkExtent2D swap_extent(
            VkSurfaceCapabilitiesKHR const& capabilities) const override;

        void init_imgui() override;

        void shutdown_imgui() override;

    public: // Operators
        sdl_window& operator=(sdl_window const&) = delete;

        sdl_window& operator=(sdl_window&&) noexcept = delete;

    private: // Data
        SDL_Window* window_;
    };
} // namespace vkchip8

[[nodiscard]]
inline constexpr SDL_Window* vkchip8::sdl_window::native_handle() const noexcept
{
    return window_;
}

#endif // !VKCHIP8_SDL_WINDOW_INCLUDED
