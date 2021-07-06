#include <limitless/core/context_observer.hpp>
#include <limitless/loaders/texture_loader.hpp>
#include <limitless/camera.hpp>
#include <limitless/scene.hpp>
#include <limitless/pipeline/renderer.hpp>
#include <limitless/instances/skeletal_instance.hpp>
#include <limitless/fx/effect_builder.hpp>
#include <limitless/util/math.hpp>
#include <limitless/serialization/effect_serializer.hpp>
#include <limitless/loaders/effect_loader.hpp>
#include <limitless/skybox/skybox.hpp>
#include <limitless/loaders/model_loader.hpp>
#include <limitless/text/font_atlas.hpp>
#include <limitless/text/text_instance.hpp>
#include <limitless/fx/emitters/beam_emitter.hpp>
#include <limitless/fx/emitters/sprite_emitter.hpp>
#include <limitless/fx/emitters/mesh_emitter.hpp>
#include <limitless/instances/effect_instance.hpp>
#include <limitless/ms/material_builder.hpp>
#include <limitless/loaders/asset_manager.hpp>
#include <limitless/assets.hpp>
#include <limitless/pipeline/forward.hpp>
#include <limitless/instances/instanced_instance.hpp>
#include <limitless/ms/material.hpp>
#include <limitless/loaders/material_loader.hpp>
#include <limitless/fx/modules/mesh_location.hpp>
#include <limitless/fx/modules/mesh_location_attachment.hpp>
#include <iostream>

using namespace Limitless;

class Game : public MouseMoveObserver, public KeyObserver, public FramebufferObserver {
private:
    ContextEventObserver context;
    Scene scene;
    Camera camera;
    Renderer render;

    Assets assets;

    bool done {};
    static constexpr glm::uvec2 window_size {800, 800};

    std::unique_ptr<TextInstance> text;
    EffectInstance* effect {};
    EffectInstance* fireball {};
    ModelInstance* bob;

    AssetManager manager {context, assets};
public:
    Game()
        : context {"Limitless-demo", window_size, {{ WindowHint::Resizable, true }}}
        , scene {context}
        , camera {window_size}
        , render {context}
        , assets {ENGINE_ASSETS_DIR}
{
        camera.setPosition({7.0f, 0.0f, 3.0f});

        if (!Limitless::ContextEventObserver::checkMinimumRequirements()) {
            throw std::runtime_error("Minimum requirements are not met!");
        }

        context.makeCurrent();
        context.setCursorMode(CursorMode::Normal);
        context.setSwapInterval(1);
        context.setStickyKeys(true);

        context.registerObserver(static_cast<KeyObserver*>(this));
        context.registerObserver(static_cast<MouseMoveObserver*>(this));
        context.registerObserver(static_cast<FramebufferObserver*>(this));

        assets.load(context);

//        addModels();
        addModelsT();
        addSpheres();
        addEffects();
//        addWarlocks();

        assets.compileShaders(context, render.getSettings());
//        manager.compileShaders(context, render.getSettings());
//        manager.wait();

        text = std::make_unique<TextInstance>("Limitless-engine", glm::vec2{20.0f, window_size.y - 40.0f}, assets.fonts.at("nunito"));
        text->setColor({0.1f, 0.8f, 0.4f, 1.0f});

        scene.lighting.directional_light = { glm::vec4{2.0f, -5.0f, 2.0f, 1.0f}, glm::vec4{0.1f, 0.7f, 1.3f, 1.0f} };
        scene.lighting.point_lights.emplace_back(glm::vec4{-1.0f, 1.3f, 2.0f, 1.0f}, glm::vec4{2.3f, 1.1f, 1.2f, 2.0f}, 4.6f);
        scene.lighting.point_lights.emplace_back(glm::vec4{3.0f, 1.3f, 2.0f, 1.0f}, glm::vec4{1.3f, 0.3f, 2.7f, 2.0f}, 4.2f);
        scene.lighting.point_lights.emplace_back(glm::vec4{8.0f, 1.3f, 2.0f, 1.0f}, glm::vec4{1.3f, 2.3f, 1.7f, 2.0f}, 4.5f);
    }

    ~Game() override {
        context.unregisterObserver(static_cast<KeyObserver*>(this));
        context.unregisterObserver(static_cast<MouseMoveObserver*>(this));
        context.unregisterObserver(static_cast<FramebufferObserver*>(this));
    }

    void addModelsT() {

        const fs::path assets_dir {ASSETS_DIR};

        manager.loadModel("bob", assets_dir / "models/boblamp/boblampclean.md5mesh");
        manager.loadModel("backpack", assets_dir / "models/backpack/backpack.obj", {ModelLoaderFlag::FlipUV});
        manager.loadModel("nanosuit", assets_dir / "models/nanosuit/nanosuit.obj");
        manager.loadModel("cyborg", assets_dir / "models/cyborg/cyborg.obj");

//        manager.loadMaterial("test1123", assets_dir / "materials/test");

        while (!manager) {
            context.clearColor({0.3f, 0.3f, 0.3f, 1.0f});

            // loading screen ;)

            context.swapBuffers();
        }
        manager.doDelayedJob();

        assets.skyboxes.add("skybox", std::make_shared<Skybox>(assets, assets_dir / "skyboxes/sky/sky.png", TextureLoaderFlags{TextureLoaderFlags::Origin::TopLeft, TextureLoaderFlags::Space::sRGB}));
        assets.fonts.add("nunito", std::make_shared<FontAtlas>(assets_dir / "fonts/nunito.ttf", 48));

        scene.setSkybox(assets.skyboxes.at("skybox"));
        scene.add<ModelInstance>(assets.models.at("backpack"), glm::vec3{2.5f, 0.5f, 5.0f})
                .setRotation(glm::vec3{0.0f, PI, 0.0f})
                .setScale(glm::vec3{0.4f});

        scene.add<ModelInstance>(assets.models.at("nanosuit"), glm::vec3{4.0f, 0.0f, 5.0f})
                .setRotation(glm::vec3{0.0f, PI, 0.0f})
                .setScale(glm::vec3{0.1f});

        scene.add<ModelInstance>(assets.models.at("cyborg"), glm::vec3{5.0f, 0.0f, 5.0f})
                .setRotation(glm::vec3{0.0f, PI, 0.0f})
                .setScale(glm::vec3{0.35f});

        bob = &scene.add<SkeletalInstance>(assets.models.at("bob"), glm::vec3{6.0f, 0.0f, 5.0f})
                .setScale(glm::vec3{0.02f})
                .setRotation(glm::vec3{0.0f, 0.0f, PI})
                .play("");

        ms::MaterialBuilder builder{assets};

        builder.setName("test")
                .add(ms::Property::Color, glm::vec4(2.0f, 1.0f, 1.0f, 1.0f))
                .setModelShaders({ModelShader::Instanced})
                .setShading(ms::Shading::Unlit)
                .build();
        auto& instanced = scene.add<InstancedInstance<ModelInstance>>(glm::vec3(0.0f));
        for (uint32_t i = 0; i < 10; ++i) {
            instanced.addInstance(std::make_unique<ModelInstance>(assets.models.at("sphere"), assets.materials.at("test"), glm::vec3{-4.0, 1.0f, -8.0 + i * 1.9}));
        }
    }

