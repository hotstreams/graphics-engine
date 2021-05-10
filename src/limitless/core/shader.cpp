#include <limitless/core/shader.hpp>
#include <limitless/core/context_initializer.hpp>
#include <string>
#include <sstream>

using namespace Limitless;

namespace {
    inline constexpr auto version_key = "Limitless::GLSL_VERSION";
    inline constexpr auto extensions_key = "Limitless::Extensions";

    inline constexpr auto bindless_texture = "GL_ARB_bindless_texture";
    inline constexpr auto bindless_texture_define = "#define BINDLESS_TEXTURE\n";
    inline constexpr auto extension_bindless_texture = "#extension GL_ARB_bindless_texture : require\n";
    inline constexpr auto bindless_samplers = "layout(bindless_sampler) uniform;\n";

    inline constexpr auto shader_storage_buffer_object = "GL_ARB_shader_storage_buffer_object";
    inline constexpr auto extension_shader_storage_buffer_object = "#extension GL_ARB_shader_storage_buffer_object : require\n";

    inline constexpr auto shading_language_420pack = "GL_ARB_shading_language_420pack";
    inline constexpr auto extension_shading_language_420pack = "#extension GL_ARB_shading_language_420pack : require\n";

    inline constexpr auto explicit_uniform_location = "GL_ARB_explicit_uniform_location";
    inline constexpr auto extension_explicit_uniform_location = "#extension GL_ARB_explicit_uniform_location : require\n";
}

Shader::Shader(fs::path _path, Type _type, const ShaderAction& action)
    : path{std::move(_path)}
    , type{_type} {
    source = getSource(path);

    replaceVersion();
    replaceExtensions();
	replaceIncludes(path.parent_path());

    if (action) {
        action(*this);
    }

    id = glCreateShader(static_cast<GLenum>(type));
}

Shader::~Shader() {
    if (id != 0) {
        glDeleteShader(id);
    }
}

void Shader::checkStatus() const {
    GLint success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success) {
        GLint log_size = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_size);

        std::string log;
        log.resize(log_size);
        glGetShaderInfoLog(id, log_size, &log_size, log.data());

        {
            std::ofstream file("shader_compilation_error");
            file << path << std::endl;
            file << log << std::endl;
            file << "-------------" << std::endl;
            file << "Shader source:" << source << std::endl;
        }

        throw shader_compilation_error(log);
    }
}

void Shader::replaceVersion() noexcept {
    static const auto version = "#version " + std::to_string(ContextInitializer::major_version) + std::to_string(ContextInitializer::minor_version) + "0 core";

    replaceKey(version_key, version);
}

void Shader::replaceKey(const std::string& key, const std::string& value) noexcept {
    size_t found = 0;
    for (;;) {
        found = source.find(key, found);

        if (found == std::string::npos) {
            break;
        }

        source.replace(found, key.length(), value);
    }
}

void Shader::replaceExtensions() noexcept {
    std::string extensions;

    if (ContextInitializer::isExtensionSupported(shader_storage_buffer_object)) {
        extensions.append(extension_shader_storage_buffer_object);
    }

    if (ContextInitializer::isExtensionSupported(shading_language_420pack)) {
        extensions.append(extension_shading_language_420pack);
    }

    if (ContextInitializer::isExtensionSupported(explicit_uniform_location)) {
        extensions.append(extension_explicit_uniform_location);
    }

    if (ContextInitializer::isExtensionSupported(bindless_texture)) {
        extensions.append(extension_bindless_texture);
        extensions.append(bindless_texture_define);
        extensions.append(bindless_samplers);
    }

    replaceKey(extensions_key, extensions);
}

std::string Shader::getSource(const fs::path& filepath) {
    try {
        std::ifstream file(filepath);
        std::string file_source;

        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        if (file.good()) {
            std::stringstream stream;
            stream << file.rdbuf();

            file_source = stream.str();
        }

        return file_source;
    } catch (...) {
        throw shader_file_not_found(filepath.string());
    }
}

void Shader::replaceIncludes(const std::filesystem::path& base_dir) {
    static constexpr std::string_view include = "#include";

    size_t found = 0;
    for (;;) {
        found = source.find(include, found);

        if (found == std::string::npos) {
            break;
        }

        size_t beg = found + include.length() + 2;
        size_t end = source.find('"', beg);
        size_t name_length = end - beg;

        std::string file_name = source.substr(beg, name_length);

        try {
            const auto& include_src = getSource(base_dir/file_name);
            source.replace(found, include.length() + 3 + name_length, include_src);
        } catch (const shader_file_not_found& not_found) {
            throw shader_include_not_found(not_found.what());
        }
    }
}

void Shader::compile() const {
    const auto* src = source.data();

    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    checkStatus();
}

void Limitless::swap(Shader &lhs, Shader &rhs) noexcept {
    using std::swap;

    swap(lhs.source, rhs.source);
    swap(lhs.path, rhs.path);
    swap(lhs.type, rhs.type);
    swap(lhs.id, rhs.id);
}

Shader::Shader(Shader&& rhs) noexcept : Shader() {
    swap(*this, rhs);
}

Shader &Shader::operator=(Shader&& rhs) noexcept {
    swap(*this, rhs);
    return *this;
}
