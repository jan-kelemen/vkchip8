#include <chip8.hpp>

#include <algorithm>
#include <cassert>
#include <random>
#include <ranges>
#include <utility>

namespace
{
    [[nodiscard]] constexpr std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>
    individual_digits(uint16_t const operation)
    {
        auto a1 = operation & 0xF0'00;
        auto a2 = operation & 0x0F'00;
        auto a3 = operation & 0x00'F0;
        auto a4 = operation & 0x00'0F;

        return {static_cast<uint8_t>((operation & 0xF0'00) >> 12),
            static_cast<uint8_t>((operation & 0x0F'00) >> 8),
            static_cast<uint8_t>((operation & 0x00'F0) >> 4),
            static_cast<uint8_t>(operation & 0x00'0F)};
    }

    [[nodiscard]] constexpr uint16_t from_hex_digits(uint8_t digit1,
        uint8_t digit2,
        uint8_t digit3,
        uint8_t digit4)
    {
        return uint16_t{digit1} << 12 | uint16_t{digit2} << 8 |
            uint16_t{digit3} << 4 | digit4;
    }

    std::mt19937 random_engine{std::random_device{}()};

    // clang-format off
    constexpr std::array fontset{
        std::byte{0xF0}, std::byte{0x90}, std::byte{0x90}, std::byte{0x90}, std::byte{0xF0}, // 0
        std::byte{0x20}, std::byte{0x60}, std::byte{0x20}, std::byte{0x20}, std::byte{0x70}, // 1
        std::byte{0xF0}, std::byte{0x10}, std::byte{0xF0}, std::byte{0x80}, std::byte{0xF0}, // 2
        std::byte{0xF0}, std::byte{0x10}, std::byte{0xF0}, std::byte{0x10}, std::byte{0xF0}, // 3
        std::byte{0x90}, std::byte{0x90}, std::byte{0xF0}, std::byte{0x10}, std::byte{0x10}, // 4
        std::byte{0xF0}, std::byte{0x80}, std::byte{0xF0}, std::byte{0x10}, std::byte{0xF0}, // 5
        std::byte{0xF0}, std::byte{0x80}, std::byte{0xF0}, std::byte{0x90}, std::byte{0xF0}, // 6
        std::byte{0xF0}, std::byte{0x10}, std::byte{0x20}, std::byte{0x40}, std::byte{0x40}, // 7
        std::byte{0xF0}, std::byte{0x90}, std::byte{0xF0}, std::byte{0x90}, std::byte{0xF0}, // 8
        std::byte{0xF0}, std::byte{0x90}, std::byte{0xF0}, std::byte{0x10}, std::byte{0xF0}, // 9
        std::byte{0xF0}, std::byte{0x90}, std::byte{0xF0}, std::byte{0x90}, std::byte{0x90}, // A
        std::byte{0xE0}, std::byte{0x90}, std::byte{0xE0}, std::byte{0x90}, std::byte{0xE0}, // B
        std::byte{0xF0}, std::byte{0x80}, std::byte{0x80}, std::byte{0x80}, std::byte{0xF0}, // C
        std::byte{0xE0}, std::byte{0x90}, std::byte{0x90}, std::byte{0x90}, std::byte{0xE0}, // D
        std::byte{0xF0}, std::byte{0x80}, std::byte{0xF0}, std::byte{0x80}, std::byte{0xF0}, // E
        std::byte{0xF0}, std::byte{0x80}, std::byte{0xF0}, std::byte{0x80}, std::byte{0x80}  // F
    };
    // clang-format on

} // namespace

vkchip8::chip8::chip8(size_t ram_size, std::function<void(void)> beep_callback)
    : memory_{ram_size, std::byte{}}
    , beep_callback_{beep_callback}
{
}

void vkchip8::chip8::tick()
{
    uint16_t const operation{fetch()};
    execute(operation);
}

void vkchip8::chip8::tick_timers()
{
    if (delay_timer_ > 0)
    {
        --delay_timer_;
    }

    if (sound_timer_ > 0)
    {
        if (sound_timer_ == 1)
        {
            beep_callback_();
        }
        --sound_timer_;
    }
}

void vkchip8::chip8::key_event(key_event_type type, key_code code)
{
    keys_.set(static_cast<size_t>(code), static_cast<bool>(type));
}

void vkchip8::chip8::load(std::span<std::byte> program, uint16_t address)
{
    reset();
    std::ranges::copy(program, std::next(std::begin(memory_), start_address));
    program_counter_ = address;
}

void vkchip8::chip8::reset()
{
    std::ranges::copy(fontset, memory_.begin());
    std::ranges::fill(memory_ | std::views::drop(fontset.size()), std::byte{});

    program_counter_ = start_address;
    std::ranges::fill(data_registers_, uint8_t{});
    i_register_ = 0;
    sound_timer_ = 0;
    delay_timer_ = 0;
    std::ranges::fill(screen_, std::bitset<screen_width>{});
    std::ranges::fill(stack_, uint16_t{});
    stack_pointer_ = 0;
    keys_ = {};
}

uint16_t vkchip8::chip8::fetch()
{
    assert(program_counter_ + 1 < memory_.size());

    auto const rv{static_cast<uint16_t>(memory_[program_counter_]) << 8 |
        (static_cast<uint16_t>(memory_[program_counter_ + 1]))};

    program_counter_ += 2;

    return rv;
}

void vkchip8::chip8::execute(uint16_t operation)
{
    auto const& [digit1, digit2, digit3, digit4] = individual_digits(operation);

    if (operation == 0)
    {
        return;
    }
    else if (operation == 0x00'EE)
    {
        // Return from a subroutine
        program_counter_ = pop_stack();
    }
    else if (operation == 0x00'E0)
    {
        // Clear the screen
        std::ranges::fill(screen_, 0);
    }
    else if (digit1 == 0x1)
    {
        // Jump to address NNN
        program_counter_ = from_hex_digits(0, digit2, digit3, digit4);
    }
    else if (digit1 == 0x2)
    {
        // Execute subroutine at address NNN
        push_stack(program_counter_);
        program_counter_ = from_hex_digits(0, digit2, digit3, digit4);
    }
    else if (digit1 == 0x3)
    {
        // Skip the following instruction if the value of register VX equals NN
        if (data_registers_[digit2] == from_hex_digits(0, 0, digit3, digit4))
        {
            program_counter_ += 2;
        }
    }
    else if (digit1 == 0x4)
    {
        // Skip the following instruction if the value of register VX is not
        // equal to NN
        if (data_registers_[digit2] != from_hex_digits(0, 0, digit3, digit4))
        {
            program_counter_ += 2;
        }
    }
    else if (digit1 == 0x5 && digit4 == 0x0)
    {
        // Skip the following instruction if the value of register VX is equal
        // to the value of register VY
        if (data_registers_[digit2] == data_registers_[digit3])
        {
            program_counter_ += 2;
        }
    }
    else if (digit1 == 0x6)
    {
        // Store number NN in register VX
        data_registers_[digit2] = digit3 << 4 | digit4;
    }
    else if (digit1 == 0x7)
    {
        // Add the value NN to register VX
        auto current{data_registers_[digit2]};
        current += digit3 << 4 | digit4;
        data_registers_[digit2] = current;
    }
    else if (digit1 == 0x8 && digit4 == 0x0)
    {
        // Store the value of register vy in register vx
        data_registers_[digit2] = data_registers_[digit3];
    }
    else if (digit1 == 0x8 && digit4 == 0x1)
    {
        // Set VX to VX OR VY
        data_registers_[digit2] |= data_registers_[digit3];
    }
    else if (digit1 == 0x8 && digit4 == 0x2)
    {
        // Set VX to VX AND VY
        data_registers_[digit2] &= data_registers_[digit3];
    }
    else if (digit1 == 0x8 && digit4 == 0x3)
    {
        // Set VX to VX XOR VY
        data_registers_[digit2] ^= data_registers_[digit3];
    }
    else if (digit1 == 0x8 && digit4 == 0x4)
    {
        // Add VY to VX with carry to VF
        auto const vx{data_registers_[digit2]};
        auto const vy{data_registers_[digit3]};
        auto const res{vx + vy};
        data_registers_[digit2] = res;
        data_registers_[0xF] = res < vy;
    }
    else if (digit1 == 0x8 && digit4 == 0x5)
    {
        // Subtract VY from VX to VX with borrow to VF
        auto const vx{data_registers_[digit2]};
        auto const vy{data_registers_[digit3]};
        auto const res{vx - vy};
        data_registers_[digit2] = res;
        data_registers_[0xF] = vx >= vy;
    }
    else if (digit1 == 0x8 && digit4 == 0x6)
    {
        // Right shift VY by 1 to VX with carry to VF
        auto const vy{data_registers_[digit3]};
        data_registers_[digit2] = vy >> 1;
        data_registers_[0xF] = vy & 0x1;
    }
    else if (digit1 == 0x8 && digit4 == 0x7)
    {
        // Subtract VX from VY to VX with borrow to VF
        auto const vx{data_registers_[digit2]};
        auto const vy{data_registers_[digit3]};
        auto const res{vy - vx};
        data_registers_[digit2] = res;
        data_registers_[0xF] = vy >= vx;
    }
    else if (digit1 == 0x8 && digit4 == 0xE)
    {
        // Left shift VY by 1 to VX with carry to VF
        auto const vy{data_registers_[digit3]};
        data_registers_[digit2] = vy << 1;
        data_registers_[0xF] = vy & 0x80;
    }
    else if (digit1 == 0x9 && digit4 == 0x0)
    {
        // Skip the following instruction if VX != VY
        if (data_registers_[digit2] != data_registers_[digit3])
        {
            program_counter_ += 2;
        }
    }
    else if (digit1 == 0xA)
    {
        // Store memory address NNN in register I
        i_register_ = from_hex_digits(0, digit2, digit3, digit4);
    }
    else if (digit1 == 0xB)
    {
        // Jump to address NNN + V0
        program_counter_ =
            from_hex_digits(0, digit2, digit3, digit4) + data_registers_[0];
    }
    else if (digit1 == 0xC)
    {
        // Set VX to a random number with a mask of NN
        data_registers_[digit2] = std::uniform_int_distribution{0x00, 0xFF}(
            random_engine) &from_hex_digits(0, 0, digit3, digit4);
    }
    else if (digit1 == 0xD)
    {
        // Draw a sprite at position VX, VY with N bytes of sprite data stored
        // at I Set VF to 1 if any pixels are changed to unset
        auto const x_coord{data_registers_[digit2]};
        auto const y_coord{data_registers_[digit3]};
        size_t const rows{digit4};

        bool flipped{false};
        for (size_t sprite_row{}; sprite_row != rows; ++sprite_row)
        {
            auto const address{i_register_ + sprite_row};
            auto const& row_pixels{memory_[address]};
            for (size_t sprite_column{}; sprite_column != 8; ++sprite_column)
            {
                auto const sprite_bit{0x80 >> sprite_column};
                if ((static_cast<uint16_t>(row_pixels) & sprite_bit) != 0)
                {
                    auto const screen_x{
                        (x_coord + sprite_column) % screen_width};
                    auto const screen_y{(y_coord + sprite_row) % screen_height};
                    flipped |= screen_[screen_y].test(screen_x);
                    screen_[screen_y].flip(screen_x);
                }
            }
        }

        data_registers_[0xF] = flipped;
    }
    else if (digit1 == 0xE && digit3 == 0x9 && digit4 == 0xE)
    {
        auto const vx{data_registers_[digit2]};
        if (vx >= 0 && vx < keys_.size() && keys_.test(vx))
        {
            program_counter_ += 2;
        }
    }
    else if (digit1 == 0xE && digit3 == 0xA && digit4 == 0x1)
    {
        auto const vx{data_registers_[digit2]};
        if (vx >= 0 && vx < keys_.size() && !keys_.test(vx))
        {
            program_counter_ += 2;
        }
    }
    else if (digit1 == 0xF && digit3 == 0x0 && digit4 == 0x7)
    {
        data_registers_[digit2] = delay_timer_;
    }
    else if (digit1 == 0xF && digit3 == 0x0 && digit4 == 0xA)
    {
        bool pressed{false};
        for (size_t i{}; i != keys_.size(); ++i)
        {
            if (keys_.test(i))
            {
                data_registers_[digit2] = i;
                pressed = true;
                break;
            }
        }

        if (!pressed)
        {
            program_counter_ -= 2;
        }
    }
    else if (digit1 == 0xF && digit3 == 0x1 && digit4 == 0x5)
    {
        delay_timer_ = data_registers_[digit2];
    }
    else if (digit1 == 0xF && digit3 == 0x1 && digit4 == 0x8)
    {
        sound_timer_ = data_registers_[digit2];
    }
    else if (digit1 == 0xF && digit3 == 0x1 && digit4 == 0xE)
    {
        i_register_ += data_registers_[digit2];
    }
    else if (digit1 == 0xF && digit3 == 0x2 && digit4 == 0x9)
    {
        i_register_ = data_registers_[digit2] * 5;
    }
    else if (digit1 == 0xF && digit3 == 0x3 && digit4 == 0x3)
    {
        auto const vx{data_registers_[digit2]};
        memory_[i_register_] = std::byte(vx / 100);
        memory_[i_register_ + 1] = std::byte((vx / 10) % 10);
        memory_[i_register_ + 2] = std::byte(vx % 10);
    }
    else if (digit1 == 0xF && digit3 == 0x5 && digit4 == 0x5)
    {
        auto x{digit2};
        for (uint8_t i{}; i != x + 1; ++i)
        {
            memory_[i_register_++] = std::byte{data_registers_[i]};
        }
    }
    else if (digit1 == 0xF && digit3 == 0x6 && digit4 == 0x5)
    {
        auto x{digit2};
        for (uint8_t i{}; i != x + 1; ++i)
        {
            data_registers_[i] = static_cast<uint8_t>(memory_[i_register_++]);
        }
    }
    else
    {
        assert(false);
    }
}

void vkchip8::chip8::push_stack(uint16_t const value)
{
    assert(stack_pointer_ + 1 < stack_.size());
    stack_[stack_pointer_++] = value;
}

uint16_t vkchip8::chip8::pop_stack()
{
    assert(stack_pointer_ >= 1);
    return stack_[--stack_pointer_];
}
