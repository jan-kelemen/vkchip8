# vkchip8

CHIP-8 emulator with Vulkan rendering engine.

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

## References
* [matmikolay/chip-8](https://github.com/mattmikolay/chip-8)
* [aquova/chip8-book](https://github.com/aquova/chip8-book)
* [Timendus/chip8-test-suite](https://github.com/Timendus/chip8-test-suite)
