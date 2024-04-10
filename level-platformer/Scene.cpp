#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Utility.h"
#include "Scene.h"

Scene::Scene(int cap) : m_state(cap), m_entity_cap(cap) {}

Scene::~Scene() {
    for (int i = 0; i < m_entity_cap; i++) delete m_state.entities[i];
    delete[] m_state.entities;
    delete m_state.map;
    Mix_FreeChunk(m_state.jumpSfx);
    Mix_FreeMusic(m_state.bgm);
}

void Scene::initialise() {
    for (int i = 0; i < m_entity_cap; i++) {
        delete m_state.entities[i];
        m_state.entities[i] = nullptr;
    }
}

void Scene::update(float delta_time) {
    for (int i = 0; i < m_entity_cap; i++) {
        if (m_state.entities[i]) {
            m_state.entities[i]->update(delta_time, NULL, 0, m_state.map);
        }
    }
}

void Scene::render(ShaderProgram* program) {
    m_state.map->render(program);
    for (int i = m_unordered_render_start; i < m_entity_cap; i++) {
        if (m_state.entities[i]) {
            m_state.entities[i]->render(program);
        }
    }
}