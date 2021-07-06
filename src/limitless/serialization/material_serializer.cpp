#include <limitless/serialization/material_serializer.hpp>

#include <limitless/assets.hpp>
#include <limitless/ms/material_builder.hpp>
#include <limitless/serialization/uniform_serializer.hpp>
#include <limitless/serialization/asset_deserializer.hpp>
#include <limitless/util/bytebuffer.hpp>
#include <iostream>

using namespace Limitless::ms;
using namespace Limitless;

void MaterialSerializer::deserialize(ByteBuffer& buffer, Assets& assets, MaterialBuilder& builder) {
    std::map<Property, std::unique_ptr<Uniform>> properties;
    std::map<std::string, std::unique_ptr<Uniform>> uniforms;
    Blending blending{};
    Shading shading{};
    bool two_sided {};
    std::string name;
    std::string vertex_code;
    std::string fragment_code;
    std::string global_code;
    std::string tes_code;

    buffer >> name
           >> shading
           >> blending
           >> two_sided
           >> AssetDeserializer<decltype(properties)>{assets, properties}
           >> AssetDeserializer<decltype(uniforms)>{assets, uniforms}
           >> vertex_code
           >> fragment_code
           >> global_code
           >> tes_code;

    builder .setName(name)
            .setShading(shading)
            .setBlending(blending)
            .setTwoSided(two_sided)
            .set(std::move(properties))
            .set(std::move(uniforms))
            .setVertexSnippet(vertex_code)
            .setFragmentSnippet(fragment_code)
            .setGlobalSnippet(global_code)
            .setTessellationSnippet(tes_code);
}

ByteBuffer MaterialSerializer::serialize(const Material& material) {
    ByteBuffer buffer;

    buffer << VERSION;

    buffer << material.getName()
           << material.getShading()
           << material.getBlending()
           << material.getTwoSided()
           << material.getProperties()
           << material.getUniforms()
           << material.getVertexSnippet()
           << material.getFragmentSnippet()
           << material.getGlobalSnippet()
           << material.getTessellationSnippet()
           << material.getModelShaders();

    return buffer;
}

std::shared_ptr<Material> MaterialSerializer::deserialize(Assets& assets, ByteBuffer& buffer) {
    uint8_t version {};

    buffer >> version;

    if (version != VERSION) {
        throw std::runtime_error("Wrong material serializer version! " + std::to_string(VERSION) + " vs " + std::to_string(version));
    }

    MaterialBuilder builder {assets};

    deserialize(buffer, assets, builder);

    ModelShaders compile_models;
    buffer >> compile_models;

    builder.setModelShaders(compile_models);

    try {
        return builder.build();
    } catch (const resource_container_error& ex) {
        return assets.materials.at(builder.getName());
    }
}

ByteBuffer& Limitless::operator<<(ByteBuffer& buffer, const Material& material) {
    MaterialSerializer serializer;
    buffer << serializer.serialize(material);
    return buffer;
}

ByteBuffer& Limitless::operator>>(ByteBuffer& buffer, const AssetDeserializer<std::shared_ptr<Material>>& asset) {
    MaterialSerializer serializer;
    auto& [assets, material] = asset;
    material = serializer.deserialize(assets, buffer);
    return buffer;
}