    void addModels() {
        const fs::path assets_dir{ASSETS_DIR};

        assets.models.add("bob", ModelLoader::loadModel(assets, assets_dir / "models/boblamp/boblampclean.md5mesh"));
        assets.models.add("backpack", ModelLoader::loadModel(assets, assets_dir / "models/backpack/backpack.obj", {ModelLoaderFlag::FlipUV}));
        assets.models.add("nanosuit", ModelLoader::loadModel(assets, assets_dir / "models/nanosuit/nanosuit.obj"));
        assets.models.add("cyborg", ModelLoader::loadModel(assets, assets_dir / "models/cyborg/cyborg.obj"));

        assets.skyboxes.add("skybox", std::make_shared<Skybox>(assets, assets_dir / "skyboxes/sky/sky.png", TextureLoaderFlags{TextureLoaderFlags::Origin::TopLeft}));
        scene.setSkybox(assets.skyboxes.at("skybox"));

        assets.fonts.add("nunito", std::make_shared<FontAtlas>(assets_dir / "fonts/nunito.ttf", 48));

        scene.add<ModelInstance>(assets.models.at("backpack"), glm::vec3{2.5f, 0.5f, 5.0f})
                .setRotation(glm::vec3{0.0f, PI, 0.0f})
                .setScale(glm::vec3{0.4f});

        scene.add<ModelInstance>(assets.models.at("nanosuit"), glm::vec3{4.0f, 0.0f, 5.0f})
                .setRotation(glm::vec3{0.0f, PI, 0.0f})
                .setScale(glm::vec3{0.1f});

        scene.add<ModelInstance>(assets.models.at("cyborg"), glm::vec3{5.0f, 0.0f, 5.0f})
                .setRotation(glm::vec3{0.0f, PI, 0.0f})
                .setScale(glm::vec3{0.35f});

        bob = &scene.add<SkeletalInstance>(assets.models.at("bob"), glm::vec3{6.0f, 0.0f, 5.0f})
                .setScale(glm::vec3{0.02f})
                .setRotation(glm::vec3{0.0f, 0.0f, PI})
                .play("");

        ms::MaterialBuilder builder{assets};

        builder.setName("test")
                .add(ms::Property::Color, glm::vec4(2.0f, 1.0f, 1.0f, 1.0f))
                .setModelShaders({ModelShader::Instanced})
                .setShading(ms::Shading::Unlit)
                .build();

        auto& instanced = scene.add<InstancedInstance<ModelInstance>>(glm::vec3(0.0f));
        for (uint32_t i = 0; i < 10; ++i) {
            instanced.addInstance(std::make_unique<ModelInstance>(assets.models.at("sphere"), assets.materials.at("test"), glm::vec3{-4.0, 1.0f, -8.0 + i * 1.9}));
        }
    }

