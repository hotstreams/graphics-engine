#include <limitless/instances/model_instance.hpp>

#include <limitless/pipeline/shader_pass_types.hpp>
#include <limitless/models/model.hpp>

using namespace Limitless;

ModelInstance::ModelInstance(decltype(model) _model, const glm::vec3& _position, const glm::vec3& _rotation, const glm::vec3& _scale)
    : ModelInstance {nullptr, std::move(_model), _position, _rotation, _scale} {
}

ModelInstance::ModelInstance(Lighting* lighting, decltype(model) _model, const glm::vec3& _position, const glm::vec3& _rotation, const glm::vec3& _scale)
    : AbstractInstance{lighting, ModelShader::Model, _position, _rotation, _scale}
    , model{std::move(_model)} {

    try {
        auto& simple_model = dynamic_cast<Model&>(*model);
        auto& model_meshes = simple_model.getMeshes();
        auto& model_mats = simple_model.getMaterials();

        for (uint32_t i = 0; i < model_meshes.size(); ++i) {
            meshes.emplace(model_meshes[i]->getName(), MeshInstance{model_meshes[i], model_mats[i]});
        }
    } catch (...) {
        throw std::runtime_error{"Wrong model for ModelInstance"};
    }
}

void ModelInstance::draw(Context& ctx, const Assets& assets, ShaderPass pass, ms::Blending blending, const UniformSetter& uniform_setter) {
    if (hidden) {
        return;
    }

    // iterates over all meshes
    for (auto& [name, mesh] : meshes) {
        mesh.draw(ctx, assets, pass, shader_type, model_matrix, blending, uniform_setter);
    }
}

MeshInstance& ModelInstance::operator[](const std::string& mesh) {
    return meshes.at(mesh);
}

ModelInstance* ModelInstance::clone() noexcept {
    return new ModelInstance(*this);
}

void ModelInstance::calculateBoundingBox() noexcept {
    bounding_box.center = glm::vec4{position, 1.0f} + glm::vec4{model->getBoundingBox().center, 1.0f} * model_matrix;
    bounding_box.size = glm::vec4{model->getBoundingBox().size, 1.0f} * model_matrix;
}