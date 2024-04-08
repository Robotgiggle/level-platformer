#pragma once
#include "../Scene.h"

class Level1 : public Scene {
public:
    // ————— CUSTOM ENTITIES ————— //
    Entity*& e_background = m_state.entities[0];
    Entity*& e_player = m_state.entities[1];
    Entity*& e_walker = m_state.entities[2];
    Entity*& e_crawler1 = m_state.entities[3];
    Entity*& e_crawler2 = m_state.entities[4];
    Entity*& e_flyer1 = m_state.entities[5];
    Entity*& e_flyer2 = m_state.entities[6];
    Entity*& e_coin = m_state.entities[7];
    Entity*& e_healthbar = m_state.entities[8];
    Entity*& e_coinbar = m_state.entities[9];

    // ————— METHODS ————— //
    Level1(int cap);

    void initialise() override;
    void process_input() override;
    void process_event(SDL_Event event) override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;

    Entity* get_player() const override { return e_player; }
};