    void addSpheres() {
        ms::MaterialBuilder builder {assets};
        const fs::path assets_dir {ASSETS_DIR};

        context.setWindowIcon(TextureLoader::loadGLFWImage(assets, assets_dir / "icons/demo.png"));

        builder.setName("Color")
                .add(ms::Property::Color, glm::vec4(0.7f, 0.3, 0.5f, 1.0f))
                .setShading(ms::Shading::Lit)
                .build();
        scene.add<ModelInstance>(assets.models.at("sphere"), assets.materials.at("Color"), glm::vec3{10.0f, 1.0f, 0.0f});

        builder.setName("Diffuse")
                .add(ms::Property::Diffuse, TextureLoader::load(assets, assets_dir / "textures/triangle.jpg", {TextureLoaderFlags::Space::sRGB}))
                .setShading(ms::Shading::Lit)
                .build();
        scene.add<ModelInstance>(assets.models.at("sphere"), assets.materials.at("Diffuse"), glm::vec3{10.0f, 1.0f, 2.0f });

        builder.setName("EmissiveColor")
                .add(ms::Property::EmissiveColor, glm::vec4(3.0f, 0.1f, 3.0f, 1.0f))
                .setShading(ms::Shading::Unlit)
                .build();
        scene.add<ModelInstance>(assets.models.at("sphere"), assets.materials.at("EmissiveColor"), glm::vec3{10.0f, 1.0f, 4.0f });

        builder.setName("BlendMask")
                .add(ms::Property::BlendMask, TextureLoader::load(assets, assets_dir / "textures/bricks.jpg", {TextureLoaderFlags::Filter::Nearest}))
                .add(ms::Property::Color, glm::vec4(0.3f, 0.1f, 0.7f, 1.0f))
                .setShading(ms::Shading::Lit)
                .setTwoSided(true)
                .build();
        scene.add<ModelInstance>(assets.models.at("sphere"), assets.materials.at("BlendMask"), glm::vec3{10.0f, 1.0f, 6.0f });

        builder.setName("PBR")
                .add(ms::Property::MetallicTexture, TextureLoader::load(assets, assets_dir / "textures/rustediron2_metallic.png"))
                .add(ms::Property::RoughnessTexture, TextureLoader::load(assets, assets_dir / "textures/rustediron2_roughness.png"))
                .add(ms::Property::Diffuse, TextureLoader::load(assets, assets_dir / "textures/rustediron2_basecolor.png", {TextureLoaderFlags::Space::sRGB}))
                .add(ms::Property::Normal, TextureLoader::load(assets, assets_dir / "textures/rustediron2_normal.png"))
                .setShading(ms::Shading::Lit)
                .build();
        scene.add<ModelInstance>(assets.models.at("sphere"), assets.materials.at("PBR"), glm::vec3{10.0f, 1.0f, 8.0f });

        builder.setName("floor")
                .setShading(ms::Shading::Lit)
                .add(ms::Property::Diffuse, TextureLoader::load(assets, assets_dir / "textures/wood.jpg", {TextureLoaderFlags::Space::sRGB}))
                .setShading(ms::Shading::Lit)
                .setTwoSided(true)
                .build();

        scene.add<ModelInstance>(assets.models.at("plane"), assets.materials.at("floor"), glm::vec3{3.0f, 0.f, 0.0f})
                .setScale(glm::vec3{20.0f, 1.0f, 20.0f});
    }

//    void addWarlocks() {
//        ms::MaterialBuilder materialBuilder{assets};
//        fx::EffectBuilder effectBuilder{assets};
//        TextureLoader loader {assets};
//        const fs::path assets_dir {ASSETS_DIR};
//
//        materialBuilder.setName("conflagrate_flash")
//                .setShading(Limitless::ms::Shading::Unlit)
//                .setBlending(Limitless::ms::Blending::Additive)
//                .setTwoSided(true)
//                .add(ms::Property::Color, glm::vec4(1.0f))
//                .setFragmentSnippet("float r = length(uv * 2.0 - 1.0);\n"
//                                    "float d = 1 - distance(uv, vec2(0.5)) * 4;\n"
//                                    "mat_color *= clamp(step(0.2, r) * step(r, 0.35) * d, 0, 1);")
//                .addModelShader(Limitless::ModelShader::Effect)
//                .build();
//
//        materialBuilder.setName("conflagrate_aura")
//                .setShading(Limitless::ms::Shading::Unlit)
//                .setBlending(Limitless::ms::Blending::Additive)
//                .setTwoSided(true)
//                .add(ms::Property::Diffuse, loader.load(assets_dir / "textures/aura.png"))
//                .addModelShader(Limitless::ModelShader::Effect)
//                .build();
//
//        materialBuilder.setName("conflagrate_burst")
//                .setShading(Limitless::ms::Shading::Unlit)
//                .setBlending(Limitless::ms::Blending::Additive)
//                .add(ms::Property::Diffuse, loader.load(assets_dir / "textures/50.jpg"))
//                .addUniform(std::make_unique<UniformSampler>("noise", loader.load(assets_dir / "textures/noise.png")))
//                .addModelShader(Limitless::ModelShader::Effect)
//                .build();
//
//        materialBuilder.setName("deflect")
//                .setShading(Limitless::ms::Shading::Unlit)
//                .setBlending(Limitless::ms::Blending::Additive)
//                .add(ms::Property::Color, glm::vec4(0.279f, 0.918f, 1.0f, 1.0f))
//                .add(ms::Property::Diffuse, loader.load(assets_dir / "textures/glow.tga"))
//                .addUniform(std::make_unique<UniformValue<float>>("stime", 48.032f))
//                .setFragmentSnippet("mat_color *= pow(sin(3.14*(getParticleTime()-stime)), 2);")
//                .addModelShader(Limitless::ModelShader::Effect)
//                .build();
//
//        materialBuilder.setName("conflagrate_point")
//                .setShading(Limitless::ms::Shading::Unlit)
//                .setBlending(Limitless::ms::Blending::Translucent)
//                .add(ms::Property::Color, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f))
//                .setFragmentSnippet("float d = distance(uv, vec2(0.5));\n"
//                                    "\n"
//                                    "mat_color.a = clamp(1.0 - d * 3.0, 0.0, 1.0);")
//                .addModelShader(Limitless::ModelShader::Effect)
//                .build();
//
////        materialBuilder.setName("blink")
////            .addUniform(std::make_unique<UniformValue<float>>("amount", 0.5f))
////            .addUniform(std::make_unique<UniformSampler>("noise", loader.load(assets_dir / "textures/noise.png")))
////            .setShading(Limitless::ms::Shading::Lit)
////            .setBlending(Limitless::ms::Blending::Opaque)
////            .setFragmentSnippet("float level = (1.0 - amount) * 2.0 - 1.0;\n"
////                                "level = 1.0 - uv.y - level;\n"
////                                "\n"
////                                "float noise1 = texture(noise, uv).r;\n"
////                                "noise1 = pow(noise1, 2.0);\n"
////                                "float offset = (1.0 - uv.y - level) * noise1;\n"
////                                "noise1 = ceil(clamp(level, 0.0, 1.0) - noise1);\n"
////                                "\n"
////                                "mat_diffuse *= noise1;\n"
////                                "\n"
////                                "mat_emissive_color.rgb = mix(vec3(7.0, 0.5, 32.0), vec3(5.0, 5.0, 0.0), offset - 4.5);")
////            .setVertexSnippet("float level = (1.0 - amount) * 2.0 - 1.0;\n"
////                              "level = 1.0 - uv.y - level;\n"
////                              "\n"
////                              "float noise1 = texture(noise, uv).r;\n"
////                              "noise1 = pow(noise1, 2.0);\n"
////                              "float offset = (1.0 - uv.y - level) * noise1;\n"
////                              "noise1 = ceil(level - noise1);\n"
////                              "\n"
////                              "vec3 v_offset = normalize(normal) * 25.0 * vec3(0.0, 1.0, 0.0);// * clamp(offset, 0.0, 1.0);\n"
////                              "vertex_position.xyz += v_offset;")
////            .add(Limitless::ms::Property::Diffuse, loader.load(assets_dir / "textures/test.png"))
////            .add(Limitless::ms::Property::EmissiveColor, glm::vec4(1.0f))
////            .addModelShader(ModelShader::Skeletal)
////        .build();
////
////        for (auto& [id, mesh] : bob->getMeshes()) {
////            mesh.getMaterial().changeMaterial(assets.materials.at("blink"));
////        }
//
//        materialBuilder.setName("fireball_material")
//                .setFragmentSnippet("uv.y -= 0.1;\n"
//                                 "vec2 panned = vec2(uv.x + getParticleTime() * 0.5 + getParticleProperties().x, "
//                                                    "uv.y + getParticleTime() * 0.8 + getParticleProperties().y);\n"
//                                 "uv += texture(noise, panned).g * 0.3;\n"
//                                 "\n"
//                                 "vec2 offset_panned1 = vec2(uv.x + getParticleTime() * 0.66, uv.y + getParticleTime() * 0.33);\n"
//                                 "float offset1 = texture(noise, offset_panned1).r;\n"
//                                 "\n"
//                                 "vec2 offset_panned2 = vec2(uv.x + getParticleTime() * 0.45, uv.y + getParticleTime() * 0.71);\n"
//                                 "float offset2 = texture(noise, offset_panned2).r;\n"
//                                 "\n"
//                                 "float r = offset1 * offset2;\n"
//                                 "\n"
//                                 "mat_diffuse = texture(material_diffuse, uv);\n"
//                                 "mat_diffuse.rgb *= clamp((mat_diffuse.a - getParticleProperties().z) * r, 0, 1);")
//                .addUniform(std::make_unique<UniformSampler>("noise", loader.load(assets_dir / "textures/true_noise.tga")))
//                .add(ms::Property::Diffuse, loader.load(assets_dir / "textures/true_fire.tga"))
//                .add(ms::Property::EmissiveColor, glm::vec4{10.0f, 3.0f, 1.0f, 1.0f})
//                .setShading(ms::Shading::Unlit)
//                .setBlending(ms::Blending::Additive)
//                .addModelShader(Limitless::ModelShader::Effect)
//                .build();
//
//        materialBuilder.setName("fireball_ball")
//                .setVertexSnippet("vec2 uv_1 = vec2(uv.x + getParticleTime() * 0.05, uv.y + getParticleTime() * 0.0);\n"
//                                    "vec2 uv_2 = vec2(uv.x - getParticleTime() * 0.05, uv.y - getParticleTime() * 0.0);\n"
//                                    " \n"
//                                    "float s = texture(fire_mask, uv_1).r;\n"
//                                    "float t = texture(fire_mask, uv_2).r;\n"
//                                    "\n"
//                                    "vertex_position.xyz -= getMeshNormal() * texture(fire_mask, uv + vec2(s, t)).r * 0.6;")
//                .setFragmentSnippet("vec2 uv_1 = vec2(uv.x + getParticleTime() * 0.05, uv.y + getParticleTime() * 0.0);\n"
//                                  "vec2 uv_2 = vec2(uv.x - getParticleTime() * 0.05, uv.y - getParticleTime() * 0.0);\n"
//                                  " \n"
//                                  "float s = texture(fire_mask, uv_1).r;\n"
//                                  "float t = texture(fire_mask, uv_2).r;\n"
//                                  "\n"
//                                  "mat_diffuse.rgb = texture(material_diffuse, uv + vec2(s, t)).rgb;\n"
//                                  "\n"
//                                  "if (mat_diffuse.r <= 0.2) discard;")
//                .addUniform(std::make_unique<UniformSampler>("fire_mask", loader.load(assets_dir / "textures/fireball_mask.png")))
//                .add(ms::Property::Diffuse, loader.load(assets_dir / "textures/rock_lava.png"))
//                .add(ms::Property::Color, glm::vec4{15.0f, 5.0f, 0.0f, 1.0f})
//                .setShading(ms::Shading::Unlit)
//                .setBlending(ms::Blending::Opaque)
//                .addModelShader(Limitless::ModelShader::Effect)
//                .setTwoSided(true)
//                .build();
//
//        effectBuilder.create("fireball")
//                .createEmitter<Limitless::fx::SpriteEmitter>("fire")
////                .addMeshLocationAttachment(assets.models.at("bob"))
//                    .addInitialRotation(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{6.28f, 0.0f, 0.0f}))
//                    .addLifetime(std::make_unique<RangeDistribution<float>>(0.5f, 1.0f))
//                    .addInitialSize(std::make_unique<ConstDistribution<float>>(64.0f))
//                    .addSizeByLife(std::make_unique<RangeDistribution<float>>(256.0f, 512.0f), -1.0f)
//                    .addCustomMaterial(std::make_unique<RangeDistribution<float>>(0.0f, 0.9f),
//                                       std::make_unique<RangeDistribution<float>>(0.0f, 0.9f),
//                                       std::make_unique<ConstDistribution<float>>(0.0f),
//                                       nullptr)
//                    .addCustomMaterialByLife(nullptr,
//                                             nullptr,
//                                             std::make_unique<ConstDistribution<float>>(1.0f),
//                                             nullptr)
//                    .setMaterial(assets.materials.at("fireball_material"))
//                    .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Spray)
//                    .setMaxCount(1000)
//                    .setSpawnRate(500.0f)
//                .createEmitter<fx::MeshEmitter>("ball")
//                    .setMesh(assets.meshes.at("sphere"))
//                    .setMaxCount(1)
//                    .setSpawnRate(1.0f)
//                    .setMaterial(assets.materials.at("fireball_ball"))
//                    .setLocalSpace(true)
//                    .addInitialSize(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.25f)))
//                .addTime()
//                .build();
//
//        scene.add<EffectInstance>(assets.effects.at("fireball"), glm::vec3(3.0f));
//
//        effectBuilder.create("deflect")
//                     .createEmitter<fx::MeshEmitter>("shield")
//                     .setMesh(assets.meshes.at("plane"))
//                     .setMaterial(assets.materials.at("deflect"))
//                     .setLocalSpace(true)
//                     .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Spray)
//                     .setMaxCount(1)
//                     .setSpawnRate(5.0f)
//                     .addInitialSize(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.5f)))
//                     .addTime()
//                     .build();
//
//        scene.add<EffectInstance>(assets.effects.at("deflect"), glm::vec3(7.0f));
//
//        effectBuilder.create("conflagrate")
//                .createEmitter<fx::MeshEmitter>("flash")
//                    .setMesh(assets.meshes.at("plane"))
//                    .setMaterial(assets.materials.at("conflagrate_flash"))
//                    .setLocalSpace(true)
//                    .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Spray)
//                    .setMaxCount(2)
//                    .setSpawnRate(1.5f)
//                    .addInitialSize(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.1f)))
//                    .addSizeByLife(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(7.0f)), 1.0f)
//                    .addLifetime(std::make_unique<ConstDistribution<float>>(1.0f))
//                    .addInitialColor(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(5.0f, 0.0f, 5.0f, 1.0f)))
//                    .addColorByLife(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)))
//                .createEmitter<fx::MeshEmitter>("aura")
//                    .setMesh(assets.meshes.at("plane"))
//                    .setMaterial(assets.materials.at("conflagrate_aura"))
//                    .setLocalSpace(true)
//                    .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Spray)
//                    .setMaxCount(1)
//                    .setSpawnRate(5.0f)
//                    .addInitialSize(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(2.0f)))
//                    .addRotationRate(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.0f, 1.0f, 0.0f)))
//                    .addInitialColor(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(0.0f, 0.0625f, 2.0f, 1.0f)))
//                .build();
//
//        scene.add<EffectInstance>(assets.effects.at("conflagrate"), glm::vec3(5.0f));
//
//        effectBuilder.create("conflagrate_finish")
//                .createEmitter<fx::MeshEmitter>("flash")
//                    .setMesh(assets.meshes.at("sphere"))
//                    .setMaterial(assets.materials.at("conflagrate_burst"))
//                    .setLocalSpace(true)
//                    .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Spray)
//                    .setMaxCount(1)
//                    .setSpawnRate(1.0f)
//                    .addInitialSize(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(1.0f)))
//                    .addSizeByLife(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(2.0f)), 1.0f)
//                    .addLifetime(std::make_unique<ConstDistribution<float>>(1.0f))
//                    .addInitialColor(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(10.0f, 0.0f, 99.0f, 1.0f)))
//                    .addInitialRotation(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.0f)))
//                    .addRotationRate(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.0f, 3.14f, 0.0f)))
//                    .addColorByLife(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)))
//                .createEmitter<fx::SpriteEmitter>("sparks")
//                    .setMaterial(assets.materials.at("conflagrate_point"))
//                    .setLocalSpace(true)
//                    .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Burst)
//                    .setBurstCount(std::make_unique<ConstDistribution<uint32_t>>(1000))
//                    .setMaxCount(1000)
//                    .setSpawnRate(400.0f)
//                    .addInitialSize(std::make_unique<ConstDistribution<float>>(1.0f))
//                    .addSizeByLife(std::make_unique<ConstDistribution<float>>(200.0f), 1.0f)
//                    .addLifetime(std::make_unique<ConstDistribution<float>>(0.5f))
//                    .addInitialVelocity(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(-2.0f, 0.0f, -2.0f), glm::vec3(2.0f, 0.0f, 2.0f)))
//                    .addInitialAcceleration(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.0f, 1.0f, 0.0f)))
//                    .addInitialColor(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)))
//                    .addInitialMeshLocation(assets.models.at("sphere"))
//                .build();
//
//        scene.add<EffectInstance>(assets.effects.at("conflagrate_finish"), glm::vec3(9.0f));
//    }
EffectInstance* inst;
    void addEffects() {
        fx::EffectBuilder builder {assets};

        builder.create("mesh_test")
               .createEmitter<fx::SpriteEmitter>("sparks")
               .addLifetime(std::make_unique<ConstDistribution<float>>(1.0f))
//               .addLifetime(std::make_unique<ConstDistribution<float>>(9999.0f))
               .addInitialSize(std::make_unique<ConstDistribution<float>>(5.0f))
               .addInitialColor(std::make_unique<RangeDistribution<glm::vec4>>(glm::vec4{0.0f}, glm::vec4{2.0f}))
//               .addInitialMeshLocation(assets.models.at("bob"))
               .addMeshLocationAttachment(assets.models.at("bob"))
               .setMaterial(assets.materials.at("EmissiveColor"))
               .setMaxCount(1000)
               .setSpawnRate(1000.0f)
               .build();

        builder.create("effect1")
                .createEmitter<fx::SpriteEmitter>("generate")
                .addInitialVelocity(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3{-5.0f, 0.0f, -5.0f}, glm::vec3{5.0f}))
                .addInitialAcceleration(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3{0.0f}, glm::vec3{0.0f, 5.0f, 0.0f}))
                .addLifetime(std::make_unique<RangeDistribution<float>>(0.2f, 0.5f))
                .addInitialSize(std::make_unique<RangeDistribution<float>>(1.0f, 25.0f))
                .addSizeByLife(std::make_unique<ConstDistribution<float>>(0.0f))
                .addInitialColor(std::make_unique<RangeDistribution<glm::vec4>>(glm::vec4{0.0f}, glm::vec4{2.0f}))
                .setMaterial(assets.materials.at("EmissiveColor"))
                .setSpawnMode(fx::EmitterSpawn::Mode::Burst)
                .setBurstCount(std::make_unique<ConstDistribution<uint32_t>>(100))
                .setMaxCount(100)
                .setSpawnRate(1.0f)
                .setLocalSpace(true)
                .build();

        effect = &scene.add<EffectInstance>(assets.effects.at("effect1"), glm::vec3{2.f, 1.f, 8.f});
        auto& e = scene.add<EffectInstance>(assets.effects.at("effect1"), glm::vec3{4.f, 1.f, 8.f});
        scene.add<EffectInstance>(assets.effects.at("effect1"), glm::vec3{6.f, 1.f, 8.f});

        e.get<fx::SpriteEmitter>("generate").getMaterial().getEmissiveColor().setValue(glm::vec4(2.0f));

        builder.create("effect2")
                .createEmitter<fx::MeshEmitter>("generate")
                .addInitialVelocity(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3{-5.0f, 0.0f, -5.0f}, glm::vec3{5.0f}))
                .addInitialAcceleration(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3{0.0f}, glm::vec3{0.0f, 5.0f, 0.0f}))
                .addLifetime(std::make_unique<RangeDistribution<float>>(0.2f, 0.5f))
                .addInitialSize(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(1.0f)))
                .addInitialColor(std::make_unique<RangeDistribution<glm::vec4>>(glm::vec4{0.0f}, glm::vec4{2.0f}))
                .setMaterial(assets.materials.at("EmissiveColor"))
                .setMesh(assets.meshes.at("sphere"))
                .setSpawnMode(fx::EmitterSpawn::Mode::Burst)
                .setBurstCount(std::make_unique<ConstDistribution<uint32_t>>(100))
                .setMaxCount(100)
                .setSpawnRate(1.0f)
                .setLocalSpace(true)
                .build();

