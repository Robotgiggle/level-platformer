#pragma once
#include "../Scene.h"
#include "../WalkerEntity.h"

class Menu : public Scene {
public:
    // ����� CUSTOM ENTITIES ����� //
    Entity*& e_player = m_state.entities[0];

    // ����� METHODS ����� //
    Menu(int cap);

    void initialise() override;
    void process_input() override;
    void process_event(SDL_Event event) override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;

    Entity* get_player() const override { return e_player; }
};