#pragma once
#include <SDL_mixer.h>
#include "Entity.h"
#include "Map.h"

struct GlobalInfo {
    bool gameIsRunning;
    bool changeScenes;
    int lives;
};

struct GameState {
    GameState(int cap) { entities = new Entity*[cap]; }

    Entity** entities;
    Map* map;

    Mix_Music* bgm;
    Mix_Chunk* jumpSfx;

    int nextSceneID;
};

class Scene {
public:
    // ————— ATTRIBUTES ————— //
    GameState m_state;
    GlobalInfo* m_globalInfo;
    const int m_entityCap;

    // ————— VIRTUAL METHODS ————— //
    virtual void initialise() = 0;
    virtual void process_input() = 0;
    virtual void process_event(SDL_Event event) = 0;
    virtual void update(float delta_time) = 0;
    virtual void render(ShaderProgram* program) = 0;

    // ————— CONCRETE METHODS ————— //
    Scene(int cap);
    ~Scene();
    // for Some Fucking Reason template functions have to be entirely defined in the header file
    template <class EntityType, class... SpawnArgs>
    EntityType* spawn(Scene* scene, SpawnArgs... args) {
        for (int i = 0; i < m_entityCap; i++) {
            if (m_state.entities[i]) continue;
            EntityType* newEntity = new EntityType(scene, args...);
            newEntity->set_array_index(i);
            m_state.entities[i] = newEntity;
            return newEntity;
        }
        return nullptr;
    }

    // ————— GETTERS ————— //
    GameState const get_state() const { return m_state; }
    virtual Entity* get_player() const = 0;

    // ————— GETTERS ————— //
    void const set_globalInfo(GlobalInfo* new_globals) { m_globalInfo = new_globals; };
};