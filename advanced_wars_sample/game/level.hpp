#pragma once
#include <engine/renderer.hpp>
#include <resources/texture.hpp>
#include <tiledcpp/tiledcpp.hpp>
#include <utility/time.hpp>
#include <math/camera.hpp>

struct AnimationState
{
    uint32_t current_frame = 0;
    DeltaMS accum {};
};

struct Unit;

struct TileSetData
{
    TileSetData() = default;
    TileSetData(Renderer& renderer, const tpp::TileSet& tileset);

    void UpdateAnimations(const tpp::TileSet& tileset, DeltaMS delta);
    SDL_FRect GetTileRect(const tpp::TileSet& tileset, uint32_t tile_id);

    std::unordered_map<uint32_t, AnimationState> animation_states {};
    Texture texture {};
};

struct Level
{
    Level(Renderer& renderer, const std::string& source_tmx);
    void RenderMap(Renderer& renderer, const FrameCamera& camera, DeltaMS delta);
    std::optional<glm::uvec2> GetTileFromPos(const glm::ivec2& pos) const;

    void AddUnit(const glm::uvec2& coords, const Unit& unit);
    Unit* GetUnit(const glm::uvec2& coords) const;
    void RemoveUnit(const glm::uvec2& coords);
    void MoveUnit(const glm::uvec2& from, const glm::uvec2& to);

    tpp::TileMap map {};
    std::vector<TileSetData> tilesets {};
    std::vector<std::unique_ptr<Unit>> units {};
};
