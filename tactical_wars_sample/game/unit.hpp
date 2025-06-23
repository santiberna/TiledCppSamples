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
    float scale = 0.75f;
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
    std::shared_ptr<Texture> button_texture;
    std::shared_ptr<Texture> round_background;
};

void AttackUnit(Unit& attacker, Unit& defender);
const UnitStats& GetUnitStats(UnitType type);
UnitMapState SetupUnitMapState(const Level& level);
TeamAssets LoadUnitTeamAssets(Renderer& renderer, const std::string& tsx_file);
void DrawMapUnits(Renderer& renderer, GameAssets& assets, const UnitMapState& unit_map, const FrameCamera& camera, DeltaMS delta);

void DrawUnit(
    Renderer& renderer,
    const FrameCamera& camera,
    const GameAssets& assets,
    const glm::vec2& map_position,
    const glm::vec2& tile_size,
    Unit unit,
    const glm::vec4& colour);
