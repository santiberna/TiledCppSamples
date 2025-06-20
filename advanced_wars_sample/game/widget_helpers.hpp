#pragma once

#include <ui/core/menu.hpp>
#include <ui/widgets/button.hpp>
#include <ui/widgets/text_box.hpp>

namespace game_widgets
{

inline void DarkenOnHover(Button& button, DeltaMS)
{
    button.local_transform.colour = colour::LIGHT_GREY;
}

inline void DarkenOnHold(Button& button, DeltaMS)
{
    button.local_transform.colour = colour::GREY;
}

inline void DefaultColour(Button& button, DeltaMS)
{
    button.local_transform.colour = colour::WHITE;
}

inline std::unique_ptr<Button> MakeSimpleButton(glm::vec2 position, glm::vec2 size)
{
    auto button = std::make_unique<Button>();
    button->local_transform.position = position;
    button->local_transform.size = size;
    button->local_transform.colour = colour::WHITE;

    button->on_hover.connect(DarkenOnHover);
    button->on_hold.connect(DarkenOnHold);
    button->on_default.connect(DefaultColour);

    return button;
}

inline std::unique_ptr<TextBox> MakeSimpleTextBox(std::shared_ptr<Font> font, unicode::String text, float font_size)
{
    auto text_box = std::make_unique<TextBox>();
    text_box->font = font;
    text_box->text = text;
    text_box->local_transform.colour = colour::WHITE;
    text_box->font_size = font_size;

    return text_box;
}

}

// Menu MakeTestMenu(SDL_Renderer* renderer)
// {
//     FontLoadInfo info {};
//     info.codepoint_ranges.emplace_back(unicode::ASCII_CODESET);
//     info.codepoint_ranges.emplace_back(unicode::LATIN_SUPPLEMENT_CODESET);
//     info.resolutionY = 64.0f;

//     auto font = Font::SharedFromFile(renderer, "assets/Fonts/arial.ttf", info);

//     Menu menu {};
//     auto button = game_widgets::MakeSimpleButton({ 0.5f, 0.5f }, { 0.5f, 0.5f });
//     auto it = menu.AddRootNode(std::move(button));

//     auto text_box = game_widgets::MakeSimpleTextBox(font, unicode::FromUTF8("Hello World!"), 1.0f);
//     menu.AddChildNode(it, std::move(text_box));

//     return menu;
// };