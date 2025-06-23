#include <game/ui.hpp>

std::string GetTeamName(UnitTeam team)
{
    switch (team)
    {
    case UnitTeam::RED:
        return "Human";
    case UnitTeam::BLUE:
        return "Goblin";
    }
}

void NextRound(GameState& game_state, GameUI& game_ui)
{
    ++game_state.turn_index;

    size_t round_index = game_state.turn_index / game_state.teams.size();
    size_t team_index = game_state.turn_index % game_state.teams.size();

    auto team = game_state.teams.at(team_index);
    auto team_name = GetTeamName(team);

    std::string round_text = std::format("Round {}: {} Team", round_index + 1, team_name);
    game_ui.round_text->text = unicode::FromUTF8(round_text);

    for (auto& unit : game_state.unit_state.units)
    {
        if (unit.health > 0 && unit.state == UnitState::USED)
        {
            unit.state = UnitState::IDLE;
        }
    }
}