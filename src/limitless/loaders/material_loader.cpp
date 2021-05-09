#include <limitless/loaders/material_loader.hpp>

#include <ostream>
#include <fstream>

#include <limitless/util/bytebuffer.hpp>
#include <limitless/ms/material.hpp>
#include <limitless/serialization/material_serializer.hpp>
#include <limitless/loaders/asset_loader.hpp>

using namespace Limitless;
using namespace Limitless::ms;

std::shared_ptr<ms::Material> MaterialLoader::load(Context& context, Assets& assets, const RenderSettings& settings, const fs::path& _path) {
    auto path = convertPathSeparators(_path);
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    auto filesize = stream.tellg();
    ByteBuffer buffer{static_cast<size_t>(filesize)};

    stream.seekg(0, std::ios::beg);
    stream.read(buffer.cdata(), buffer.size());

    std::shared_ptr<ms::Material> material;
    buffer >> AssetDeserializer<std::shared_ptr<ms::Material>>{context, assets, settings, material};
    return material;
}

void MaterialLoader::save(const fs::path& _path, const std::shared_ptr<ms::Material>& asset) {
    auto path = convertPathSeparators(_path);
    std::ofstream stream(path, std::ios::binary);

    ByteBuffer buffer;
    buffer << *asset;

    stream.write(buffer.cdata(), buffer.size());
}