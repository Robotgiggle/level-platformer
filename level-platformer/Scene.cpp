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

Scene::Scene(int cap) : m_state(cap), m_entityCap(cap) {}

Scene::~Scene() {
    for (int i = 0; i < m_entityCap; i++) delete m_state.entities[i];
    delete[] m_state.entities;
    delete m_state.map;
    Mix_FreeChunk(m_state.jumpSfx);
    Mix_FreeMusic(m_state.bgm);
}