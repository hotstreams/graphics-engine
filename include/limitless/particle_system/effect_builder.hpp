#pragma once

#include <limitless/instances/effect_instance.hpp>
#include <limitless/particle_system/mesh_emitter.hpp>
#include <limitless/shader_types.hpp>

namespace LimitlessEngine {
    class Material;
    class Context;

    class EffectBuilder {
    private:
        std::shared_ptr<EffectInstance> effect;
        std::string effect_name;
        std::string last_emitter;

        Context& context;
        Assets& assets;

        EffectBuilder& setModules(decltype(Emitter::modules)&& modules) noexcept;
        friend class EmitterSerializer;
    public:
        EffectBuilder(Context& context, Assets& assets) noexcept;
        ~EffectBuilder() = default;

        EffectBuilder& setBurstCount(std::unique_ptr<Distribution<uint32_t>> burst_count) noexcept;
        EffectBuilder& setMaterial(std::shared_ptr<Material> material);
        EffectBuilder& setMesh(std::shared_ptr<AbstractMesh> mesh);
        EffectBuilder& setDuration(std::chrono::duration<float> duration) noexcept;
        EffectBuilder& setLocalPosition(const glm::vec3& local_position) noexcept;
        EffectBuilder& setLocalRotation(const glm::vec3& local_rotation) noexcept;
        EffectBuilder& setSpawnMode(EmitterSpawn::Mode mode) noexcept;
        EffectBuilder& setLocalSpace(bool _local_space) noexcept;
        EffectBuilder& setEmitterType(EmitterType type) noexcept;
        EffectBuilder& setMaxCount(uint64_t max_count) noexcept;
        EffectBuilder& setSpawnRate(float spawn_rate) noexcept;
        EffectBuilder& setSpawn(EmitterSpawn&& spawn) noexcept;
        EffectBuilder& setLoops(int loops) noexcept;

        EffectBuilder& create(std::string name);

        template<typename T>
        EffectBuilder& createEmitter(std::string name) {
            last_emitter = name;
            effect->emitters.emplace(std::move(name), new T());
            return *this;
        }

        template<typename T, typename... Args>
        EffectBuilder& addModule(EmitterModuleType module_type, Args&&... args) {
            if constexpr (std::is_same_v<T, InitialVelocity>)

            effect->emitters[last_emitter]->modules.emplace(module_type, new T{std::forward<Args>(args)...});
            return *this;
        }

        std::shared_ptr<EffectInstance> build(const MaterialShaders& material_shaders = {MaterialShader::Forward, MaterialShader::DirectionalShadow});
    };
}
