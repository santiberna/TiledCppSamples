#include <game/cursor.hpp>

#include <queue>
#include <unordered_set>

SelectedCursorState CalculateSelectedCursorState(const GameState& game_state, const glm::ivec2& tile)
{
    SelectedCursorState state {};
    state.selected_unit_tile = tile;

    // Flood fill to find available paths
    {
        auto unit = game_state.unit_state.units.at(tile.x, tile.y);
        uint32_t unit_move_range = GetUnitStats(unit.type).movement_range;

        std::vector<UnitPath> found_paths {};
        std::unordered_set<glm::ivec2> visited {};

        struct TraversalStep
        {
            UnitPath current_path {};
            uint32_t remaining_range {};
        };

        std::queue<TraversalStep> steps;

        TraversalStep first;
        first.current_path.tiles.emplace_back(tile);
        first.remaining_range = unit_move_range;

        steps.emplace(first);

        auto is_outside_level = [](const Level& level, const glm::ivec2& tile_pos)
        {
            auto level_size = level.map.getMapGridSize();
            return tile_pos.x < 0 || tile_pos.x >= (int)level_size.x || tile_pos.y < 0 || tile_pos.y >= (int)level_size.y;
        };

        auto is_enemy_occupied = [](const UnitMapState& all_units, const glm::ivec2& tile_pos, Unit own_unit)
        {
            auto unit = all_units.units.at(tile_pos.x, tile_pos.y);
            if (unit.health > 0)
            {
                return own_unit.team != unit.team;
            }
            return false;
        };

        auto is_obstacle = [](const Level& level, const glm::ivec2& tile_pos)
        {
            return level.tile_travel_costs.at(tile_pos.x, tile_pos.y) == 1;
        };

        while (!steps.empty())
        {
            auto next = steps.front();
            steps.pop();

            auto tail = next.current_path.tiles.back();

            if (visited.contains(tail))
            {
                continue;
            }

            visited.emplace(tail);
            auto success = state.move_tiles.emplace(tail, next.current_path);
            assert(success.second); // Ensure we don't overwrite existing paths

            glm::ivec2 dirs[4] = {
                glm::ivec2(1, 0), glm::ivec2(-1, 0), glm::ivec2(0, 1), glm::ivec2(0, -1)
            };

            for (auto dir : dirs)
            {
                auto next_pos = glm::ivec2(tail) + dir;
                auto next_cost = next.remaining_range;

                if (next_cost == 0)
                    continue;
                if (is_outside_level(game_state.current_level, next_pos))
                    continue;
                if (is_obstacle(game_state.current_level, next_pos))
                    continue;
                if (is_enemy_occupied(game_state.unit_state, next_pos, unit))
                    continue;

                TraversalStep new_step {};
                new_step.current_path = next.current_path;
                new_step.current_path.tiles.emplace_back(next_pos);
                new_step.remaining_range = next_cost - 1;

                steps.emplace(new_step);
            }
        }
    }

    return state;
}

CursorUpdateResult UpdateState(std::monostate&, GameState&, const glm::ivec2&, bool, DeltaMS)
{
    assert(false && "Should never be called");
    return CursorUpdateResult {};
}

CursorUpdateResult UpdateState(DefaultCursorState& state, GameState& game_state, const glm::ivec2& mouse_tile, bool mouse_click, DeltaMS dt)
{
    CursorUpdateResult result {};
    auto level_size = game_state.current_level.map.getMapGridSize();

    if (mouse_tile.x < 0 || mouse_tile.x >= (int)level_size.x || mouse_tile.y < 0 || mouse_tile.y >= (int)level_size.y)
    {
        return result; // Out of bounds, do nothing
    }

    if (mouse_click)
    {
        auto& unit = game_state.unit_state.units.at(mouse_tile.x, mouse_tile.y);
        auto current_team = game_state.teams.at(game_state.turn_index % game_state.teams.size());

        if (unit.health > 0 && unit.team == current_team && unit.state == UnitState::IDLE)
        {
            unit.state = UnitState::MOVING;
            result.new_state = CalculateSelectedCursorState(game_state, mouse_tile);
        }
    }

    auto tile_size = game_state.unit_state.map_tile_size;
    DrawTileRectCommand draw { mouse_tile, glm::vec2(tile_size), glm::vec4(1.0f, 1.0f, 1.0f, 0.4f) };
    result.draw_commands.emplace_back(draw);

    return result;
}

