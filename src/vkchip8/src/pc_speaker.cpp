#include <pc_speaker.hpp>

#include <SDL_audio.h>

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <thread>

namespace
{
    void callback([[maybe_unused]] void* userdata, Uint8* stream, int len)
    {
        constexpr auto pi2{std::numbers::pi_v<float> * 2};
        constexpr auto freq{440.f};

        auto time{0.f};
        auto* const snd{reinterpret_cast<short*>(stream)};
        len /= sizeof(*snd);
        for (int i = 0; i != len; i++)
        {
            snd[i] = static_cast<short>(32000 * sin(time));

            time += freq * pi2 / 48000.0f;
            if (time >= pi2)
            {
                time -= pi2;
            }
        }
    }
} // namespace

vkchip8::pc_speaker::pc_speaker()
{
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = 48000;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = 4096;
    spec.callback = callback;
    spec.userdata = nullptr;

    SDL_AudioSpec aspec;
    if ((device_id_ = SDL_OpenAudioDevice(nullptr, 0, &spec, &aspec, 0)) <= 0)
    {
        throw std::runtime_error{SDL_GetError()};
    }
}

vkchip8::pc_speaker::~pc_speaker() { SDL_CloseAudioDevice(device_id_); }

void vkchip8::pc_speaker::tick()
{
    if (beep_on_ && std::chrono::steady_clock::now() >= beep_until_)
    {
        SDL_PauseAudioDevice(device_id_, 1);
        beep_on_ = false;
    }
}

void vkchip8::pc_speaker::beep()
{
    SDL_PauseAudioDevice(device_id_, 0);
    beep_on_ = true;
    beep_until_ =
        std::chrono::steady_clock::now() + std::chrono::milliseconds{100};
}
