#include <resources/font.hpp>
#include <math/camera.hpp>
#include <utility/colours.hpp>

struct WorldText
{
    SDL_FRect rect {};
    std::shared_ptr<Font> font {};
    unicode::String text {};
    glm::vec4 colour = colour::WHITE;

    void Draw(Renderer& renderer, float text_scale, const FrameCamera& camera)
    {
        auto screen_rect = camera.ToScreenRect(rect);

        CodepointDraw::LayoutParams params {};
        params.scale = 1.0f;
        params.rect = { screen_rect.w, screen_rect.h };

        if (renderer.IsDebugRendering())
        {
            renderer.RenderRect(screen_rect, colour);
        }

        auto layout = CodepointDraw::LayoutText(*font, text, params);

        for (auto& codepoint : layout)
        {
            auto src_rect = font->GetAtlas().GetSpriteFRect(codepoint.atlas_index).value();
            auto draw_off = glm::vec2 { screen_rect.x, screen_rect.y } + codepoint.offset;

            SDL_FRect dst_rect {};
            dst_rect.x = draw_off.x;
            dst_rect.y = draw_off.y;
            dst_rect.h = src_rect.h * text_scale;
            dst_rect.w = src_rect.w * text_scale;

            renderer.RenderTextureRect(
                font->GetAtlas().GetTexture(),
                dst_rect, &src_rect, colour);

            if (renderer.IsDebugRendering())
            {
                glm::vec2 center = glm::vec2(dst_rect.x, dst_rect.y) + glm::vec2(dst_rect.w, dst_rect.h) * 0.5f;
                renderer.RenderRect(center, { dst_rect.w, dst_rect.h }, colour);
            }
        }
    }
};