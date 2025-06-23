#pragma once

#include <ui/core/menu.hpp>
#include <ui/widgets/button.hpp>
#include <ui/widgets/sprite.hpp>
#include <ui/widgets/text_box.hpp>

#include <game/cursor.hpp>

namespace widgets
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

inline std::unique_ptr<UISprite> MakeSimpleUISprite(std::shared_ptr<Texture> texture)
{
    auto image = std::make_unique<UISprite>();
    image->sprite = texture;
    image->lock_aspect = false;

    return image;
}

}

struct GameUI
{
    Menu menu {};
    bool next_round = false;
    TextBox* round_text;
};

inline std::unique_ptr<GameUI> SetupGameUI(Renderer& renderer, const GameAssets& assets)
{
    std::unique_ptr<GameUI> ui = std::make_unique<GameUI>();
    Menu& menu = ui->menu;

    {
        auto panel = std::make_unique<UINode>();
        panel->local_transform.position = { 0.5, 0.0f };
        panel->local_transform.pivot = { 0.5f, 0.0f };
        panel->local_transform.size = { 0.6, 0.1 };

        auto panel_it = menu.AddRootNode(std::move(panel));
        auto text_box = widgets::MakeSimpleTextBox(assets.text_font, unicode::FromUTF8(""), 1.0f);
        ui->round_text = text_box.get();

        menu.AddChildNode(panel_it, std::move(text_box));
    }

    {
        auto button = widgets::MakeSimpleButton({ 1.2f, 0.9f }, { 0.35f, 0.15f });

        auto button_back = widgets::MakeSimpleUISprite(assets.button_texture);
        auto text_box = widgets::MakeSimpleTextBox(assets.text_font, unicode::FromUTF8("Next Round"), 1.0f);
        text_box->local_transform.colour = colour::BLACK;

        button->on_click.connect([ui = ui.get()](Button&)
            { ui->next_round = true; });

        auto it = menu.AddRootNode(std::move(button));
        menu.AddChildNode(it, std::move(button_back));
        menu.AddChildNode(it, std::move(text_box));
    }

    return ui;
};

void NextRound(GameState& game_state, GameUI& game_ui);