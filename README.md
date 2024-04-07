# vkchip8 [![GitHub Build status](https://github.com/jan-kelemen/vkchip8/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/jan-kelemen/vkchip8/actions/workflows/ci.yml/badge.svg?branch=master)

CHIP-8 emulator with Vulkan rendering engine.

## Try it out
First argument to the emulator is ROM file which is loaded, simply run:
```
vkchip8.exe roms/pong.rom
```

Or you can try it with some other ROMs available online.

## Building
Necessary build tools are:
* CMake 3.27 or higher
* Conan 2.2 or higher
  * See [installation instructions](https://docs.conan.io/2/installation.html)
* One of supported compilers:
  * Clang-18
  * GCC-13
  * Visual Studio 2022 (MSVC v193)

```
conan install . --profile=conan/clang-18 --profile=conan/dependencies --build=missing --settings build_type=Release
cmake --preset release
cmake --build --preset=release
```

Note: When building with Clang, Conan Center package of pulseaudio fails to build. Use updated recipe from https://github.com/jan-kelemen/conan-recipes of pulseaudio and SDL.
```
git clone git@github.com:jan-kelemen/conan-recipes.git
conan create conan-recipes/recipes/sdl/all --version 2.30.1
conan create conan-recipes/recipes/pulseaudio/meson --version 17.0
```

And then execute the build commands.

## References
* [matmikolay/chip-8](https://github.com/mattmikolay/chip-8)
* [aquova/chip8-book](https://github.com/aquova/chip8-book)
* [Timendus/chip8-test-suite](https://github.com/Timendus/chip8-test-suite)
