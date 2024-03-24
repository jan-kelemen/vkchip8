from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout

require_conan_version = ">=2.0"

class VKCHIP8Conan(ConanFile):
    name = "vkchip8"
    settings = "os", "compiler", "build_type", "arch"
    version = "0.1"

    exports_sources = "cmake", "src", "CMakeLists.txt", "LICENSE"

    def requirements(self):
        self.requires("glm/cci.20230113")
        self.requires("fmt/10.2.1")
        self.requires("imgui/1.90.4")
        self.requires("sdl/2.28.5")
        self.requires("spdlog/1.13.0")
        self.requires("vulkan-headers/1.3.268.0")
        self.requires("vulkan-loader/1.3.268.0")

    def build_requirements(self):
        self.tool_requires("cmake/[^3.27]")
        self.test_requires("catch2/[^3.5.2]")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = "ConanPresets.json"
        tc.generate()

        cmake = CMakeDeps(self)
        cmake.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

