#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <limitless/instances/abstract_instance.hpp>
#include <unordered_map>

namespace Limitless {
    namespace fx {
        class AbstractEmitter;
        class SpriteEmitter;
        class MeshEmitter;
        class EffectBuilder;
    }

    class EffectInstance : public AbstractInstance {
    private:
        std::unordered_map<std::string, std::unique_ptr<fx::AbstractEmitter>> emitters;
        std::string name;

        friend void swap(EffectInstance&, EffectInstance&) noexcept;
        bool isDone() const noexcept;

        void calculateBoundingBox() noexcept override;

        friend class fx::EffectBuilder;
        friend class EffectSerializer;
        EffectInstance() noexcept;
    public:
        EffectInstance(Lighting* lighting, const std::shared_ptr<EffectInstance>& effect, const glm::vec3& position) noexcept;
        EffectInstance(const std::shared_ptr<EffectInstance>& effect, const glm::vec3& position) noexcept;
        ~EffectInstance() override = default;

        EffectInstance(const EffectInstance&) noexcept;
        EffectInstance(EffectInstance&&) noexcept = default;

        EffectInstance* clone() noexcept override;

        EffectInstance& setPosition(const glm::vec3& position) noexcept override;
        EffectInstance& setRotation(const glm::quat& rotation) noexcept override;
        EffectInstance& rotateBy(const glm::quat& rotation) noexcept override;

        template<typename T>
        const T& get(const std::string& name) const {
            try {
                return static_cast<T&>(*emitters.at(name));
            } catch (...) {
                throw std::runtime_error("No such emitter " + name);
            }
        }

        template<typename T>
        T& get(const std::string& name) {
            try {
                return static_cast<T&>(*emitters.at(name));
            } catch (...) {
                throw std::runtime_error("No such emitter " + name);
            }
        }

        const auto& getName() const noexcept { return name; }
        const auto& getEmitters() const noexcept { return emitters; }

        auto& getEmitters() noexcept { return emitters; }

        void update(Context& context, Camera& camera) override;

        void draw(Context& ctx, const Assets& assets, ShaderPass shader_type, ms::Blending blending, const UniformSetter& uniform_set) override;
    };

    void swap(EffectInstance&, EffectInstance&) noexcept;
}