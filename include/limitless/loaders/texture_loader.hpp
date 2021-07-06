#pragma once

#include <limitless/core/texture.hpp>
#include <limitless/core/context_debug.hpp>
#include <limitless/util/filesystem.hpp>
#include <set>

namespace Limitless {
    class Assets;
    class TextureBuilder;

    class TextureLoaderFlags {
    public:
        enum class Origin { TopLeft, BottomLeft };
        enum class Filter { Linear, Nearest };
        enum class Compression { None, Default, DXT1, DXT5, BC7, RGTC };
        enum class DownScale { None, x2, x4, x8 };
        enum class Space { sRGB, Linear };

        Origin origin { Origin::BottomLeft };
        Filter filter { Filter::Linear };
        Compression compression { Compression::Default };
        DownScale downscale { DownScale::None };
        Texture::Wrap wrapping { Texture::Wrap::Repeat };

        // only for 3 or 4 channels now
        Space space { Space::Linear };

        bool mipmap {true};
        bool anisotropic_filter {true};
        float anisotropic_value {0.0f}; // 0.0f for max supported

        bool border {false};
        glm::vec4 border_color {0.0f};

        TextureLoaderFlags() = default;
        TextureLoaderFlags(Origin _origin) noexcept : origin { _origin } {}
        TextureLoaderFlags(Origin _origin, Space _space) noexcept : origin { _origin }, space {_space} {}
        TextureLoaderFlags(Filter _filter) noexcept : filter { _filter } {}
        TextureLoaderFlags(Filter _filter, Texture::Wrap _wrap) noexcept : filter {_filter}, wrapping {_wrap} {}
        TextureLoaderFlags(Space _space) noexcept : space { _space } {}
    };

    class texture_loader_exception : std::runtime_error {
    public:
        explicit texture_loader_exception(const char* msg) : std::runtime_error(msg) {}
    };

    class TextureLoader final {
    private:
        static void setFormat(TextureBuilder& builder, const TextureLoaderFlags& flags, int channels);
        static void setTextureParameters(TextureBuilder& builder, const TextureLoaderFlags& flags);
        static void setAnisotropicFilter(const std::shared_ptr<Texture>& texture, const TextureLoaderFlags& flags);
        static void setDownScale(int& width, int& height, int channels, unsigned char*& data, const TextureLoaderFlags& flags);
        TextureLoader() = default;
        ~TextureLoader() = default;
    public:
        static std::shared_ptr<Texture> load(Assets& assets, const fs::path& path, const TextureLoaderFlags& flags = {});
        static std::shared_ptr<Texture> loadCubemap(Assets& assets, const fs::path& path, const TextureLoaderFlags& flags = {});

        static GLFWimage loadGLFWImage(Assets& assets, const fs::path& path, const TextureLoaderFlags& flags = {});
    };
}