#include <pc_speaker.hpp>

#include <SDL_audio.h>

#include <cmath>
#include <stdexcept>
#include <thread>

namespace
{
    void callback([[maybe_unused]] void* userdata, Uint8* stream, int len)
    {
        static float time = 0;
        static float freq = 440;
        short* snd = reinterpret_cast<short*>(stream);
        len /= sizeof(*snd);
        for (int i = 0; i < len;
             i++) // Fill array with frequencies, mathy-math stuff
        {
            snd[i] = 32000 * sin(time);

            time += freq * M_PI * 2 / 48000.0;
            if (time >= M_PI * 2)
                time -= M_PI * 2;
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
    spec.userdata = this;

    SDL_AudioSpec aspec;
    if ((device_id_ = SDL_OpenAudioDevice(nullptr, 0, &spec, &aspec, 0)) <= 0)
    {
        throw std::runtime_error{SDL_GetError()};
    }
}

vkchip8::pc_speaker::~pc_speaker() { }

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
