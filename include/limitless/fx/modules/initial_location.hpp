#pragma once

#include <limitless/fx/modules/module.hpp>
#include <limitless/fx/modules/distribution.hpp>

namespace Limitless::fx {
    template<typename Particle>
    class InitialLocation : public Module<Particle> {
    private:
        std::unique_ptr<Distribution<glm::vec3>> distribution;
    public:
        explicit InitialLocation(std::unique_ptr<Distribution<glm::vec3>> _distribution) noexcept
            : Module<Particle>(ModuleType::InitialLocation)
            , distribution{std::move(_distribution)} {}

        ~InitialLocation() override = default;

        InitialLocation(const InitialLocation& module)
            : Module<Particle>(module.type)
            , distribution {module.distribution->clone()} {}

        void initialize([[maybe_unused]] AbstractEmitter& emitter, Particle& particle, [[maybe_unused]] size_t index) noexcept override {
            particle.getPosition() += distribution->get();
        }

        [[nodiscard]] InitialLocation* clone() const override {
            return new InitialLocation(*this);
        }

        [[nodiscard]] auto& getDistribution() noexcept { return distribution; }
        [[nodiscard]] const auto& getDistribution() const noexcept { return distribution; }
    };
}
