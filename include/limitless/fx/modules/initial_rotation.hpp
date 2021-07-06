#pragma once

#include <limitless/fx/modules/module.hpp>
#include <limitless/fx/emitters/emitter.hpp>

namespace Limitless::fx {
    template<typename Particle>
    class InitialRotation : public Module<Particle> {
    private:
        std::unique_ptr<Distribution<glm::vec3>> distribution;
    public:
        explicit InitialRotation(std::unique_ptr<Distribution<glm::vec3>> _distribution) noexcept
            : Module<Particle>(ModuleType::InitialRotation)
            , distribution{std::move(_distribution)} {}

        ~InitialRotation() override = default;

        InitialRotation(const InitialRotation& module)
            : Module<Particle>(module.type)
            , distribution {module.distribution->clone()} {}

        void initialize(AbstractEmitter& emitter, Particle& particle, [[maybe_unused]] size_t index) noexcept override {
            const auto rot = emitter.getRotation() * emitter.getLocalRotation();
            particle.getRotation() += distribution->get() * rot;
        }

        [[nodiscard]] InitialRotation* clone() const override {
            return new InitialRotation(*this);
        }

        [[nodiscard]] auto& getDistribution() noexcept { return distribution; }
        [[nodiscard]] const auto& getDistribution() const noexcept { return distribution; }
    };
}
