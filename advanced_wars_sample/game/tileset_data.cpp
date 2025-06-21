#include <game/tileset_data.hpp>

TileSetDrawData CreateTileSetDrawData(Renderer& renderer, tpp::TileSet& tileset)
{
    TileSetDrawData draw_data {};

    auto& image = tileset.getImage();
    auto image_size = glm::uvec2 { image.getSize().x, image.getSize().y };

    draw_data.spritesheet_texture = Texture::FromData(renderer, image.getData(), image_size).value();
    image.freeData();

    for (uint32_t i = 0; i < tileset.getTileCount(); ++i)
    {
        if (auto* anim = tileset.getTileAnimation(i))
        {
            draw_data.animation_states[i] = AnimationState {};
        }
    }

    return draw_data;
}

void UpdateAnimationData(const tpp::TileSet& tileset, TileSetDrawData& tile_set_data, DeltaMS delta)
{
    for (auto& [tile_id, anim_state] : tile_set_data.animation_states)
    {
        auto* anim = tileset.getTileAnimation(tile_id);

        if (anim->frames.empty())
        {
            continue;
        }

        anim_state.accum += delta;
        auto current_delay = DeltaMS(anim->frames.at(anim_state.current_frame).duration_ms);

        while (anim_state.accum > current_delay)
        {
            anim_state.accum -= current_delay;
            anim_state.current_frame = (anim_state.current_frame + 1) % anim->frames.size();
            current_delay = DeltaMS(anim->frames.at(anim_state.current_frame).duration_ms);
        }
    }
}

SDL_FRect GetTileRect(const tpp::TileSet& tileset, const TileSetDrawData& draw_data, uint32_t tile_id)
{
    if (auto it = draw_data.animation_states.find(tile_id); it != draw_data.animation_states.end())
    {
        auto frame = it->second.current_frame;
        auto* anim = tileset.getTileAnimation(tile_id);

        tile_id = anim->frames.at(frame).tile_id;
    }

    auto rect = tileset.getTileRect(tile_id).value();

    SDL_FRect src_rect {
        (float)rect.start.x, (float)rect.start.y, (float)rect.size.x, (float)rect.size.y
    };

    return src_rect;
}