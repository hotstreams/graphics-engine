#pragma once

#include <memory>

namespace LimitlessEngine {
    class Material;

    class MaterialLoader {
    public:
        static std::shared_ptr<Material> load(const std::string& asset_name);
        static void save(const std::string& asset_name);
    };
}