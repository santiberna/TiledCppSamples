#include <engine/common.hpp>

#include <SDL3/SDL_main.h>
#include <engine/window.hpp>
#include <game/game_bindings.hpp>
#include <game/level.hpp>
#include <game/unit.hpp>
#include <resources/font.hpp>
#include <utility/colours.hpp>
#include <utility/log.hpp>

struct GameState
{
    Level current_level;
    UnitMapState unit_state {};
};

int main(int, char*[])
{
    SDL::Init();

    {
        auto window = std::make_unique<Window>("TankOn!", glm::uvec2(1600, 900));
        window->GetRenderer().SetVSync(true);
        auto& renderer = window->GetRenderer();

        GameInput input_data { window->GetInput() };

        PersistentCamera camera {};
        camera.resolution = window->GetSize();

        GameState game_state {};
        game_state.current_level = LoadLevel(renderer, "assets/maps/GoalMap.tmx");
        game_state.unit_state = SetupUnitMapState(game_state.current_level);

        GameAssets assets {};
        assets.team_assets[UnitTeam::RED] = LoadUnitTeamAssets(renderer, "assets/maps/Soldier.tsx");
        assets.team_assets[UnitTeam::BLUE] = LoadUnitTeamAssets(renderer, "assets/maps/Soldier2.tsx");

        FontLoadInfo font_info {};
        font_info.codepoint_ranges.emplace_back(unicode::ASCII_CODESET);
        font_info.codepoint_ranges.emplace_back(unicode::LATIN_SUPPLEMENT_CODESET);
        assets.text_font = Font::SharedFromFile(renderer, "assets/fonts/forward.ttf", font_info);

        {
            Unit red_unit {};
            red_unit.team = UnitTeam::RED;
            red_unit.health = 100;

            game_state.unit_state.units.at(5, 3) = red_unit;
            game_state.unit_state.units.at(5, 4) = red_unit;
            game_state.unit_state.units.at(4, 4) = red_unit;

            Unit blue_unit {};
            blue_unit.team = UnitTeam::BLUE;
            blue_unit.health = 100;
            blue_unit.facingRight = false;

            game_state.unit_state.units.at(9, 2) = blue_unit;
            game_state.unit_state.units.at(11, 1) = blue_unit;
            game_state.unit_state.units.at(10, 1) = blue_unit;
        }

        // Cursor cursor {};
        // cursor.PushState(std::make_unique<CursorDefaultState>());

        Timer timer {};

        while (input_data.running)
        {
            auto deltatime = timer.GetElapsed();
            input_data.mouse_state = InputState::NONE;
            timer.Reset();

            window->ProcessEvents();

            camera.zoom = input_data.camera_zoom;
            camera.translation += input_data.movement * deltatime.count();
            auto frame_camera = camera.MakeFrameCamera();

            // Units

            // red_team_assets->tile_data.UpdateAnimations(red_team_assets->tileset, deltatime);
            // blue_team_assets->tile_data.UpdateAnimations(blue_team_assets->tileset, deltatime);

            // level.RenderMap(renderer, frame_camera, deltatime);

            // // Selection rect

            // cursor.DoCursorLogic(
            //     level, renderer, frame_camera, deltatime,
            //     input_data.mouse_pos,
            //     input_data.mouse_state);

            renderer.ClearScreen(colour::BLACK);
            DrawLevel(renderer, game_state.current_level, frame_camera, deltatime);
            DrawMapUnits(renderer, assets, game_state.unit_state, frame_camera, deltatime);

            window->RenderPresent();
        }
    }

    // for (uint32_t j = 0; j < map.getMapGridSize().y; j++)
    // {
    //     for (uint32_t i = 0; i < map.getMapGridSize().x; i++)
    //     {

    //         auto& unit = units.at(i + j * map.getMapGridSize().x);
    //         if (unit)
    //         {
    //             unit->RenderUnit(renderer, *this, { i, j }, camera);
    //         }
    //     }
    // }

    SDL::Shutdown();
    return 0;
}