CursorUpdateResult UpdateState(SelectedCursorState& state, GameState& game_state, const glm::ivec2& mouse_tile, bool mouse_click, DeltaMS dt)
{
    CursorUpdateResult result {};

    if (mouse_click)
    {
        Unit& unit = game_state.unit_state.units.at(state.selected_unit_tile.x, state.selected_unit_tile.y);

        // If we click on a tile that is already in the move_tiles, we can confirm the move
        if (auto it = state.move_tiles.find(mouse_tile); it != state.move_tiles.end())
        {
            auto& resident_unit = game_state.unit_state.units.at(mouse_tile.x, mouse_tile.y);

            if (resident_unit.health == 0 || it->first == state.selected_unit_tile)
            {
                auto tail_end = it->second.tiles.back();

                // Last tile is to the left
                if (state.selected_unit_tile.x > tail_end.x)
                {
                    unit.facingRight = false;
                }
                // Last Tile to the right
                else if (state.selected_unit_tile.x < tail_end.x)
                {
                    unit.facingRight = true;
                }

                result.new_state = ConfirmationCursorState { 0.0f, state.selected_unit_tile, it->second };
            }
        }
        else
        {
            // Otherwise, reset to default state
            unit.state = UnitState::IDLE;
            result.new_state = DefaultCursorState {};
        }
    }

    for (auto& [tile, path] : state.move_tiles)
    {
        auto tile_size = game_state.unit_state.map_tile_size;
        DrawTileRectCommand draw { tile, glm::vec2(tile_size), glm::vec4 { 0.5f, 0.5f, 1.0f, 0.4f } };
        result.draw_commands.emplace_back(draw);
    }

    return result;
}

