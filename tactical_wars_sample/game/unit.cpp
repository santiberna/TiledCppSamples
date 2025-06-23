#include <game/text.hpp>
#include <game/unit.hpp>
#include <utility/colours.hpp>

UnitMapState SetupUnitMapState(const Level& level)
{
    UnitMapState unit_map {};
    unit_map.map_tile_size = { level.map.getMapTileSize().x, level.map.getMapTileSize().y };
    unit_map.units = tpp::Array2D<Unit>(level.map.getMapGridSize().x, level.map.getMapGridSize().y, Unit {});
    return unit_map;
}

TeamAssets LoadUnitTeamAssets(Renderer& renderer, const std::string& tsx_file)
{
    TeamAssets assets {};
    assets.tileset = tpp::TileSet::fromTSX(tsx_file).value();
    assets.draw_data = CreateTileSetDrawData(renderer, assets.tileset);

    if (auto* props = assets.tileset.getProperties())
    {
        assets.idle_anim_index = props->get<int>("IdleAnimation").value_or(0);
        assets.move_anim_index = props->get<int>("MoveAnimation").value_or(0);
        assets.exhausted_anim = props->get<int>("UsedAnimation").value_or(0);
    }

    return assets;
}

static const std::vector<UnitStats> UNIT_STATS = {
    { 3 }, // SOLDIER
};

const UnitStats& GetUnitStats(UnitType type)
{
    return UNIT_STATS.at(static_cast<uint8_t>(type));
}

static tpp::Array2D<float> InitAttackMatrix()
{
    tpp::Array2D<float> matrix { 1, 1 };
    matrix.at(static_cast<uint32_t>(UnitType::SOLDIER), static_cast<uint32_t>(UnitType::SOLDIER)) = 0.55f; // Soldier vs Soldier
    return matrix;
}

static const tpp::Array2D<float> UNIT_ATTACK_MATRIX = InitAttackMatrix();

void AttackUnit(Unit& attacker, Unit& defender)
{
    auto attacker_damage = UNIT_ATTACK_MATRIX.at(static_cast<uint32_t>(attacker.type), static_cast<uint32_t>(defender.type)) * attacker.health;
    defender.health -= (int8_t)attacker_damage;

    if (defender.health < 0)
    {
        defender = Unit {};
        return;
    }

    auto defender_damage = UNIT_ATTACK_MATRIX.at(static_cast<uint32_t>(defender.type), static_cast<uint32_t>(attacker.type)) * defender.health;
    attacker.health -= (int8_t)defender_damage;

    if (attacker.health < 0)
    {
        attacker = Unit {};
        return;
    }
}

void DrawUnit(Renderer& renderer, const FrameCamera& camera, const GameAssets& assets, const glm::vec2& map_position, const glm::vec2& tile_size, Unit unit, const glm::vec4& colour)
{
    auto& unit_assets = assets.team_assets.at(unit.team);
    auto& texture = unit_assets.draw_data.spritesheet_texture;

    glm::vec2 sprite_size = { unit_assets.tileset.getTileSize().x, unit_assets.tileset.getTileSize().y };
    SDL_FRect src {};

    if (unit.state == UnitState::IDLE)
    {
        src = GetTileRect(unit_assets.tileset, unit_assets.draw_data, unit_assets.idle_anim_index);
    }
    else if (unit.state == UnitState::MOVING)
    {
        src = GetTileRect(unit_assets.tileset, unit_assets.draw_data, unit_assets.move_anim_index);
    }
    else
    {
        src = GetTileRect(unit_assets.tileset, unit_assets.draw_data, unit_assets.exhausted_anim);
    }

    glm::vec2 offset = (sprite_size * unit_assets.scale - glm::vec2(tile_size)) * 0.5f;

    SDL_FRect dest {};
    dest.x = tile_size.x * map_position.x - offset.x;
    dest.y = tile_size.y * map_position.y - offset.y;
    dest.h = src.h * unit_assets.scale;
    dest.w = src.w * unit_assets.scale;

    renderer.RenderTextureRect(texture, camera.ToScreenRect(dest), &src, colour, !unit.facingRight);
}

void DrawMapUnits(Renderer& renderer, GameAssets& assets, const UnitMapState& unit_map, const FrameCamera& camera, DeltaMS delta)
{
    for (auto& [unit, sheet] : assets.team_assets)
    {
        UpdateAnimationData(sheet.tileset, sheet.draw_data, delta);
    }

    for (auto it = unit_map.units.begin(); it != unit_map.units.end(); ++it)
    {
        auto unit = *it;
        if (unit.health <= 0)
        {
            continue;
        }

        DrawUnit(renderer, camera, assets, { it.getIndices().x, it.getIndices().y }, unit_map.map_tile_size, unit, colour::WHITE);
    }

    for (auto it = unit_map.units.begin(); it != unit_map.units.end(); ++it)
    {
        auto unit = *it;

        if (unit.health <= 0)
        {
            continue;
        }

        // Health text
        auto health_display = (unit.health + 10 - 1) / 10;

        if (health_display == 10)
        {
            // Max health
            continue;
        }

        glm::vec2 map_position = { it.getIndices().x, it.getIndices().y };

        {
            auto text = unicode::FromUTF8(std::to_string(health_display));
            auto scale = camera.ToScreenRect(SDL_FRect { 0.0f, 0.0f, 1.0f, 1.0f }).w / assets.text_font->GetFontMetrics().resolution * 9.0f;

            glm::vec2 position = {
                unit_map.map_tile_size.x * map_position.x + unit_map.map_tile_size.x * 0.75f,
                unit_map.map_tile_size.y * map_position.y + unit_map.map_tile_size.y * 0.75f
            };

            DrawText(renderer, *assets.text_font, text, camera.ToScreenPoint(position), colour::WHITE, scale);
        }
    }
}
