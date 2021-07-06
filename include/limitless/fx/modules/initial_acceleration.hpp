#pragma once

#include <limitless/fx/modules/module.hpp>

namespace Limitless::fx {
    template<typename Particle>
    class InitialAcceleration : public Module<Particle> {
    private:
        std::unique_ptr<Distribution<glm::vec3>> distribution;
    public:
        explicit InitialAcceleration(std::unique_ptr<Distribution<glm::vec3>> _distribution) noexcept
            : Module<Particle>(ModuleType::InitialAcceleration)
            , distribution{std::move(_distribution)} {}

        ~InitialAcceleration() override = default;

        InitialAcceleration(const InitialAcceleration& module)
            : Module<Particle>(module.type)
            , distribution{module.distribution->clone()} {}

        void initialize(AbstractEmitter& emitter, Particle& particle, [[maybe_unused]] size_t index) noexcept override {
            const auto rot = emitter.getRotation() * emitter.getLocalRotation();
            particle.getAcceleration() = distribution->get() * rot;
        }

        [[nodiscard]] InitialAcceleration* clone() const override {
            return new InitialAcceleration(*this);
        }

        [[nodiscard]] auto& getDistribution() noexcept { return distribution; }
        [[nodiscard]] const auto& getDistribution() const noexcept { return distribution; }
    };
}