//        EffectLoader::save(assets.getBaseDir() / "effects/test2", assets.effects.at("effect2"));
//        EffectLoader::load(context, assets, RenderSettings{}, assets.getBaseDir() / "effects/test2");

        effect = &scene.add<EffectInstance>(assets.effects.at("effect2"), glm::vec3{5.f, 1.f, -5.f});

        fx::EffectBuilder beam_builder{assets};
        beam_builder.create("test_beam")
                     .createEmitter<fx::BeamEmitter>("test")
                     .setMaterial(assets.materials.at("EmissiveColor"))
                     .addLifetime(std::make_unique<ConstDistribution<float>>(1.0f))
                     .setMaxCount(1)
                     .setSpawnRate(1.0f)
                     .build();

//        EffectLoader::save(assets.getBaseDir() / "effects/test3", assets.effects.at("test_beam"));
//        EffectLoader::load(context, assets, RenderSettings{}, assets.getBaseDir() / "effects/test3");

        scene.add<EffectInstance>(assets.effects.at("test_beam"), glm::vec3{8.0f, 2.0f, 8.0f});

        const auto& module = scene.add<EffectInstance>(assets.effects.at("mesh_test"), glm::vec3{0.0f, 0.0f, 0.0f})
            .get<fx::SpriteEmitter>("sparks")
            .getModule(fx::ModuleType::MeshLocationAttachment);
