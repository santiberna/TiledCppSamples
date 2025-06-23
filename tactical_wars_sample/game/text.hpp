#include <resources/font.hpp>

void DrawText(
    Renderer& renderer,
    const Font& font,
    const unicode::String& text,
    const glm::vec2& position,
    const glm::vec4& colour,
    float text_scale);
