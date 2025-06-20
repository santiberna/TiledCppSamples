#pragma once

#include <game/level.hpp>
#include <resources/font.hpp>
#include <game/text.hpp>

enum UnitTeam
{
    RED,
    BLUE
};

struct UnitSpriteSheet
{
    UnitSpriteSheet(Renderer& renderer, const std::string tsx_file, std::shared_ptr<Font> font)
    {
        health_font = font;
        tileset = tpp::TileSet::fromTSX(tsx_file).value();
        tile_data = TileSetData(renderer, tileset);

        if (auto* props = tileset.getProperties())
        {
            idle_anim = props->get<int>("IdleAnimation").value_or(0);
            move_anim = props->get<int>("MoveAnimation").value_or(0);
        }
    }

    std::shared_ptr<Font> health_font {};
    tpp::TileSet tileset {};
    TileSetData tile_data;
    int idle_anim = 0;
    int move_anim = 0;
};

struct Unit
{
    std::shared_ptr<UnitSpriteSheet> unit_spritesheet {};

    UnitTeam team {};
    int32_t health = 100;
    uint32_t movement_range = 3;
    bool is_moving {};
    bool facingRight = true;

    void RenderUnit(Renderer& renderer, const Level& level, const glm::vec2& map_position, const FrameCamera& camera, bool ghost_effect = false)
    {
        SDL_FRect src {};

        if (!is_moving)
            src = unit_spritesheet->tile_data.GetTileRect(unit_spritesheet->tileset, unit_spritesheet->idle_anim);
        else
            src = unit_spritesheet->tile_data.GetTileRect(unit_spritesheet->tileset, unit_spritesheet->move_anim);

        glm::vec4 colour = glm::vec4(1.0f);
        if (ghost_effect)
            colour.w = 0.5f;

        auto map_tile_size = level.map.getMapTileSize();

        auto offset = tpp::IVec2(unit_spritesheet->tileset.getTileSize()) - tpp::IVec2(level.map.getMapTileSize());

        SDL_FRect dest {};
        dest.x = map_tile_size.x * map_position.x - (float)offset.x / 2;
        dest.y = map_tile_size.y * map_position.y - (float)offset.y / 2;
        dest.h = src.h;
        dest.w = src.w;

        renderer.RenderTextureRect(unit_spritesheet->tile_data.texture, camera.ToScreenRect(dest), &src, colour, facingRight);

        // Health text
        auto health_display = (health + 10 - 1) / 10;

        if (!ghost_effect)
        {
            WorldText text {};
            text.text = unicode::FromUTF8(std::to_string(health_display));
            text.font = unit_spritesheet->health_font;

            SDL_FRect text_rect = {
                map_tile_size.x * map_position.x + map_tile_size.x * 0.75f,
                map_tile_size.y * map_position.y + map_tile_size.y * 0.75f,
                (float)map_tile_size.x,
                (float)map_tile_size.y
            };

            text.rect = text_rect;

            auto screen_rect = camera.ToScreenRect(text_rect);
            auto zoom = screen_rect.w / (float)map_tile_size.x;

            text.Draw(renderer, zoom / unit_spritesheet->health_font->GetFontMetrics().resolution * 6.0f, camera);
        }
    }
};
