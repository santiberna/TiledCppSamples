#include <game/cursor.hpp>
#include <queue>

void CursorDefaultState::Update(Cursor& cursor, Level& level, const glm::ivec2& tile_pos, bool mouse_click, DeltaMS dt)
{
    auto level_size = level.map.getMapGridSize();

    if (tile_pos.x < 0 || tile_pos.x >= (int)level_size.x || tile_pos.y < 0 || tile_pos.y >= (int)level_size.y)
    {
        hovered_tile = std::nullopt;
        return;
    }

    hovered_tile = tile_pos;

    if (mouse_click)
    {
        if (auto unit = level.GetUnit(hovered_tile.value()))
        {
            unit->is_moving = true;
            cursor.PushState(std::make_unique<CursorSelectedUnitState>(level, tile_pos));
        }
    }
}

void CursorDefaultState::Render(Renderer& renderer, const Level& level, const FrameCamera& camera)
{
    if (hovered_tile)
    {
        auto map_tile_size = level.map.getMapTileSize();
        DrawTileRect(renderer, camera,
            { hovered_tile->x, hovered_tile->y },
            { map_tile_size.x, map_tile_size.y },
            { 1.0f, 1.0f, 1.0f, 0.4f });
    }
}

std::vector<Path> FloodFillPathfind(const Level& level, const glm::ivec2& unit_pos, const Unit* unit)
{
    int32_t unit_move_range = unit->movement_range;
    std::vector<Path> paths {};
    std::unordered_set<glm::ivec2> visited {};

    struct TraversalStep
    {
        Path current_path {};
        int32_t remaining_range {};
    };

    std::queue<TraversalStep> steps;

    TraversalStep first;
    first.current_path.tiles.emplace_back(unit_pos);
    first.remaining_range = unit_move_range;

    steps.emplace(first);

    auto is_outside_level = [](const Level& level, const glm::ivec2& tile_pos)
    {
        auto level_size = level.map.getMapGridSize();
        return tile_pos.x < 0 || tile_pos.x >= (int)level_size.x || tile_pos.y < 0 || tile_pos.y >= (int)level_size.y;
    };

    auto is_enemy_occupied = [](const Level& level, const glm::ivec2& tile_pos, const Unit* own_unit)
    {
        if (auto unit = level.GetUnit(tile_pos))
        {
            return own_unit->team != unit->team;
        }
        return false;
    };

    auto is_walkable = [](const Level& level, const glm::ivec2& tile_pos)
    {
        auto& terrain = level.map.getTileLayers().front();
        auto tile = terrain.tile_ids.at(tile_pos.x, tile_pos.y);
        auto& tileset = level.map.getTileSets().at(tile.getTileset());

        if (auto* tile_props = tileset.getTileProperties(tile.getId()))
        {
            auto obstacle = tile_props->get<bool>("Obstacle").value_or(false);
            return obstacle;
        }
        return false;
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
        paths.emplace_back(next.current_path);

        glm::ivec2 dirs[4] = {
            glm::ivec2(1, 0), glm::ivec2(-1, 0), glm::ivec2(0, 1), glm::ivec2(0, -1)
        };

        for (auto dir : dirs)
        {
            auto next_pos = tail + dir;
            auto next_cost = next.remaining_range;

            if (next_cost == 0)
                continue;
            if (is_outside_level(level, next_pos))
                continue;
            if (is_walkable(level, next_pos))
                continue;
            if (is_enemy_occupied(level, next_pos, unit))
                continue;

            TraversalStep new_step {};
            new_step.current_path = next.current_path;
            new_step.current_path.tiles.emplace_back(next_pos);
            new_step.remaining_range = next_cost - 1;

            steps.emplace(new_step);
        }
    }

    return paths;
}

CursorSelectedUnitState::CursorSelectedUnitState(Level& level, const glm::ivec2& tile)
    : tile_pos(tile)
    , unit(level.GetUnit(tile))
{
    assert(unit != nullptr);
    auto paths_found = FloodFillPathfind(level, tile, unit);
    move_tiles = { paths_found.begin(), paths_found.end() };
}

void CursorSelectedUnitState::Update(Cursor& cursor, Level& level, const glm::ivec2& tile, bool mouse_click, DeltaMS dt)
{
    if (mouse_click)
    {
        Path test_path;
        test_path.tiles.emplace_back(tile);

        if (auto it = move_tiles.find(test_path); it != move_tiles.end())
        {
            if (level.GetUnit(tile) == nullptr)
            {
                cursor.PushState(std::make_unique<CursorConfirmState>(level, tile_pos, *it));
            }
            else
            {
                unit->is_moving = false;
                cursor.PopState();
            }
        }
        else
        {
            unit->is_moving = false;
            cursor.PopState();
        }
    }
}

void CursorSelectedUnitState::Render(Renderer& renderer, const Level& level, const FrameCamera& camera)
{
    auto map_tile_size = level.map.getMapTileSize();

    for (auto tile : move_tiles)
    {
        DrawTileRect(renderer, camera,
            tile.tiles.back(),
            { map_tile_size.x, map_tile_size.y },
            { 0.5f, 0.5f, 1.0f, 0.4f });
    }
}

CursorConfirmState::CursorConfirmState(Level& level, const glm::ivec2& tile, const Path& taken_path)
    : selected_path(taken_path)
    , tile_pos(tile)
    , unit(level.GetUnit(tile))
{
    // Last tile is to the left
    if (tile_pos.x > selected_path.tiles.back().x)
    {
        unit->facingRight = false;
    }
    // Last Tile to the right
    else if (tile_pos.x < selected_path.tiles.back().x)
    {
        unit->facingRight = true;
    }
}

void CursorConfirmState::Update(Cursor& cursor, Level& level, const glm::ivec2& tile, bool mouse_click, DeltaMS dt)
{
    interpolation += dt.count() * 0.005f;

    if (mouse_click)
    {
        if (tile == selected_path.tiles.back())
        {
            level.MoveUnit(tile_pos, tile);

            unit->is_moving = false;
            cursor.PopState(2);
        }
        else
        {
            cursor.PopState();
        }
    }
}

void CursorConfirmState::Render(Renderer& renderer, const Level& level, const FrameCamera& camera)
{
    { // Draw ghost unit
        size_t interpolate_start = size_t(interpolation);
        size_t interpolate_end = interpolate_start + 1;
        float t = glm::fract(interpolation);

        if (interpolate_start >= selected_path.tiles.size() - 1)
        {
            unit->RenderUnit(renderer, level, selected_path.tiles.back(), camera, true);
        }
        else
        {
            glm::vec2 tile = selected_path.tiles.at(interpolate_start);
            glm::vec2 target_tile = selected_path.tiles.at(interpolate_end);

            unit->RenderUnit(renderer, level,
                tile * (1.0f - t) + target_tile * t,
                camera, true);
        }
    }

    auto map_tile_size = level.map.getMapTileSize();

    DrawTileRect(renderer, camera,
        selected_path.tiles.back(),
        { map_tile_size.x, map_tile_size.y },
        { 0.5f, 0.5f, 1.0f, 0.4f });
}