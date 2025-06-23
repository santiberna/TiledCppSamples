#include <engine/common.hpp>

#include <SDL3/SDL_main.h>
#include <engine/window.hpp>
#include <game/cursor.hpp>
#include <game/game_bindings.hpp>
#include <game/level.hpp>
#include <game/ui.hpp>
#include <game/unit.hpp>
#include <resources/font.hpp>
#include <utility/colours.hpp>
#include <utility/log.hpp>

int main(int, char*[])
{
    SDL::Init();

    {
        auto window = std::make_unique<Window>("Tactical Wars!", glm::uvec2(1600, 900));
        auto& renderer = window->GetRenderer();

        renderer.SetVSync(true);
        renderer.SetDebugRendering(false);

        GameInput input_data { window->GetInput() };

        GameState game_state {};
        game_state.current_level = LoadLevel(renderer, "assets/maps/FinalMap.tmx");
        game_state.unit_state = SetupUnitMapState(game_state.current_level);
        game_state.teams = { UnitTeam::RED, UnitTeam::BLUE };

        PersistentCamera camera {};
        camera.resolution = window->GetSize();

        {
            auto grid_size = game_state.current_level.map.getMapGridSize();
            auto tile_size = game_state.current_level.map.getMapTileSize();

            camera.translation = { grid_size.x * tile_size.x * 0.5f, grid_size.y * tile_size.y * 0.5f };
        }

        GameAssets assets {};
        assets.team_assets[UnitTeam::RED] = LoadUnitTeamAssets(renderer, "assets/maps/Spearman.tsx");
        assets.team_assets[UnitTeam::BLUE] = LoadUnitTeamAssets(renderer, "assets/maps/Goblin.tsx");

        FontLoadInfo font_info {};
        font_info.codepoint_ranges.emplace_back(unicode::ASCII_CODESET);
        font_info.codepoint_ranges.emplace_back(unicode::LATIN_SUPPLEMENT_CODESET);
        assets.text_font = Font::SharedFromFile(renderer, "assets/fonts/forward.ttf", font_info);

        assets.button_texture = Texture::SharedFromFile(renderer, "assets/images/button.png");
        assets.round_background = Texture::SharedFromFile(renderer, "assets/images/round_background.png");

        auto game_ui = SetupGameUI(renderer, assets);
        NextRound(game_state, *game_ui);

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

        Cursor cursor {};
        Timer timer {};

        while (input_data.running)
        {
            auto deltatime = timer.GetElapsed();
            input_data.mouse_state = InputState::NONE;
            timer.Reset();

            window->ProcessEvents();

            if (game_ui->next_round)
            {
                NextRound(game_state, *game_ui);
                game_ui->next_round = false;
            }

            camera.zoom = input_data.camera_zoom;
            camera.translation += input_data.movement * deltatime.count();
            auto frame_camera = camera.MakeFrameCamera();

            auto cursor_commands = UpdateCursorInput(
                cursor,
                game_state,
                frame_camera.ToWorld(input_data.mouse_pos),
                input_data.mouse_state == InputState::PRESSED,
                deltatime);

            renderer.ClearScreen(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));

            DrawLevel(renderer, game_state.current_level, frame_camera, deltatime);
            DrawMapUnits(renderer, assets, game_state.unit_state, frame_camera, deltatime);
            DrawCursorInput(renderer, assets, cursor_commands, frame_camera);

            UICursorInfo info {};
            info.cursor_position = input_data.mouse_pos;
            info.cursor_state = input_data.mouse_state;
            info.deltatime = deltatime;

            game_ui->menu.Draw(renderer, window->GetSize(), info);
            window->RenderPresent();
        }
    }

    SDL::Shutdown();
    return 0;
}