CursorUpdateResult UpdateState(ConfirmationCursorState& state, GameState& game_state, const glm::ivec2& mouse_tile, bool mouse_click, DeltaMS dt)
{
    CursorUpdateResult result {};
    Unit& unit = game_state.unit_state.units.at(state.selected_unit_tile.x, state.selected_unit_tile.y);
    auto tail_end = glm::ivec2(state.selected_path.tiles.back());

    constexpr glm::ivec2 DIRECTIONS[4] = {
        glm::ivec2(1, 0), glm::ivec2(-1, 0), glm::ivec2(0, 1), glm::ivec2(0, -1)
    };

    // Draw visualization
    state.interpolation += dt.count() * 0.005f;

    {
        size_t interpolate_start = size_t(state.interpolation);
        size_t interpolate_end = interpolate_start + 1;
        float t = glm::fract(state.interpolation);

        glm::vec2 position {};

        if (interpolate_start >= state.selected_path.tiles.size() - 1)
        {
            position = state.selected_path.tiles.back();
        }
        else
        {
            glm::vec2 tile = state.selected_path.tiles.at(interpolate_start);
            glm::vec2 target_tile = state.selected_path.tiles.at(interpolate_end);

            position = tile * (1.0f - t) + target_tile * t;
        }

        UnitDrawCommand command {};
        command.unit = game_state.unit_state.units.at(state.selected_unit_tile.x, state.selected_unit_tile.y);
        command.tile_size = game_state.unit_state.map_tile_size;
        command.colour = glm::vec4(1.0f, 1.0f, 1.0f, 0.6f);
        command.map_position = position;

        result.draw_commands.emplace_back(command);
    }

    // Draw target position

    auto tile_size = game_state.unit_state.map_tile_size;
    DrawTileRectCommand draw { tail_end, glm::vec2(tile_size), glm::vec4 { 0.5f, 0.5f, 1.0f, 0.4f } };
    result.draw_commands.emplace_back(draw);

    // Draw attack tiles

    for (auto dir : DIRECTIONS)
    {
        auto adjacent_tile = tail_end + dir;
        auto map_grid_size = tpp::IVec2(game_state.current_level.map.getMapGridSize());

        bool inside_level = adjacent_tile.x >= 0 && adjacent_tile.x < map_grid_size.x && adjacent_tile.y >= 0 && adjacent_tile.y < map_grid_size.y;

        if (inside_level)
        {
            auto unit_at_adjacent = game_state.unit_state.units.at(adjacent_tile.x, adjacent_tile.y);
            if (unit_at_adjacent.health > 0 && unit_at_adjacent.team != unit.team)
            {
                // Draw attack tile
                DrawTileRectCommand draw_attack { adjacent_tile, glm::vec2(tile_size), glm::vec4 { 1.0f, 0.5f, 0.5f, 0.4f } };
                result.draw_commands.emplace_back(draw_attack);
            }
        }
    }

    if (mouse_click)
    {
        auto& target_spot = game_state.unit_state.units.at(tail_end.x, tail_end.y);

        auto hasEnemyToAttack = [DIRECTIONS](
                                    UnitTeam own_team,
                                    UnitMapState& map,
                                    const glm::ivec2& mouse_tile,
                                    const glm::ivec2& tail_end) -> Unit*
        {
            auto it = std::find(std::begin(DIRECTIONS), std::end(DIRECTIONS), mouse_tile - tail_end);
            if (it != std::end(DIRECTIONS))
            {
                auto& unit_at_adjacent = map.units.at(mouse_tile.x, mouse_tile.y);
                if (unit_at_adjacent.health > 0 && unit_at_adjacent.team != own_team)
                {
                    return &unit_at_adjacent;
                }
            }
            return nullptr;
        };

        if (mouse_tile == tail_end)
        {
            unit.state = UnitState::USED;
            result.new_state = DefaultCursorState {};
            std::swap(unit, target_spot);
        }
        else if (auto* enemy = hasEnemyToAttack(unit.team, game_state.unit_state, mouse_tile, tail_end))
        {
            // Attack unit
            unit.state = UnitState::USED;
            result.new_state = DefaultCursorState {};
            std::swap(unit, target_spot);

            AttackUnit(target_spot, *enemy);
        }
        else
        {
            result.new_state = CalculateSelectedCursorState(game_state, state.selected_unit_tile);
        }
    }

    return result;
}

CursorUpdateResult UpdateCursorInput(Cursor& cursor, GameState& game_state, const glm::vec2& mouse_world_pos, bool mouse_click, DeltaMS dt)
{
    glm::ivec2 mouse_tile = glm::ivec2(mouse_world_pos / glm::vec2(game_state.unit_state.map_tile_size));

    auto result = std::visit([&](auto& state)
        { return UpdateState(state, game_state, mouse_tile, mouse_click, dt); }, cursor.state);

    if (!std::holds_alternative<std::monostate>(result.new_state))
    {
        cursor.state = result.new_state;
    }

    return result;
}

struct DrawCommandVisitor
{
    Renderer& renderer;
    const FrameCamera& camera;
    const GameAssets& assets;

    void operator()(const DrawTileRectCommand& command) const
    {
        SDL_FRect rect = camera.ToScreenRect(SDL_FRect {
            (float)command.tile_pos.x * command.tile_size.x,
            (float)command.tile_pos.y * command.tile_size.y,
            command.tile_size.x, command.tile_size.y });

        renderer.RenderFilledRect(rect, command.colour);
    };

    void operator()(const UnitDrawCommand& command) const
    {
        DrawUnit(renderer, camera, assets, command.map_position, command.tile_size, command.unit, command.colour);
    }
};

void DrawCursorInput(Renderer& renderer, const GameAssets& assets, const CursorUpdateResult& result, const FrameCamera& camera)
{
    for (const auto& command : result.draw_commands)
    {
        std::visit(DrawCommandVisitor { renderer, camera, assets }, command);
    }
}