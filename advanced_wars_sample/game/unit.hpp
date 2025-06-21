#pragma once
#include <game/level.hpp>
#include <game/tileset_data.hpp>
#include <math/camera.hpp>
#include <resources/font.hpp>

enum class UnitTeam : uint8_t
{
    RED,
    BLUE
};

enum class UnitType : uint8_t
{
    SOLDIER
};

enum class UnitState : uint8_t
{
    IDLE,
    MOVING,
    USED
};

struct TeamAssets
{
    tpp::TileSet tileset {};
    TileSetDrawData draw_data {};

    uint32_t idle_anim_index = 0;
    uint32_t move_anim_index = 0;
    uint32_t exhausted_anim = 1;
};

struct UnitStats
{
    uint32_t movement_range = 3;
};

struct Unit
{
    UnitTeam team = UnitTeam::RED;
    UnitType type = UnitType::SOLDIER;
    UnitState state = UnitState::IDLE;
    bool facingRight = true;
    int8_t health = 0;
};

struct UnitMapState
{
    glm::uvec2 map_tile_size {};
    tpp::Array2D<Unit> units;
};

struct GameAssets
{
    std::unordered_map<UnitTeam, TeamAssets> team_assets;
    std::shared_ptr<Font> text_font;
};

UnitMapState SetupUnitMapState(const Level& level);
TeamAssets LoadUnitTeamAssets(Renderer& renderer, const std::string& tsx_file);
void DrawMapUnits(Renderer& renderer, GameAssets& assets, const UnitMapState& unit_map, const FrameCamera& camera, DeltaMS delta);