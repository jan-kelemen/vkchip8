#ifndef VKCHIP8_PC_SPEAKER_INCLUDED
#define VKCHIP8_PC_SPEAKER_INCLUDED

#include <chrono>

namespace vkchip8
{
    class [[nodiscard]] pc_speaker final
    {
    public: // Construction
        pc_speaker();

    public: // Destruction
        ~pc_speaker();

    public: // Interface
        void tick();

        void beep();

    private: // Data
        uint32_t device_id_{};
        bool beep_on_{};
        std::chrono::steady_clock::time_point beep_until_;
    };

} // namespace vkchip8
#endif // !VKCHIP8_PC_SPEAKER_INCLUDED
