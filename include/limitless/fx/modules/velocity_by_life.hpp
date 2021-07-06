#pragma once

#include <limitless/fx/modules/module.hpp>

namespace Limitless::fx {
    template<typename Particle>
    class VelocityByLife : public Module<Particle> {
    private:
        std::unique_ptr<Distribution<glm::vec3>> distribution;
    public:
        explicit VelocityByLife(std::unique_ptr<Distribution<glm::vec3>> _distribution) noexcept
            : Module<Particle>(ModuleType::VelocityByLife)
            , distribution {std::move(_distribution)} {}

        ~VelocityByLife() override = default;

        VelocityByLife(const VelocityByLife& module)
            : Module<Particle>(module.type)
            , distribution {module.distribution->clone()} {}

        void update(AbstractEmitter& emitter, std::vector<Particle>& particles, float dt, [[maybe_unused]] Context& ctx, [[maybe_unused]] const Camera& camera) noexcept override {
            const auto rot = emitter.getRotation() * emitter.getLocalRotation();
            for (auto& particle : particles) {
                const auto tick = particle.getLifetime() / dt;
                const auto tick_vel = (rot * distribution->get() - particle.getVelocity()) / tick;
                particle.getVelocity() += tick_vel;
            }
        }

        [[nodiscard]] VelocityByLife* clone() const override {
            return new VelocityByLife(*this);
        }

        [[nodiscard]] auto& getDistribution() noexcept { return distribution; }
        [[nodiscard]] const auto& getDistribution() const noexcept { return distribution; }
    };
}