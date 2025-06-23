#pragma once

#include <resources/texture.hpp>
#include <tiledcpp/tiledcpp.hpp>
#include <unordered_map>
#include <utility/time.hpp>

struct AnimationState
{
    uint32_t current_frame = 0;
    DeltaMS accum {};
};

struct TileSetDrawData
{
    std::unordered_map<uint32_t, AnimationState> animation_states {};
    Texture spritesheet_texture {};
};

TileSetDrawData CreateTileSetDrawData(Renderer& renderer, tpp::TileSet& tileset);
SDL_FRect GetTileRect(const tpp::TileSet& tileset, const TileSetDrawData& draw_data, uint32_t tile_id);
void UpdateAnimationData(const tpp::TileSet& tileset, TileSetDrawData& tile_set_data, DeltaMS delta);