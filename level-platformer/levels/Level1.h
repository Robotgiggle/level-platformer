#pragma once
#include "../Scene.h"

class Level1 : public Scene {
public:
    // ————— CUSTOM ENTITIES ————— //
    Entity*& e_player = m_state.entities[0];
    Entity*& e_walker = m_state.entities[1];
    Entity*& e_crawler1 = m_state.entities[2];
    Entity*& e_crawler2 = m_state.entities[3];

    // ————— METHODS ————— //
    Level1(int cap);

    void initialise() override;
    void process_input() override;
    void process_event(SDL_Event event) override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;

    Entity* get_player() const override { return e_player; }
};