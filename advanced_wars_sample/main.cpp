#include <engine/common.hpp>

#include <SDL3/SDL_main.h>
#include <engine/window.hpp>
#include <utility/colours.hpp>
#include <utility/log.hpp>

#include <game/cursor.hpp>
#include <game/game_bindings.hpp>
#include <game/level.hpp>
#include <game/unit.hpp>
#include <ui/core/menu.hpp>
#include <ui/widgets/text_input.hpp>

int main(int, char*[])
{
    SDL::Init();

    {
        auto window = std::make_unique<Window>("TankOn!", glm::uvec2(1600, 900));
        window->GetRenderer().SetVSync(true);
        auto& renderer = window->GetRenderer();

        FontLoadInfo font_info {};
        font_info.codepoint_ranges.emplace_back(unicode::ASCII_CODESET);
        font_info.codepoint_ranges.emplace_back(unicode::LATIN_SUPPLEMENT_CODESET);
        auto font = Font::SharedFromFile(renderer, "assets/fonts/forward.ttf", font_info);

        GameInput input_data { window->GetInput() };

        PersistentCamera camera {};
        camera.resolution = window->GetSize();

        auto level = Level(renderer, "assets/maps/GoalMap.tmx");
        auto red_team_assets = std::make_shared<UnitSpriteSheet>(UnitSpriteSheet(renderer, "assets/maps/Soldier.tsx", font));
        auto blue_team_assets = std::make_shared<UnitSpriteSheet>(UnitSpriteSheet(renderer, "assets/maps/Soldier2.tsx", font));

        auto red_unit = Unit { red_team_assets, UnitTeam::RED };
        level.AddUnit({ 5, 3 }, red_unit);
        level.AddUnit({ 5, 4 }, red_unit);
        level.AddUnit({ 4, 4 }, red_unit);

        auto blue_unit = Unit { blue_team_assets, UnitTeam::BLUE };
        level.AddUnit({ 9, 2 }, blue_unit);
        level.AddUnit({ 11, 1 }, blue_unit);
        level.AddUnit({ 10, 1 }, blue_unit);

        Cursor cursor {};
        cursor.PushState(std::make_unique<CursorDefaultState>());

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

            red_team_assets->tile_data.UpdateAnimations(red_team_assets->tileset, deltatime);
            blue_team_assets->tile_data.UpdateAnimations(blue_team_assets->tileset, deltatime);

            renderer.ClearScreen(colour::BLACK);
            level.RenderMap(renderer, frame_camera, deltatime);

            // Selection rect

            cursor.DoCursorLogic(
                level, renderer, frame_camera, deltatime,
                input_data.mouse_pos,
                input_data.mouse_state);

            window->RenderPresent();
        }
    }

    SDL::Shutdown();
    return 0;
}