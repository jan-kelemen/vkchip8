#ifndef VKCHIP8_CHIP8_INCLUDED
#define VKCHIP8_CHIP8_INCLUDED

#include <array>
#include <bitset>
#include <cstddef>
#include <functional>
#include <span>
#include <vector>

namespace vkchip8
{
    enum class key_code : uint8_t
    {
        k0,
        k1,
        k2,
        k3,
        k4,
        k5,
        k6,
        k7,
        k8,
        k9,
        kA,
        kB,
        kC,
        kD,
        kE,
        kF
    };

    enum class key_event_type
    {
        released,
        pressed
    };

    class [[nodiscard]] chip8 final
    {
    public: // Constants
        static constexpr size_t screen_width{64};
        static constexpr size_t screen_height{32};
        static constexpr size_t stack_size{16};
        static constexpr size_t memory_size{4096};
        static constexpr uint16_t start_address{0x200};

    public: // Construction
        chip8() : chip8{memory_size} { }

        explicit chip8(
            size_t ram_size,
            std::function<void(void)> beep_callback = []() {});

        chip8(chip8 const&) = default;
        chip8(chip8&&) noexcept = default;

    public: // Destruction
        ~chip8() = default;

    public: // Interface
        void tick();

        void key_event(key_event_type type, key_code code);

        void load(std::span<std::byte> program,
            uint16_t address = start_address);

        [[nodiscard]] std::array<std::bitset<screen_width>,
            screen_height> const&
        screen_data() const
        {
            return screen_;
        }

    public: // Operators
        chip8& operator=(chip8 const&) = default;
        chip8& operator=(chip8&&) noexcept = default;

    private: // Helpers
        void reset();

        [[nodiscard]] uint16_t fetch();
        void execute(uint16_t operation);
        void tick_timers();

        void push_stack(uint16_t value);
        [[nodiscard]] uint16_t pop_stack();

    public: // Data
        std::function<void(void)> beep_callback_;

        std::vector<std::byte> memory_;
        uint16_t program_counter_{start_address};
        std::array<uint8_t, 16> data_registers_{};
        uint16_t i_register_{};
        uint8_t sound_timer_{};
        uint8_t delay_timer_{};
        std::array<std::bitset<screen_width>, screen_height> screen_{};
        std::array<uint16_t, stack_size> stack_{};
        size_t stack_pointer_{};
        std::bitset<16> keys_{};
    };
} // namespace vkchip8

#endif // !VKCHIP8_CHIP8_INCLUDED