//
        static_cast<fx::MeshLocationAttachment<fx::SpriteParticle>&>(*module)
        .attachModelInstance(bob);

        ms::MaterialBuilder materialBuilder {assets};
        materialBuilder.setName("beam_lightning")
                .add(ms::Property::EmissiveColor, glm::vec4(5.0f, 4.8f, 3.8f, 1.0f))
                .setShading(ms::Shading::Unlit)
                .setBlending(ms::Blending::Opaque)
                .addModelShader(ModelShader::Effect)
                .build();

        builder.create("lightning")
                .createEmitter<fx::BeamEmitter>("beam")
                .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Burst)
                .setBurstCount(std::make_unique<ConstDistribution<uint32_t>>(10))
                .setMaxCount(10)
                .addTime()
                .setSpawnRate(10.0f)
                .addLifetime(std::make_unique<ConstDistribution<float>>(5.0f))
                .addInitialSize(std::make_unique<ConstDistribution<float>>(8.0f))
                .addBeamInitialTarget(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.0f)))
                .addBeamInitialDisplacement(std::make_unique<ConstDistribution<float>>(0.5f))
                .addBeamInitialOffset(std::make_unique<ConstDistribution<float>>(0.1f))
                .addBeamInitialRebuild(std::make_unique<ConstDistribution<float>>(2.0f))
                .setMaterial(assets.materials.at("beam_lightning"))
                .addBeamSpeed(std::make_unique<ConstDistribution<float>>(1.0f))
                .build();

        scene.add<EffectInstance>(assets.effects.at("lightning"), glm::vec3{5.0f});
        scene.add<EffectInstance>(assets.effects.at("lightning"), glm::vec3{10.0f, 10.0f, -10.0f});

        const fs::path assets_dir {ASSETS_DIR};

        materialBuilder.setName("blink")
            .add(Limitless::ms::Property::Color, glm::vec4(1.0f))
            .add(Limitless::ms::Property::Diffuse, TextureLoader::load(assets, assets_dir / "textures/fireball_mask.png"))
            .add(Limitless::ms::Property::Normal, TextureLoader::load(assets, assets_dir / "textures/blink.jpg"))
            .setShading(Limitless::ms::Shading::Unlit)
            .setBlending(Limitless::ms::Blending::Translucent)
            .setTwoSided(true)
            .setFragmentSnippet("float tile = getParticleProperties().z;\n"
                                "float erode = getParticleProperties().y;\n"
                                "float dist = getParticleProperties().x;\n"
                                "\n"
                                "float r = texture(material_diffuse, uv * tile).r;\n"
                                "\n"
                                "vec2 uv1 = vec2(0.0, dist) * r + uv;\n"
                                "\n"
                                "float m_a = texture(material_normal, uv1).r;\n"
                                "\n"
                                "mat_color.a = mix(0.0, m_a, pow(r, erode));"
                                "mat_diffuse.rgb = vec3(1.0);")
            .addModelShader(Limitless::ModelShader::Effect)
            .build();

        materialBuilder.setName("blink_particles")
                .add(Limitless::ms::Property::Diffuse, TextureLoader::load(assets, assets_dir / "textures/glow.tga"))
                .setShading(Limitless::ms::Shading::Unlit)
                .setBlending(Limitless::ms::Blending::Additive)
                .addModelShader(Limitless::ModelShader::Effect)
                .build();

        builder.create("blink")
