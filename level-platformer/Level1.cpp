#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Level1.h"
#include "Utility.h"

// terrain map
const int LV1_WIDTH = 10,
          LV1_HEIGHT = 8;
const int LV1_DATA[] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
};

// sprite filepaths
const char SPRITESHEET_FILEPATH[] = "assets/default_player.png",
           NPC_FILEPATH[] = "assets/default_npc.png",
           MAP_TILES_FILEPATH[] = "assets/default_platform.png";

// audio filepaths
const char MUSIC_FILEPATH[] = "assets/default_music.mp3";

// useful constants
const float ACC_OF_GRAVITY = -4.91f;

// constructor definition
Level1::Level1(int cap) : Scene(cap) {}

// other methods
void Level1::initialise() {
    // ————— TERRAIN ————— //
    GLuint map_texture_id = Utility::load_texture(MAP_TILES_FILEPATH);
    m_state.map = new Map(LV1_WIDTH, LV1_HEIGHT, LV1_DATA, map_texture_id, 1.0f, 2, 1);

    // ————— PLAYER ————— //
    // create entity
    e_player = new Entity(this);
    e_player->set_array_index(0);

    // setup basic attributes
    e_player->set_motion_type(Entity::SIDE_ON);
    e_player->set_position(glm::vec3(4.5f, 3.0f, 0.0f));
    e_player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY, 0.0f));
    e_player->set_speed(1.8f);
    e_player->set_rot_speed(1.0f);
    e_player->set_scale(glm::vec3(0.765f, 0.9f, 0.0f));
    e_player->set_sprite_scale(glm::vec3(0.765f, 0.9f, 0.0f));
    e_player->m_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    e_player->m_jumping_power = 4.5f;

    // setup walking animation
    e_player->m_walking[Entity::DOWN] = new int[4] { 0, 4, 8, 12 };
    e_player->m_walking[Entity::LEFT] = new int[4] { 1, 5, 9, 13 };
    e_player->m_walking[Entity::UP] = new int[4] { 2, 6, 10, 14 };
    e_player->m_walking[Entity::RIGHT] = new int[4] { 3, 7, 11, 15 };
    e_player->m_animation_indices = e_player->m_walking[Entity::RIGHT];
    e_player->setup_anim(4, 4, 4, 6);

    // ————— WALKER NPC ————— //
    // create entity
    e_walker = new WalkerEntity(this,0);
    e_walker->set_array_index(1);

    // setup basic attributes
    e_walker->set_motion_type(Entity::SIDE_ON);
    e_walker->set_position(glm::vec3(6.0f, 2.0f, 0.0f));
    e_walker->set_movement(glm::vec3(0.0f));
    e_walker->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY, 0.0f));
    e_walker->set_speed(1.0f);
    e_walker->set_rot_speed(1.0f);
    e_walker->set_scale(glm::vec3(0.765f, 0.9f, 0.0f));
    e_walker->set_sprite_scale(glm::vec3(0.765f, 0.9f, 0.0f));
    e_walker->m_texture_id = Utility::load_texture(NPC_FILEPATH);

    //// setup walking animation
    e_walker->m_walking[Entity::DOWN] = new int[4] { 0, 4, 8, 12 };
    e_walker->m_walking[Entity::LEFT] = new int[4] { 1, 5, 9, 13 };
    e_walker->m_walking[Entity::UP] = new int[4] { 2, 6, 10, 14 };
    e_walker->m_walking[Entity::RIGHT] = new int[4] { 3, 7, 11, 15 };
    e_walker->m_animation_indices = e_walker->m_walking[Entity::LEFT];
    e_walker->setup_anim(4, 4, 4);

    // ————— AUDIO ————— //
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_state.bgm = Mix_LoadMUS(MUSIC_FILEPATH);
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 3);

    m_state.jumpSfx = Mix_LoadWAV("assets/default_jump.wav");
    Mix_VolumeChunk(m_state.jumpSfx, MIX_MAX_VOLUME / 2);
}

void Level1::process_event(SDL_Event event) {
    // process event triggers
    switch (event.type) {
    case SDL_KEYDOWN:
        // process keydown triggers specifically
        switch (event.key.keysym.sym) {
        case SDLK_SPACE:
            if (e_player->m_collided_bottom) {
                e_player->m_is_jumping = true;
                Mix_PlayChannel(-1, m_state.jumpSfx, 0);
            }
            break;
        default:
            break;
        }
    }
}

void Level1::process_input()
{
    // reset forced-movement if no player input
    e_player->set_movement(glm::vec3(0.0f));
    e_player->set_rotation(0.0f);

    // event triggers are *NOT* handled in this function, unlike before
    // see process_event() for event handling

    // process held keys
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_LEFT]) {
        e_player->move_left();
        e_player->m_animation_indices = e_player->m_walking[e_player->LEFT];
    } else if (key_state[SDL_SCANCODE_RIGHT]) {
        e_player->move_right();
        e_player->m_animation_indices = e_player->m_walking[e_player->RIGHT];
    }
    if (key_state[SDL_SCANCODE_Q]) {
        e_player->rotate_anticlockwise();
        e_player->m_animation_indices = e_player->m_walking[e_player->LEFT];
    } else if (key_state[SDL_SCANCODE_E]) {
        e_player->rotate_clockwise();
        e_player->m_animation_indices = e_player->m_walking[e_player->RIGHT];
    }

    // normalize forced-movement vector so you don't go faster diagonally
    if (glm::length(e_player->get_movement()) > 1.0f) {
        e_player->set_movement(glm::normalize(e_player->get_movement()));
    }
}

void Level1::update(float delta_time) {
    e_player->update(delta_time, NULL, 0, m_state.map);
    e_walker->update(delta_time, NULL, 0, m_state.map);
}

void Level1::render(ShaderProgram* program) {
    m_state.map->render(program);
    e_player->render(program);
    e_walker->render(program);
}