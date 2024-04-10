#pragma once
#include "../Scene.h"

class EndScreen : public Scene {
public:
    // ————— CUSTOM ENTITIES ————— //
    Entity*& e_background = m_state.entities[0];
    Entity*& e_player = m_state.entities[1];
    Entity*& e_walker1 = m_state.entities[2];
    Entity*& e_walker2 = m_state.entities[3];

    // ————— FONT ————— //
    GLuint m_font_id;

    // ————— METHODS ————— //
    EndScreen(int cap);

    void initialise() override;
    void process_input() override;
    void process_event(SDL_Event event) override;
    void render(ShaderProgram* program) override;

    Entity* get_player() const override { return e_player; }
};