//                .createEmitter<fx::MeshEmitter>("smoke_purple")
                .createEmitter<fx::SpriteEmitter>("smoke_purple")
                    .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Burst)
                    .setBurstCount(std::make_unique<ConstDistribution<uint32_t>>(10))
                    .setMaxCount(10)
                    .setSpawnRate(10.0f)
                    .addInitialSize(std::make_unique<RangeDistribution<float>>(128.0f, 256.0f))
                    .addSizeByLife(std::make_unique<ConstDistribution<float>>(512.0f))
                    .addInitialRotation(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(0.0f), glm::vec3(6.28f)))
                    .addInitialLocation(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(0.0f), glm::vec3(0.1f)))
                    .addInitialVelocity(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(-0.05f, 0.01f, -0.05f), glm::vec3(0.05f)))
                    .addInitialAcceleration(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(-0.01f, 0.2f, -0.01f), glm::vec3(0.01f, 0.2f, 0.01f)))
//                    .addRotationRate(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.5f)))
                    .addLifetime(std::make_unique<ConstDistribution<float>>(2.0f))
                    .setMaterial(assets.materials.at("blink"))
//                    .setMesh(assets.meshes.at("sphere"))
                    .addInitialColor(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(0.54f * 2.0f, 0.0f, 1.0f * 2.0f, 1.0f)))
                    .addColorByLife(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(0.54f * 4.0f, 0.0f, 1.0f * 4.0f, 1.0f)))
                    .addCustomMaterial(std::make_unique<RangeDistribution<float>>(0.0f, 0.3f),
                                       std::make_unique<RangeDistribution<float>>(0.0f, 2.5f),
                                       std::make_unique<RangeDistribution<float>>(0.0, 5.0f),
                                       nullptr)
                    .addCustomMaterialByLife(std::make_unique<ConstDistribution<float>>(1.0f),
                                             std::make_unique<ConstDistribution<float>>(10.0f),
                                             nullptr,
                                             nullptr)
//                .createEmitter<fx::MeshEmitter>("smoke_black")
                .createEmitter<fx::SpriteEmitter>("smoke_black")
                    .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Burst)
                    .setBurstCount(std::make_unique<ConstDistribution<uint32_t>>(10))
                    .setMaxCount(10)
                    .setSpawnRate(10.0f)
//                    .addInitialSize(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(0.1f), glm::vec3(0.15f)))
                    .addInitialSize(std::make_unique<RangeDistribution<float>>(64.0f, 128.0f))
//                    .addSizeByLife(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.5f)), 1.0f)
                    .addSizeByLife(std::make_unique<ConstDistribution<float>>(256.0f))
                    .addInitialRotation(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(0.0f), glm::vec3(6.28f)))
                    .addInitialLocation(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(0.0f), glm::vec3(0.1f)))
                    .addLifetime(std::make_unique<ConstDistribution<float>>(2.0f))
                    .setMaterial(assets.materials.at("blink"))
//                    .setMesh(assets.meshes.at("sphere"))
                    .addInitialColor(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)))
                    .addCustomMaterial(std::make_unique<RangeDistribution<float>>(0.0f, 0.3f),
                                       std::make_unique<RangeDistribution<float>>(0.0f, 3.0f),
                                       std::make_unique<RangeDistribution<float>>(0.0, 5.0f),
                                       nullptr)
                    .addCustomMaterialByLife(std::make_unique<ConstDistribution<float>>(1.0f),
                                             std::make_unique<ConstDistribution<float>>(10.0f),
                                             nullptr,
                                             nullptr)
