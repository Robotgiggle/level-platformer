#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "../glm/mat4x4.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../ShaderProgram.h"
#include "../Utility.h"
#include "../entities/WalkerEntity.h"
#include "EndScreen.h"

// terrain map
const int END_WIDTH = 10,
          END_HEIGHT = 7;
const int END_DATA[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
};

// sprite filepaths
const char SPRITESHEET_FILEPATH[] = "assets/player.png",
           BACKGROUND_FILEPATH[] = "assets/background.png",
           WALKER_FILEPATH[] = "assets/walker.png",
           FONT_FILEPATH[] = "assets/pixel_font.png",
           MAP_TILES_FILEPATH[] = "assets/map_tiles.png";

// audio filepaths
const char MUSIC_FILEPATH[] = "assets/default_music.mp3",
           JUMP_FILEPATH[] = "assets/default_jump.wav";

// useful constants
const float ACC_OF_GRAVITY = -4.91f;

// constructor definition
EndScreen::EndScreen(int cap) : Scene(cap) {}

// other methods
void EndScreen::initialise() {
    // ————— PURGE ENTITIES ————— //
    Scene::initialise();

    // ————— NEXT SCENE ————— //
    m_state.nextSceneID = -1;

    // ————— FONT ————— //
    m_font_id = Utility::load_texture(FONT_FILEPATH);

    // ————— TERRAIN ————— //
    GLuint map_texture_id = Utility::load_texture(MAP_TILES_FILEPATH);
    m_state.map = new Map(END_WIDTH, END_HEIGHT, END_DATA, map_texture_id, 1.0f, 6, 4);

    // ————— BACKGROUND ————— //
    // create entity
    e_background = new Entity(this);

    // setup basic attributes
    e_background->set_position(glm::vec3(4.5f, 3.25f, 0.0f));
    e_background->set_sprite_scale(glm::vec3(10.0f, 7.5f, 0.0f));
    e_background->m_texture_id = Utility::load_texture(BACKGROUND_FILEPATH);

    e_background->update(0.0f, NULL, 0, m_state.map);

    // ————— PLAYER ————— //
    // create entity
    e_player = new Entity(this);

    // setup basic attributes
    e_player->set_motion_type(Entity::SIDE_ON);
    e_player->set_position(glm::vec3(0.5f, 1.0f, 0.0f));
    e_player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY, 0.0f));
    e_player->set_speed(1.8f);
    e_player->set_rot_speed(1.0f);
    e_player->set_scale(glm::vec3(0.715f, 0.88f, 0.0f));
    e_player->set_sprite_scale(glm::vec3(0.715f, 0.88f, 0.0f));
    e_player->m_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    e_player->m_jumping_power = 4.5f;

    // setup walking animation
    e_player->m_walking[Entity::LEFT] = new int[4] { 0, 2 };
    e_player->m_walking[Entity::RIGHT] = new int[4] { 1, 3 };
    e_player->m_animation_indices = e_player->m_walking[Entity::RIGHT];
    e_player->setup_anim(2, 2, 2, 6);

    // ————— WALKER ————— //
    // create entity
    e_walker1 = new WalkerEntity(this, 1);
    e_walker2 = new WalkerEntity(this, 0);

    // setup basic attributes
    e_walker1->set_position(glm::vec3(1.0f, 1.0f, 0.0f));
    e_walker2->set_position(glm::vec3(8.0f, 1.0f, 0.0f));
    for (Entity* walker : { e_walker1, e_walker2 }) {
        walker->set_motion_type(Entity::SIDE_ON);
        walker->set_movement(glm::vec3(0.0f));
        walker->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY, 0.0f));
        walker->set_speed(2.0f);
        walker->set_scale(glm::vec3(0.72f, 0.9f, 0.0f));
        walker->set_sprite_scale(glm::vec3(0.765f, 0.9f, 0.0f));
        walker->m_texture_id = Utility::load_texture(WALKER_FILEPATH);

        // setup walking animation
        walker->m_walking[Entity::LEFT] = new int[4] { 0, 2 };
        walker->m_walking[Entity::RIGHT] = new int[4] { 1, 3 };
        walker->m_animation_indices = walker->m_walking[Entity::RIGHT];
        walker->setup_anim(2, 2, 2, 6);
    }
    e_walker1->m_animation_indices = e_walker1->m_walking[Entity::RIGHT];
    e_walker2->m_animation_indices = e_walker2->m_walking[Entity::LEFT];

    // ————— AUDIO ————— //
    m_state.bgm = Mix_LoadMUS(MUSIC_FILEPATH);

    m_state.jumpSfx = Mix_LoadWAV(JUMP_FILEPATH);
    Mix_VolumeChunk(m_state.jumpSfx, MIX_MAX_VOLUME / 2);
}

void EndScreen::process_event(SDL_Event event) {
    // process event triggers
    switch (event.type) {
    case SDL_KEYDOWN:
        // process keydown triggers specifically
        switch (event.key.keysym.sym) {
        case SDLK_RETURN:
            m_globalInfo->gameIsRunning = false;
            break;
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

void EndScreen::process_input()
{
    // reset forced-movement if no player input
    e_player->set_movement(glm::vec3(0.0f));
    e_player->set_rotation(0.0f);

    // no movement if you're dead
    if (m_globalInfo->playerDead) return;

    // event triggers are *NOT* handled in this function, unlike before
    // see process_event() for event handling

    // process held keys
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_LEFT]) {
        e_player->move_left();
        e_player->m_animation_indices = e_player->m_walking[e_player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT]) {
        e_player->move_right();
        e_player->m_animation_indices = e_player->m_walking[e_player->RIGHT];
    }
    if (key_state[SDL_SCANCODE_Q]) {
        e_player->rotate_anticlockwise();
        e_player->m_animation_indices = e_player->m_walking[e_player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_E]) {
        e_player->rotate_clockwise();
        e_player->m_animation_indices = e_player->m_walking[e_player->RIGHT];
    }

    // normalize forced-movement vector so you don't go faster diagonally
    if (glm::length(e_player->get_movement()) > 1.0f) {
        e_player->set_movement(glm::normalize(e_player->get_movement()));
    }
}

//void EndScreen::update(float delta_time) {
//    Scene::update(delta_time);
//}

void EndScreen::render(ShaderProgram* program) {
    e_background->render(program);
    m_state.map->render(program);

    if (m_globalInfo->lives > 0) {
        e_player->render(program);
        Utility::draw_text(program, m_font_id, "You Win!", 1.0f, 0.0f, glm::vec3(0.75f, 5.2f, 0.0f));
    } else {
        e_walker1->render(program);
        e_walker2->render(program);
        Utility::draw_text(program, m_font_id, "Game Over", 1.0f, 0.0f, glm::vec3(0.5f, 5.2f, 0.0f));
    }
    Utility::draw_text(program, m_font_id, "Press ENTER to quit", 0.5f, 0.0f, glm::vec3(0.0f, 3.3f, 0.0f));
}