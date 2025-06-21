#include <game/unit.hpp>

UnitMapState SetupUnitMapState(const Level& level)
{
    UnitMapState unit_map {};
    unit_map.map_tile_size = { level.map.getMapTileSize().x, level.map.getMapTileSize().y };
    unit_map.units = tpp::Array2D<Unit>(level.map.getMapGridSize().x, level.map.getMapGridSize().y, Unit {});
    return unit_map;
}

TeamAssets LoadUnitTeamAssets(Renderer& renderer, const std::string& tsx_file)
{
    TeamAssets assets {};
    assets.tileset = tpp::TileSet::fromTSX(tsx_file).value();
    assets.draw_data = CreateTileSetDrawData(renderer, assets.tileset);

    if (auto* props = assets.tileset.getProperties())
    {
        assets.idle_anim_index = props->get<int>("IdleAnimation").value_or(0);
        assets.move_anim_index = props->get<int>("MoveAnimation").value_or(0);
    }

    return assets;
}

void DrawMapUnits(Renderer& renderer, GameAssets& assets, const UnitMapState& unit_map, const FrameCamera& camera, DeltaMS delta)
{
    for (auto& [unit, sheet] : assets.team_assets)
    {
        UpdateAnimationData(sheet.tileset, sheet.draw_data, delta);
    }

    for (auto it = unit_map.units.begin(); it != unit_map.units.end(); ++it)
    {
        auto unit = *it;

        if (unit.health <= 0)
        {
            continue;
        }

        auto& unit_assets = assets.team_assets[unit.team];
        auto& texture = unit_assets.draw_data.spritesheet_texture;

        glm::vec2 map_position = { it.getIndices().x, it.getIndices().y };
        glm::vec2 sprite_size = { unit_assets.tileset.getTileSize().x, unit_assets.tileset.getTileSize().y };
        SDL_FRect src {};

        if (unit.state == UnitState::IDLE)
        {
            src = GetTileRect(unit_assets.tileset, unit_assets.draw_data, unit_assets.idle_anim_index);
        }
        else if (unit.state == UnitState::MOVING)
        {
            src = GetTileRect(unit_assets.tileset, unit_assets.draw_data, unit_assets.move_anim_index);
        }
        else
        {
            src = GetTileRect(unit_assets.tileset, unit_assets.draw_data, unit_assets.exhausted_anim);
        }

        glm::vec2 offset = (sprite_size - glm::vec2(unit_map.map_tile_size)) * 0.5f;

        SDL_FRect dest {};
        dest.x = unit_map.map_tile_size.x * map_position.x - offset.x;
        dest.y = unit_map.map_tile_size.y * map_position.y - offset.y;
        dest.h = src.h;
        dest.w = src.w;

        renderer.RenderTextureRect(texture, camera.ToScreenRect(dest), &src, glm::vec4(1.0f), unit.facingRight);

        //         // Health text
        //         auto health_display = (health + 10 - 1) / 10;

        //         if (!ghost_effect)
        //         {
        //             WorldText text {};
        //             text.text = unicode::FromUTF8(std::to_string(health_display));
        //             text.font = unit_spritesheet->health_font;

        //             SDL_FRect text_rect = {
        //                 map_tile_size.x * map_position.x + map_tile_size.x * 0.75f,
        //                 map_tile_size.y * map_position.y + map_tile_size.y * 0.75f,
        //                 (float)map_tile_size.x,
        //                 (float)map_tile_size.y
        //             };

        //             text.rect = text_rect;

        //             auto screen_rect = camera.ToScreenRect(text_rect);
        //             auto zoom = screen_rect.w / (float)map_tile_size.x;

        //             text.Draw(renderer, zoom / unit_spritesheet->health_font->GetFontMetrics().resolution * 6.0f, camera);
        //         }
        //     }
    }
}