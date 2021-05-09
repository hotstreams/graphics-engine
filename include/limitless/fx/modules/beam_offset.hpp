#pragma once

#include <limitless/fx/modules/module.hpp>

namespace Limitless::fx {
    template<typename Particle>
    class BeamOffset : public Module<Particle> {
    private:
        std::unique_ptr<Distribution<float>> distribution;
    public:
        explicit BeamOffset(std::unique_ptr<Distribution<float>> _distribution) noexcept
            : Module<Particle>(ModuleType::Beam_InitialOffset)
            , distribution {std::move(_distribution)} {}

        ~BeamOffset() override = default;

        BeamOffset(const BeamOffset& module)
            : Module<Particle>(module.type)
            , distribution {module.distribution->clone()} {}

        void initialize([[maybe_unused]] AbstractEmitter& emitter, Particle& particle) noexcept override {
            particle.min_offset = distribution->get();
        }

        [[nodiscard]] BeamOffset* clone() const override {
            return new BeamOffset(*this);
        }

        [[nodiscard]] auto& getDistribution() noexcept { return distribution; }
    };


}