//                .createEmitter<fx::SpriteEmitter>("particles_purple")
//                    .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Burst)
//                    .setBurstCount(std::make_unique<ConstDistribution<uint32_t>>(20))
//                    .setMaxCount(20)
//                    .setSpawnRate(20.0f)
//                    .addInitialSize(std::make_unique<ConstDistribution<float>>(8.0f))
//                    .addInitialVelocity(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(-0.1f, 0.0f, -0.1f), glm::vec3(0.1f)))
//                    .addInitialAcceleration(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(-0.1f, 0.1f, -0.1f), glm::vec3(0.1f)))
//                    .addSizeByLife(std::make_unique<ConstDistribution<float>>(0.0f), 1.0f)
//                    .addInitialLocation(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(0.0f), glm::vec3(0.1f)))
//                    .addInitialRotation(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(0.0f), glm::vec3(6.28f)))
//                    .addLifetime(std::make_unique<ConstDistribution<float>>(2.0f))
//                    .setMaterial(assets.materials.at("blink_particles"))
//                    .addInitialColor(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(0.84f * 5.0f, 2.0f, 1.0f * 5.0f, 1.0f)))
//                .createEmitter<fx::SpriteEmitter>("particles_black")
//                    .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Burst)
//                    .setBurstCount(std::make_unique<ConstDistribution<uint32_t>>(20))
//                    .setMaxCount(20)
//                    .setSpawnRate(20.0f)
//                    .addInitialSize(std::make_unique<ConstDistribution<float>>(32.0f))
//                    .addInitialVelocity(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(-0.25f, 0.0f, -0.25f), glm::vec3(0.25f)))
//                    .addInitialAcceleration(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(-0.5f, 0.1f, -0.5f), glm::vec3(0.5f)))
//                    .addSizeByLife(std::make_unique<ConstDistribution<float>>(0.0f), 1.0f)
//                    .addInitialRotation(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(0.0f), glm::vec3(6.28f)))
//                    .addLifetime(std::make_unique<ConstDistribution<float>>(2.0f))
//                    .setMaterial(assets.materials.at("blink_particles"))
//                    .addInitialColor(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(0.54f * 6.0f, 0.0f, 1.0f * 6.0f, 1.0f)))
                .build();

        scene.add<EffectInstance>(assets.effects.at("blink"), glm::vec3{-3.0f, 3.0f, 3.0f});

        materialBuilder.setName("shield")
            .setShading(Limitless::ms::Shading::Unlit)
            .setBlending(Limitless::ms::Blending::Additive)
            .addModelShader(Limitless::ModelShader::Effect)
            .add(Limitless::ms::Property::Color, glm::vec4(1.0f))
            .addUniform(std::make_unique<UniformSampler>("maintexture", TextureLoader::load(assets, assets_dir / "textures/shield_texture.jpg")))
            .addUniform(std::make_unique<UniformSampler>("noise", TextureLoader::load(assets, assets_dir / "textures/noise.png")))
            .addUniform(std::make_unique<UniformValue<float>>("vertex_offset_freq", 2.0f))
            .addUniform(std::make_unique<UniformValue<glm::vec3>>("vertex_offset_dir", glm::vec3(0.2)))
            .setFragmentSnippet(""
                                "float fres = fresnel(getMeshNormal(), camera_position.xyz - getParticlePosition(), 3.5);"
                                "mat_color.rgb *= 1.0 - texture(maintexture, uv).r;"
                                "mat_color.rgb *= fres * vec3(33.0 / 255.0 * 15.0f, 99.0 / 255.0 * 15.0f, 1000.0 / 255.0 * 25.0f);")
            .setVertexSnippet("vertex_position += sin(getParticleTime() * vertex_offset_freq) * getMeshNormal() * vertex_offset_dir *texture(noise, getParticleTime() + uv).r;")

            .setGlobalSnippet("#include \"../expressions/fresnel.glsl\"")
        .build();

        materialBuilder.setName("shield_hit")
                .setShading(Limitless::ms::Shading::Unlit)
                .setBlending(Limitless::ms::Blending::Translucent)
                .addModelShader(Limitless::ModelShader::Effect)
                .setTwoSided(true)
                .add(Limitless::ms::Property::Color, glm::vec4(8.1f, 0.5f, 12.3f, 1.0f))
                .addUniform(std::make_unique<UniformSampler>("maintexture", TextureLoader::load(assets, assets_dir / "textures/shield_texture.jpg")))
                .addUniform(std::make_unique<UniformSampler>("noise", TextureLoader::load(assets, assets_dir / "textures/noise.png")))
                .addUniform(std::make_unique<UniformValue<float>>("vertex_offset_freq", 2.0f))
                .addUniform(std::make_unique<UniformValue<glm::vec3>>("vertex_offset_dir", glm::vec3(0.2)))
                .addUniform(std::make_unique<UniformValue<glm::vec3>>("hit_pos", glm::vec3(0.9, 2.0, 0.0)))
                .setFragmentSnippet(""
                                    "float fres = sphere_mask(getParticlePosition(), hit_pos, 0.4, getParticleProperties().x);"
                                    "mat_color *= 1.0 - texture(maintexture, uv).r;"
                                    "mat_color.a *= fres;")
                .setGlobalSnippet("#include \"../functions/sphere_mask.glsl\"\n")
                .setVertexSnippet("vertex_position += sin(getParticleTime() * vertex_offset_freq) * getMeshNormal() * vertex_offset_dir * texture(noise, getParticleTime() + uv).r;")
                .build();

        builder.create("shield")
                .createEmitter<fx::MeshEmitter>("shield")
                        .addInitialSize(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.5f)))
                        .setMaterial(assets.materials.at("shield"))
                        .setMesh(assets.meshes.at("sphere"))
                        .addTime()
                        .setMaxCount(1)
                        .build();

        builder.create("shield_hit")
                .createEmitter<fx::MeshEmitter>("hit")
                        .addInitialSize(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.5f)))
                        .setMaterial(assets.materials.at("shield_hit"))
                        .setMesh(assets.meshes.at("sphere"))
                        .addCustomMaterial(std::make_unique<ConstDistribution<float>>(1.0f), nullptr, nullptr, nullptr)
                        .addCustomMaterialByLife(std::make_unique<ConstDistribution<float>>(0.3f), nullptr, nullptr, nullptr)
                        .addLifetime(std::make_unique<ConstDistribution<float>>(1.0f))
                        .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Spray)
                        .setDuration(std::chrono::seconds(1))
                        .addTime()
                        .setMaxCount(1)
                        .setSpawnRate(1.0f)
                        .build();

        inst = &scene.add<EffectInstance>(assets.effects.at("shield"), glm::vec3{0.0f, 2.0f, 0.0f});

        materialBuilder.setName("fireball_material")
                .setFragmentSnippet("uv.y -= 0.1;\n"
                                    "vec2 panned = vec2(uv.x + getParticleTime() * 0.5 + getParticleProperties().x, "
                                    "uv.y + getParticleTime() * 0.8 + getParticleProperties().y);\n"
                                    "uv += texture(noise, panned).g * 0.3;\n"
                                    "\n"
                                    "vec2 offset_panned1 = vec2(uv.x + getParticleTime() * 0.66, uv.y + getParticleTime() * 0.33);\n"
                                    "float offset1 = texture(noise, offset_panned1).r;\n"
                                    "\n"
                                    "vec2 offset_panned2 = vec2(uv.x + getParticleTime() * 0.45, uv.y + getParticleTime() * 0.71);\n"
                                    "float offset2 = texture(noise, offset_panned2).r;\n"
                                    "\n"
                                    "float r = offset1 * offset2;\n"
                                    "\n"
                                    "mat_diffuse = texture(material_diffuse, uv);\n"
                                    "mat_diffuse.rgb *= clamp((mat_diffuse.a - getParticleProperties().z) * r, 0, 1);")
                .addUniform(std::make_unique<UniformSampler>("noise", TextureLoader::load(assets, assets_dir / "textures/true_noise.tga")))
                .add(ms::Property::Diffuse, TextureLoader::load(assets, assets_dir / "textures/true_fire.tga"))
                .add(ms::Property::EmissiveColor, glm::vec4{10.0f, 3.0f, 1.0f, 1.0f})
                .setShading(ms::Shading::Unlit)
                .setBlending(ms::Blending::Additive)
                .addModelShader(Limitless::ModelShader::Effect)
                .build();

        materialBuilder.setName("fireball_sparks")
                .setFragmentSnippet("mat_emissive_color.rgb *= circle(uv, 0.7);")
                .setGlobalSnippet("#include \"../functions/circle.glsl\"")
                //.addUniform(std::make_unique<UniformSampler>("noise", loader.load(assets_dir / "textures/true_noise.tga")))
                .add(ms::Property::EmissiveColor, glm::vec4{5.0f, 1.5f, 1.0f, 1.0f})
                .setShading(ms::Shading::Unlit)
                .setBlending(ms::Blending::Additive)
                .addModelShader(Limitless::ModelShader::Effect)
                .build();

        materialBuilder.setName("fireball_ball")
                .setVertexSnippet("vec2 uv_1 = vec2(uv.x + getParticleTime() * 0.05, uv.y + getParticleTime() * 0.0);\n"
                                  "vec2 uv_2 = vec2(uv.x - getParticleTime() * 0.05, uv.y - getParticleTime() * 0.0);\n"
                                  " \n"
                                  "float s = texture(fire_mask, uv_1).r;\n"
                                  "float t = texture(fire_mask, uv_2).r;\n"
                                  "\n"
                                  "vertex_position.xyz -= getMeshNormal() * texture(fire_mask, uv + vec2(s, t)).r * 0.6;")
                .setFragmentSnippet("vec2 uv_1 = vec2(uv.x + getParticleTime() * 0.05, uv.y + getParticleTime() * 0.0);\n"
                                    "vec2 uv_2 = vec2(uv.x - getParticleTime() * 0.05, uv.y - getParticleTime() * 0.0);\n"
                                    " \n"
                                    "float s = texture(fire_mask, uv_1).r;\n"
                                    "float t = texture(fire_mask, uv_2).r;\n"
                                    "\n"
                                    "mat_diffuse.rgb = texture(material_diffuse, uv + vec2(s, t)).rgb;\n"
                                    "\n"
                                    "if (mat_diffuse.r <= 0.2) discard;")
                .addUniform(std::make_unique<UniformSampler>("fire_mask", TextureLoader::load(assets, assets_dir / "textures/fireball_mask.png")))
                .add(ms::Property::Diffuse, TextureLoader::load(assets, assets_dir / "textures/rock_lava.png"))
                .add(ms::Property::Color, glm::vec4{15.0f, 5.0f, 0.0f, 1.0f})
                .setShading(ms::Shading::Unlit)
                .setBlending(ms::Blending::Opaque)
                .addModelShader(Limitless::ModelShader::Effect)
                .setTwoSided(true)
                .build();

        builder.create("fireball")
                .createEmitter<Limitless::fx::SpriteEmitter>("fire")
                    .addInitialRotation(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{6.28f, 0.0f, 0.0f}))
                    .addLifetime(std::make_unique<RangeDistribution<float>>(0.5f, 0.7f))
                    .addInitialSize(std::make_unique<ConstDistribution<float>>(256.0f))
                    .addSizeByLife(std::make_unique<ConstDistribution<float>>(512.0f))
                    .addCustomMaterial(std::make_unique<RangeDistribution<float>>(0.0f, 0.9f),
                                       std::make_unique<RangeDistribution<float>>(0.0f, 0.9f),
                                       std::make_unique<ConstDistribution<float>>(0.0f),
                                       nullptr)
                    .addCustomMaterialByLife(nullptr,
                                             nullptr,
                                             std::make_unique<ConstDistribution<float>>(1.0f),
                                             nullptr)
                    .setMaterial(assets.materials.at("fireball_material"))
                    .setSpawnMode(Limitless::fx::EmitterSpawn::Mode::Spray)
                    .addInitialVelocity(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(1.0, -1.0, -1.0), glm::vec3(2.0, 1.0, 1.0)))
                    .setMaxCount(1000)
                    .setSpawnRate(200.0f)
                .createEmitter<fx::MeshEmitter>("ball")
                    .setMesh(assets.meshes.at("sphere"))
                    .setMaxCount(1)
                    .setSpawnRate(200.0f)
                    .setMaterial(assets.materials.at("fireball_ball"))
                    .setLocalSpace(true)
                    .addInitialSize(std::make_unique<ConstDistribution<glm::vec3>>(glm::vec3(0.2f)))
                    .addTime()
                .createEmitter<fx::SpriteEmitter>("sparks")
                    .addLifetime(std::make_unique<RangeDistribution<float>>(0.5f, 0.7f))
                    .addInitialSize(std::make_unique<ConstDistribution<float>>(25.0f))
                    .addSizeByLife(std::make_unique<ConstDistribution<float>>(0.0f))
                    .setMaterial(assets.materials.at("fireball_sparks"))
                    .addInitialVelocity(std::make_unique<RangeDistribution<glm::vec3>>(glm::vec3(1.0, -1.0, -1.0), glm::vec3(2.0, 1.0, 1.0)))
                    .addInitialColor(std::make_unique<RangeDistribution<glm::vec4>>(glm::vec4(1.5, 1.0, 1.0, 1.0), glm::vec4(2.5, 2.5, 2.5, 2.5)))
                    .addColorByLife(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(1.5, 0.5, 0.0, 1.0)))
                    .addInitialMeshLocation(assets.meshes.at("sphere"), glm::vec3(0.25f), glm::vec3(0.0f))
                    .setMaxCount(200)
                    .setSpawnRate(200.0f)
                .createEmitter<fx::SpriteEmitter>("glow")
                    .addInitialSize(std::make_unique<ConstDistribution<float>>(128.0f))
                    .addSizeByLife(std::make_unique<ConstDistribution<float>>(256.0f))
                    .setMaterial(assets.materials.at("fireball_sparks"))
                    .setLocalSpace(true)
                    .setMaxCount(1)
                    .addInitialColor(std::make_unique<ConstDistribution<glm::vec4>>(glm::vec4(0.3f)))
                    .setSpawnRate(5.0f)
                .build();

        fireball = &scene.add<EffectInstance>(assets.effects.at("fireball"), glm::vec3{3.0f, 3.0f, 3.0f});
    }

    void onMouseMove(glm::dvec2 pos) override {
        static glm::dvec2 last_move = { 0, 0 };

        auto offset = glm::vec2{ pos.x - last_move.x, last_move.y - pos.y };
        last_move = pos;

        camera.mouseMove(offset);
    }

    void onKey(int key, [[maybe_unused]] int scancode, InputState state, [[maybe_unused]] Modifier modifier) override {
        if (key == GLFW_KEY_ESCAPE && state == InputState::Pressed) {
            done = true;
        }

        if (key == GLFW_KEY_1 && state == InputState::Pressed) {
            render.getSettings().physically_based_render = !render.getSettings().physically_based_render;
            render.update(context, assets, scene);
        }

        if (key == GLFW_KEY_SPACE && state == InputState::Pressed) {
            scene.add<EffectInstance>(assets.effects.at("lightning"), glm::vec3{10.0f, 10.0f, -10.0f});
        }

        if (key == GLFW_KEY_2 && state == InputState::Pressed) {
            render.getSettings().normal_mapping = !render.getSettings().normal_mapping;
            render.update(context, assets, scene);
        }

        if (key == GLFW_KEY_3 && state == InputState::Pressed) {
            render.getSettings().shading_model = render.getSettings().shading_model == ShadingModel::BlinnPhong ? ShadingModel::Phong : ShadingModel::BlinnPhong;
            render.update(context, assets, scene);
        }

        if (key == GLFW_KEY_4 && state == InputState::Pressed) {
            render.getSettings().directional_csm = !render.getSettings().directional_csm;
            render.update(context, assets, scene);
        }
    }

    void onFramebufferChange(glm::uvec2 size) override {
        camera.updateProjection(size);
    }

    void handleInput(float delta) noexcept {
        if (context.isPressed(GLFW_KEY_7)) {
            bob->setPosition(bob->getPosition() + glm::vec3(0.1, 0.0, 0.0));
        }
        if (context.isPressed(GLFW_KEY_8)) {
            bob->setPosition(bob->getPosition() - glm::vec3(0.1, 0.0, 0.0));
        }

        if (context.isPressed(GLFW_KEY_W)) {
            camera.movement(CameraMovement::Forward, delta);
        }

        if (context.isPressed(GLFW_KEY_S)) {
            camera.movement(CameraMovement::Backward, delta);
        }

        if (context.isPressed(GLFW_KEY_A)) {
            camera.movement(CameraMovement::Left, delta);
        }

        if (context.isPressed(GLFW_KEY_D)) {
            camera.movement(CameraMovement::Right, delta);
        }

        if (context.isPressed(GLFW_KEY_Y)) {
            fireball->setPosition(fireball->getPosition() + glm::vec3(0.1, 0.0, 0.0));
        }

        if (context.isPressed(GLFW_KEY_U)) {
            fireball->setPosition(fireball->getPosition() - glm::vec3(0.1, 0.0, 0.0));
        }
    }

    void updateEffect(float delta) {
        if (effect) {
            effect->rotateBy(glm::vec3{0.0f, PI * delta * 2.0f, 0.0f});
        }
    }

    std::chrono::steady_clock::time_point last_time = std::chrono::steady_clock::now();
    void gameLoop() {
        using namespace std::chrono;
        while (!context.shouldClose() && !done) {
            auto time = steady_clock::now();
            auto delta = duration_cast<duration<float>>(time - last_time).count();
            last_time = time;

            updateEffect(delta);

            render.draw(context, assets, scene, camera);
            text->draw(context, assets);

            context.swapBuffers();
            context.pollEvents();
            handleInput(delta);
        }
    }
};

int main() {
    Game game;
    game.gameLoop();
    return 0;
}