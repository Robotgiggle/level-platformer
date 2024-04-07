#pragma once
#include "../Scene.h"
#include "../WalkerEntity.h"

class Level3 : public Scene {
public:
    // ����� CUSTOM ENTITIES ����� //
    Entity*& e_player = m_state.entities[0];
    Entity*& e_background = m_state.entities[1];

    // ����� METHODS ����� //
    Level3(int cap);

    void initialise() override;
    void process_input() override;
    void process_event(SDL_Event event) override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;

    Entity* get_player() const override { return e_player; }
};