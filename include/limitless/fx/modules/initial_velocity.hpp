#pragma once

#include <limitless/fx/modules/module.hpp>

namespace Limitless::fx {
    template<typename Particle>
    class InitialVelocity : public Module<Particle> {
    private:
        std::unique_ptr<Distribution<glm::vec3>> distribution;
    public:
        explicit InitialVelocity(std::unique_ptr<Distribution<glm::vec3>> _distribution) noexcept
            : Module<Particle>(ModuleType::InitialVelocity)
            , distribution {std::move(_distribution)} {}

        ~InitialVelocity() override = default;

        InitialVelocity(const InitialVelocity& module)
            : Module<Particle>(module.type)
            , distribution {module.distribution->clone()} {}

        void initialize(AbstractEmitter& emitter, Particle& particle, [[maybe_unused]] size_t index) noexcept override {
            const auto rot = emitter.getRotation() * emitter.getLocalRotation();
            particle.getVelocity() = distribution->get() * rot;
        }

        [[nodiscard]] InitialVelocity* clone() const override {
            return new InitialVelocity(*this);
        }

        [[nodiscard]] auto& getDistribution() noexcept { return distribution; }
        [[nodiscard]] const auto& getDistribution() const noexcept { return distribution; }
    };
}
