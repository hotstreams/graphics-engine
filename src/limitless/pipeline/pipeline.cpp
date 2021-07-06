#include <limitless/pipeline/pipeline.hpp>

#include <limitless/scene.hpp>
#include <limitless/core/uniform_setter.hpp>
#include <limitless/pipeline/render_pass.hpp>
#include <limitless/core/framebuffer.hpp>
#include <limitless/pipeline/sceneupdate_pass.hpp>

using namespace Limitless;

void Pipeline::draw(Context& context, const Assets& assets, Scene& scene, Camera& camera) {
    Instances instances;

    for (const auto& pass : passes) {
        pass->update(scene, instances, context, camera);
        if (dynamic_cast<Limitless::SceneUpdatePass*>(pass.get())) {
            instances = scene.getWrappers();
        }
    }

    UniformSetter setter;
    for (const auto& pass : passes) {
        pass->draw(instances, context, assets, camera, setter);
        pass->addSetter(setter);
    }
}

void Pipeline::update([[maybe_unused]] ContextEventObserver& ctx, [[maybe_unused]] Scene& scene, [[maybe_unused]] const RenderSettings& settings) {

}

void Pipeline::clear() {
    passes.clear();
}
