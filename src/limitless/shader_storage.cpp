#include <limitless/shader_storage.hpp>
#include <limitless/core/shader_compiler.hpp>

using namespace Limitless;

bool Limitless::operator<(const ShaderKey& lhs, const ShaderKey& rhs) noexcept {
    return std::tie(lhs.material_type, lhs.model_type, lhs.material_index) < std::tie(rhs.material_type, rhs.model_type, rhs.material_index);
}

ShaderProgram& ShaderStorage::get(const std::string& name) const {
    try {
        return *shaders.at(name);
    } catch (const std::out_of_range& e) {
        throw shader_storage_error("No such shader " + name);
    }
}

ShaderProgram& ShaderStorage::get(ShaderPass material_type, ModelShader model_type, uint64_t material_index) const {
    try {
        return *material_shaders.at({material_type, model_type, material_index});
    } catch (const std::out_of_range& e) {
        throw shader_storage_error("No such material shader");
    }
}

void ShaderStorage::add(std::string name, std::shared_ptr<ShaderProgram> program) {
    std::unique_lock lock(mutex);
    const auto result = shaders.emplace(std::move(name), std::move(program));
    if (!result.second) {
        throw shader_storage_error{"Shader already contains" + name};
    }
}

void ShaderStorage::add(ShaderPass material_type, ModelShader model_type, uint64_t material_index, std::shared_ptr<ShaderProgram> program) {
    std::unique_lock lock(mutex);
    const auto result = material_shaders.emplace(ShaderKey{material_type, model_type, material_index}, std::move(program));
    if (!result.second) {
        throw shader_storage_error{"Shader already exists"};
    }
}

bool ShaderStorage::contains(const std::string& shader_name) noexcept {
	std::unique_lock lock(mutex);
	return shaders.find(shader_name) != shaders.end();
}

bool ShaderStorage::contains(ShaderPass material_type, ModelShader model_type, uint64_t material_index) noexcept {
    std::unique_lock lock(mutex);
    return material_shaders.find({material_type, model_type, material_index}) != material_shaders.end();
}

bool ShaderStorage::contains(const fx::UniqueEmitterShaderKey& emitter_type) noexcept {
    std::unique_lock lock(mutex);
    return emitters.find(emitter_type) != emitters.end();
}

ShaderProgram& ShaderStorage::get(const fx::UniqueEmitterShaderKey& emitter_type) const {
    try {
        return *emitters.at(emitter_type);
    } catch (const std::out_of_range& e) {
        throw shader_storage_error("No such sprite emitter shader");
    }
}

void ShaderStorage::add(const fx::UniqueEmitterShaderKey& emitter_type, std::shared_ptr<ShaderProgram> program) {
    std::unique_lock lock(mutex);
    const auto result = emitters.emplace(emitter_type, std::move(program));
    if (!result.second) {
        throw shader_storage_error{"Shader already contains emitter"};
    }
}
//
//void ShaderStorage::add(MaterialShader material_type, const fx::UniqueMeshEmitter& emitter_type, std::shared_ptr<ShaderProgram> program) {
//    std::unique_lock lock(mutex);
//    const auto result = mesh_emitter_shaders.emplace(MeshEmitterKey{material_type, emitter_type}, std::move(program));
//    if (!result.second) {
//        throw shader_storage_error{"Shader already exists"};
//    }
//}
//
//void ShaderStorage::add(MaterialShader material_type, const fx::UniqueBeamEmitter& emitter_type, std::shared_ptr<ShaderProgram> program) {
//    std::unique_lock lock(mutex);
//    const auto result = beam_emitter_shaders.emplace(BeamEmitterKey{material_type, emitter_type}, std::move(program));
//    if (!result.second) {
//        throw shader_storage_error{"Shader already exists"};
//    }
//}

void ShaderStorage::initialize(Context& ctx, const fs::path& shader_dir) {
    ShaderCompiler compiler {ctx};

    add("blur", compiler.compile(shader_dir / "postprocessing/blur"));
    add("brightness", compiler.compile(shader_dir / "postprocessing/brightness"));
    add("postprocess", compiler.compile(shader_dir / "postprocessing/postprocess"));
    add("text", compiler.compile(shader_dir / "pipeline/text"));
    add("text_selection", compiler.compile(shader_dir / "pipeline/text_selection"));
}

void ShaderStorage::clearMaterialShaders() {
    material_shaders.clear();
}

void ShaderStorage::clearEffectShaders() {
    emitters.clear();
}

void ShaderStorage::add(const ShaderStorage& other) {
    for (auto&& [key, value] : other.shaders) {
        shaders.emplace(key, value);
    }

    for (auto&& [key, value] : other.material_shaders) {
        material_shaders.emplace(key, value);
    }

    for (auto&& [key, value] : other.emitters) {
        emitters.emplace(key, value);
    }
}
