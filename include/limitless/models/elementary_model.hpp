#pragma once

#include <limitless/models/abstract_model.hpp>

namespace Limitless {
    class ElementaryModel : public AbstractModel {
    protected:
        ElementaryModel(std::string name);
    public:
        ~ElementaryModel() override = default;

        ElementaryModel(const ElementaryModel&) = delete;
        ElementaryModel& operator=(const ElementaryModel&) = delete;

        ElementaryModel(ElementaryModel&&) noexcept = default;
        ElementaryModel& operator=(ElementaryModel&&) noexcept = default;

        [[nodiscard]] const auto& getMesh() const { return meshes.at(0); }
    };
}