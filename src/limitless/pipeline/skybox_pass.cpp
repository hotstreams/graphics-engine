#include <limitless/pipeline/skybox_pass.hpp>

#include <limitless/skybox/skybox.hpp>
#include <limitless/scene.hpp>

using namespace Limitless;

void SkyboxPass::draw([[maybe_unused]] Instances& instances, Context& ctx, const Assets& assets, [[maybe_unused]] const Camera& camera, [[maybe_unused]] const UniformSetter& setter) {
    if (skybox) {
        skybox->draw(ctx, assets);
    }
}

void SkyboxPass::update(Scene& scene, [[maybe_unused]] Instances& instances, [[maybe_unused]] Context& ctx, [[maybe_unused]] const Camera& camera) {
	skybox = scene.getSkybox().get();
}

SkyboxPass::SkyboxPass(RenderPass* prev)
    : RenderPass(prev) {
}