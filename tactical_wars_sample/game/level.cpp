#include <game/level.hpp>

Level LoadLevel(Renderer& renderer, const std::string& map_path)
{
    Level level {};
    level.map = tpp::TileMap::fromTMX(map_path).value();

    for (auto& tileset : level.map.getTileSets())
    {
        level.tile_set_data.emplace_back(CreateTileSetDrawData(renderer, tileset));
    }

    auto map_size = level.map.getMapGridSize();
    level.tile_travel_costs = tpp::Array2D<uint8_t>(map_size.x, map_size.y, 0);

    auto* layer = level.map.findTileLayer("Terrain");
    assert(layer);

    for (auto it = layer->tile_ids.begin(); it != layer->tile_ids.end(); ++it)
    {
        auto& tile = *it;
        auto& set = level.map.getTileSets().at(tile.getTileset());

        if (auto* props = set.getTileProperties(tile.getId()))
        {
            if (auto travel_cost = props->get<bool>("Obstacle"))
            {
                level.tile_travel_costs.at(it.getIndices()) = travel_cost.value() ? 1 : 0;
            }
        }
    }

    return level;
}

void DrawLevel(Renderer& renderer, Level& level, const FrameCamera& camera, DeltaMS delta)
{
    for (uint32_t i = 0; i < level.tile_set_data.size(); i++)
    {
        UpdateAnimationData(level.map.getTileSets().at(i), level.tile_set_data.at(i), delta);
    }

    for (auto& layer : level.map.getTileLayers())
    {
        auto map_tile_size = level.map.getMapTileSize();

        for (auto it = layer.tile_ids.begin(); it != layer.tile_ids.end(); ++it)
        {
            auto tile_id = *it;

            if (!tile_id.isValid())
            {
                continue;
            }

            auto coords = it.getIndices();

            auto src_rect = GetTileRect(
                level.map.getTileSets().at(tile_id.getTileset()),
                level.tile_set_data.at(tile_id.getTileset()),
                tile_id.getId());

            auto& texture = level.tile_set_data.at(tile_id.getTileset()).spritesheet_texture;

            SDL_FRect dst_rect {
                (float)(coords.x * map_tile_size.x),
                (float)(coords.y * map_tile_size.y),
                (float)(map_tile_size.x),
                (float)(map_tile_size.y),
            };

            renderer.RenderTextureRect(texture, camera.ToScreenRect(dst_rect), &src_rect);
        }
    }
}