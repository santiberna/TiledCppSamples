#include <game/text.hpp>

static float CalculateKerning(const Font& font, const unicode::String& text, size_t index)
{
    float out {};
    if (index == text.size() - 1)
    {
        out = 0.0f;
    }
    else
    {
        out = font.GetKerning(text[index], text[index + 1]);
    }

    return out;
}

void DrawText(
    Renderer& renderer,
    const Font& font,
    const unicode::String& text,
    const glm::vec2& position,
    const glm::vec4& colour,
    float text_scale)
{
    auto font_metrics = font.GetFontMetrics();
    std::vector<CodepointDraw> layout {};

    glm::vec2 pen_position = position;

    for (size_t i = 0; i < text.size(); ++i)
    {
        auto glyph = font.GetCodepointInfo(text[i]);
        float kerning = CalculateKerning(font, text, i);

        glm::vec2 glyph_draw_offset = glm::vec2 {
            glyph.left_bearing,
            (glyph.offset.y + font_metrics.ascent)
        } * text_scale;

        CodepointDraw codepoint_draw {};
        codepoint_draw.atlas_index = glyph.atlas_index;
        codepoint_draw.offset = pen_position + glyph_draw_offset;

        layout.emplace_back(codepoint_draw);
        pen_position.x += (glyph.advance + kerning) * text_scale;
    }

    for (auto& codepoint : layout)
    {
        auto src_rect = font.GetAtlas().GetSpriteFRect(codepoint.atlas_index).value();

        SDL_FRect dst_rect {};
        dst_rect.x = codepoint.offset.x;
        dst_rect.y = codepoint.offset.y;
        dst_rect.h = src_rect.h * text_scale;
        dst_rect.w = src_rect.w * text_scale;

        renderer.RenderTextureRect(
            font.GetAtlas().GetTexture(),
            dst_rect, &src_rect, colour);

        if (renderer.IsDebugRendering())
        {
            renderer.RenderRect(dst_rect, colour);
        }
    }
}