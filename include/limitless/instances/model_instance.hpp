#pragma once

#include <limitless/instances/abstract_instance.hpp>
#include <limitless/instances/mesh_instance.hpp>

namespace Limitless {
    class AbstractModel;

    class ModelInstance : public AbstractInstance {
    protected:
        std::unordered_map<std::string, MeshInstance> meshes;
        std::shared_ptr<AbstractModel> model;

        void calculateBoundingBox() noexcept override;
    public:
        ModelInstance(decltype(model) model, const glm::vec3& position, const glm::vec3& rotation = glm::vec3{0.0f}, const glm::vec3& scale = glm::vec3{1.0f});
        ModelInstance(Lighting* lighting, decltype(model) model, const glm::vec3& position, const glm::vec3& rotation = glm::vec3{0.0f}, const glm::vec3& scale = glm::vec3{1.0f});

        ~ModelInstance() override = default;

        ModelInstance(const ModelInstance&) = default;
        ModelInstance(ModelInstance&&) noexcept = default;

        ModelInstance* clone() noexcept override;

        MeshInstance& operator[](const std::string& mesh);

        const auto& getMeshes() const noexcept { return meshes; }
        auto& getMeshes() noexcept { return meshes; }

        using AbstractInstance::draw;
        void draw(Context& ctx, const Assets& assets, ShaderPass shader_type, ms::Blending blending, const UniformSetter& uniform_setter) override;
    };
}