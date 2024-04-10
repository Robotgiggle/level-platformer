#pragma once
#include <SDL_mixer.h>
#include "Entity.h"
#include "Map.h"

struct GlobalInfo {
    bool gameIsRunning = true;
    bool changeScenes = false;
    bool gamePaused = false;
    bool playerDead = false;
    float deathTimer;
    int lives = 3;
    int coins = 0;
};

struct GameState {
    GameState(int cap) { entities = new Entity*[cap]{}; }

    Entity** entities;
    Map* map;

    Mix_Music* bgm;
    Mix_Chunk* jumpSfx;
    Mix_Chunk* coinSfx;
    Mix_Chunk* stompSfx;
    Mix_Chunk* deathSfx;

    int nextSceneID;
};

class Scene {
protected:
    // enforces abstractness without having any pure virtuals
    Scene(int cap);
public:
    // ————— ATTRIBUTES ————— //
    GameState m_state;
    GlobalInfo* m_global_info;
    const int m_entity_cap;
    int m_unordered_render_start = 0;
    float m_timer = 0.0f;

    // ————— VIRTUAL METHODS ————— //
    virtual void initialise();
    virtual void process_input() { return; }
    virtual void process_event(SDL_Event event) { return; }
    virtual void update(float delta_time);
    virtual void render(ShaderProgram* program);

    // ————— CONCRETE METHODS ————— //
    ~Scene();
    template <class EntityType, class... SpawnArgs>
    EntityType* spawn(Scene* scene, SpawnArgs... args) {
        for (int i = 0; i < m_entity_cap; i++) {
            if (m_state.entities[i]) continue;
            EntityType* newEntity = new EntityType(scene, args...);
            newEntity->set_array_index(i);
            m_state.entities[i] = newEntity;
            return newEntity;
        }
        std::cout << "Spawn failed, cap of " << m_entity_cap << " entities is full!" << std::endl;
        return nullptr;
    }

    // ————— GETTERS ————— //
    GameState const get_state() const { return m_state; }
    virtual Entity* get_player() const = 0;
};