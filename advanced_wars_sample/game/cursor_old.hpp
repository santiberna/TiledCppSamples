#pragma once
#include <game/level.hpp>
#include <engine/input.hpp>
#include <unordered_set>
#include <game/unit.hpp>
#include <stack>

struct Cursor;

inline void DrawTileRect(Renderer& renderer, const FrameCamera& camera, const glm::vec2& tile_coords, const glm::vec2& tile_size, const glm::vec4& colour)
{
    SDL_FRect select_rect {};
    select_rect.x = tile_coords.x * tile_size.x;
    select_rect.y = tile_coords.y * tile_size.y;
    select_rect.w = tile_size.x;
    select_rect.h = tile_size.y;

    renderer.RenderFilledRect(camera.ToScreenRect(select_rect), colour);
}

struct Path
{
    std::vector<glm::ivec2> tiles {};
    bool operator==(const Path& o) const { return tiles.back() == o.tiles.back(); }
};

namespace std
{

template <>
struct hash<Path>
{
    std::size_t operator()(const Path& o) const
    {
        return std::hash<glm::ivec2> {}(o.tiles.back());
    }
};

}

struct ICursorState
{
    virtual ~ICursorState() = default;
    virtual void Update(Cursor& cursor, Level& level, const glm::ivec2& tile_pos, bool mouse_click, DeltaMS dt) = 0;
    virtual void Render(Renderer& renderer, const Level& level, const FrameCamera& camera) = 0;
};

struct CursorDefaultState : public ICursorState
{
    virtual ~CursorDefaultState() = default;

    std::optional<glm::ivec2> hovered_tile {};

    void Update(Cursor& cursor, Level& level, const glm::ivec2& tile_pos, bool mouse_click, DeltaMS dt) override;
    void Render(Renderer& renderer, const Level& level, const FrameCamera& camera) override;
};

struct CursorSelectedUnitState : public ICursorState
{
    CursorSelectedUnitState(Level& level, const glm::ivec2& tile);
    virtual ~CursorSelectedUnitState() = default;

    std::unordered_set<Path> move_tiles {};
    glm::ivec2 tile_pos {};
    Unit* unit {};

    void Update(Cursor& cursor, Level& level, const glm::ivec2& tile_pos, bool mouse_click, DeltaMS dt) override;
    void Render(Renderer& renderer, const Level& level, const FrameCamera& camera) override;
};

struct CursorConfirmState : public ICursorState
{
    CursorConfirmState(Level& level, const glm::ivec2& tile, const Path& taken_path);
    virtual ~CursorConfirmState() = default;

    Path selected_path {};
    glm::ivec2 tile_pos {};
    Unit* unit {};
    float interpolation = 0.0f;

    void Update(Cursor& cursor, Level& level, const glm::ivec2& tile_pos, bool mouse_click, DeltaMS dt) override;
    void Render(Renderer& renderer, const Level& level, const FrameCamera& camera) override;
};

struct Cursor
{
    void DoCursorLogic(Level& level, Renderer& renderer, const FrameCamera& camera, DeltaMS dt, const glm::vec2& mouse_pos, InputState mouse_state)
    {
        auto level_tile_size = level.map.getMapTileSize();
        glm::ivec2 tile_pos = camera.ToWorld(mouse_pos) / glm::vec2 { level_tile_size.x, level_tile_size.y };

        if (!state_stack.empty())
        {
            state_stack.top()->Update(*this, level, tile_pos, mouse_state == InputState::PRESSED, dt);
            state_stack.top()->Render(renderer, level, camera);
        }
    }

    void PushState(std::unique_ptr<ICursorState>&& state) { state_stack.push(std::move(state)); }

    void PopState(size_t count = 1)
    {
        assert(state_stack.size() >= count);
        for (size_t i = 0; i < count; i++)
            state_stack.pop();
    }

private:
    std::stack<std::unique_ptr<ICursorState>> state_stack {};
};