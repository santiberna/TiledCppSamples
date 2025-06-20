#include <game/level.hpp>
#include <game/unit.hpp>

TileSetData::TileSetData(Renderer& renderer, const tpp::TileSet& tileset)
{
    auto& image = tileset.getImage();
    texture = Texture::FromData(renderer, image.getData(), { image.getSize().x, image.getSize().y }).value();

    for (uint32_t i = 0; i < tileset.getTileCount(); i++)
    {
        if (auto* meta = tileset.getTileAnimation(i))
        {
            animation_states.emplace(i, AnimationState {});
        }
    }
}

void TileSetData::UpdateAnimations(const tpp::TileSet& tileset, DeltaMS delta)
{
    for (auto&& [index, state] : animation_states)
    {
        auto anim = tileset.getTileAnimation(index);
        assert(anim);

        if (anim->frames.empty())
            return;

        state.accum += delta;

        auto current_delay = DeltaMS(anim->frames.at(state.current_frame).duration_ms);

        while (state.accum > current_delay)
        {
            state.accum -= DeltaMS(current_delay);

            state.current_frame = (state.current_frame + 1) % anim->frames.size();
            current_delay = DeltaMS(anim->frames.at(state.current_frame).duration_ms);
        }
    }
}

SDL_FRect TileSetData::GetTileRect(const tpp::TileSet& tileset, uint32_t tile_id)
{
    if (auto it = animation_states.find(tile_id); it != animation_states.end())
    {
        auto frame = it->second.current_frame;
        auto* anim = tileset.getTileAnimation(tile_id);

        tile_id = anim->frames.at(frame).tile_id;
    }

    auto rect = tileset.getTileRect(tile_id).value();

    SDL_FRect src_rect {
        (float)rect.start.x, (float)rect.start.y, (float)rect.size.x, (float)rect.size.y
    };

    return src_rect;
}

Level::Level(Renderer& renderer, const std::string& source_tmx)
{
    map = tpp::TileMap::fromTMX(source_tmx).value();

    for (auto& tileset : map.getTileSets())
    {
        tilesets.emplace_back(TileSetData(renderer, tileset));
    }

    units.resize(map.getMapGridSize().x * map.getMapGridSize().y);
}

void Level::RenderMap(Renderer& renderer, const FrameCamera& camera, DeltaMS delta)
{
    for (uint32_t i = 0; i < tilesets.size(); i++)
    {
        tilesets.at(i).UpdateAnimations(map.getTileSets().at(i), delta);
    }

    for (auto& layer : map.getTileLayers())
    {
        auto map_tile_size = map.getMapTileSize();

        for (auto it = layer.tile_ids.begin(); it != layer.tile_ids.end(); ++it)
        {
            auto tile_id = *it;
            auto coords = it.getIndices();

            auto src_rect = tilesets.at(tile_id.getTileset()).GetTileRect(map.getTileSets().at(tile_id.getTileset()), tile_id.getId());
            auto& texture = tilesets.at(tile_id.getTileset()).texture;

            SDL_FRect dst_rect {
                (float)(coords.x * map_tile_size.x),
                (float)(coords.y * map_tile_size.y),
                (float)(map_tile_size.x),
                (float)(map_tile_size.y),
            };

            renderer.RenderTextureRect(texture, camera.ToScreenRect(dst_rect), &src_rect);
        }
    }

    for (uint32_t j = 0; j < map.getMapGridSize().y; j++)
    {
        for (uint32_t i = 0; i < map.getMapGridSize().x; i++)
        {

            auto& unit = units.at(i + j * map.getMapGridSize().x);
            if (unit)
            {
                unit->RenderUnit(renderer, *this, { i, j }, camera);
            }
        }
    }
}

std::optional<glm::uvec2> Level::GetTileFromPos(const glm::ivec2& pos) const
{
    if (pos.x < 0.0f || pos.y < 0.0f)
        return std::nullopt;

    auto tile_size = map.getMapTileSize();
    uint32_t x = pos.x / tile_size.x;
    uint32_t y = pos.y / tile_size.y;

    auto map_size = map.getMapGridSize();

    if (x >= map_size.x || y >= map_size.y)
        return std::nullopt;

    return glm::uvec2 { x, y };
}

void Level::AddUnit(const glm::uvec2& coords, const Unit& unit)
{
    auto map_grid_size = map.getMapGridSize();
    units.at(coords.x + coords.y * map_grid_size.x) = std::make_unique<Unit>(unit);
}

Unit* Level::GetUnit(const glm::uvec2& coords) const
{
    auto map_grid_size = map.getMapGridSize();
    return units.at(coords.x + coords.y * map_grid_size.x).get();
}

void Level::RemoveUnit(const glm::uvec2& coords)
{
    auto map_grid_size = map.getMapGridSize();
    units.at(coords.x + coords.y * map_grid_size.x).reset();
}

void Level::MoveUnit(const glm::uvec2& from, const glm::uvec2& to)
{
    auto map_grid_size = map.getMapGridSize();

    if (auto& dst = units.at(to.x + to.y * map_grid_size.x); dst == nullptr)
    {
        dst = std::move(units.at(from.x + from.y * map_grid_size.x));
    }
}