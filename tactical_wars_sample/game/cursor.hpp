#pragma once

#include <game/level.hpp>
#include <game/unit.hpp>
#include <math/types.hpp>
#include <unordered_map>
#include <variant>
#include <vector>

struct GameState
{
    Level current_level;
    UnitMapState unit_state {};

    std::vector<UnitTeam> teams {};
    uint32_t turn_index = -1;
};

struct UnitPath
{
    std::vector<glm::uvec2> tiles;
};

struct DefaultCursorState
{
};

struct SelectedCursorState
{
    glm::uvec2 selected_unit_tile {};
    std::unordered_map<glm::uvec2, UnitPath> move_tiles;
};

struct ConfirmationCursorState
{
    float interpolation {};
    glm::uvec2 selected_unit_tile {};
    UnitPath selected_path {};
};

using CursorStateVariant = std::variant<std::monostate, DefaultCursorState, SelectedCursorState, ConfirmationCursorState>;

struct UnitDrawCommand
{
    glm::vec2 map_position {};
    glm::vec2 tile_size {};
    Unit unit {};
    glm::vec4 colour {};
};

struct DrawTileRectCommand
{
    glm::uvec2 tile_pos;
    glm::vec2 tile_size;
    glm::vec4 colour;
};

using CursorDrawTileCommand = std::variant<DrawTileRectCommand, UnitDrawCommand>;

struct CursorUpdateResult
{
    CursorStateVariant new_state;
    std::vector<CursorDrawTileCommand> draw_commands {};
};

struct Cursor
{
    CursorStateVariant state = DefaultCursorState {};
};

CursorUpdateResult UpdateCursorInput(Cursor& cursor, GameState& game_state, const glm::vec2& mouse_pos, bool mouse_click, DeltaMS dt);
void DrawCursorInput(Renderer& renderer, const GameAssets& assets, const CursorUpdateResult& result, const FrameCamera& camera);