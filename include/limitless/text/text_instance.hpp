#pragma once

#include <limitless/models/text_model.hpp>
#include <string>
#include <optional>

namespace Limitless {
    class FontAtlas;
    class Context;
    class Assets;

    class TextInstance {
    private:
        bool hidden {false};
        TextModel text_model;
        std::string text;
        glm::vec2 position;
        glm::vec4 color {};
        glm::vec2 size {1.0f};
        std::shared_ptr<FontAtlas> font;

        std::optional<TextModel> selection_model {};
        glm::vec4 selection_color {};

        glm::mat4 model_matrix {1.0f};
        void calculateModelMatrix() noexcept;
    public:
        TextInstance(std::string text, const glm::vec2& position, std::shared_ptr<FontAtlas> font);
        TextInstance(size_t count, const glm::vec2& position, std::shared_ptr<FontAtlas> font);

        void hide() noexcept { hidden = true; }
        void reveal() noexcept { hidden = false; }

        TextInstance& setText(std::string text);
        TextInstance& setColor(const glm::vec4& color) noexcept;
        TextInstance& setPosition(const glm::vec2& position) noexcept;
        TextInstance& setSize(const glm::vec2& size) noexcept;
        TextInstance& setSelectionColor(const glm::vec4& color) noexcept;

        TextInstance& setSelection(size_t begin, size_t end);
        TextInstance& removeSelection() noexcept;

        [[nodiscard]] const auto& getColor() const noexcept { return color; }
        [[nodiscard]] const auto& getPosition() const noexcept { return position; }
        [[nodiscard]] auto isHidden() const noexcept { return hidden; }
        [[nodiscard]] auto isVisible() const noexcept { return !hidden; }

        void draw(Context& context, const Assets& assets);
    };
}
