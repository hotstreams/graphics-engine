#pragma once

#include <limitless/fx/modules/module.hpp>

namespace Limitless::fx {
    template<typename Particle>
    class Lifetime : public Module<Particle> {
    private:
        std::unique_ptr<Distribution<float>> distribution;
    public:
        explicit Lifetime(std::unique_ptr<Distribution<float>> _distribution) noexcept
            : Module<Particle>(ModuleType::Lifetime)
            , distribution {std::move(_distribution)} {}

        ~Lifetime() override = default;

        Lifetime(const Lifetime& module)
            : Module<Particle>(module.type)
            , distribution {module.distribution->clone()} {}

        void initialize([[maybe_unused]] AbstractEmitter& emitter, Particle& particle, [[maybe_unused]] size_t index) noexcept override {
            particle.getLifetime() = distribution->get();
        }

        void update([[maybe_unused]] AbstractEmitter& emitter, std::vector<Particle>& particles, float dt, [[maybe_unused]] Context& ctx, [[maybe_unused]] const Camera& camera) noexcept override {
            for (auto& particle : particles) {
                particle.getLifetime() -= dt;
            }
        }

        [[nodiscard]] Lifetime* clone() const override {
            return new Lifetime(*this);
        }

        [[nodiscard]] auto& getDistribution() noexcept { return distribution; }
        [[nodiscard]] const auto& getDistribution() const noexcept { return distribution; }
    };
}
