#pragma once
#include <engine/input.hpp>

struct GameInput
{
    GameInput(InputEventSystem& input)
    {
        input.OnCloseRequested().connect([&]()
            { running = false; });

        input.OnKeyPress(SDLK_W).connect([&](bool pressed)
            { movement.y += pressed ? -1.0f : 1.0f; });
        input.OnKeyPress(SDLK_S).connect([&](bool pressed)
            { movement.y += pressed ? 1.0f : -1.0f; });
        input.OnKeyPress(SDLK_A).connect([&](bool pressed)
            { movement.x += pressed ? -1.0f : 1.0f; });
        input.OnKeyPress(SDLK_D).connect([&](bool pressed)
            { movement.x += pressed ? 1.0f : -1.0f; });

        input.OnMouseMove().connect([&](const glm::vec2& pos)
            { mouse_pos = pos; });
        input.OnButtonClick(SDL_BUTTON_LEFT).connect([&](bool pressed)
            { mouse_state = pressed ? InputState::PRESSED : InputState::RELEASED; });

        auto zoom_correct = [&](const glm::vec2& scroll)
        {
            camera_zoom *= std::powf(1.05f, scroll.y);
            camera_zoom = glm::clamp(camera_zoom, 0.5f, 5.0f);
        };

        input.OnMouseWheel().connect(zoom_correct);
    }

    bool running = true;
    InputState mouse_state {};
    glm::vec2 mouse_pos {};
    glm::vec2 movement {};
    float camera_zoom = 3.0f;
};