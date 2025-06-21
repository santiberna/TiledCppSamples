#pragma once
#include <game/tileset_data.hpp>
#include <math/camera.hpp>

struct Level
{
    tpp::TileMap map {};
    std::vector<TileSetDrawData> tile_set_data {};
    tpp::Array2D<uint8_t> tile_travel_costs {};
};

Level LoadLevel(Renderer& renderer, const std::string& map_path);
void DrawLevel(Renderer& renderer, Level& level, const FrameCamera& camera, DeltaMS delta);