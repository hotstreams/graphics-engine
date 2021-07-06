#pragma once

#include <limitless/fx/modules/module.hpp>

namespace Limitless::fx {
    template<typename Particle>
    class InitialColor : public Module<Particle> {
    private:
        std::unique_ptr<Distribution<glm::vec4>> distribution;
    public:
        explicit InitialColor(std::unique_ptr<Distribution<glm::vec4>> _distribution) noexcept
            : Module<Particle>(ModuleType::InitialColor)
            , distribution {std::move(_distribution)} {}

        ~InitialColor() override = default;

        InitialColor(const InitialColor& module)
            : Module<Particle>(module.type)
            , distribution{module.distribution->clone()} {}

        void initialize([[maybe_unused]] AbstractEmitter& emitter, Particle& particle, [[maybe_unused]] size_t index) noexcept override {
            particle.color = distribution->get();
        }

        [[nodiscard]] InitialColor* clone() const override {
            return new InitialColor(*this);
        }

        [[nodiscard]] auto& getDistribution() noexcept { return distribution; }
        [[nodiscard]] const auto& getDistribution() const noexcept { return distribution; }
    };
}
